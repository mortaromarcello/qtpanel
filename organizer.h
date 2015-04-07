#ifndef ORGANIZER_H
#define ORGANIZER_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSettings>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QItemDelegate>
#include <QFocusEvent>
#include <QLineEdit>
#include <phonon>
#include <QComboBox>
#include <QPushButton>
#include <QFontComboBox>
#include "calendarworkinghours.h"

class QToolButton;
class Organizer;

//---------------------------QBalloonTip-------------------------------
class QBalloonTip : public QWidget
{
public:
	static QWidget *showBalloon(const QString& title, const QString& msg, int timeout=0, bool showArrow = true);
	static void hideBalloon();

private:
	QBalloonTip(const QString& title, const QString& msg,QWidget* parent=0);
	~QBalloonTip();
	void balloon(int, bool);

protected:
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *);
	void mousePressEvent(QMouseEvent *e);
	void timerEvent(QTimerEvent *e);

private:
	QPixmap pixmap;
	int timerId;
};

//-----------------------CQTableWidget-------------------------------
class CQTableWidget:public QTableWidget
{
	Q_OBJECT

public:
	bool isBeingEdited() {(this->state() == QAbstractItemView::EditingState) ? true : false;}

signals:
	void focusLost();

protected:
	void focusOutEvent(QFocusEvent * event) {
		if ((event->reason() == Qt::MouseFocusReason) ||
			(event->reason() == Qt::TabFocusReason) ||
			(event->reason() == Qt::ActiveWindowFocusReason))
			emit focusLost();
	}
	void keyPressEvent (QKeyEvent * event);
};

//Custom QTableWidget for the same thing.

class CQTextEdit:public QTextEdit
{
	Q_OBJECT

signals:
	void focusLost();

protected:
	void focusOutEvent(QFocusEvent * event) {
		if ((event->reason() == Qt::MouseFocusReason) ||
			(event->reason() == Qt::TabFocusReason) ||
			(event->reason() == Qt::ActiveWindowFocusReason))
		emit focusLost();
	}
	/*void dragEnterEvent(QDragEnterEvent *event);
	//void dropEvent(QDropEvent *e);*/
	void insertFromMimeData(const QMimeData *source);
	bool canInsertFromMimeData(const QMimeData *source) const;
};

//LineEdit with clear button on the right.
class LineEdit : public QLineEdit
{
	Q_OBJECT

public:
	LineEdit(Organizer *parent = 0);

protected:
	void resizeEvent(QResizeEvent *);

private slots:
	void updateCloseButton(const QString &text);
	void clearAndSetDateBack();
private:
	QToolButton *clearButton;
	Organizer *Parent;
};

//Delegate for the schedule
class ScheduleDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	ScheduleDelegate(QObject *parent = 0);
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

// class organizer
class Organizer: public QMainWindow
{
	Q_OBJECT
public:
	Organizer();
	~Organizer();
	bool init();
	CalendarWorkingHours* calendar() {return m_calendar;}

public slots:
	void updateDay();
	void remind();
	void playSound();
	void saveSchedule();
	void loadSchedule();
	void saveJournal();
	void loadJournal();
	void updateCalendar();
	void modifyText();
	void setFont();
	void insertRowToEnd();
	void deleteRow();
	void searchPrev();
	void searchNext();
	void search();
	void saveCalendarColumnWidths();
	void setCalendarColumnWidths();
private:
	void readSettings();
	void saveSettings();
	void addItems();
	QWidget* getCalendarPage();
	void emptyTable();
	bool tableExistsDB(QString &tablename);
	QSqlDatabase m_db;
	QString m_nameDatabase;
	QSettings * m_settings;
	QStackedWidget* m_mainWid;
	CalendarWorkingHours* m_calendar;
	CQTableWidget* m_tableWid;
	CQTextEdit* m_journal;
	Qt::DayOfWeek m_firstDayOfWeek;
	int m_rowCount;
	// fonts
	QFont m_font;
	QFontComboBox* m_fontBox;
	QComboBox* m_comboSize;
	QPushButton* m_buttonBold;
	QPushButton* m_buttonItalic;
	QPushButton* m_buttonUnderline;
	// search
	QLineEdit* m_searchField;
	QString m_oldSearchString;
	int m_searchCurrentIndex;
	QList<QDate> m_dateList;
	//
	QString m_dateFormat;
	QString m_selDate;
	QLabel* m_noteLabel;
	QLabel* m_day;
	ScheduleDelegate m_scheduleDelegate;
	// timer
	QTimer* m_timer;
	unsigned long long int m_timeout;
	// sound
	bool m_sound;
	Phonon::MediaObject* m_mediaObject;
	Phonon::AudioOutput* m_audioOutput;
	Phonon::Path m_mediaPath;
};


#endif // ORGANIZER_H

