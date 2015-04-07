#include "calendarworkinghours.h"
#include "ui_workinghours.h"
#include "mydebug.h"
#include "config.h"
#include <QPainter>
#include <QDialog>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

//------------------------------WorkingHours----------------------------
WorkingHours::WorkingHours(const QDate& date, Schedule schedule)
	: m_date(date),
	m_schedule(schedule) {
	switch(schedule) {
		case Rest:
		case Holyday:
		case SickLeave:
			m_startShedule = QTime();
			m_endIntermedieSchedule = QTime();
			m_startIntermedieSchedule = QTime();
			m_endSchedule = QTime();
			break;
		case FirstTurn:
			m_startShedule = QTime(22, 0);
			m_endIntermedieSchedule = QTime();
			m_startIntermedieSchedule = QTime();
			m_endSchedule = QTime(6, 0);
			break;
		case SecondTurn:
			m_startShedule = QTime(6, 0);
			m_endIntermedieSchedule = QTime();
			m_startIntermedieSchedule = QTime();
			m_endSchedule = QTime(14, 0);
			break;
		case ThirdTurn:
			m_startShedule = QTime(14, 0);
			m_endIntermedieSchedule = QTime();
			m_startIntermedieSchedule = QTime();
			m_endSchedule = QTime(22, 0);
			break;
		case TurnBroken: break;
		case Office: break;
		default: break;
	}
}

//------------------------------CalendarWorkingHours--------------------
CalendarWorkingHours::CalendarWorkingHours()
	: dateSet(QVector<WorkingHours>()), m_currentWorkingHours(WorkingHours()) {
	m_settings = new QSettings(qtpanel_ORGANIZATION, "calendar");
	QString path = QFileInfo(m_settings->fileName()).path();
	m_xmlWriter.setFileName(path + QDir::separator() + "dateset.xml");
	m_xmlReader.setFileName(path + QDir::separator() + "dateset.xml");
	connect(this, SIGNAL(activated(QDate)), this, SLOT(activated(QDate)));
	initSchemes();
	xmlRead();
}

CalendarWorkingHours::~CalendarWorkingHours()
{
	xmlWrite();
}

void CalendarWorkingHours::addColoredDate(WorkingHours date)
{
	dateSet.append(date);
}

QDate CalendarWorkingHours::addWorkingHoursScheme(QDate date, SchemeWorkingHours scheme)
{
	int i = 0;
	while (i < scheme.size()) {
		addColoredDate(WorkingHours(date, scheme[i]));
		date = date.addDays(1);
		i++;
	}
	return date;
}

//
QDate CalendarWorkingHours::addWorkingHoursCycle(QDate date, SchemeWorkingHours scheme, int cycle)
{
	int i = 0; int j = 0;
	while(i < cycle) {
		if (j == scheme.size()) j = 0;
		date = addWorkingHoursScheme(date, scheme);
		j++; i++;
	}
}

void CalendarWorkingHours::activated(const QDate& date)
{
	QDialog dialog;
	Ui_WorkingHoursSettings* settingsUi;
	QMap<WorkingHours::Schedule, QString> schedules;
	QStringList values;
	WorkingHours wH;
	int pos = dateSet.indexOf(WorkingHours(date));
	if (pos != -1)
		wH = dateSet[pos];
	else return;
	schedules[WorkingHours::Rest] = tr("Rest");
	schedules[WorkingHours::Holyday] = tr("Holyday");
	schedules[WorkingHours::SickLeave] = tr("Sick Leave");
	schedules[WorkingHours::FirstTurn] = tr("First Turn");
	schedules[WorkingHours::SecondTurn] = tr("Second Turn");
	schedules[WorkingHours::ThirdTurn] = tr("Third Turn");
	schedules[WorkingHours::TurnBroken] = tr("Turn Broken");
	values = schedules.values();
	settingsUi = new Ui_WorkingHoursSettings();
	settingsUi->setupUi(&dialog);
	settingsUi->schedule->addItems(values);
	settingsUi->schedule->setCurrentIndex(values.indexOf(schedules[wH.schedule()]));
	settingsUi->time_start->setTime(wH.startSchedule());
	settingsUi->time_end_intermedie->setTime(wH.endIntermedieSchedule());
	settingsUi->time_start_intermedie->setTime(wH.startIntermedieSchedule());
	settingsUi->time_end->setTime(wH.endSchedule());
	if(dialog.exec() == QDialog::Accepted) {
		;
	}
}

void CalendarWorkingHours::xmlWrite()
{
	if (!m_xmlWriter.xmlOpen())
	{
#ifdef __DEBUG__
		MyDBG << "Error writing file.";
#else
		qDebug("Error writing file.");
#endif
		return;
	}
	m_xmlWriter.writeStartDocument();
	m_xmlWriter.writeStartElement("dateSets");
	for (int i = 0; i < dateSet.size(); i++) {
		m_xmlWriter.writeStartElement("WorkingHours");
		m_xmlWriter.writeTextElement("Date", dateSet[i].date().toString());
		m_xmlWriter.writeTextElement("Schedule", QString::number(dateSet[i].schedule()));
		m_xmlWriter.writeTextElement("startSchedule", dateSet[i].startSchedule().toString());
		m_xmlWriter.writeTextElement("endIntermedieSchedule", dateSet[i].endIntermedieSchedule().toString());
		m_xmlWriter.writeTextElement("startIntermedieSchedule", dateSet[i].startIntermedieSchedule().toString());
		m_xmlWriter.writeTextElement("endSchedule", dateSet[i].endSchedule().toString());
		m_xmlWriter.writeEndElement();
	}
	m_xmlWriter.writeEndElement();
	m_xmlWriter.writeEndDocument();
	m_xmlWriter.xmlClose();
}

bool CalendarWorkingHours::xmlRead()
{
	if (!m_xmlReader.xmlOpen()) {
#ifdef __DEBUG__
		MyDBG << "Error opening file.";
#else
		qDebug("Error opening file.");
#endif
		return false;
	}
	while (!m_xmlReader.atEnd()) {
		if (m_xmlReader.hasError()) {
			m_xmlReader.xmlErrorString();
			m_xmlReader.xmlClose();
			return false;
		}
		while (m_xmlReader.readNextStartElement()) {
			if (m_xmlReader.name() == "WorkingHours") {
				while (m_xmlReader.readNextStartElement()) {
					if (m_xmlReader.name() == "Date")
						xmlReadDate();
					else if (m_xmlReader.name() == "Schedule")
						xmlReadSchedule();
					else if (m_xmlReader.name() == "startSchedule")
						xmlReadStartSchedule();
					else if (m_xmlReader.name() == "endIntermedieSchedule")
						xmlReadEndIntermedieSchedule();
					else if (m_xmlReader.name() == "startIntermedieSchedule")
						xmlReadStartIntermedieSchedule();
					else if (m_xmlReader.name() == "endSchedule")
						xmlReadEndSchedule();
				}
				if (!dateSet.contains(m_currentWorkingHours))
					dateSet.append(m_currentWorkingHours);
			}
		}
	}
	m_xmlReader.xmlClose();
	return true;
}

void CalendarWorkingHours::xmlReadDate()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlConfigReader.name == "Date");
#ifdef __DEBUG__
	MyDBG << m_xmlReader.name().toString();
#endif
	QDate date = QDate::fromString(m_xmlReader.readElementText());
	m_currentWorkingHours.setDate(date);
}

void CalendarWorkingHours::xmlReadSchedule()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlConfigReader.name == "Schedule");
#ifdef __DEBUG__
	MyDBG << m_xmlReader.name().toString();
#endif
	WorkingHours::Schedule schedule = (WorkingHours::Schedule)m_xmlReader.readElementText().toInt();
	m_currentWorkingHours.setSchedule(schedule);
}

void CalendarWorkingHours::xmlReadStartSchedule()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlConfigReader.name == "startSchedule");
#ifdef __DEBUG__
	MyDBG << m_xmlReader.name().toString();
#endif
	QTime time = QTime::fromString(m_xmlReader.readElementText());
	m_currentWorkingHours.setStartSchedule(time);
}

void CalendarWorkingHours::xmlReadEndIntermedieSchedule()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlConfigReader.name == "endIntermedieSchedule");
#ifdef __DEBUG__
	MyDBG << m_xmlReader.name().toString();
#endif
	QTime time = QTime::fromString(m_xmlReader.readElementText());
	m_currentWorkingHours.setEndIntermedieSchedule(time);

}

void CalendarWorkingHours::xmlReadStartIntermedieSchedule()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlConfigReader.name == "startIntermedieSchedule");
#ifdef __DEBUG__
	MyDBG << m_xmlReader.name().toString();
#endif
	QTime time = QTime::fromString(m_xmlReader.readElementText());
	m_currentWorkingHours.setStartIntermedieSchedule(time);

}

void CalendarWorkingHours::xmlReadEndSchedule()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlConfigReader.name == "endSchedule");
#ifdef __DEBUG__
	MyDBG << m_xmlReader.name().toString();
#endif
	QTime time = QTime::fromString(m_xmlReader.readElementText());
	m_currentWorkingHours.setEndSchedule(time);

}

void CalendarWorkingHours::initSchemes()
{
	for (int i = 0; i < 4; i++)
		m_threeTurn42.append(WorkingHours::FirstTurn);
	for (int i = 0; i < 2; i++)
		m_threeTurn42.append(WorkingHours::Rest);
	for (int i = 0; i < 4; i++)
		m_threeTurn42.append(WorkingHours::SecondTurn);
	for (int i = 0; i < 2; i++)
		m_threeTurn42.append(WorkingHours::Rest);
	for (int i = 0; i < 4; i++)
		m_threeTurn42.append(WorkingHours::ThirdTurn);
	for (int i = 0; i < 2; i++)
		m_threeTurn42.append(WorkingHours::Rest);
	//-------------------------------------------
	for (int i = 0; i < 4; i++)
		m_twoTurn42.append(WorkingHours::SecondTurn);
	for (int i = 0; i < 2; i++)
		m_twoTurn42.append(WorkingHours::Rest);
	for (int i = 0; i < 4; i++)
		m_twoTurn42.append(WorkingHours::ThirdTurn);
	for (int i = 0; i < 2; i++)
		m_twoTurn42.append(WorkingHours::Rest);
}

//
void CalendarWorkingHours::addWorkingHoursInYear(QDate date, SchemeWorkingHours scheme)
{
	int count = date.daysTo(QDate(date.year(), 12, 31))/(scheme.size());
	addWorkingHoursCycle(date, scheme, count);
}
//

void CalendarWorkingHours :: removeColoredDate(WorkingHours date)
{
	int pos = dateSet.indexOf(WorkingHours(date));
	if (pos != -1) 
		dateSet.remove(pos);
}

void CalendarWorkingHours :: paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
	QTextStream cout(stdout, QIODevice::WriteOnly);
	int pos = dateSet.indexOf(WorkingHours(date));
	WorkingHours wH = dateSet.at(pos);
	if((pos != -1) && (date != selectedDate())) {
		QFont font;
		switch(wH.schedule()) {
			case WorkingHours:: Rest:
				font = painter->font();
				font.setBold(true);
				font.setItalic(true);
				font.setPointSize(8);
				painter->setFont(font);
				painter->fillRect(rect, Qt::red);
				painter->save();
				painter->setPen(Qt::white);
				painter->drawText(rect, Qt::AlignCenter, QString::number(date.day())+" (R)");
				painter->restore();
				break;
			case WorkingHours::SecondTurn:
				font = painter->font();
				font.setBold(true);
				font.setItalic(false);
				font.setPointSize(8);
				painter->setFont(font);
				painter->fillRect(rect, Qt::green);
				painter->save();
				painter->setPen(Qt::yellow);
				painter->drawText(rect, Qt::AlignCenter, QString::number(date.day())+" (2)");
				painter->restore();
				break;
			case WorkingHours::ThirdTurn:
				font = painter->font();
				font.setBold(true);
				font.setItalic(false);
				font.setPointSize(8);
				painter->setFont(font);
				painter->fillRect(rect, Qt::blue);
				painter->save();
				painter->setPen(Qt::yellow);
				painter->drawText(rect, Qt::AlignCenter, QString::number(date.day())+" (3)");
				painter->restore();
				break;
			default:
				break;
		}
	}
	else
		QCalendarWidget :: paintCell(painter,rect,date);
}
