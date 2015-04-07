#include "config.h"
#include "sessionapplet.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QStyleFactory>
#include <QTimer>
#include <QProcess>
#include <QFileDialog>
#include "dpisupport.h"
#include "panelwindow.h"
#include "ui_appletsessionsettings.h"
#include "panelapplication.h"

static const char* menuStyleSheet =
"QMenu { border: %1px solid %2; border-top-left-radius: %3px; border-top-right-radius: %4px; border-bottom-left-radius: %5px; border-bottom-right-radius: %6px; background-color: gray; }\n"
"QMenu::item { height: 24px; background-color: transparent; color: %7; padding-left: 32px; padding-right: 16px; padding-top: 2px; padding-bottom: 2px;}\n"
"QMenu::item::selected { background-color: %8; border: %1px solid white; padding-left: 32px; padding-right: 16px; padding-top: 2px; padding-bottom: 2px; border-top-left-radius: %3px; border-top-right-radius: %4px; border-bottom-left-radius: %5px; border-bottom-right-radius: %6px; color: %9 }\n"
"QMenu::icon { left: 2px; }\n";

SessionApplet::SessionApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_menuOpened(false),
	m_colorText(QColor("white")),
	m_colorBackground(QColor("gray")),
	m_colorSelectedText(QColor("orangered")),
	m_colorSelectedBackground(QColor("lightgray")),
	m_colorBorder(QColor("greenyellow")),
	m_menu(new QMenu()),
	m_pixmapItem(new QGraphicsPixmapItem(this)),
	m_settingsUi(new Ui::AppletSessionSettings()),
	m_icon(QString(qtpanel_IMAGES_TARGET) + "/user.svg"),
	m_commandLogout("openbox --exit"),
	m_commandHalt("gksudo \"shutdown -h -P now\""),
	m_commandReboot("gksudo reboot"),
	m_radiusBorderTopLeft(0),
	m_radiusBorderTopRight(0),
	m_radiusBorderBottomLeft(0),
	m_radiusBorderBottomRight(0),
	m_heightBorder(1)
{
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	m_iconLogout 	= QIcon(QString(qtpanel_IMAGES_TARGET) + "/system-logout.svg");
	m_iconHalt 		= QIcon(QString(qtpanel_IMAGES_TARGET) + "/system-halt.svg");
	m_iconReboot 	= QIcon(QString(qtpanel_IMAGES_TARGET) + "/system-reboot.svg");
	m_iconHibernate = QIcon(QString(qtpanel_IMAGES_TARGET) + "/system-hibernate.svg");
	m_iconSuspend 	= QIcon(QString(qtpanel_IMAGES_TARGET) + "/system-suspend.svg");
	m_menu->setAttribute(Qt::WA_TranslucentBackground, true);
	m_menu->setWindowOpacity(0.80);
	m_menu->setStyle(QStyleFactory::create("cleanlooks"));
	m_menu->setFont(m_panelWindow->font());
	m_menu->setStyleSheet(QString(menuStyleSheet)
		.arg(m_heightBorder)					// 1
		.arg(m_colorBorder.name())				// 2
		.arg(m_radiusBorderTopLeft)				// 3
		.arg(m_radiusBorderTopRight)			// 4
		.arg(m_radiusBorderBottomLeft)			// 5
		.arg(m_radiusBorderBottomRight)			// 6
		.arg(m_colorText.name())				// 7
		.arg(m_colorSelectedBackground.name())	// 8
		.arg(m_colorSelectedText.name())		// 9
	);
	createActions();
	createMenu();
	m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	m_pixmapItem->setOffset(4, 4);
}

SessionApplet::~SessionApplet()
{
	delete m_pixmapItem;
	delete m_settingsUi;
	delete m_menu;
}

bool SessionApplet::init()
{
	setInteractive(true);
	return true;
}

void SessionApplet::clicked()
{
	m_menuOpened = true;
	animateHighlight();
	m_menu->move(localToScreen(QPoint(0, m_size.height())));
	m_menu->exec();
	m_menuOpened = false;
	animateHighlight();
}

void SessionApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("icon", m_icon.name());
	writer->writeTextElement("icon-logout", m_iconLogout.name());
	writer->writeTextElement("icon-halt", m_iconHalt.name());
	writer->writeTextElement("icon-reboot", m_iconReboot.name());
	writer->writeTextElement("icon-hibernate", m_iconHibernate.name());
	writer->writeTextElement("icon-suspend", m_iconSuspend.name());
	writer->writeTextElement("command-logout", m_commandLogout);
	writer->writeTextElement("command-halt", m_commandHalt);
	writer->writeTextElement("command-reboot", m_commandReboot);
	writer->writeTextElement("command-hibernate", m_commandHibernate);
	writer->writeTextElement("command-suspend", m_commandSuspend);
	writer->writeTextElement("color-text", m_colorText.name());
	writer->writeTextElement("color-background", m_colorBackground.name());
	writer->writeTextElement("color-selected-text", m_colorSelectedText.name());
	writer->writeTextElement("color-selected-background", m_colorSelectedBackground.name());
	writer->writeTextElement("color-border", m_colorBorder.name());
	writer->writeTextElement("radius-border-top-left", QString("%1").arg(m_radiusBorderTopLeft));
	writer->writeTextElement("radius-border-top-right", QString("%1").arg(m_radiusBorderTopRight));
	writer->writeTextElement("radius-border-bottom-left", QString("%1").arg(m_radiusBorderBottomLeft));
	writer->writeTextElement("radius-border-bottom-right", QString("%1").arg(m_radiusBorderBottomRight));
	writer->writeTextElement("height-border", QString("%1").arg(m_heightBorder));
	writer->writeEndElement();
}

void SessionApplet::showConfigurationDialog()
{
	QDialog dialog;
	QStringList colors = QColor::colorNames();
	QList <QColor> colorList;
	for (int i = 0; i < colors.size(); i++)
		colorList << QColor(colors.at(i));
	m_settingsUi->setupUi(&dialog);
	//
	m_settingsUi->icon_session->setIcon(m_icon);
	m_settingsUi->icon_session->setText(m_icon.name());
	QObject::connect(m_settingsUi->icon_session, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
	//
	m_settingsUi->icon_logout->setIcon(m_iconLogout);
	m_settingsUi->icon_logout->setText(m_iconLogout.name());
	QObject::connect(m_settingsUi->icon_logout, SIGNAL(clicked()), this, SLOT(buttonIconLogoutClicked()));
	m_settingsUi->command_logout->setText(m_commandLogout);
	//
	m_settingsUi->icon_halt->setIcon(m_iconHalt);
	m_settingsUi->icon_halt->setText(m_iconHalt.name());
	QObject::connect(m_settingsUi->icon_halt, SIGNAL(clicked()), this, SLOT(buttonIconHaltClicked()));
	m_settingsUi->command_halt->setText(m_commandHalt);
	//
	m_settingsUi->icon_reboot->setIcon(m_iconReboot);
	m_settingsUi->icon_reboot->setText(m_iconReboot.name());
	QObject::connect(m_settingsUi->icon_reboot, SIGNAL(clicked()), this, SLOT(buttonIconRebootClicked()));
	m_settingsUi->command_reboot->setText(m_commandReboot);
	//
	m_settingsUi->icon_hibernate->setIcon(m_iconHibernate);
	m_settingsUi->icon_hibernate->setText(m_iconHibernate.name());
	QObject::connect(m_settingsUi->icon_hibernate, SIGNAL(clicked()), this, SLOT(buttonIconHibernateClicked()));
	m_settingsUi->command_hibernate->setText(m_commandHibernate);
	//
	m_settingsUi->icon_suspend->setIcon(m_iconSuspend);
	m_settingsUi->icon_suspend->setText(m_iconSuspend.name());
	QObject::connect(m_settingsUi->icon_suspend, SIGNAL(clicked()), this, SLOT(buttonIconSuspendClicked()));
	m_settingsUi->command_suspend->setText(m_commandSuspend);
	////
	m_settingsUi->radius_border_top_left->setValue(m_radiusBorderTopLeft);
	m_settingsUi->radius_border_top_right->setValue(m_radiusBorderTopRight);
	m_settingsUi->radius_border_bottom_left->setValue(m_radiusBorderBottomLeft);
	m_settingsUi->radius_border_bottom_right->setValue(m_radiusBorderBottomRight);
	m_settingsUi->height_border->setValue(m_heightBorder);
	//
	m_settingsUi->color_text->addItems(colors);
	m_settingsUi->color_text->setCurrentIndex(colorList.indexOf(m_colorText));
	//
	m_settingsUi->color_background->addItems(colors);
	m_settingsUi->color_background->setCurrentIndex(colorList.indexOf(m_colorBackground));
	//
	m_settingsUi->color_selected_text->addItems(colors);
	m_settingsUi->color_selected_text->setCurrentIndex(colorList.indexOf(m_colorSelectedText));
	//
	m_settingsUi->color_selected_background->addItems(colors);
	m_settingsUi->color_selected_background->setCurrentIndex(colorList.indexOf(m_colorSelectedBackground));
	//
	m_settingsUi->color_border->addItems(colors);
	m_settingsUi->color_border->setCurrentIndex(colorList.indexOf(m_colorBorder));
	//
	if(dialog.exec() == QDialog::Accepted) {
		m_configChanged = true;
		//
		m_icon = m_settingsUi->icon_session->icon();
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
		// logout
		m_iconLogout = m_settingsUi->icon_logout->icon();
		m_logout->setIcon(m_iconLogout);
		m_commandLogout = m_settingsUi->command_logout->text();
		//
		m_commandLogout.isEmpty() ? m_logout->setVisible(false):m_logout->setVisible(true);
		m_logout->isVisible() ? m_separator1->setVisible(true):m_separator1->setVisible(false);
		// halt
		m_iconHalt = m_settingsUi->icon_halt->icon();
		m_halt->setIcon(m_iconHalt);
		m_commandHalt = m_settingsUi->command_halt->text();
		m_commandHalt.isEmpty() ? m_halt->setVisible(false):m_halt->setVisible(true);
		(m_halt->isVisible() || m_reboot->isVisible()) ? m_separator2->setVisible(true):m_separator2->setVisible(false);
		// reboot
		m_iconReboot = m_settingsUi->icon_reboot->icon();
		m_reboot->setIcon(m_iconReboot);
		m_commandReboot = m_settingsUi->command_reboot->text();
		m_commandReboot.isEmpty() ? m_reboot->setVisible(false):m_reboot->setVisible(true);
		(m_halt->isVisible() || m_reboot->isVisible()) ? m_separator2->setVisible(true):m_separator2->setVisible(false);
		// hibernate
		m_iconHibernate = m_settingsUi->icon_hibernate->icon();
		m_hibernate->setIcon(m_iconHibernate);
		m_commandHibernate = m_settingsUi->command_hibernate->text();
		m_commandHibernate.isEmpty() ? m_hibernate->setVisible(false):m_hibernate->setVisible(true);
		// suspend
		m_iconSuspend = m_settingsUi->icon_suspend->icon();
		m_suspend->setIcon(m_iconSuspend);
		m_commandSuspend = m_settingsUi->command_suspend->text();
		m_commandSuspend.isEmpty() ? m_suspend->setVisible(false):m_suspend->setVisible(true);
		//
		m_heightBorder = m_settingsUi->height_border->value();
		m_colorBorder = QColor(m_settingsUi->color_border->currentText());
		m_radiusBorderTopLeft = m_settingsUi->radius_border_top_left->value();
		m_radiusBorderTopRight = m_settingsUi->radius_border_top_right->value();
		m_radiusBorderBottomLeft = m_settingsUi->radius_border_bottom_left->value();
		m_radiusBorderBottomRight = m_settingsUi->radius_border_bottom_right->value();
		m_colorText = QColor(m_settingsUi->color_text->currentText());
		m_colorSelectedBackground = QColor(m_settingsUi->color_selected_background->currentText());
		m_colorSelectedText = QColor(m_settingsUi->color_selected_text->currentText());
		m_menu->setStyleSheet(QString(menuStyleSheet)
			.arg(m_heightBorder)					// 1
			.arg(m_colorBorder.name())				// 2
			.arg(m_radiusBorderTopLeft)				// 3
			.arg(m_radiusBorderTopRight)			// 4
			.arg(m_radiusBorderBottomLeft)			// 5
			.arg(m_radiusBorderBottomRight)			// 6
			.arg(m_colorText.name())				// 7
			.arg(m_colorSelectedBackground.name())	// 8
			.arg(m_colorSelectedText.name())		// 9
		);
	}
}

void SessionApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}

void SessionApplet::layoutChanged()
{
	static const int delta = 8;
	//m_rectItem->setRect(delta, delta, m_size.width() - 2*delta, m_size.height() - 2*delta);

}

QSize SessionApplet::desiredSize()
{
	return QSize(m_pixmapItem->boundingRect().size().width()+8, m_pixmapItem->boundingRect().size().height());
	//return QSize(64, 64);
}

void SessionApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
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

void SessionApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(isUnderMouse())
		showContextMenu(localToScreen(QPoint(0, 0)));
}

bool SessionApplet::xmlRead()
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
									if (m_xmlConfigReader.name() == "icon")
										xmlReadIcon();
									else if (m_xmlConfigReader.name() == "icon-logout")
										xmlReadIconLogout();
									else if (m_xmlConfigReader.name() == "icon-halt")
										xmlReadIconHalt();
									else if (m_xmlConfigReader.name() == "icon-reboot")
										xmlReadIconReboot();
									else if (m_xmlConfigReader.name() == "icon-hibernate")
										xmlReadIconHibernate();
									else if (m_xmlConfigReader.name() == "icon-suspend")
										xmlReadIconSuspend();
									else if (m_xmlConfigReader.name() == "command-logout")
										xmlReadCommandLogout();
									else if (m_xmlConfigReader.name() == "command-halt")
										xmlReadCommandHalt();
									else if (m_xmlConfigReader.name() == "command-reboot")
										xmlReadCommandReboot();
									else if (m_xmlConfigReader.name() == "command-hibernate")
										xmlReadCommandHibernate();
									else if (m_xmlConfigReader.name() == "command-suspend")
										xmlReadCommandSuspend();
									else if (m_xmlConfigReader.name() == "color-text")
										xmlReadColorText();
									else if (m_xmlConfigReader.name() == "color-background")
										xmlReadColorBackground();
									else if (m_xmlConfigReader.name() == "color-selected-text")
										xmlReadColorSelectedText();
									else if (m_xmlConfigReader.name() == "color-selected-background")
										xmlReadColorSelectedBackground();
									else if (m_xmlConfigReader.name() == "color-border")
										xmlReadColorBorder();
									else if (m_xmlConfigReader.name() == "radius-border-top-left")
										xmlReadRadiusBorderTopLeft();
									else if (m_xmlConfigReader.name() == "radius-border-top-right")
										xmlReadRadiusBorderTopRight();
									else if (m_xmlConfigReader.name() == "radius-border-bottom-left")
										xmlReadRadiusBorderBottomLeft();
									else if (m_xmlConfigReader.name() == "radius-border-bottom-right")
										xmlReadRadiusBorderBottomRight();
									else if (m_xmlConfigReader.name() == "height-border")
										xmlReadHeightBorder();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void SessionApplet::xmlReadIcon()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_icon = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon(QString(qtpanel_IMAGES_TARGET) + "/user.svg"));
#ifdef __DEBUG__
	MyDBG << m_icon.name();
#endif
}
void SessionApplet::xmlReadIconLogout()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon-logout");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_iconLogout = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon::fromTheme("none"));
#ifdef __DEBUG__
	MyDBG << m_iconLogout.name();
#endif
}
void SessionApplet::xmlReadIconHalt()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon-halt");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_iconHalt = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon::fromTheme("none"));
#ifdef __DEBUG__
	MyDBG << m_iconHalt.name();
#endif
}
void SessionApplet::xmlReadIconReboot()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon-reboot");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_iconReboot = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon::fromTheme("none"));
#ifdef __DEBUG__
	MyDBG << m_iconReboot.name();
#endif
}
void SessionApplet::xmlReadIconHibernate()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon-hibernate");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_iconHibernate = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon::fromTheme("none"));
#ifdef __DEBUG__
	MyDBG << m_iconHibernate.name();
#endif
}
void SessionApplet::xmlReadIconSuspend()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon-suspend");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_iconSuspend = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon::fromTheme("none"));
#ifdef __DEBUG__
	MyDBG << m_iconSuspend.name();
#endif
}

void SessionApplet::xmlReadCommandLogout()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "command-logout");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_commandLogout = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_commandLogout;
#endif
}

void SessionApplet::xmlReadCommandHalt()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "command-halt");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_commandHalt = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_commandHalt;
#endif
}

void SessionApplet::xmlReadCommandReboot()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "command-reboot");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_commandReboot = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_commandReboot;
#endif
}
void SessionApplet::xmlReadCommandHibernate()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "command-hibernate");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_commandHibernate = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_commandHibernate;
#endif
}

void SessionApplet::xmlReadCommandSuspend()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "command-suspend");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_commandSuspend = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_commandSuspend;
#endif
}

void SessionApplet::xmlReadColorText()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "color-text");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_colorText = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_colorText.name();
#endif
}

void SessionApplet::xmlReadColorBackground()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "color-background");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_colorBackground = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_colorBackground.name();
#endif
}

void SessionApplet::xmlReadColorSelectedText()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "color-selected-text");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_colorSelectedText = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_colorSelectedText.name();
#endif
}

void SessionApplet::xmlReadColorSelectedBackground()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "color-selected-background");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_colorSelectedBackground = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_colorSelectedBackground.name();
#endif
}

void SessionApplet::xmlReadColorBorder()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "color-border");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_colorBorder = QColor(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << m_colorBorder.name();
#endif
}

void SessionApplet::xmlReadRadiusBorderTopLeft()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "radius-border-top-left");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_radiusBorderTopLeft = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_radiusBorderTopLeft;
#endif
}

void SessionApplet::xmlReadRadiusBorderTopRight()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "radius-border-top-right");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_radiusBorderTopRight = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_radiusBorderTopRight;
#endif
}

void SessionApplet::xmlReadRadiusBorderBottomLeft()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "radius-border-bottom-left");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_radiusBorderBottomLeft = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_radiusBorderBottomLeft;
#endif
}

void SessionApplet::xmlReadRadiusBorderBottomRight()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "radius-border-bottom-right");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_radiusBorderBottomRight = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_radiusBorderBottomRight;
#endif
}

void SessionApplet::xmlReadHeightBorder()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "height-border");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_heightBorder = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_heightBorder;
#endif
}

void SessionApplet::logout()
{
	qDebug() << "clicked logout" << m_commandLogout;
	if (m_panelWindow->isConfigAppletsChanged())
		m_panelWindow->xmlWrite();
	QProcess::startDetached(m_commandLogout);

}

void SessionApplet::halt()
{
	qDebug() << "clicked halt" << m_commandHalt;
	if (m_panelWindow->isConfigAppletsChanged())
		m_panelWindow->xmlWrite();
	QProcess::startDetached(m_commandHalt);
}

void SessionApplet::reboot()
{
	qDebug() << "clicked reboot" << m_commandReboot;
	if (m_panelWindow->isConfigAppletsChanged())
		m_panelWindow->xmlWrite();
	QProcess::startDetached(m_commandReboot);
}

void SessionApplet::hibernate()
{
	qDebug() << "clicked hibernate" << m_commandHibernate;
	if (m_panelWindow->isConfigAppletsChanged())
		m_panelWindow->xmlWrite();
	QProcess::startDetached(m_commandHibernate);
}

void SessionApplet::suspend()
{
	qDebug() << "clicked suspend" << m_commandSuspend;
	if (m_panelWindow->isConfigAppletsChanged())
		m_panelWindow->xmlWrite();
	QProcess::startDetached(m_commandSuspend);
}

void SessionApplet::buttonIconClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QIcon icon(namefile);
		m_settingsUi->icon_session->setIcon(icon);
		m_settingsUi->icon_session->setText(icon.name());
	}
}

void SessionApplet::buttonIconLogoutClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QIcon icon(namefile);
		m_settingsUi->icon_logout->setIcon(icon);
		m_settingsUi->icon_logout->setText(icon.name());
	}
}

void SessionApplet::buttonIconHaltClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QIcon icon(namefile);
		m_settingsUi->icon_halt->setIcon(icon);
		m_settingsUi->icon_halt->setText(icon.name());
	}
}

void SessionApplet::buttonIconRebootClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QIcon icon(namefile);
		m_settingsUi->icon_reboot->setIcon(icon);
		m_settingsUi->icon_reboot->setText(icon.name());
	}
}

void SessionApplet::buttonIconHibernateClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QIcon icon(namefile);
		m_settingsUi->icon_hibernate->setIcon(icon);
		m_settingsUi->icon_hibernate->setText(icon.name());
	}
}

void SessionApplet::buttonIconSuspendClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QIcon icon(namefile);
		m_settingsUi->icon_suspend->setIcon(icon);
		m_settingsUi->icon_suspend->setText(icon.name());
	}
}

void SessionApplet::createActions()
{
	// logout
	m_logout = new QAction(m_iconLogout, tr("&Logout"), this);
	connect(m_logout, SIGNAL(triggered()), this, SLOT(logout()));
	m_commandLogout.isEmpty() ? m_logout->setVisible(false):m_logout->setVisible(true);
	// halt
	m_halt = new QAction(m_iconHalt, tr("&Halt"), this);
	connect(m_halt, SIGNAL(triggered()), this, SLOT(halt()));
	m_commandHalt.isEmpty() ? m_halt->setVisible(false):m_halt->setVisible(true);
	// reboot
	m_reboot = new QAction(m_iconReboot, tr("&Reboot"), this);
	connect(m_reboot, SIGNAL(triggered()), this, SLOT(reboot()));
	m_commandReboot.isEmpty() ? m_reboot->setVisible(false):m_reboot->setVisible(true);
	// hibernate
	m_hibernate = new QAction(m_iconHibernate, tr("H&ibernate"), this);
	connect(m_hibernate, SIGNAL(triggered()), this, SLOT(hibernate()));
	m_commandHibernate.isEmpty() ? m_hibernate->setVisible(false):m_hibernate->setVisible(true);
	// suspend
	m_suspend = new QAction(m_iconSuspend, tr("&Suspend"), this);
	connect(m_suspend, SIGNAL(triggered()), this, SLOT(suspend()));
	m_commandSuspend.isEmpty() ? m_suspend->setVisible(false):m_suspend->setVisible(true);
}

void SessionApplet::createMenu()
{
	m_menu->addAction(m_logout);
	m_separator1 = m_menu->addSeparator();
	m_logout->isVisible() ? m_separator1->setVisible(true):m_separator1->setVisible(false);
	m_menu->addAction(m_halt);
	m_menu->addAction(m_reboot);
	m_separator2 = m_menu->addSeparator();
	(m_halt->isVisible() || m_reboot->isVisible()) ? m_separator2->setVisible(true):m_separator2->setVisible(false);
	m_menu->addAction(m_hibernate);
	m_menu->addAction(m_suspend);
}
