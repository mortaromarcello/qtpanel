#include "docklauncherapplet.h"

#include <QDateTime>
#include <QTimer>
#include <QPainter>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include "textgraphicsitem.h"
#include "panelapplication.h"
#include "panelwindow.h"
//#include "x11support.h"
#include "animationutils.h"
#include "dpisupport.h"
#include "ui_launchersettings.h"
//
//#include "xfitman.h"
//

DockLauncherItem::DockLauncherItem(DockLauncherApplet* dockLauncherApplet, Launcher* launcher):
	m_dragging(false),
	m_highlightIntensity(0.0),
	m_urgencyHighlightIntensity(0.0),
	m_dockLauncherApplet(dockLauncherApplet),
	m_launcher(launcher),
	m_animationTimer(new QTimer()),
	m_settingsUi(new Ui::LauncherSettings()),
	//
	//m_label(new QLabel(0, Qt::FramelessWindowHint)),
	//
	//m_textItem(new TextGraphicsItem(this)),
	m_iconItem(new QGraphicsPixmapItem(this))
{
	m_animationTimer->setInterval(20);
	m_animationTimer->setSingleShot(true);
	connect(m_animationTimer, SIGNAL(timeout()), this, SLOT(animate()));
	//
	//m_label->setScaledContents(true);
	//m_label->setStyleSheet("border: 1px solid white;"
					//"border-radius: 4px;"
	//				"background-color: white;"
	//				"qproperty-alignment: AlignCenter;"
	//				"qproperty-text: 'This is some rather long text.';"
	//				"qproperty-wordWrap: true;"
	//				);
	//
	setParentItem(m_dockLauncherApplet);
	setAcceptsHoverEvents(true);
	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
	//m_textItem->setColor(Qt::white);
	//m_textItem->setFont(m_dockLauncherApplet->panelWindow()->font());
}

DockLauncherItem::~DockLauncherItem()
{
	delete m_iconItem;
	//delete m_textItem;
	delete m_animationTimer;
}

void DockLauncherItem::updateContent()
{
	//QFontMetrics fontMetrics(m_textItem->font());
	//QString shortName = fontMetrics.elidedText(m_client->name(), Qt::ElideRight, m_targetSize.width() - adjustHardcodedPixelSize(36));
	//m_textItem->setText(shortName);
	//m_textItem->setPos(adjustHardcodedPixelSize(28), m_dockLauncherApplet->panelWindow()->textBaseLine());
	m_iconItem->setPixmap(m_launcher->icon().pixmap(adjustHardcodedPixelSize(24)).scaledToHeight(24));
	m_iconItem->setPos(adjustHardcodedPixelSize(4), m_targetSize.height()/2 - adjustHardcodedPixelSize(12));
	update();
}

void DockLauncherItem::setTargetPosition(const QPoint& targetPosition)
{
	m_targetPosition = targetPosition;
	//updateLauncherIconGeometry();
}

void DockLauncherItem::setTargetSize(const QSize& targetSize)
{
	m_targetSize = targetSize;
	//updateLauncherIconGeometry();
	updateContent();
}

void DockLauncherItem::moveInstantly()
{
	m_position = m_targetPosition;
	m_size = m_targetSize;
	setPos(m_position.x(), m_position.y());
	update();
}

void DockLauncherItem::startAnimation()
{
	if(!m_animationTimer->isActive())
		m_animationTimer->start();
}

void DockLauncherItem::animate()
{
	bool needAnotherStep = false;
	static const qreal highlightAnimationSpeed = 0.15;
	qreal targetIntensity = isUnderMouse() ? 1.0 : 0.0;
	m_highlightIntensity = AnimationUtils::animate(m_highlightIntensity, targetIntensity, highlightAnimationSpeed, needAnotherStep);
	static const qreal urgencyHighlightAnimationSpeed = 0.015;
	qreal targetUrgencyIntensity = 0.0;
	m_urgencyHighlightIntensity = AnimationUtils::animate(m_urgencyHighlightIntensity, targetUrgencyIntensity, urgencyHighlightAnimationSpeed, needAnotherStep);
	if(!m_dragging)
	{
		static const int positionAnimationSpeed = 24;
		static const int sizeAnimationSpeed = 24;
		m_position.setX(AnimationUtils::animateExponentially(m_position.x(), m_targetPosition.x(), 0.2, positionAnimationSpeed, needAnotherStep));
		m_position.setY(AnimationUtils::animateExponentially(m_position.y(), m_targetPosition.y(), 0.2, positionAnimationSpeed, needAnotherStep));
		m_size.setWidth(AnimationUtils::animate(m_size.width(), m_targetSize.width(), sizeAnimationSpeed, needAnotherStep));
		m_size.setHeight(AnimationUtils::animate(m_size.height(), m_targetSize.height(), sizeAnimationSpeed, needAnotherStep));
		setPos(m_position.x(), m_position.y());
	}
	update();
	if(needAnotherStep)
		m_animationTimer->start();
}

void DockLauncherItem::buttonIconClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/" + launcher()->icon().themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QString name;
		QIcon icon;
		name = QFileInfo(namefile).baseName();
		if (QIcon::hasThemeIcon(name)) {
			icon = QIcon::fromTheme(name);
			m_settingsUi->icon->setText(name);
			launcher()->setIconNameFile(QString());
		}
		else {
			icon = QIcon(QPixmap(namefile));
			m_settingsUi->icon->setText(namefile);
			launcher()->setIconNameFile(namefile);
		}
		m_settingsUi->buttonIcon->setIcon(icon);
		m_settingsUi->buttonIcon->setText(icon.name());
	}
#ifdef __DEBUG__
	MyDBG << "Name icon:"<< namefile;
#endif
}

void DockLauncherItem::iconChanged(QString nameicon)
{
	QIcon icon;
	if (QIcon::hasThemeIcon(nameicon)) {
		icon = QIcon::fromTheme(nameicon);
		launcher()->setIconNameFile(QString());
	}
	else if (QFileInfo(nameicon).exists()) {
		icon = QIcon(QPixmap(nameicon));
		launcher()->setIconNameFile(nameicon);
	}
	else
		icon = QIcon::fromTheme("none");
	m_settingsUi->buttonIcon->setIcon(icon);
}

void DockLauncherItem::buttonCommandClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load command"), QDir::currentPath(), tr("Commands (*)"));
	if (!namefile.isNull()) {
		m_settingsUi->command->setText(namefile);
	}
#ifdef __DEBUG__
	MyDBG << "Command:"<< namefile;
#endif
}

void DockLauncherItem::commandChanged(QString command)
{
	QIcon icon = QIcon::fromTheme(QFileInfo(command).fileName(), QIcon::fromTheme("none"));
	m_settingsUi->buttonIcon->setIcon(icon);
	//m_settingsUi->buttonIcon->setText(icon.name());
	m_settingsUi->icon->setText(icon.name());
}

void DockLauncherItem::configure()
{
	QDialog dialog;
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->command->setText(launcher()->command());
	QObject::connect(m_settingsUi->command, SIGNAL(textChanged(QString)), this, SLOT(commandChanged(QString)));
	m_settingsUi->buttonCommand->setIcon(QIcon::fromTheme("document-open"));
	QObject::connect(m_settingsUi->buttonCommand, SIGNAL(clicked()), this, SLOT(buttonCommandClicked()));
	m_settingsUi->tooltip->setText(launcher()->tooltip());
	if (!launcher()->icon().name().isEmpty())
		m_settingsUi->icon->setText(launcher()->icon().name());
	else {
		if (!launcher()->iconNameFile().isEmpty())
			m_settingsUi->icon->setText(launcher()->iconNameFile());
	}
	QObject::connect(m_settingsUi->icon, SIGNAL(textChanged(QString)), this, SLOT(iconChanged(QString)));
	m_settingsUi->buttonIcon->setIcon(launcher()->icon());
	m_settingsUi->buttonIcon->setText(launcher()->icon().name());
	QObject::connect(m_settingsUi->buttonIcon, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
	if(dialog.exec() == QDialog::Accepted) {
		launcher()->setCommand(m_settingsUi->command->text());
		launcher()->setIcon(m_settingsUi->buttonIcon->icon());
		launcher()->setTooltip(m_settingsUi->tooltip->text());
		updateContent();
		m_dockLauncherApplet->setConfigChanged(true);
	}
}

void DockLauncherItem::add()
{
	QDialog dialog;
	m_settingsUi->setupUi(&dialog);
	QObject::connect(m_settingsUi->command, SIGNAL(textChanged(QString)), this, SLOT(commandChanged(QString)));
	m_settingsUi->buttonCommand->setIcon(QIcon::fromTheme("document-open"));
	QObject::connect(m_settingsUi->buttonCommand, SIGNAL(clicked()), this, SLOT(buttonCommandClicked()));
	QObject::connect(m_settingsUi->icon, SIGNAL(textChanged(QString)), this, SLOT(iconChanged(QString)));
	QObject::connect(m_settingsUi->buttonIcon, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
	if(dialog.exec() == QDialog::Accepted) {
		m_dockLauncherApplet->registerDockLauncherItem(new DockLauncherItem(m_dockLauncherApplet, new Launcher(m_settingsUi->buttonIcon->icon(), m_settingsUi->command->text(), m_settingsUi->tooltip->text())), this);
	}
}

void DockLauncherItem::remove()
{
	m_dockLauncherApplet->unregisterDockLauncherItem(this);
}
/*void DockLauncherItem::close()
{
	X11Support::closeWindow(m_launcher->handle());
}*/

QRectF DockLauncherItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width() - 1, m_size.height() - 1);
}

void DockLauncherItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{/*
	painter->setPen(Qt::NoPen);
	QPointF center(m_size.width()/2.0, m_size.height() + adjustHardcodedPixelSize(32));
	QRectF rect(0.0, adjustHardcodedPixelSize(4), m_size.width(), m_size.height() - adjustHardcodedPixelSize(8));
	static const qreal roundRadius = adjustHardcodedPixelSize(3);
	QRadialGradient gradient(center, adjustHardcodedPixelSize(200), center);
	gradient.setColorAt(0.0, QColor(255, 255, 255, 80 + static_cast<int>(80*m_highlightIntensity)));
	gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRoundedRect(rect, roundRadius, roundRadius);
	//painter->drawEllipse(rect);
	if(m_urgencyHighlightIntensity > 0.001)
	{
		QRadialGradient gradient(center, adjustHardcodedPixelSize(200), center);
		gradient.setColorAt(0.0, QColor(255, 100, 0, static_cast<int>(160*m_urgencyHighlightIntensity)));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
		painter->setBrush(QBrush(gradient));
		painter->drawRoundedRect(rect, roundRadius, roundRadius);
		//painter->drawEllipse(rect);
	}*/
}

void DockLauncherItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	if(isUnderMouse()) {
		m_iconItem->setPixmap(m_launcher->icon().pixmap(adjustHardcodedPixelSize(32)).scaledToHeight(32));
		m_iconItem->setPos(adjustHardcodedPixelSize(0), m_targetSize.height()/2 - adjustHardcodedPixelSize(16));
	}
	startAnimation();
}

void DockLauncherItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	m_iconItem->setPixmap(m_launcher->icon().pixmap(adjustHardcodedPixelSize(24)).scaledToHeight(24));
	m_iconItem->setPos(adjustHardcodedPixelSize(4), m_targetSize.height()/2 - adjustHardcodedPixelSize(12));
	startAnimation();
}

void DockLauncherItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		m_dragging = true;
		m_mouseDownPosition = event->scenePos();
		m_dragStartPosition = m_position;
		m_dockLauncherApplet->draggingStarted();
		setZValue(1.0); // Be on top when dragging.
		//
		m_iconItem->setPixmap(m_launcher->icon().pixmap(adjustHardcodedPixelSize(24)).scaledToHeight(24));
		m_iconItem->setPos(adjustHardcodedPixelSize(4), m_targetSize.height()/2 - adjustHardcodedPixelSize(12));
		//
	}
}

void DockLauncherItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		m_dragging = false;
		m_dockLauncherApplet->draggingStopped();
		setZValue(0.0); // No more on top.
		startAnimation(); // Item can be out of it's regular, start animation to bring it back.
	}
	if(isUnderMouse())
	{
		if(event->button() == Qt::LeftButton)
		{
			static const qreal clickMouseMoveTolerance = 10.0;
			if((event->scenePos() - m_mouseDownPosition).manhattanLength() < clickMouseMoveTolerance)
			{
				//if (m_client->isActive())
				//	X11Support::minimizeWindow(m_client->handle());
				//else
				//
				{
				//	if (xfitMan().getActiveDesktop() != m_client->getDesktop())
				//		xfitMan().setActiveDesktop(m_client->getDesktop());
				//	m_client->activate();
				}
				//
				m_launcher->runCommand();
				#ifdef __DEBUG__
				MyDBG << "mouseReleaseEvent:" << m_launcher->command();
				#endif
			}
		}
		if(event->button() == Qt::RightButton && !m_dragging)
		{
			QMenu menu;
			menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure"), this, SLOT(configure()));
			menu.addAction(QIcon::fromTheme("list-add"), tr("Add launcher"), this, SLOT(add()));
			menu.addAction(QIcon::fromTheme("list-remove"), tr("Remove launcher"), this, SLOT(remove()));
			QAction* result = menu.exec(event->screenPos());
		}
	}
}

void DockLauncherItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	// Mouse events are sent only when mouse button is pressed.
	if(!m_dragging)
		return;
	// TODO: Vertical orientation support.
	QPointF delta = event->scenePos() - m_mouseDownPosition;
	m_position.setX(m_dragStartPosition.x() + static_cast<int>(delta.x()));
	if(m_position.x() < 0)
		m_position.setX(0);
	if(m_position.x() >= m_dockLauncherApplet->size().width() - m_targetSize.width())
		m_position.setX(m_dockLauncherApplet->size().width() - m_targetSize.width());
	setPos(m_position.x(), m_position.y());
	int criticalShift = m_targetSize.width()*55/100;
	if(m_position.x() < m_targetPosition.x() - criticalShift)
		m_dockLauncherApplet->moveItem(this, false);
	if(m_position.x() > m_targetPosition.x() + criticalShift)
		m_dockLauncherApplet->moveItem(this, true);
	update();
}

/*void DockLauncherItem::updateLauncherIconGeometry()
{
	QPointF topLeft = m_dockLauncherApplet->mapToScene(m_targetPosition);
	QVector<unsigned long> values;
	values.resize(4);
	values[0] = static_cast<unsigned long>(topLeft.x()) + m_dockLauncherApplet->panelWindow()->pos().x();
	values[1] = static_cast<unsigned long>(topLeft.y()) + m_dockLauncherApplet->panelWindow()->pos().y();
	values[2] = m_targetSize.width();
	values[3] = m_targetSize.height();
	//X11Support::setWindowPropertyCardinalArray(m_client->handle(), "_NET_WM_ICON_GEOMETRY", values);
}*/

/*bool DockLauncherItem::isUrgent()
{
	if(m_client->isUrgent())
		return true;
	return false;
}*/

Launcher::Launcher(const QIcon& icon, const QString& command, const QString& tooltip):
	m_icon(icon),
	m_command(command),
	m_tooltip(tooltip),
	m_iconNameFile(QString())
{
}

Launcher::~Launcher()
{
}

/*void Launcher::windowPropertyChanged(unsigned long atom)
{
	if(atom == X11Support::atom("_NET_WM_WINDOW_TYPE") || atom == X11Support::atom("_NET_WM_STATE")) {
#ifdef __DEBUG__
		MyDBG << QString("atom:%1").arg(atom == X11Support::atom("_NET_WM_WINDOW_TYPE")?"_NET_WM_WINDOW_TYPE":"_NET_WM_STATE");
#endif
		updateVisibility();
	}
	if(atom == X11Support::atom("_NET_WM_VISIBLE_NAME") || atom == X11Support::atom("_NET_WM_NAME") || atom == X11Support::atom("WM_NAME")) {
		updateName();
#ifdef __DEBUG__
		MyDBG << "atom:_NET_WM_VISIBLE_NAME || _NET_WM_NAME || WM_NAME ->" << name();
#endif
	}
	if(atom == X11Support::atom("_NET_WM_ICON")) {
#ifdef __DEBUG__
		MyDBG << "atom:_NET_WM_ICON";
#endif
		updateIcon();
	}
	if(atom == X11Support::atom("WM_HINTS")) {
#ifdef __DEBUG__
	MyDBG << "atom:WM_HINTS";
#endif
		updateUrgency();
	}
	if(atom == X11Support::atom("_NET_WM_ACTIVE_WINDOW")) {
#ifdef __DEBUG__
		MyDBG << "atom:_NET_WM_ACTIVE_WINDOW";
#endif
	}
}

void Launcher::updateVisibility()
{
	QVector<unsigned long> windowTypes = X11Support::getWindowPropertyAtomsArray(m_handle, "_NET_WM_WINDOW_TYPE");
	QVector<unsigned long> windowStates = X11Support::getWindowPropertyAtomsArray(m_handle, "_NET_WM_STATE");
	// Show only regular windows in dock.
	// When no window type is set, assume it's normal window.
	m_visible = (windowTypes.size() == 0) || (windowTypes.size() == 1 && windowTypes[0] == X11Support::atom("_NET_WM_WINDOW_TYPE_NORMAL"));
	// Don't show window if requested explicitly in window states.
	if(windowStates.contains(X11Support::atom("_NET_WM_STATE_SKIP_TASKBAR")))
		m_visible = false;
}

void Launcher::updateName()
{
	m_name = X11Support::getWindowName(m_handle);
}

void Launcher::updateIcon()
{
	m_icon = X11Support::getWindowIcon(m_handle);
}

void Launcher::updateUrgency()
{
	m_isUrgent = X11Support::getWindowUrgency(m_handle);
}

bool Launcher::isActive()
{
	return (handle() == xfitMan().getActiveWindow());
}

void Launcher::updatePixmap()
{
	//MyDBG << "updatePixmap" << handle() << isActive() << xfitMan().getActiveDesktop() << "==" << getDesktop() << (xfitMan().getActiveDesktop() == getDesktop());
	if (isActive()) {
		m_pixmap = QPixmap::grabWindow(handle()).scaledToHeight(96);
		//MyDBG << m_pixmap.isNull();
	}
}
int Launcher::getDesktop()
{
	return xfitMan().getWindowDesktop(handle());
}

void Launcher::activate()
{
	xfitMan().raiseWindow(handle());
}*/

DockLauncherApplet::DockLauncherApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_dragging(false)
{
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	if (m_dockLauncherItems.isEmpty())
		m_dockLauncherItems.append(new DockLauncherItem(this, new Launcher(QIcon::fromTheme("internet-web-browser"), "x-www-browser", "")));
}

DockLauncherApplet::~DockLauncherApplet()
{
	m_dockLauncherItems.clear();
	//
	//
}

bool DockLauncherApplet::init()
{
	//updateLauncherList();
	//updateActiveWindow();
	updateLayout();
	for(int i = 0; i < m_dockLauncherItems.size(); i++)
		m_dockLauncherItems[i]->moveInstantly();
	return true;
}

void DockLauncherApplet::updateLayout()
{
	// TODO: Vertical orientation support.
	int freeSpace = m_size.width();
	//int spaceForOneLauncher = (m_dockLauncherItems.size() > 0) ? freeSpace/m_dockLauncherItems.size() : 0;
	int spaceForOneLauncher = (m_dockLauncherItems.size() > 0) ? freeSpace/m_dockLauncherItems.size() : 0;;
	int currentPosition = 0;
	for(int i = 0; i < m_dockLauncherItems.size(); i++)
	{
		int spaceForThisLauncher = spaceForOneLauncher;
		static const int maxSpace = adjustHardcodedPixelSize(32);//(256);
		if(spaceForThisLauncher > maxSpace)
			spaceForThisLauncher = maxSpace;
		m_dockLauncherItems[i]->setTargetPosition(QPoint(currentPosition, 0));
		m_dockLauncherItems[i]->setTargetSize(QSize(spaceForThisLauncher - 4, m_size.height()));
		m_dockLauncherItems[i]->startAnimation();
		currentPosition += spaceForThisLauncher;
	}
	update();
}

void DockLauncherApplet::draggingStarted()
{
#ifdef __DEBUG__
	MyDBG << "draggingStarted";
#endif
	m_dragging = true;
}

void DockLauncherApplet::draggingStopped()
{
#ifdef __DEBUG__
	MyDBG << "draggingStopped";
#endif
	m_dragging = false;
	// Since we don't update it when dragging, we should do it now.
	//updateLauncherList();
}

void DockLauncherApplet::moveItem(DockLauncherItem* dockLauncherItem, bool right)
{
	int currentIndex = m_dockLauncherItems.indexOf(dockLauncherItem);
	if(right)
	{
		if(currentIndex != (m_dockLauncherItems.size() - 1))
		{
			m_dockLauncherItems.remove(currentIndex);
			m_dockLauncherItems.insert(currentIndex + 1, dockLauncherItem);
			updateLayout();
		}
	}
	else
	{
		if(currentIndex != 0)
		{
			m_dockLauncherItems.remove(currentIndex);
			m_dockLauncherItems.insert(currentIndex - 1, dockLauncherItem);
			updateLayout();
		}
	}
}

void DockLauncherApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	//
	for (int i = 0; i < m_dockLauncherItems.size(); i++) {
		writer->writeStartElement("launcheritem");
		if (!m_dockLauncherItems[i]->launcher()->icon().name().isEmpty())
			writer->writeTextElement("icon", m_dockLauncherItems[i]->launcher()->icon().name());
		else
			writer->writeTextElement("icon", m_dockLauncherItems[i]->launcher()->iconNameFile());
		writer->writeTextElement("command", m_dockLauncherItems[i]->launcher()->command());
		writer->writeTextElement("tooltip", m_dockLauncherItems[i]->launcher()->tooltip());
		writer->writeEndElement();
	}
	//
	writer->writeEndElement();
}

void DockLauncherApplet::layoutChanged()
{
	updateLayout();
}

bool DockLauncherApplet::xmlRead()
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
									if (m_xmlConfigReader.name() == "launcheritem") {
										DockLauncherItem* item = new DockLauncherItem(this, new Launcher());
										while (m_xmlConfigReader.readNextStartElement())
											if (m_xmlConfigReader.name() == "icon")
												xmlReadIcon(item);
											else if (m_xmlConfigReader.name() == "command")
												xmlReadCommand(item);
											else if (m_xmlConfigReader.name() == "tooltip")
												xmlReadTooltip(item);
										registerDockLauncherItem(item);
									}
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void DockLauncherApplet::xmlReadIcon(DockLauncherItem* item)
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	QString nameIcon = m_xmlConfigReader.readElementText();
	item->launcher()->setIcon(QIcon::fromTheme(nameIcon));
	if (QIcon::hasThemeIcon(nameIcon))
		item->launcher()->setIcon(QIcon::fromTheme(nameIcon));
	else {
		item->launcher()->setIconNameFile(nameIcon);
		item->launcher()->setIcon(QIcon(QPixmap(item->launcher()->iconNameFile())));
	}
#ifdef __DEBUG__
	MyDBG << item->launcher()->icon().name();
#endif
}

void DockLauncherApplet::xmlReadCommand(DockLauncherItem* item)
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "command");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	//m_command = m_xmlConfigReader.readElementText();
	item->launcher()->setCommand(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << item->launcher()->command();
#endif
}

void DockLauncherApplet::xmlReadTooltip(DockLauncherItem* item)
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "tooltip");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	//m_textToolTip = m_xmlConfigReader.readElementText();
	item->launcher()->setTooltip(m_xmlConfigReader.readElementText());
#ifdef __DEBUG__
	MyDBG << item->launcher()->tooltip();
#endif
}

QSize DockLauncherApplet::desiredSize()
{
	//QSize size(32, 32);
	//for (int i = 0; i < m_dockLauncherItems.size();i++)
	//{
	//	if (!m_dockLauncherItems[i]->size().isNull())
	//		size += QSize(0, m_dockLauncherItems[i]->size().height());
	//}
	//MyDBG << "desiredSize;" << size;
	//if (!size.isNull())
	//return size;
	//else return QSize(32, 32);
	return QSize(m_dockLauncherItems.size() * 28, 28);//(-1, -1); // Take all available space.
}

void DockLauncherApplet::registerDockLauncherItem(DockLauncherItem* dockLauncherItem, DockLauncherItem* prev)
{
	int i;
	if (prev) {
		i = m_dockLauncherItems.indexOf(prev);
		m_dockLauncherItems.insert(i+1, dockLauncherItem);
	}
	else
		m_dockLauncherItems.append(dockLauncherItem);
	updateLayout();
	m_panelWindow->updateLayout();
	m_configChanged = true;
}

void DockLauncherApplet::unregisterDockLauncherItem(DockLauncherItem* dockLauncherItem)
{
	m_dockLauncherItems.remove(m_dockLauncherItems.indexOf(dockLauncherItem));
	delete dockLauncherItem;
	updateLayout();
	m_panelWindow->updateLayout();
	m_configChanged = true;
}

/*void DockLauncherApplet::updateLauncherList()
{
#ifdef __DEBUG__
	MyDBG << "updateLauncherList";
#endif
	if(m_dragging)
		return; // Don't want new dock items to appear (or old to be removed) while rearranging them with drag and drop.
	//QVector<unsigned long> windows = X11Support::getWindowPropertyWindowsArray(X11Support::rootWindow(), "_NET_CLIENT_LIST");
	// Handle new clients.
	for(int i = 0; i < m_dockLauncherItems.size(); i++)
	{
		Launcher* launcher = getLauncherFromDockLaucherItems(i);
		if (!launcher)
		{
			// Skip our own windows.
			//if(QWidget::find(windows[i]))
			//	continue;
			launcher = new Launcher(windows[i]);
			if (launcher->isVisible()) {
				registerDockLaucherItem(new DockLaucherItem(this, launcher));
#ifdef __DEBUG__
				MyDBG << "added launcher:" << launcher->name();
#endif
			}
		}
	}
	// Handle removed clients.
	for(;;)
	{
		bool clientRemoved = false;
		foreach(DockLaucherItem* item, m_dockItems)
		{
			int handle = item->launcher()->handle();
			if(!windows.contains(handle))
			{
#ifdef __DEBUG__
				MyDBG << "removed launcher:" << item->launcher()->name();
#endif
				unregisterDockLaucherItem(item);
				clientRemoved = true;
				break;
			}
		}
		if(!clientRemoved)
			break;
	}
}*/

Launcher* DockLauncherApplet::getLauncherFromDockLauncherItems(int pos)
{
	//DockLaucherItem* item;
	//for (int i = 0; i < m_dockItems.size(); i++) {
	//	item = m_dockItems[i];
	//	if (item->launcher()->handle() == handle)
	if (pos < m_dockLauncherItems.size())
		return m_dockLauncherItems[pos]->launcher();
	else
		return NULL;
}

DockLauncherItem* DockLauncherApplet::getDockLauncherItem(int pos)
{
	//DockLaucherItem* item;
	//for (int i = 0; i < m_dockItems.size(); i++) {
	//	item = m_dockItems[i];
	//	if (item->launcher()->handle() == handle)
	//		return item;
	if (pos < m_dockLauncherItems.size())
		return m_dockLauncherItems[pos];
	else
		return NULL;
}

/*void DockLauncherApplet::windowPropertyChanged(unsigned long window, unsigned long atom)
{
	if(window == X11Support::rootWindow()) {
		if(atom == X11Support::atom("_NET_CLIENT_LIST")) {
#ifdef __DEBUG__
			MyDBG << "atom:_NET_CLIENT_LIST";
#endif
			updateLauncherList();
		}
		if(atom == X11Support::atom("_NET_ACTIVE_WINDOW")) {
#ifdef __DEBUG__
			MyDBG << "atom:_NET_ACTIVE_WINDOW";
#endif
		}
		return;
	}
	if(getLauncherFromDockLaucherItems(window) != NULL) {
		getLauncherFromDockLaucherItems(window)->windowPropertyChanged(atom);
		if (atom == X11Support::atom("_NET_WM_VISIBLE_NAME") || atom == X11Support::atom("_NET_WM_NAME") || atom == X11Support::atom("WM_NAME"))
			getDockLaucherItem(window)->updateContent();
	}
}*/
