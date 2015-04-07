#include "clockapplet.h"

#include <QTimer>
#include <QDateTime>
#include <QGraphicsScene>
#include <QToolTip>
#include <QCalendarWidget>
#include <QMenu>
#include <QProcess>
#include "textgraphicsitem.h"
#include "panelwindow.h"
#include "organizer.h"
#include "panelapplication.h"
#include "ui_appletclocksettings.h"

ClockApplet::ClockApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_timer(new QTimer()),
	m_textItem(new TextGraphicsItem(this)),
	m_organizer(new Organizer()),
	m_useOrganizer(false),
	m_timeFormat("H:m"),
	m_settingsUi(new Ui::AppletClockSettings())
{
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateContent()));
	m_textItem->setColor(Qt::white);
	m_textItem->setFont(m_panelWindow->font());
	m_organizer->setWindowFlags(Qt::FramelessWindowHint);
	m_organizer->setStyleSheet("QMainWindow {border: 1px solid black}");
	m_organizer->resize(m_organizer->sizeHint());
}

ClockApplet::~ClockApplet()
{
	delete m_textItem;
	delete m_timer;
	delete m_organizer;
}

void ClockApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("useorganizer", (m_useOrganizer) ? "true" : "false");
	writer->writeTextElement("timeformat", QString("%1").arg(m_timeFormat));
	writer->writeEndElement();
}

bool ClockApplet::init()
{
	updateContent();
	setInteractive(true);
	return true;
}

void ClockApplet::layoutChanged()
{
	m_textItem->setPos((m_size.width() - m_textItem->boundingRect().size().width())/2.0, m_panelWindow->textBaseLine());
	update();
}

void ClockApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
#ifdef __DEBUG__
	MyDBG << "MousePressEvent";
#endif
	if (event->buttons() == Qt::RightButton)
		showContextMenu(localToScreen(QPoint(0, 0)));
	if (event->buttons() == Qt::LeftButton && m_useOrganizer) {
		if (m_organizer->isHidden()) {
			int x, y;
			x = pos().x()+size().width()-m_organizer->width();
			x = (x < 0) ? 0 : x;
			y = (m_panelWindow->verticalAnchor() == PanelWindow::Top) ? size().height():localToScreen(pos().toPoint()).y() - m_organizer->height()-size().height();
			QPoint point = QPoint(x, y);
			m_organizer->move(point);
			m_organizer->show();
		}
		else m_organizer->hide();
	}
}

void ClockApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
}

bool ClockApplet::xmlRead()
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
									if (m_xmlConfigReader.name() == "useorganizer")
										xmlReadUseOrganizer();
									else if (m_xmlConfigReader.name() == "timeformat")
										xmlReadTimeFormat();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void ClockApplet::xmlReadUseOrganizer()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "useorganizer");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_useOrganizer = (m_xmlConfigReader.readElementText() == "true") ? true : false;
}

void ClockApplet::xmlReadTimeFormat()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "timeformat");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_timeFormat = m_xmlConfigReader.readElementText();
}

void ClockApplet::updateContent()
{
	//QDateTime dateTimeNow = QDateTime::currentDateTime();
	//m_text = dateTimeNow.toString();
	QDate dateNow = QDate::currentDate();
	m_textToolTip = dateNow.toString();
	QTime timeNow = QTime::currentTime();
	m_text = timeNow.toString(m_timeFormat);
	m_textItem->setText(m_text);
	update();
	scheduleUpdate();
}

QSize ClockApplet::desiredSize()
{
	return QSize(m_textItem->boundingRect().width() + 8, m_textItem->boundingRect().height() + 8);
}

void ClockApplet::showConfigurationDialog()
{
	QDialog dialog;
	QStringList formats = QStringList() << "h:m" << "h:mm" << "hh:mm" << "h:m:s" << "h:m:ss" << "h:mm:ss" << "hh:mm:ss" << "h:m ap" << "h:mm ap" << "hh:mm ap" << "h:m:s ap" << "h:m:ss ap" << "h:mm:ss ap" << "hh:mm:ss ap";
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->useorganizer->setChecked(m_useOrganizer);
	m_settingsUi->timeFormat->addItems(formats);
	m_settingsUi->timeFormat->setCurrentIndex(formats.indexOf(m_timeFormat));
	if(dialog.exec() == QDialog::Accepted) {
		m_useOrganizer = m_settingsUi->useorganizer->isChecked();
		m_timeFormat = m_settingsUi->timeFormat->currentText();
		// restart:
		//qApp->quit();
		//QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
		updateContent();
		m_panelWindow->updateLayout();
	}
}

void ClockApplet::scheduleUpdate()
{
	m_timer->setInterval(1000 - QDateTime::currentDateTime().time().msec());
	m_timer->start();
}

void  ClockApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}
