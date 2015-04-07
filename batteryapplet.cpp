#include <QToolTip>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QProcess>
#include "batteryapplet.h"
#include "dpisupport.h"
#include "ui_appletbatterysettings.h"
#include "config.h"
#include "panelapplication.h"

BatteryApplet::BatteryApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_pixmapItem(new QGraphicsPixmapItem(this)),
	m_percBeforeHalt(5),
	m_delta(adjustHardcodedPixelSize(4)),
	m_settingsUi(new Ui::AppletBatterySettings())
{
	batteryIconMissing = "battery-missing.png";
	//batteryIcons << "battery-empty" << "battery-low" << "battery-good" << "battery-full";
	batteryIcons << "battery-low.png" << "battery-caution.png" << "battery-040.png" << "battery-060.png" << "battery-080.png" << "battery-100.png";
	batteryIconsCharging << "battery-charging-low.png" << "battery-charging-caution.png" << "battery-charging-040.png" << "battery-charging-060.png" << "battery-charging-080.png" << "battery-charging.png";
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
}

BatteryApplet::~BatteryApplet()
{
	delete m_pixmapItem;
	delete m_settingsUi;
	m_timer.stop();
}

bool BatteryApplet::init()
{
	setInteractive(true);
	if (typeBattery()) {
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
		m_timer.start(2000);
		setIcon(percentage());
	}
	else
		setIcon(-1);
	return true;
}

QSize BatteryApplet::desiredSize()
{
	return QSize(adjustHardcodedPixelSize(32), adjustHardcodedPixelSize(32));
}

void BatteryApplet::update()
{
	Battery::update();
	m_textToolTip = Battery::info();
	setIcon(percentage());
	if (percentage() <= m_percBeforeHalt && !isCharging()) {
		m_timer.stop();
		if (m_panelWindow->isConfigAppletsChanged())
			m_panelWindow->xmlWrite();
		QProcess::startDetached("gksudo \"shutdown -h -P now\"");
	}
}

void BatteryApplet::setIcon(int percentage)
{
	if (percentage == -1) {
		m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIconMissing);
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	}
	else if (percentage >= 0 && percentage < 5) {
		if (isCharging())
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIconsCharging.at(0));
		else
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIcons.at(0));
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	}
	else if (percentage >= 5 && percentage < 20) {
		if (isCharging())
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIconsCharging.at(1));
		else
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIcons.at(1));
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	}
	else if (percentage >= 20 && percentage < 40) {
		if (isCharging())
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIconsCharging.at(2));
		else
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIcons.at(2));
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	}
	else if (percentage >= 40 && percentage < 60) {
		if (isCharging())
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIconsCharging.at(3));
		else
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIcons.at(3));
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	}
	else if (percentage >= 60 && percentage < 80) {
		if (isCharging())
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIconsCharging.at(4));
		else
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIcons.at(4));
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	}
	else if (percentage >= 80 && percentage <= 100) {
		if (isCharging())
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIconsCharging.at(5));
		else
			m_icon = QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + batteryIcons.at(5));
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	}
}

void BatteryApplet::layoutChanged()
{
	m_pixmapItem->setOffset(m_delta, m_delta);
}

bool BatteryApplet::xmlRead()
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
									//if (m_xmlConfigReader.name() == "icon-missing")
									//	xmlReadIconMissing();
									//else if (m_xmlConfigReader.name() == "icons")
									//	xmlReadIcons();
									//else if (m_xmlConfigReader.name() == "icons-charging")
									//	xmlReadIconsCharging();
									/*else*/ if (m_xmlConfigReader.name() == "perc-before-halt")
										xmlReadPercBeforeHalt();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void BatteryApplet::xmlReadIconMissing()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon-missing");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	batteryIconMissing = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << batteryIconMissing;
#endif
}

void BatteryApplet::xmlReadIcons()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icons");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	batteryIcons = QString(m_xmlConfigReader.readElementText()).split(",");
#ifdef __DEBUG__
	MyDBG << batteryIcons;
#endif
}

void BatteryApplet::xmlReadIconsCharging()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icons-charging");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	batteryIconsCharging = QString(m_xmlConfigReader.readElementText()).split(",");
#ifdef __DEBUG__
	MyDBG << batteryIconsCharging;
#endif
}

void BatteryApplet::xmlReadPercBeforeHalt()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "perc-before-halt");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_percBeforeHalt = m_xmlConfigReader.readElementText().toInt();
#ifdef __DEBUG__
	MyDBG << m_percBeforeHalt;
#endif
}

void BatteryApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	//writer->writeTextElement("icon-missing", batteryIconMissing);
	//writer->writeTextElement("icons", batteryIcons.join(","));
	//writer->writeTextElement("icons-charging", batteryIconsCharging.join(","));
	writer->writeTextElement("perc-before-halt", QString("%1").arg(m_percBeforeHalt));
	writer->writeEndElement();
}

void BatteryApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}

void BatteryApplet::showConfigurationDialog()
{
	QDialog dialog;
	m_settingsUi->setupUi(&dialog);
	//m_settingsUi->icons->setText(batteryIcons.join(","));
	//m_settingsUi->icons_charging->setText(batteryIconsCharging.join(","));
	//m_settingsUi->icon_missing->setText(batteryIconMissing);
	m_settingsUi->perc_before_halt->setValue(m_percBeforeHalt);
	if(dialog.exec() == QDialog::Accepted) {
		//batteryIcons = m_settingsUi->icons->text().split(",");
		//batteryIconsCharging = m_settingsUi->icons_charging->text().split(",");
		//batteryIconMissing = m_settingsUi->icon_missing->text();
		m_percBeforeHalt = m_settingsUi->perc_before_halt->value();
		m_configChanged = true;
	}
}

void BatteryApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void BatteryApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (isUnderMouse())
		showContextMenu(localToScreen(QPoint(0, 0)));
}
