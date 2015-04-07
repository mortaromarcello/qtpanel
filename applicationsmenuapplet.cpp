#include "applicationsmenuapplet.h"

#include <QMenu>
#include <QStyle>
#include <QPixmap>
#include <QGraphicsScene>
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include "textgraphicsitem.h"
#include "panelwindow.h"
#include "desktopapplications.h"
#include "dpisupport.h"
#include "config.h"
#include "mydebug.h"
#include "ui_appletapplicationsmenusettings.h"
#include "panelapplication.h"

int ApplicationsMenuStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
	if(metric == QStyle::PM_SmallIconSize)
		return adjustHardcodedPixelSize(24);
	else
		return QCleanlooksStyle::pixelMetric(metric, option, widget);
}

SubMenu::SubMenu(QMenu* parent, const QString& title, const QString& category, const QString& icon)
{
	m_menu = new QMenu(parent); // Will be deleted automatically.
	//
	m_menu->setAttribute(Qt::WA_TranslucentBackground, true);
	//
	m_menu->setStyle(parent->style());
	m_menu->setFont(parent->font());
	m_menu->setTitle(title);
	m_menu->setIcon(QIcon::fromTheme(icon));
	m_menu->menuAction()->setIconVisibleInMenu(true);
	m_category = category;
}

static const char* menuStyleSheet =
"QMenu { border: %1px solid %2; border-top-left-radius: %3px; border-top-right-radius: %4px; border-bottom-left-radius: %5px; border-bottom-right-radius: %6px; background-color: %7; margin-right: 2px }\n"
"QMenu::item { height: %8px; background-color: transparent; color: %9; padding-left: %10px; padding-right: %11px; padding-top: %12px; padding-bottom: %13px; margin-right: 2px }\n"
"QMenu::item::selected { background-color: %14; border: %1px solid white; padding-left: %10px; padding-right: %11px; padding-top: %12px; padding-bottom: %13px; border-top-left-radius: %3px; border-top-right-radius: %4px; border-bottom-left-radius: %5px; border-bottom-right-radius: %6px; color: %15 }\n"
"QMenu::icon { left: %16px; }\n";

ApplicationsMenuApplet::ApplicationsMenuApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_menuOpened(false),
	m_opacity(0.8),
	m_backgroundMenuColor("gray"),
	m_textMenuColor("white"),
	m_selectedMenuColor("darkGray"),
	m_backgroundSelectedMenuColor("white"),
	m_menuBorderColor("black"),
	m_menuBorderWidth(1),
	m_menuBorderRadiusTopLeft(0),
	m_menuBorderRadiusTopRight(0),
	m_menuBorderRadiusBottomLeft(0),
	m_menuBorderRadiusBottomRight(0)
{
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	m_menu = new QMenu();
	//
	m_menu->setAttribute(Qt::WA_TranslucentBackground, true);
	//
	m_menu->setStyle(&m_style);
	m_menu->setFont(m_panelWindow->font());
	m_menu->setStyleSheet(QString(menuStyleSheet)
		.arg(adjustHardcodedPixelSize(m_menuBorderWidth))				// 1
		.arg(m_menuBorderColor.name())									// 2
		.arg(adjustHardcodedPixelSize(m_menuBorderRadiusTopLeft))		// 3
		.arg(adjustHardcodedPixelSize(m_menuBorderRadiusTopRight))		// 4
		.arg(adjustHardcodedPixelSize(m_menuBorderRadiusBottomLeft))	// 5
		.arg(adjustHardcodedPixelSize(m_menuBorderRadiusBottomRight))	// 6
		.arg(m_backgroundMenuColor.name())								// 7
		.arg(adjustHardcodedPixelSize(24))								// 8
		.arg(m_textMenuColor.name())									// 9
		.arg(adjustHardcodedPixelSize(32))								// 10
		.arg(adjustHardcodedPixelSize(16))								// 11
		.arg(adjustHardcodedPixelSize(2))								// 12
		.arg(adjustHardcodedPixelSize(2))								// 13
		.arg(m_selectedMenuColor.name())								// 14
		.arg(m_backgroundSelectedMenuColor.name())						// 15
		.arg(adjustHardcodedPixelSize(2))								// 16
	);
	m_subMenus.append(SubMenu(m_menu, tr("Accessories"), "Utility", "applications-accessories"));
	m_subMenus.append(SubMenu(m_menu, tr("Development"), "Development", "applications-development"));
	m_subMenus.append(SubMenu(m_menu, tr("Education"), "Education", "applications-science"));
	m_subMenus.append(SubMenu(m_menu, tr("Office"), "Office", "applications-office"));
	m_subMenus.append(SubMenu(m_menu, tr("Graphics"), "Graphics", "applications-graphics"));
	m_subMenus.append(SubMenu(m_menu, tr("Multimedia"), "AudioVideo", "applications-multimedia"));
	m_subMenus.append(SubMenu(m_menu, tr("Games"), "Game", "applications-games"));
	m_subMenus.append(SubMenu(m_menu, tr("Network"), "Network", "applications-internet"));
	m_subMenus.append(SubMenu(m_menu, tr("System"), "System", "preferences-system"));
	m_subMenus.append(SubMenu(m_menu, tr("Settings"), "Settings", "preferences-desktop"));
	m_subMenus.append(SubMenu(m_menu, tr("Other"), "Other", "applications-other"));
//
	setOpacity(m_opacity);
	m_pixmapItem = new QGraphicsPixmapItem(this);
	if (m_icon.name().isEmpty())
		m_icon = QIcon::fromTheme("start-here-symbolic");
	m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	m_pixmapItem->setOffset(4, 4);
//
	m_textItem = new TextGraphicsItem(this);
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_panelWindow->font());
	m_textItem->setText(tr("Applications"));
	m_settingsUi = new Ui::AppletApplicationsMenuSettings;
}

ApplicationsMenuApplet::~ApplicationsMenuApplet()
{
	foreach(QAction* action, m_actions)
	{
		delete action;
	}

	delete m_textItem;
	//
	delete m_pixmapItem;
	delete m_settingsUi;
	//
	delete m_menu;
}

bool ApplicationsMenuApplet::init()
{
	setInteractive(true);

	connect(DesktopApplications::instance(), SIGNAL(applicationUpdated(DesktopApplication)), this, SLOT(applicationUpdated(DesktopApplication)));
	connect(DesktopApplications::instance(), SIGNAL(applicationRemoved(QString)), this, SLOT(applicationRemoved(QString)));

	QList<DesktopApplication> apps = DesktopApplications::instance()->applications();
	foreach(const DesktopApplication& app, apps)
	applicationUpdated(app);

	return true;
}

QSize ApplicationsMenuApplet::desiredSize()
{
	return QSize(m_textItem->boundingRect().size().width() + 8 + m_pixmapItem->boundingRect().size().width(), m_textItem->boundingRect().size().height());
}

void ApplicationsMenuApplet::clicked()
{
	m_menuOpened = true;
	animateHighlight();

	m_menu->move(localToScreen(QPoint(0, m_size.height())));
	m_menu->exec();

	m_menuOpened = false;
	animateHighlight();
}

void ApplicationsMenuApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("menu-icon", m_icon.name());
	writer->writeTextElement("opacity", QString("%1").arg(m_opacity));
	writer->writeTextElement("background-menu-color", m_backgroundMenuColor.name());
	writer->writeTextElement("text-menu-color", m_textMenuColor.name());
	writer->writeTextElement("selected-menu-color", m_selectedMenuColor.name());
	writer->writeTextElement("background-selected-menu-color", m_backgroundSelectedMenuColor.name());
	writer->writeTextElement("menu-border-radius-top-left", QString("%1").arg(m_menuBorderRadiusTopLeft));
	writer->writeTextElement("menu-border-radius-top-right", QString("%1").arg(m_menuBorderRadiusTopRight));
	writer->writeTextElement("menu-border-radius-bottom-left", QString("%1").arg(m_menuBorderRadiusBottomLeft));
	writer->writeTextElement("menu-border-radius-bottom-right", QString("%1").arg(m_menuBorderRadiusBottomRight));
	writer->writeTextElement("menu-border-width", QString("%1").arg(m_menuBorderWidth));
	writer->writeTextElement("menu-border-color", m_menuBorderColor.name());
	writer->writeEndElement();
}

void ApplicationsMenuApplet::showContextMenu(const QPoint& point)
{

	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("application-exit"), tr("Quit panel"), QApplication::instance(), SLOT(quit()));
	menu.exec(point);
}

void ApplicationsMenuApplet::layoutChanged()
{
	m_textItem->setPos(8 + m_pixmapItem->boundingRect().size().width(), m_panelWindow->textBaseLine());
}

bool ApplicationsMenuApplet::isHighlighted()
{
	return m_menuOpened || Applet::isHighlighted();
}

bool ApplicationsMenuApplet::xmlRead()
{
	if (!m_xmlConfigReader.xmlOpen()) {
#ifdef __DEBUG__
		MyDBG << "Error opening file.";
#else
		qDebug("Error opening file.");
#endif
		return false;
	}
	while (!m_xmlConfigReader.atEnd()) {
		if (m_xmlConfigReader.hasError()) {
			m_xmlConfigReader.xmlErrorString();
			m_xmlConfigReader.xmlClose();
			return false;
		}
		while (m_xmlConfigReader.readNextStartElement())
			if (m_xmlConfigReader.name() == "applet")
			while (m_xmlConfigReader.readNextStartElement())
				if (m_xmlConfigReader.name() == "type")
					if (m_xmlConfigReader.readElementText() == getNameApplet())
						while (m_xmlConfigReader.readNextStartElement())
							if (m_xmlConfigReader.name() == "config")
								while (m_xmlConfigReader.readNextStartElement())
									if (m_xmlConfigReader.name() == "menu-icon")
										xmlReadMenuIcon();
									else if (m_xmlConfigReader.name() == "opacity")
										xmlReadOpacity();
									else if (m_xmlConfigReader.name() == "background-menu-color")
										xmlReadBackgroundMenuColor();
									else if (m_xmlConfigReader.name() =="text-menu-color")
										xmlReadTextMenuColor();
									else if (m_xmlConfigReader.name() =="selected-menu-color")
										xmlReadSelectedMenuColor();
									else if (m_xmlConfigReader.name() =="background-selected-menu-color")
										xmlReadBackgroundSelectedMenuColor();
									else if (m_xmlConfigReader.name() == "menu-border-radius-top-left")
										xmlReadMenuBorderRadiusTopLeft();
									else if (m_xmlConfigReader.name() == "menu-border-radius-top-right")
										xmlReadMenuBorderRadiusTopRight();
									else if (m_xmlConfigReader.name() == "menu-border-radius-bottom-left")
										xmlReadMenuBorderRadiusBottomLeft();
									else if (m_xmlConfigReader.name() == "menu-border-radius-bottom-right")
										xmlReadMenuBorderRadiusBottomRight();
									else if (m_xmlConfigReader.name() == "menu-border-width")
										xmlReadMenuBorderWidth();
									else if (m_xmlConfigReader.name() == "menu-border-color")
										xmlReadMenuBorderColor();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void ApplicationsMenuApplet::xmlReadMenuIcon()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "menu-icon");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_icon = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon::fromTheme("none"));
#ifdef __DEBUG__
	MyDBG << m_icon.name();
#endif
}

void ApplicationsMenuApplet::xmlReadOpacity()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "opacity");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_opacity = m_xmlConfigReader.readElementText().toDouble();
#ifdef __DEBUG__
	MyDBG << m_opacity;
#endif
}

void ApplicationsMenuApplet::xmlReadBackgroundMenuColor()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "background-menu-color");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_backgroundMenuColor = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_backgroundMenuColor;
#endif
}

void ApplicationsMenuApplet::xmlReadTextMenuColor()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "text-menu-color");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_textMenuColor = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_textMenuColor;
#endif
}

void ApplicationsMenuApplet::xmlReadSelectedMenuColor()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "selected-menu-color");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_selectedMenuColor = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_selectedMenuColor;
#endif
}

void ApplicationsMenuApplet::xmlReadBackgroundSelectedMenuColor()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "background-selected-menu-color");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_backgroundSelectedMenuColor = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_backgroundSelectedMenuColor;
#endif
}

void ApplicationsMenuApplet::xmlReadMenuBorderRadiusTopLeft()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "menu-border-radius-top-left");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_menuBorderRadiusTopLeft = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_menuBorderRadiusTopLeft;
#endif
}

void ApplicationsMenuApplet::xmlReadMenuBorderRadiusTopRight()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "menu-border-radius-top-right");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_menuBorderRadiusTopRight = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_menuBorderRadiusTopRight;
#endif
}

void ApplicationsMenuApplet::xmlReadMenuBorderRadiusBottomLeft()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "menu-border-radius-bottom-left");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_menuBorderRadiusBottomLeft = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_menuBorderRadiusBottomLeft;
#endif
}

void ApplicationsMenuApplet::xmlReadMenuBorderRadiusBottomRight()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "menu-border-radius-bottom-right");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_menuBorderRadiusBottomRight = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_menuBorderRadiusBottomRight;
#endif
}

void ApplicationsMenuApplet::xmlReadMenuBorderWidth()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "menu-border-width");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_menuBorderWidth = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_menuBorderWidth;
#endif
}

void ApplicationsMenuApplet::xmlReadMenuBorderColor()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "menu-border-color");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_menuBorderColor = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_menuBorderColor;
#endif
}

void ApplicationsMenuApplet::actionTriggered()
{
	DesktopApplications::instance()->launch(static_cast<QAction*>(sender())->data().toString());
}

void ApplicationsMenuApplet::applicationUpdated(const DesktopApplication& app)
{
	applicationRemoved(app.path());

	if(app.isNoDisplay())
		return;

	QAction* action = new QAction(m_menu);
	action->setIconVisibleInMenu(true);
	action->setData(app.path());
	action->setText(app.name());
	action->setIcon(QIcon(QPixmap::fromImage(app.iconImage())));

	connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));

	// Add to relevant menu.
	int subMenuIndex = m_subMenus.size() - 1; // By default put it in "Other".
	for(int i = 0; i < m_subMenus.size() - 1; i++) // Without "Other".
	{
		if(app.categories().contains(m_subMenus[i].category()))
		{
			subMenuIndex = i;
			break;
		}
	}

	QMenu* menu = m_subMenus[subMenuIndex].menu();
	QList<QAction*> actions = menu->actions();
	QAction* before = NULL;
	for(int i = 0; i < actions.size(); i++)
	{
		if(actions[i]->text().compare(action->text(), Qt::CaseInsensitive) > 0)
		{
			before = actions[i];
			break;
		}
	}

	if(menu->actions().isEmpty())
	{
		QList<QAction*> actions = m_menu->actions();
		QAction* before = NULL;
		for(int i = 0; i < actions.size(); i++)
		{
			if(actions[i]->text().compare(menu->title(), Qt::CaseInsensitive) > 0)
			{
				before = actions[i];
				break;
			}
		}

		m_menu->insertMenu(before, menu);
	}

	menu->insertAction(before, action);


	m_actions[app.path()] = action;
}

void ApplicationsMenuApplet::applicationRemoved(const QString& path)
{
	if(m_actions.contains(path))
	{
		delete m_actions[path];
		m_actions.remove(path);
	}

	for(int i = 0; i < m_subMenus.size(); i++)
	{
		if(m_subMenus[i].menu()->actions().isEmpty())
			m_menu->removeAction(m_subMenus[i].menu()->menuAction());
	}
}

void ApplicationsMenuApplet::buttonIconClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QString name;
		name = QFileInfo(namefile).baseName();
		QIcon icon = QIcon::fromTheme(name);
		m_settingsUi->menu_icon->setIcon(icon);
		m_settingsUi->menu_icon->setText(icon.name());
	}
}

void ApplicationsMenuApplet::showConfigurationDialog()
{
	QDialog dialog;
	QStringList colors = QColor::colorNames();
	QList <QColor> colorList;
	for (int i = 0; i < colors.size(); i++)
		colorList << QColor(colors.at(i));
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->menu_icon->setIcon(m_icon);
	m_settingsUi->menu_icon->setText(m_icon.name());
	QObject::connect(m_settingsUi->menu_icon, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
	m_settingsUi->opacity->setRange(0.0, 1.0);
	m_settingsUi->opacity->setSingleStep(0.1);
	m_settingsUi->opacity->setValue(m_opacity);
	m_settingsUi->background_menu_color->addItems(colors);
	m_settingsUi->background_menu_color->setCurrentIndex(colorList.indexOf(m_backgroundMenuColor));
	m_settingsUi->text_menu_color->addItems(colors);
	m_settingsUi->text_menu_color->setCurrentIndex(colorList.indexOf(m_textMenuColor));
	m_settingsUi->selected_menu_color->addItems(colors);
	m_settingsUi->selected_menu_color->setCurrentIndex(colorList.indexOf(m_selectedMenuColor));
	m_settingsUi->background_selected_menu_color->addItems(colors);
	m_settingsUi->background_selected_menu_color->setCurrentIndex(colorList.indexOf(m_backgroundSelectedMenuColor));
	m_settingsUi->menu_border_radius_top_left->setValue(m_menuBorderRadiusTopLeft);
	m_settingsUi->menu_border_radius_top_right->setValue(m_menuBorderRadiusTopRight);
	m_settingsUi->menu_border_radius_bottom_left->setValue(m_menuBorderRadiusBottomLeft);
	m_settingsUi->menu_border_radius_bottom_right->setValue(m_menuBorderRadiusBottomRight);
	m_settingsUi->menu_border_width->setValue(m_menuBorderWidth);
	m_settingsUi->menu_border_color->addItems(colors);
	m_settingsUi->menu_border_color->setCurrentIndex(colorList.indexOf(m_menuBorderColor));
	if(dialog.exec() == QDialog::Accepted) {
		m_icon = m_settingsUi->menu_icon->icon();
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
		update();
		m_opacity = m_settingsUi->opacity->value();
		setOpacity(m_opacity);
		m_backgroundMenuColor = QColor(m_settingsUi->background_menu_color->currentText());
		m_textMenuColor = QColor(m_settingsUi->text_menu_color->currentText());
		m_selectedMenuColor = QColor(m_settingsUi->selected_menu_color->currentText());
		m_backgroundSelectedMenuColor = QColor(m_settingsUi->background_selected_menu_color->currentText());
		m_menuBorderRadiusTopLeft = m_settingsUi->menu_border_radius_top_left->value();
		m_menuBorderRadiusTopRight = m_settingsUi->menu_border_radius_top_right->value();
		m_menuBorderRadiusBottomLeft = m_settingsUi->menu_border_radius_bottom_left->value();
		m_menuBorderRadiusBottomRight = m_settingsUi->menu_border_radius_bottom_right->value();
		m_menuBorderWidth = m_settingsUi->menu_border_width->value();
		m_menuBorderColor = QColor(m_settingsUi->menu_border_color->currentText());
		m_menu->setStyleSheet(QString(menuStyleSheet)
			.arg(adjustHardcodedPixelSize(m_menuBorderWidth))				// 1
			.arg(m_menuBorderColor.name())									// 2
			.arg(adjustHardcodedPixelSize(m_menuBorderRadiusTopLeft))		// 3
			.arg(adjustHardcodedPixelSize(m_menuBorderRadiusTopRight))		// 4
			.arg(adjustHardcodedPixelSize(m_menuBorderRadiusBottomLeft))	// 5
			.arg(adjustHardcodedPixelSize(m_menuBorderRadiusBottomRight))	// 6
			.arg(m_backgroundMenuColor.name())								// 7
			.arg(adjustHardcodedPixelSize(24))								// 8
			.arg(m_textMenuColor.name())									// 9
			.arg(adjustHardcodedPixelSize(32))								// 10
			.arg(adjustHardcodedPixelSize(16))								// 11
			.arg(adjustHardcodedPixelSize(2))								// 12
			.arg(adjustHardcodedPixelSize(2))								// 13
			.arg(m_selectedMenuColor.name())								// 14
			.arg(m_backgroundSelectedMenuColor.name())						// 15
			.arg(adjustHardcodedPixelSize(2))								// 16
		);
		m_configChanged = true;
	}
}

void ApplicationsMenuApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
#ifdef __DEBUG__
	MyDBG << "MousePressEvent";
#endif
	if(isUnderMouse()) {
		if(event->button() == Qt::LeftButton)
		{
			// FIXME: Workaround.
			// For some weird reason, if clicked() function is called directly, and menu is opened,
			// this item will receive hover enter event on menu close. But it shouldn't (mouse is outside).
			// Probably somehow related to taking a mouse grab when one is already active.
			QTimer::singleShot(1, this, SLOT(clicked()));
		}
		if (event->buttons() == Qt::RightButton)
		showContextMenu(localToScreen(QPoint(0, 0)));
	}
}

void ApplicationsMenuApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
}

void ApplicationsMenuApplet::setOpacity(qreal opacity)
{
	m_menu->setWindowOpacity(m_opacity);
	for (int i = 0; i < m_subMenus.size(); i++) {
		SubMenu menu = m_subMenus.at(i);
		menu.menu()->setWindowOpacity(m_opacity);
	}
}
