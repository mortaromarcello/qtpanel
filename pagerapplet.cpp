#include <QApplication>
#include <QTimer>
#include <X11/Xatom.h>
#include <QMenu>
#include <QDialog>
#include "ui_appletpagersettings.h"
#include "panelapplication.h"
#include "pagerapplet.h"
#include "x11support.h"

PagerApplet::PagerApplet(PanelWindow* panelWindow)
	:Applet(panelWindow),
	m_timeout(200),
	m_usePixmaps(false),
	m_currentDesktop(xfitMan().getActiveDesktop()),
	m_rootmap(xfitMan().getRootPixmap()),
	m_timer(new QTimer(this))
{
	m_numDesktop = xfitMan().getNumDesktop();
#ifdef __DEBUG__
	MyDBG << "num desktops:" << m_numDesktop;
#endif
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	initializePixmapDesktops();
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

PagerApplet::~PagerApplet()
{
	reset();
	delete m_timer;
}

void PagerApplet::reset()
{
	for (int i = 0; i < m_listPixmapDesktops.size(); i++) {
		delete m_listPixmapDesktops.at(i);
	}
	m_listPixmapDesktops.clear();
}

bool PagerApplet::init()
{
	setInteractive(true);
	m_timer->start(m_timeout);
	return true;
}

void PagerApplet::initializePixmapDesktops()
{
	for (int i = 0; i < m_numDesktop; i++) {
		QGraphicsPixmapItem* item = new QGraphicsPixmapItem(this);
		m_listPixmapDesktops.append(item);
		if (i != m_currentDesktop) {
			item->setOpacity(0.5);
			item->setPixmap(QPixmap::fromX11Pixmap(m_rootmap).scaledToHeight(32));
		}
		else {
			if (m_usePixmaps)
				item->setPixmap(QPixmap::grabWindow(QApplication::desktop()->winId()).scaledToHeight(32));
			else
				item->setPixmap(QPixmap::fromX11Pixmap(m_rootmap).scaledToHeight(32));
		}
		item->setOffset(item->pixmap().width()*(i), 0);
	}
}

void PagerApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("timeout", QString::number(m_timeout));
	writer->writeTextElement("use-pixmaps", (m_usePixmaps) ? "true" : "false");
	writer->writeEndElement();
}

void PagerApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}

QSize PagerApplet::desiredSize()
{
	return QSize(m_listPixmapDesktops.at(0)->pixmap().width()*m_numDesktop, 32);
}

void PagerApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsPixmapItem* item = m_listPixmapDesktops.at(0);
	int newDesktop = (int)(event->pos().x() / item->pixmap().width());
	if (m_currentDesktop != newDesktop)
		xfitMan().setActiveDesktop(newDesktop);
	if(isUnderMouse())
		if(event->buttons() == Qt::RightButton)
			showContextMenu(localToScreen(QPoint(0, 0)));
}

void PagerApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
}

bool PagerApplet::xmlRead()
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
									if (m_xmlConfigReader.name() == "timeout")
										xmlReadTimeout();
									else if (m_xmlConfigReader.name() == "use-pixmaps")
										xmlReadUsePixmaps();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void PagerApplet::xmlReadTimeout()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "timeout");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	m_timeout = m_xmlConfigReader.readElementText().toInt();
}

void PagerApplet::xmlReadUsePixmaps()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "use-pixmaps");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	m_usePixmaps = (m_xmlConfigReader.readElementText() == "true") ? true : false;
}

void PagerApplet::layoutChanged()
{
}

void PagerApplet::showConfigurationDialog()
{
	QDialog dialog;
	Ui::AppletPagerSettings settingsUi;
	settingsUi.setupUi(&dialog);
	settingsUi.timeout->setMinimum(1);
	settingsUi.timeout->setMaximum(10000);
	settingsUi.timeout->setValue(m_timeout);
	settingsUi.timeout->setEnabled(m_usePixmaps);
	settingsUi.use_pixmaps->setChecked(m_usePixmaps);
	if(dialog.exec() == QDialog::Accepted) {
		m_timeout = settingsUi.timeout->value();
		m_usePixmaps = settingsUi.use_pixmaps->isChecked();
		m_timer->stop();
		reset();
		initializePixmapDesktops();
		m_timer->start(m_timeout);
		m_configChanged = true;
	}
}

void PagerApplet::update()
{
	if (m_numDesktop != xfitMan().getNumDesktop())
		QTimer::singleShot(1, PanelApplication::instance(), SLOT(reinit()));
	if (xfitMan().getActiveDesktop() != m_currentDesktop) {
		m_currentDesktop = xfitMan().getActiveDesktop();
		for (int i = 0; i < m_numDesktop; i++) {
			QGraphicsPixmapItem* item = m_listPixmapDesktops.at(i);
			if (i != m_currentDesktop)
				item->setOpacity(0.5);
			else
				item->setOpacity(1.0);
		}
	}
	if (m_usePixmaps)
		m_listPixmapDesktops.at(m_currentDesktop)->setPixmap(QPixmap::grabWindow(QApplication::desktop()->winId()).scaledToHeight(32));
}
