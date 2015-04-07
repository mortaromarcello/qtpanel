#ifndef CALENDARWORKINGHOURS_H
#define CALENDARWORKINGHOURS_H
#include <QCalendarWidget>
#include <QFile>
#include <QDataStream>
#include <QSettings>
#include "xmlconfigwriter.h"
#include "xmlconfigreader.h"

class WorkingHours
{
public:
	enum Schedule {
		Rest,
		Holyday,
		SickLeave,
		FirstTurn,
		SecondTurn,
		ThirdTurn,
		TurnBroken,
		Office
	};
	WorkingHours()
		: m_date(QDate()),
		m_schedule(Rest),
		m_startShedule(QTime()),
		m_endIntermedieSchedule(QTime()),
		m_startIntermedieSchedule(QTime()),
		m_endSchedule(QTime()) {}          
	WorkingHours(const QDate& date, Schedule schedule = Rest);
	const QDate& date() const {return m_date;}
	const Schedule schedule() const {return m_schedule;}
	const QTime& startSchedule() const {return m_startShedule;}
	const QTime& endIntermedieSchedule() const {return m_endIntermedieSchedule;}
	const QTime& startIntermedieSchedule() const {return m_startIntermedieSchedule;}
	const QTime& endSchedule() const {return m_endSchedule;}
	void setDate(const QDate& date) {m_date = date;}
	void setSchedule(WorkingHours::Schedule schedule) {m_schedule = schedule;}
	void setStartSchedule(const QTime& time) {m_startShedule = time;}
	void setEndIntermedieSchedule(const QTime& time) {m_endIntermedieSchedule = time;}
	void setStartIntermedieSchedule(const QTime& time) {m_startIntermedieSchedule = time;}
	void setEndSchedule(const QTime& time) {m_endSchedule = time;}
	bool operator==(const WorkingHours& other) const {return other.date() == this->date();}

private:
	QDate m_date;
	Schedule m_schedule;
	QTime m_startShedule;
	QTime m_endIntermedieSchedule;
	QTime m_startIntermedieSchedule;
	QTime m_endSchedule;
};

typedef QList<WorkingHours::Schedule> SchemeWorkingHours;

//---------------------------------------------------------------------
inline uint qHash(const QDate &d) { return d.year()+d.month()+d.day(); }

//----------------CalendarWorkingHours-------------------------------------------
class CalendarWorkingHours:public QCalendarWidget
{
	Q_OBJECT
public:
	CalendarWorkingHours();
	~CalendarWorkingHours();
	void addColoredDate(WorkingHours date);
	void removeColoredDate(WorkingHours date);
	//
	QDate addWorkingHours(QDate date, int workDays, WorkingHours::Schedule schedule, int restDays);
	QDate addWorkingHoursScheme(QDate date, SchemeWorkingHours scheme);
	QDate addWorkingHoursCycle(QDate date, SchemeWorkingHours scheme, int cycle);
	void addWorkingHoursInYear(QDate date, SchemeWorkingHours scheme);
	//
	int getDateSetSize() {return dateSet.size();}
	const SchemeWorkingHours& twoTurn42() const {return m_twoTurn42;}

public slots:
	void activated(const QDate& date);

 protected:
	void xmlWrite();
	bool xmlRead();
	void xmlReadDate();
	void xmlReadSchedule();
	void xmlReadStartSchedule();
	void xmlReadEndIntermedieSchedule();
	void xmlReadStartIntermedieSchedule();
	void xmlReadEndSchedule();
	QVector<WorkingHours> dateSet;

private:
	void initSchemes();
	WorkingHours m_currentWorkingHours;
	virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
	SchemeWorkingHours m_threeTurn42, m_twoTurn42;
	QSettings* m_settings;
	XmlConfigWriter m_xmlWriter;
	XmlConfigReader m_xmlReader;
};

#endif // CALENDARWORKINGHOURS_H
