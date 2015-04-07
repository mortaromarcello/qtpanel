#include "battery.h"
#include "mydebug.h"
#include <QFile>
#include <QDir>

Battery::Battery()
{
	static int battery_num 	= 1;
	m_typeBattery			= false;
	m_capacityUnit			= "mAh";
	m_energyFull			= -1;
	m_chargeFull			= -1;
	m_voltageNow			= -1;
	m_energyFullDesign		= -1;
	m_chargeFullDesign		= -1;
	m_energyNow				= -1;
	m_chargeNow				= -1;
	m_currentNow			= -1;
	m_powerNow				= -1;
	m_state					= QString();
	m_batteryNum			= battery_num;
	m_seconds				= -1;
	m_percentage			= -1;
	m_poststr				= QString();
	battery_num++;
	QDir dir( ACPI_PATH_SYS_POWER_SUPPY);
	if (!dir.exists()) 
	{
		qWarning() << QObject::tr("NO ACPI/sysfs support in kernel.");
		return;
	}
	dir.setFilter(QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
	QStringList list = dir.entryList();
#ifdef __DEBUG__
	MyDBG << list;
#endif
	for (int i = 0; i < list.size(); i++) {
		m_path = list.at(i);
		update();
		if (m_typeBattery == true)
			break;
	}
}

Battery::~Battery()
{
}

QString Battery::parseInfoFile(const QString & sys_file)
{
	QString value;
	QString path(QString(ACPI_PATH_SYS_POWER_SUPPY) + "/" + m_path + "/" + sys_file);
	QFile filename(path);
#ifdef __DEBUG__
	MyDBG << path;
#endif
	if (filename.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&filename);
		in >> value;
		filename.close();
	}
#ifdef __DEBUG__
	MyDBG << value;
#endif
	return value;
}

int Battery::getIntFromInfoFile(const QString & sys_file)
{
	QString fileContent = parseInfoFile(sys_file);
	if (!fileContent.isEmpty())
		return fileContent.toInt() / 1000;
	return -1;
}

QString Battery::getQStringFromInfoFile(const QString & sys_file)
{
	return parseInfoFile(sys_file);
}

bool Battery::batteryInserted(const QString & path)
{
	if (path.isEmpty())
		return false;
	QDir dirname(QString(ACPI_PATH_SYS_POWER_SUPPY) + "/" + m_path + "/");
	if (dirname.exists())
		return true;
	else
		return false;
}

void Battery::update()
{
	QString gctmp;
	if (!batteryInserted(m_path))
		return;
	/* read from sysfs */
	m_chargeNow 	= getIntFromInfoFile("charge_now");
	m_energyNow 	= getIntFromInfoFile("energy_now");
	m_currentNow 	= getIntFromInfoFile("current_now");
	m_powerNow   	= getIntFromInfoFile("power_now");
	/* FIXME: Some battery drivers report -1000 when the discharge rate is
	* unavailable. Others use negative values when discharging. Best we can do
	* is to treat -1 as an error, and take the absolute value otherwise.
	* Ideally the kernel would not export the sysfs file when the value is not
	* available. */
	if (m_currentNow < -1)
		m_currentNow = - m_currentNow;
	if (m_powerNow < -1)
		m_powerNow = - m_powerNow;
	m_chargeFull			= getIntFromInfoFile("charge_full");
	m_energyFull			= getIntFromInfoFile("energy_full");
	m_chargeFullDesign		= getIntFromInfoFile("charge_full_design");
	m_energyFullDesign		= getIntFromInfoFile("energy_full_design");
	m_voltageNow			= getIntFromInfoFile("voltage_now");
	gctmp					= getQStringFromInfoFile("type");
	if (gctmp.toLower() == "battery")
		m_typeBattery = true;
	else
		m_typeBattery = false;
	m_state = getQStringFromInfoFile("status");
	if (m_state.isEmpty())
		m_state = getQStringFromInfoFile("state");
	if (m_state.isEmpty()) {
		if (m_chargeNow != -1 || m_energyNow != -1
			|| m_chargeFull != -1 || m_energyFull != -1)
			m_state = "available";
		else
			m_state = "unavailable";
	}
	/* convert energy values (in mWh) to charge values (in mAh) if needed and possible */
	if (m_energyFull != -1 && m_chargeFull == -1) {
		if (m_voltageNow != -1 && m_voltageNow != 0) {
			m_chargeFull = m_energyFull * 1000 / m_voltageNow;
		}
		else {
			m_chargeFull = m_energyFull;
			m_capacityUnit = "mWh";
		}
	}
	if (m_energyFullDesign != -1 && m_chargeFullDesign == -1) {
		if (m_voltageNow != -1 && m_voltageNow != 0) {
			m_chargeFullDesign = m_energyFullDesign * 1000 / m_voltageNow;
		}
		else {
			m_chargeFullDesign = m_energyFullDesign;
			m_capacityUnit = "mWh";
		}
	}
	if (m_energyNow != -1 && m_chargeNow == -1) {
		if (m_voltageNow != -1 && m_voltageNow != 0) {
			m_chargeNow = m_energyNow * 1000 / m_voltageNow;
			if (m_currentNow != -1)
				m_currentNow = m_currentNow * 1000 / m_voltageNow;
		}
		else {
			m_chargeNow = m_energyNow;
		}
	}
	if (m_powerNow != -1 && m_currentNow == -1) {
		if (m_voltageNow != -1 && m_voltageNow != 0)
			m_currentNow = m_powerNow * 1000 / m_voltageNow;
		}
		if (m_chargeFull < MIN_CAPACITY)
			m_percentage = 0;
		else {
			int promille = (m_chargeNow * 1000) / m_chargeFull;
			m_percentage = (promille + 5) / 10; /* round properly */
		}
		if (m_percentage > 100)
			m_percentage = 100;
		if (m_currentNow == -1) {
			m_poststr = QObject::tr("rate information unavailable");
			m_seconds = -1;
		}
		else if (m_state == "charging") {
			if (m_currentNow > MIN_PRESENT_RATE) {
				m_seconds = 3600 * (m_chargeFull - m_chargeNow) / m_currentNow;
				m_poststr = QObject::tr(" until charged");
			}
			else {
				m_poststr = QObject::tr("charging at zero rate - will never fully charge.");
				m_seconds = -1;
			}
		}
		else if (m_state == "discharging") {
			if (m_currentNow > MIN_PRESENT_RATE) {
				m_seconds = 3600 * m_chargeNow / m_currentNow;
				m_poststr = QObject::tr(" remaining");
			}
			else {
				m_poststr = QObject::tr("discharging at zero rate - will never fully discharge.");
				m_seconds = -1;
			}
		}
	else {
		m_poststr = QString();
		m_seconds = -1;
	}
}

QString Battery::info()
{
	return QString(QObject::tr("%1 n. %2:\nState => %3\nPercentage => %4%"))
					.arg(BATTERY_DESC)
					.arg(m_batteryNum - 1)
					.arg(m_state)
					.arg(percentage());
}

bool Battery::isCharging()
{
	if (m_state.isEmpty())
		return true; // Same as "Unkown"
	return ((m_state.toLower() == "unknown")	||
			(m_state.toLower() == "full")    	||
			(m_state.toLower() == "charging"));
}
