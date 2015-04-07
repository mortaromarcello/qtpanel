#ifndef BATTERY_H
#define BATTERY_H

#include <QString>

#define BUF_SIZE 1024
#define ACPI_PATH_SYS_POWER_SUPPY	"/sys/class/power_supply"
#define MIN_CAPACITY				0.01
#define MIN_PRESENT_RATE			0.01
#define BATTERY_DESC				"Battery"

class Battery
{
public:
	Battery();
	~Battery();
	bool isCharging();
	int seconds() { return m_seconds; }
	int percentage() { return m_percentage; }
	bool typeBattery() { return m_typeBattery; }
	void update();
	QString info();
protected:
	int getIntFromInfoFile(const QString & sys_file);
	QString getQStringFromInfoFile(const QString & sys_file);
	bool batteryInserted(const QString & path);
private:
	QString parseInfoFile(const QString & sys_file);
	int m_batteryNum;
	/* path to battery dir */
	QString m_path;
	/* sysfs file contents */
	int m_chargeNow;
	int m_energyNow;
	int m_currentNow;
	int m_powerNow;
	int m_voltageNow;
	int m_chargeFullDesign;
	int m_energyFullDesign;
	int m_chargeFull;
	int m_energyFull;
	/* extra info */
	int m_seconds;
	int m_percentage;
	QString m_state, m_poststr;
	QString m_capacityUnit;
	bool m_typeBattery;
};

#endif // BATTERY_H
