#include "panelwindow.h"

//
#include <QStyleFactory>
//
#include <QResizeEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QDir>
#include <QDialog>
#include <QTimer>
#include "mydebug.h"
#include "config.h"
#include "x11support.h"
#include "dpisupport.h"
#include "panelapplication.h"
// Applets
#include "applicationsmenuapplet.h"
#include "backlightapplet.h"
#include "batteryapplet.h"
#include "clockapplet.h"
#include "demoapplet.h"
#include "dockapplet.h"
#include "docklauncherapplet.h"
#include "launcherapplet.h"
#include "mediaapplet.h"
#include "memoryapplet.h"
#include "pagerapplet.h"
#include "sessionapplet.h"
#include "spacerapplet.h"
#include "trayapplet.h"
#include "volumeapplet.h"
#include "dialogappletoptions.h"

PanelWindowGraphicsItem::PanelWindowGraphicsItem(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow)
{
	setZValue(-10.0); // Background.
	setAcceptedMouseButtons(Qt::RightButton);
}

PanelWindowGraphicsItem::~PanelWindowGraphicsItem()
{
}

QRectF PanelWindowGraphicsItem::boundingRect() const
{
	return QRectF(0.0, 0.0, m_panelWindow->width(), m_panelWindow->height());
}

void PanelWindowGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	QColor color(PanelApplication::instance()->getColorApplication());
	color.setAlpha(PanelApplication::instance()->getTrasparency());
	painter->setBrush(color);
	painter->drawRect(boundingRect());
	static const int borderThickness = 3;
	if(m_panelWindow->verticalAnchor() == PanelWindow::Top)
	{
		QLinearGradient gradient(0.0, m_panelWindow->height() - borderThickness, 0.0, m_panelWindow->height());
		gradient.setSpread(QGradient::RepeatSpread);
		gradient.setColorAt(0.0, QColor(255, 255, 255, 0));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 128));
		painter->setBrush(QBrush(gradient));
		painter->drawRect(0.0, m_panelWindow->height() - borderThickness, m_panelWindow->width(), borderThickness);
	}
	else
	{
		QLinearGradient gradient(0.0, 0.0, 0.0, borderThickness);
		gradient.setSpread(QGradient::RepeatSpread);
		gradient.setColorAt(0.0, QColor(255, 255, 255, 128));
		gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
		painter->setBrush(QBrush(gradient));
		painter->drawRect(0.0, 0.0, m_panelWindow->width(), borderThickness);
	}
}

void PanelWindowGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void PanelWindowGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(isUnderMouse())
	{
		m_panelWindow->showPanelContextMenu(QPoint(static_cast<int>(event->pos().x()), static_cast<int>(event->pos().y())));
	}
}

PanelWindow::PanelWindow()
	: m_dockMode(false),
	m_screen(0),
	m_verticalAnchor(Top),
	m_layoutPolicy(Normal),
	m_xmlConfigReader(qtpanel_APPLETS_FILE_CONFIG),
	m_xmlConfigWriter(qtpanel_APPLETS_FILE_CONFIG),
	m_scene(new QGraphicsScene()),
	m_panelItem(new PanelWindowGraphicsItem(this)),
	m_view(new QGraphicsView(m_scene, this)),
	m_trolltechConf(new QSettings(QDir::homePath()+QDir::separator()+".config"+QDir::separator()+"Trolltech.conf", QSettings::NativeFormat))
{
	m_trolltechConf->beginGroup("qt");
	setStyleSheet(QString("background-color: %1").arg(PanelApplication::instance()->getColorApplication()));
	setAttribute(Qt::WA_TranslucentBackground);
	m_scene->setBackgroundBrush(QBrush(Qt::NoBrush));
	m_scene->addItem(m_panelItem);
	m_view->setStyleSheet("border-style: none;");
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setRenderHint(QPainter::Antialiasing);
	m_view->move(0, 0);
	xmlRead();
	resize(adjustHardcodedPixelSize(512), adjustHardcodedPixelSize(48));
}

PanelWindow::~PanelWindow()
{
	if (isConfigAppletsChanged())
		xmlWrite();
	while(!m_applets.isEmpty())
	{
		delete m_applets[m_applets.size() - 1];
		m_applets.resize(m_applets.size() - 1);
	}
	delete m_view;
	delete m_panelItem;
	delete m_scene;
}

bool PanelWindow::init()
{
	for(int i = 0; i < m_applets.size();)
	{
		if(!m_applets[i]->init())
			m_applets.remove(i);
		else
			i++;
	}
}

void PanelWindow::setDockMode(bool dockMode)
{
	m_dockMode = dockMode;

	setAttribute(Qt::WA_X11NetWmWindowTypeDock, m_dockMode);

	if(!m_dockMode)
	{
		// No need to reserve space anymore.
		X11Support::removeWindowProperty(winId(), "_NET_WM_STRUT");
		X11Support::removeWindowProperty(winId(), "_NET_WM_STRUT_PARTIAL");
	}

	// When in dock mode, panel should appear on all desktops.
	unsigned long desktop = m_dockMode ? 0xFFFFFFFF : 0;
	X11Support::setWindowPropertyCardinal(winId(), "_NET_WM_DESKTOP", desktop);

	updateLayout();
	updatePosition();
}

void PanelWindow::setScreen(int screen)
{
	m_screen = screen;
	updateLayout();
	updatePosition();
}

void PanelWindow::setVerticalAnchor(Anchor verticalAnchor)
{
	m_verticalAnchor = verticalAnchor;
	updatePosition();
}

void PanelWindow::setLayoutPolicy(LayoutPolicy layoutPolicy)
{
	m_layoutPolicy = layoutPolicy;
	updateLayout();
}

void PanelWindow::updatePosition()
{
	if(!m_dockMode)
		return;

	QRect screenGeometry = QApplication::desktop()->screenGeometry(m_screen);

	int x;
	int y;

	switch(m_verticalAnchor)
	{
	case Top:
		y = screenGeometry.top();
		break;
	case Bottom:
		y = screenGeometry.bottom() - height() + 1;
		break;
	default:
		Q_ASSERT(false);
		break;
	}

	move(x, y);

	// Update reserved space.
	if(m_dockMode)
	{
		QVector<unsigned long> values; // Values for setting _NET_WM_STRUT_PARTIAL property.
		values.fill(0, 12);
		switch(m_verticalAnchor)
		{
		case Top:
			values[2] = y + height();
			values[8] = x;
			values[9] = x + width();
			break;
		case Bottom:
			values[3] = QApplication::desktop()->height() - y;
			values[10] = x;
			values[11] = x + width();
			break;
		default:
			break;
		}
		X11Support::setWindowPropertyCardinalArray(winId(), "_NET_WM_STRUT_PARTIAL", values);
		values.resize(4);
		X11Support::setWindowPropertyCardinalArray(winId(), "_NET_WM_STRUT", values);
	}

	// Update "blur behind" hint.
	QVector<unsigned long> values;
	values.resize(4);
	values[0] = 0;
	values[1] = 0;
	values[2] = width();
	values[3] = height();
	X11Support::setWindowPropertyCardinalArray(winId(), "_KDE_NET_WM_BLUR_BEHIND_REGION", values);
}

const QFont& PanelWindow::font() const
{
	return PanelApplication::instance()->panelFont();
}

int PanelWindow::textBaseLine()
{
	QFontMetrics metrics(font());
	return (height() - metrics.height())/2 + metrics.ascent();
}

void PanelWindow::resizeEvent(QResizeEvent* event)
{
	m_view->resize(event->size());
	m_view->setSceneRect(0, 0, event->size().width(), event->size().height());
	updateLayout();
	updatePosition();
}

void PanelWindow::updateLayout()
{
	// TODO: Vertical orientation support.

	static const int spacing = adjustHardcodedPixelSize(2);

	if(m_layoutPolicy != Normal && !m_dockMode)
	{
		int desiredSize = 0;
		if(m_layoutPolicy == AutoSize)
		{
			for(int i = 0; i < m_applets.size(); i++)
			{
				if(m_applets[i]->desiredSize().width() >= 0)
					desiredSize += m_applets[i]->desiredSize().width();
				else
					desiredSize += 64; // Spacer applets don't really make sense on auto-size panel.
			}
			desiredSize += spacing*(m_applets.size() - 1);
			if(desiredSize < 0)
				desiredSize = 0;
		}
		if(m_layoutPolicy == FillSpace)
		{
			QRect screenGeometry = QApplication::desktop()->screenGeometry(m_screen);
			desiredSize = screenGeometry.width();
		}

		if(desiredSize != width())
			resize(desiredSize, height());
	}

	// Get total amount of space available for "spacer" applets (that take all available free space).
	int freeSpace = width() - spacing*(m_applets.size() - 1);
	int numSpacers = 0;
	for(int i = 0; i < m_applets.size(); i++)
	{
		if(m_applets[i]->desiredSize().width() >= 0)
			freeSpace -= m_applets[i]->desiredSize().width();
		else
			numSpacers++;
	}
	int spaceForOneSpacer = numSpacers > 0 ? (freeSpace/numSpacers) : 0;

	// Calculate rectangles for each applet.
	int spacePos = 0;
	for(int i = 0; i < m_applets.size(); i++)
	{
		QPoint appletPosition(spacePos, 0);
		QSize appletSize = m_applets[i]->desiredSize();

		if(appletSize.width() < 0)
		{
			if(numSpacers > 1)
			{
				appletSize.setWidth(spaceForOneSpacer);
				freeSpace -= spaceForOneSpacer;
				numSpacers--;
			}
			else
			{
				appletSize.setWidth(freeSpace);
				freeSpace = 0;
				numSpacers--;
			}
		}

		appletSize.setHeight(height());

		m_applets[i]->setPosition(appletPosition);
		m_applets[i]->setSize(appletSize);

		spacePos += appletSize.width() + spacing;
	}
}

void PanelWindow::showPanelContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("application-exit"), tr("Quit panel"), QApplication::instance(), SLOT(quit()));
	menu.exec(pos() + point);
}

bool PanelWindow::xmlRead()
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
					xmlReadType();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void PanelWindow::xmlReadType()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "type");
	QString type = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString() << "applet:" << type;
#endif
	if (type == "ApplicationsMenuApplet")
		m_applets.append(new ApplicationsMenuApplet(this));
	else if (type == "BacklightApplet")
		m_applets.append(new BacklightApplet(this));
	else if (type == "BatteryApplet")
		m_applets.append(new BatteryApplet(this));
	else if (type == "ClockApplet")
		m_applets.append(new ClockApplet(this));
	else if(type == "DemoApplet")
		m_applets.append(new DemoApplet(this));
	else if (type == "DockApplet")
		m_applets.append(new DockApplet(this));
		//
	else if (type == "DockLauncherApplet")
		m_applets.append(new DockLauncherApplet(this));
		//
	else if (type == "LauncherApplet")
		m_applets.append(new LauncherApplet(this));
	else if (type == "MediaApplet")
		m_applets.append(new MediaApplet(this));
	else if (type == "MemoryApplet")
		m_applets.append(new MemoryApplet(this));
	else if (type == "PagerApplet")
		m_applets.append(new PagerApplet(this));
	else if (type == "SessionApplet")
		m_applets.append(new SessionApplet(this));
	else if (type == "SpacerApplet")
		m_applets.append(new SpacerApplet(this));
	else if (type == "TrayApplet")
		m_applets.append(new TrayApplet(this));
	else if (type == "VolumeApplet")
		m_applets.append(new VolumeApplet(this));
}

void PanelWindow::xmlWrite()
{
	if (!m_xmlConfigWriter.xmlOpen())
	{
#ifdef __DEBUG__
		MyDBG << "Error writing file.";
#else
		qDebug("Error writing file.");
#endif
		return;
	}
	m_xmlConfigWriter.writeStartDocument();
	m_xmlConfigWriter.writeStartElement("applets");
	m_xmlConfigWriter.writeAttribute("version", "1.0");
    for (int i = 0; i < m_applets.size(); i++) {
		m_xmlConfigWriter.writeStartElement("applet");
		m_xmlConfigWriter.writeTextElement("type", m_applets.at(i)->getNameApplet());
		m_applets.at(i)->xmlWrite(&m_xmlConfigWriter);
		m_xmlConfigWriter.writeEndElement();
	}
	m_xmlConfigWriter.writeEndElement();
	m_xmlConfigWriter.writeEndDocument();
	m_xmlConfigWriter.xmlClose();
}

void PanelWindow::showConfigurationDialog()
{
	DialogAppletOptions dialog(this, m_applets);
	if(dialog.exec() == QDialog::Accepted) {
#ifdef __DEBUG__
		for(int i = 0; i < m_applets.size(); i++)
			MyDBG << m_applets.at(i)->getNameApplet();
#endif
		m_applets = dialog.getApplets();
		xmlWrite();
		QTimer::singleShot(1, PanelApplication::instance(), SLOT(reinit()));
	}
}

bool PanelWindow::isConfigAppletsChanged()
{
	for (int i = 0; i <  m_applets.size(); i++)
		if (m_applets.at(i)->isConfigChanged())
			return true;
	return false;
}
