#include "organizer.h"
#include "config.h"
#include <QMessageBox>
#include <QDir>
#include <QHeaderView>
#include <QFontComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimeEdit>
#include <QLineEdit>
#include <QToolButton>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>

//----------------------------QBalloonTip------------------------------
static QBalloonTip *theSolitaryBalloonTip = 0;

QWidget *QBalloonTip::showBalloon(const QString& title, const QString& message, int timeout, bool showArrow)
{
	hideBalloon();
	theSolitaryBalloonTip = new QBalloonTip(title, message);
	if (timeout < 0)
		timeout = 10000;
	theSolitaryBalloonTip->balloon(timeout, showArrow);
	return theSolitaryBalloonTip;
}

void QBalloonTip::hideBalloon()
{
	if (!theSolitaryBalloonTip)
		return;
	theSolitaryBalloonTip->hide();
	delete theSolitaryBalloonTip;
	theSolitaryBalloonTip = 0;
}

QBalloonTip::QBalloonTip(const QString& title, const QString& message,QWidget* parent)
	: QWidget(parent, Qt::ToolTip), timerId(-1)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setObjectName( "QBalloonTip");
	QFile File(":/icons/BallonStyle.qss");
	File.open(QFile::ReadOnly);
	QString StyleSheet = QString::fromUtf8(File.readAll().data());
	File.close();
	setStyleSheet(StyleSheet);
	QLabel *titleLabel = new QLabel;
	titleLabel->installEventFilter(this);
	titleLabel->setText(title);
	QFont f = titleLabel->font();
	f.setBold(true);
	titleLabel->setFont(f);
	titleLabel->setTextFormat(Qt::PlainText);
	QPushButton *closeButton = new QPushButton;
	closeButton->setIcon(style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
	closeButton->setIconSize(QSize(18, 18));
	closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	closeButton->setFixedSize(18, 18);
	QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	QLabel *msgLabel = new QLabel;
	msgLabel->installEventFilter(this);
	msgLabel->setText(message);
	msgLabel->setTextFormat(Qt::PlainText);
	msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	int limit = QApplication::desktop()->availableGeometry(msgLabel).size().width() / 3;
	if (msgLabel->sizeHint().width() > limit) {
		msgLabel->setWordWrap(true);
		msgLabel->setFixedSize(limit, msgLabel->heightForWidth(limit));
	}
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(titleLabel, 0, 0, 1, 2);
	layout->addWidget(closeButton, 0, 2);
	layout->addWidget(msgLabel, 1, 0, 1, 3);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	layout->setMargin(3);
	setLayout(layout);
	QPalette pal = palette();
	pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xe1));
	setPalette(pal);
}

QBalloonTip::~QBalloonTip()
{
	theSolitaryBalloonTip = 0;
}

void QBalloonTip::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.drawPixmap(rect(), pixmap);
}

void QBalloonTip::resizeEvent(QResizeEvent *ev)
{
	QWidget::resizeEvent(ev);
}

void QBalloonTip::balloon(int msecs, bool showArrow)
{
	QSize sh = sizeHint();
	QRect desktopRect = QApplication::desktop()->availableGeometry();
	QPoint bottomRight = desktopRect.bottomRight();
	const QPoint& pos = mapToGlobal(QPoint(bottomRight.x(),bottomRight.y()/*-sh.height()*/));
	const QPoint& localpos = QPoint(pos);
	const QRect& prect = desktopRect;
	QRect scr = prect;
	const int border = 1;
	const int ah = 18, ao = 18, aw = 18, rc = 7;
	bool arrowAtTop = (localpos.y() - sh.height() - ah < 0);//scr.height());
	bool arrowAtLeft = (localpos.x() + sh.width() - ao < scr.width());
	setContentsMargins(border + 3, border + (arrowAtTop ? ah : 0) + 2, border + 3, border + (arrowAtTop ? 0 : ah) + 2);
	updateGeometry();
	sh = sizeHint();
	int ml, mr, mt, mb;
	QSize sz = sizeHint();
	if (!arrowAtTop) {
		ml = mt = 0;
		mr = sz.width() - 1;
		mb = sz.height() - ah - 1;
	} else {
		ml = 0;
		mt = ah;
		mr = sz.width() - 1;
		mb = sz.height() - 1;
	}
	QPainterPath path;
	path.moveTo(ml + rc, mt);
	if (arrowAtTop && arrowAtLeft) {
		if (showArrow) {
			path.lineTo(ml + ao, mt);
			path.lineTo(ml + ao, mt - ah);
			path.lineTo(ml + ao + aw, mt);
		}
		move(qMax(pos.x() - ao, scr.left() + 2), pos.y());
	} else if (arrowAtTop && !arrowAtLeft) {
		if (showArrow) {
			path.lineTo(mr - ao - aw, mt);
			path.lineTo(mr - ao, mt - ah);
			path.lineTo(mr - ao, mt);
		}
		move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2), pos.y());
	}
	path.lineTo(mr - rc, mt);
	path.arcTo(QRect(mr - rc*2, mt, rc*2, rc*2), 90, -90);
	path.lineTo(mr, mb - rc);
	path.arcTo(QRect(mr - rc*2, mb - rc*2, rc*2, rc*2), 0, -90);
	if (!arrowAtTop && !arrowAtLeft) {
		if (showArrow) {
			path.lineTo(mr - ao, mb);
			path.lineTo(mr - ao, mb + ah);
			path.lineTo(mr - ao - aw, mb);
		}
		move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2),
			pos.y() - sh.height());
	} else if (!arrowAtTop && arrowAtLeft) {
		if (showArrow) {
			path.lineTo(ao + aw, mb);
			path.lineTo(ao, mb + ah);
			path.lineTo(ao, mb);
		}
		move(qMax(pos.x() - ao, scr.x() + 2), pos.y() - sh.height());
	}
	path.lineTo(ml + rc, mb);
	path.arcTo(QRect(ml, mb - rc*2, rc*2, rc*2), -90, -90);
	path.lineTo(ml, mt + rc);
	path.arcTo(QRect(ml, mt, rc*2, rc*2), 180, -90);
	if (msecs > 0)
		timerId = startTimer(msecs);
	show();
}

void QBalloonTip::mousePressEvent(QMouseEvent *e)
{
	close();
}

void QBalloonTip::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == timerId) {
		killTimer(timerId);
		if (!underMouse())
			close();
		return;
	}
	QWidget::timerEvent(e);
}

//---------------------------------------------------------------------
//Needed by custom delegate to be able to delete from the absenceTable, all because there is no 
//way to create an empty QDateTimeEdit
//We previously called setObjectName for these tables
void CQTableWidget::keyPressEvent (QKeyEvent * event)
{
	if(this->objectName() == QString("tableWid")) //Schedule table
		if(this->currentColumn()!= 0)
			if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
				this->currentItem()->setText(" ");
				return;
			}
	QTableWidget::keyPressEvent(event);
}

void CQTextEdit::insertFromMimeData( const QMimeData *source )
{
	if (source->hasFormat("text/uri-list"))
	{
		QTextCursor cursor = this->textCursor();
		QTextDocument *document = this->document();
		QString origin = source->urls()[0].toString();
		origin.remove("file://");
		QImage *image = new QImage(origin); 
		if(!image->isNull())
		{
			QVariant variant;
			variant = *image;
			document->addResource(QTextDocument::ImageResource,origin,variant);
			 cursor.insertImage(origin);
		}
		delete image;
	}
	else
		QTextEdit::insertFromMimeData(source);
}

bool CQTextEdit::canInsertFromMimeData( const QMimeData *source ) const
{
	if (source->hasFormat("text/uri-list"))
		return true;
	else
		return QTextEdit::canInsertFromMimeData(source);
}

//-----------------------------------------LINEEDIT WITH CLEAR BUTTON----------------------------------------
LineEdit::LineEdit(Organizer *parent): QLineEdit(parent)
{
	clearButton = new QToolButton(this);
	Parent = parent;
	QPixmap pixmap = QIcon(QString(qtpanel_IMAGES_TARGET) + "/clear.svg").pixmap(24);
	clearButton->setIcon(QIcon(pixmap));
	//clearButton->setIconSize(pixmap.size());
	clearButton->setCursor(Qt::ArrowCursor);
	clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	clearButton->hide();
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clearAndSetDateBack()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(clearButton->sizeHint().width() + frameWidth + 1));
	QSize msz = minimumSizeHint();
	setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2), qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

void LineEdit::resizeEvent(QResizeEvent *)
{
	QSize sz = clearButton->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	clearButton->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height())/2);
}

void LineEdit::updateCloseButton(const QString& text)
{
	clearButton->setVisible(!text.isEmpty());
}

void LineEdit::clearAndSetDateBack()
{
	this->clear();
	Parent->calendar()->setSelectedDate(QDate::currentDate());
	Parent->updateDay(); 
	Parent->loadJournal();
	Parent->loadSchedule();
	Parent->setCalendarColumnWidths();
}

//--------------Schedule Delegate --------------
ScheduleDelegate::ScheduleDelegate(QObject *parent) : QItemDelegate(parent)
{}

QWidget* ScheduleDelegate::createEditor(QWidget *parent,const QStyleOptionViewItem &/* option*/,const QModelIndex &index) const
{
	if(index.column() != 0)
	{
		QTimeEdit *editor = new QTimeEdit(QTime::currentTime().addSecs(60),parent);
		editor->setDisplayFormat("hh:mm");
		return editor;
	}
	else 
	{
		QLineEdit *editor = new QLineEdit(parent);
		return editor;
	}
}

void ScheduleDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const
{
	if(index.column() != 0)
	{ 
		QTimeEdit *timeEditor = qobject_cast< QTimeEdit *>(editor);
		if (timeEditor) 
			model->setData(index, timeEditor->time().toString("hh:mm"));
	}
	else 
	{
		QLineEdit *eventEdit = static_cast<QLineEdit*>(editor);
		if (eventEdit) 
			model->setData(index, eventEdit->text());
	}
}

void ScheduleDelegate::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

//--------------Organizer---------------------------------------------
Organizer::Organizer()
{
	m_settings = new QSettings(qtpanel_ORGANIZATION, "organizer");
	readSettings();
	m_db = QSqlDatabase::addDatabase("QSQLITE");
	m_db.setDatabaseName(m_nameDatabase);
	if (!m_db.open()) {
		QMessageBox::critical(0,tr("Error"), tr("Could not connect to database!"), QMessageBox::Ok); 
	}
#ifdef __DEBUG__
	MyDBG << "connect to DB" << m_db.databaseName();
#endif
	addItems();
	m_timer = new QTimer();
	m_timer->start(m_timeout);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(remind()));
	init();
}

Organizer::~Organizer()
{
	m_db.close();
	saveSettings();
	delete m_settings;
	delete m_timer;
}

bool Organizer::init()
{
	m_mediaObject = new Phonon::MediaObject();
	m_mediaObject->setCurrentSource(Phonon::MediaSource(QString(qtpanel_SOUNDS_TARGET) + "/sound.wav"));
	m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	m_mediaPath = Phonon::createPath(m_mediaObject, m_audioOutput);
	return true;
}

// slots
void Organizer::updateDay()
{
	QDate date = m_calendar->selectedDate();
	m_selDate = date.toString(m_dateFormat);
	//
	//m_calendar->addColoredDate(date.year(),date.month(), date.day());
	//
	QString Date ="<center><font size=\"4\">" + tr("Schedule for ") + m_selDate + ":</font></center>";
	m_day->setText(Date);
	QString note ="<center><font size=\"4\">" + tr("Important notes, journal for ") + m_selDate +":</font></center>";
	m_noteLabel->setText(note);
}

void Organizer::remind()
{
	//QTextStream cout(stdout, QIODevice::WriteOnly);
	//Parse the table for reminders
	for(int i = 0; i < m_tableWid->rowCount(); i++)
	{
		QTableWidgetItem* item = m_tableWid->item(i, 3);
		//cout << (long long)item << "\n";
		if(item != 0)
		{
			QString timeString = item->text();
			item = m_tableWid->item(i, 0);
			QString eventString;
			if(item != 0)
				eventString = item->text();
			else
				eventString = "";
			//This is because there are to excepted formats for the timeString
			// Example: 12:00 is equal to 12
			if(timeString.indexOf(":") != -1)
			{
				//Cut the string and make it into a QTime object
				if(timeString.size() == 4)
					timeString = "0" + timeString;
				QString hourString = timeString;
				hourString.remove(hourString.indexOf(":"), 3); 
				QString minuteString = timeString;
				minuteString.remove(0, 3);
				bool ok;
				bool ok2;
				int hour = hourString.toInt(&ok, 10);
				int minute = minuteString.toInt(&ok2, 10);
				if ((ok && ok2) && (hour < 24) && (hour >= 0) && (minute >= 0) && (minute <= 60))
				{
					QTime time = QTime(hour,minute, 0, 0);
					QDate date = m_calendar->selectedDate();
					if((time < QTime::currentTime()) && (date == QDate::currentDate()))
					{
						if (m_sound)
							playSound(); 
						item = m_tableWid->item(i, 3);
						item->setText("");
						saveSchedule();
						/*Pops up a baloon in the system tray notifying the user about the event
						This stays active for 2 hours (7200 seconds), I guess that's enough for the user to
						notice it.*/
						QBalloonTip::showBalloon("qOrganizer", eventString, 7200000);
						item = m_tableWid->item(i, 3);
						item->setText("");
						saveSchedule();
					}
				} 
				else
				{
					QMessageBox::warning(NULL, "qOrganizer",tr("Couldn't set reminder!\nThere are two excepted formats for reminders.\nOne is hh:mm.\nExample:17:58.\nOr the other one is just writing the hour.\nExample: 3 is equal to 3:00 or to 03:00" ) , "OK");
					item = m_tableWid->item(i, 3);
					item->setText("");
					saveSchedule();
				}
			}
			else
			{
				if((timeString.size() == 2) || (timeString.size() == 1))
				{
					bool ok;
					int hour = timeString.toInt(&ok,10);
					if((ok) && (hour < 24) && (hour >= 0)) 
					{
						QTime time = QTime(hour, 0, 0, 0);
						QDate date = m_calendar->selectedDate();
						if ((time < QTime::currentTime()) && (date == QDate::currentDate()))
						{
							if(m_sound)
								playSound();
							item = m_tableWid->item(i, 3);
							item->setText("");
							saveSchedule();
							QMessageBox::about(this, "qOrganizer", eventString); 
							//This will look pretty and not block the sound since it's not modal.                 
						}
					} 
					else 
					{
						if((timeString.size() != 0) && (timeString != " ") && (timeString != "  ")&&(timeString != "   ")) 
						{
							QMessageBox::warning(NULL, "qOrganizer",tr("Couldn't set reminder!\nThere are two excepted formats for reminders.\nOne is hh:mm.\nExample:17:58.\nOr the other one is just writing the hour.\nExample: 3 is equal to 3:00 or to 03:00" ) , "OK");
							item = m_tableWid->item(i, 3);
							item->setText("");
							saveSchedule();
						}
					}
				}
				else 
				{
					if((timeString.size() != 0) && (timeString != " ") && (timeString != "  ") && (timeString != "   ")) 
					{
						QMessageBox::warning(NULL, "qOrganizer",tr("Couldn't set reminder!\nThere are two excepted formats for reminders.\nOne is hh:mm.\nExample:17:58.\nOr the other one is just writing the hour.\nExample: 3 is equal to 3:00 or to 03:00" ) , "OK");
						item = m_tableWid->item(i, 3);
						item->setText("");        
						saveSchedule();
					}
				}
			}
		}
	}
}
//

void Organizer::playSound()
{
	m_mediaObject->play();
}

void Organizer::saveSchedule()
{
	if(!m_db.isOpen())
		return;
	QTextStream cout(stdout, QIODevice::WriteOnly);
	//Get selected date
	QDate date = m_calendar->selectedDate();
	QString tableName = "schedule_" + date.toString("MM_dd_yyyy");
	cout << "Saving schedule to database:" << tableName << "\n";
	//Variables
	QSqlQuery query(m_db); 
	//Create database if it doesn't exist
	query.exec("CREATE TABLE `" + tableName + "` (`_event_` TEXT,`_from_` TEXT,`_until_` TEXT,`_reminder_` TEXT)");
	query.exec("DELETE FROM `" + tableName + "`");
	if(query.lastError().type() != QSqlError::NoError) 
	{
		cout << " A database error occured when saving schedule:\n";
		cout << " " << query.lastError().text() << "\n";
		cout << " This could be caused by running several instances of qOrganizer\n";
		cout << " Saving aborted\n";
	}
	else
	{
		int x = m_tableWid->currentRow();
		int y = m_tableWid->currentColumn();
		for(int i = 0; i < m_tableWid->rowCount(); i++)
		{
			QStringList textList;
			//textList.clear();
			for(int j = 0; j < m_tableWid->columnCount(); j++)
			{
				m_tableWid->setCurrentCell(i, j); 
				QTableWidgetItem *item = m_tableWid->currentItem();
				if(item != 0)
					textList.append(item->text());
				else textList.append("");
			}
			QString command = "INSERT INTO `" + tableName + "` VALUES"+"(:myevent,:myfrom,:myuntil,:myreminder)";
			query.prepare(command);
			query.bindValue(":myevent", textList[0]);
			query.bindValue(":myfrom", textList[1]);
			query.bindValue(":myuntil", textList[2]);
			query.bindValue(":myreminder", textList[3]);
			query.exec();
		}
		m_tableWid->setCurrentCell(x, y); 
	}
}

void Organizer::loadSchedule()
{
	bool didntExist = 0;
	if(!m_db.isOpen())
		return;
	emptyTable();
	QTextStream cout(stdout, QIODevice::WriteOnly);
	QDate date = m_calendar->selectedDate();
	QString tableName = "schedule_" + date.toString("MM_dd_yyyy");
	cout << "Loading schedule from database:" << tableName << "\n";
	QSqlQuery query(m_db); 
	if(!tableExistsDB(tableName))
		didntExist = 1;
	//Create table if it does not exist.
	query.exec("CREATE TABLE `" + tableName + "` (`_event_` TEXT,`_from_` TEXT,`_until_` TEXT,`_reminder_` TEXT)");
	//Select all from the table
	query.exec("SELECT _event_,_from_,_until_,_reminder_ FROM `" + tableName + "`");
	//Row counter, it will get incremented
	int i = -1;
	while(query.next())
	{
		i++;
		if(i >= m_tableWid->rowCount())
			insertRowToEnd();;
		for(int j = 0; j < m_tableWid->columnCount(); j++)
		{
			QString text = query.value(j).toString();
			QTableWidgetItem* item = new QTableWidgetItem(QTableWidgetItem::Type);
			item->setText(text);
			//if(tableWid->item(i,j)!=0) delete tableWid->takeItem(i,j);
			m_tableWid->setItem(i, j, item); 
		}
	}
	m_tableWid->setRowCount(i + 1);
	//If the table didn't exist then set the preferred nr of rows
	if(didntExist) 
	{
		emptyTable();
		saveSchedule();
	}
}


void Organizer::saveJournal()
{
	QString text;
	if(!m_db.isOpen())
		return;
	QTextStream cout(stdout, QIODevice::WriteOnly);
	QDate date = m_calendar->selectedDate();
	QString tableName ="jurnal_" + date.toString("MM_dd_yyyy");
	cout << "Saving jurnal to database:" << tableName << "\n";
	QSqlQuery query(m_db); 
	//Create table if it does not exist.
	query.exec("CREATE TABLE `" + tableName + "` (`_text_` TEXT)");
	//Delete the table contents
	query.exec("DELETE FROM `" + tableName + "`");
	text = m_journal->toHtml();
	query.prepare("INSERT INTO `" + tableName + "` VALUES" + " (:mytext)");
	query.bindValue(":mytext", text);
	query.exec();
}

void Organizer::loadJournal()
{
	if(!m_db.isOpen())
		return;
	QTextStream cout(stdout, QIODevice::WriteOnly);
	QDate date = m_calendar->selectedDate();
	QString tableName = "jurnal_" + date.toString("MM_dd_yyyy");
	cout << "Loading jurnal from database:" << tableName << "\n";
	QSqlQuery query(m_db); 
	//Create table if it does not exist.
	query.exec("CREATE TABLE `" + tableName + "` (`_text_` TEXT)");
	//Select all from the table
	query.exec("SELECT _text_ FROM `" + tableName + "`");
	QString text;
	while(query.next())
		text = query.value(0).toString();
	m_journal->setHtml(text);
}

void Organizer::updateCalendar()
{
	saveCalendarColumnWidths();
	loadSchedule();
	loadJournal();
	setCalendarColumnWidths();
}

void Organizer::modifyText()
{
	/*When a new character apears select it and change it's font to the currently selected font*/
	QTextCursor cursor = m_journal->textCursor();
	if(cursor.selectedText().isEmpty())
	{
		cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
		m_journal->setCurrentFont(m_font);
		cursor.movePosition(QTextCursor::Right);
	}
}

void Organizer::setFont() //Set the currently selected font
{
	m_font = m_fontBox->currentFont();
	m_font.setPointSize((m_comboSize->currentText()).toInt());
	//m_fontBox->adjustSize();
	if(m_buttonBold->isChecked())
		m_font.setBold(true);
	if(m_buttonItalic->isChecked()) 
		m_font.setItalic(true);
	if(m_buttonUnderline->isChecked()) 
		m_font.setUnderline(true);
	m_journal->setCurrentFont(m_font);
}

void Organizer::insertRowToEnd()
{
	m_tableWid->setRowCount(m_tableWid->rowCount() + 1);
}

//deletes a row from the schedule table
void Organizer::deleteRow()
{
	m_tableWid->removeRow(m_tableWid->currentRow());
}

void Organizer::searchPrev()
{
	QString searchedString = m_searchField->text();
	if(searchedString != m_oldSearchString)
	{
		m_searchCurrentIndex = 0;
		search();
	}
	if (m_searchCurrentIndex < 0)
		m_searchCurrentIndex = 0;
	if (m_searchCurrentIndex == 0)
		m_searchCurrentIndex = m_dateList.count();
	if ((m_dateList.isEmpty()) && (!searchedString.isEmpty())) 
	{
		QMessageBox::warning(NULL, "Organizer", tr("No entries found!"), "OK");
	}
	else if ((!searchedString.isEmpty()) && (searchedString != "|"))
	{
		if (m_searchCurrentIndex > 0) 
		{
			m_searchCurrentIndex--;
			m_calendar->setSelectedDate(m_dateList[m_searchCurrentIndex]);
			updateDay(); 
			loadJournal();
			loadSchedule();
			setCalendarColumnWidths();
		}
	}
	m_oldSearchString = searchedString;
}


void Organizer::searchNext()
{
	QString searchedString = m_searchField->text();
	if(searchedString != m_oldSearchString)
	{
		m_searchCurrentIndex = -1;
		search();
	}
	if (m_searchCurrentIndex >= m_dateList.count() - 1)
		m_searchCurrentIndex = -1;
	if((m_dateList.isEmpty()) && (!searchedString.isEmpty())) 
	{
		QMessageBox::warning(NULL, "Organizer", tr("No entries found!"), "OK");
	}
	else
	{
		if ((!searchedString.isEmpty()) && (searchedString != "|"))
		{
			if(m_searchCurrentIndex < m_dateList.count() - 1)
			{
				m_searchCurrentIndex++;
				m_calendar->setSelectedDate(m_dateList[m_searchCurrentIndex]);
				updateDay(); 
				loadJournal();
				loadSchedule();
				setCalendarColumnWidths();
			}
		}
	}
	m_oldSearchString = searchedString;
}

//This is responsible for searching through events
bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
	return s1.toLower() < s2.toLower();
}

void Organizer::search()
{
	QTextStream cout(stdout, QIODevice::WriteOnly);
	QString searchString = m_searchField->text();
	QList<QString> searchResultList;
	QString tempText;
	m_dateList.clear();
	if(!m_db.isOpen())
		return;
	QSqlQuery query(m_db); 
	QStringList tableList = m_db.tables(QSql::Tables);
	QStringList scheduleList = tableList.filter("schedule");
	QStringList jurnalList = tableList.filter("jurnal");
	QStringList entryList = scheduleList + jurnalList;
	qSort(entryList.begin(), entryList.end(), caseInsensitiveLessThan);
	foreach(QString entry,entryList)
	{
		if(entry.indexOf("schedule")!=-1)
		{
			tempText="";
			query.exec("SELECT _event_,_from_,_until_,_reminder_ FROM `"+entry+"`");
			while(query.next())
			{
				tempText+=query.value(0).toString()+"|"+query.value(1).toString()+"|"+query.value(2).toString()+"|"+query.value(3).toString()+"|"+"\n";
			}
			if(tempText.indexOf(searchString,0,Qt::CaseInsensitive)!=-1)
			{ 
				cout <<"Found:"<< entry <<"\n";
				searchResultList.append(entry);
			}
		}
		else //jurnal
		{
			query.exec("SELECT _text_ FROM `"+entry+"`");
			while(query.next())
			{
				QString text = query.value(0).toString();
				if(text.indexOf(searchString,0,Qt::CaseInsensitive)!=-1)
				{
					cout << "Found:" << entry<<"\n";
					searchResultList.append(entry);
				}
			}
		}
	}
	foreach(QString entry,searchResultList)
	{
		entry.remove(QString(".txt"),Qt::CaseSensitive);
		entry.remove(QString(".html"),Qt::CaseSensitive);
		entry.remove(QString("schedule_"),Qt::CaseSensitive);
		entry.remove(QString("jurnal_"),Qt::CaseSensitive);    
		QDate date = QDate::fromString(entry,"MM_dd_yyyy");
		m_dateList.append(date);
	}
}

void Organizer::saveCalendarColumnWidths()
{
	QTextStream cout(stdout, QIODevice::WriteOnly);
	cout << "Saving calendar column widths\n";
	//Schedule table
	for(int i = 0; i < m_tableWid->columnCount(); i++)
		m_settings->setValue("schedule" + QString::number(i), m_tableWid->columnWidth(i));
}

void Organizer::setCalendarColumnWidths()
{
	QTextStream cout(stdout, QIODevice::WriteOnly);
	cout << "Setting calendar column widths\n";
	//Schedule table
	for(int i = 0; i < m_tableWid->columnCount(); i++)
		if (m_settings->value("schedule" + QString::number(i)).toInt() != 0)
			m_tableWid->setColumnWidth(i, m_settings->value("schedule" + QString::number(i)).toInt());
}

void Organizer::readSettings()
{
	QString settingsPath = QFileInfo(m_settings->fileName()).path();
	if (!QDir(settingsPath).exists())
		QDir().mkdir(settingsPath);
	m_nameDatabase 		= m_settings->value("namedatabase", settingsPath + QDir::separator() + "organizer.db").toString();
	m_firstDayOfWeek 	= Qt::DayOfWeek(m_settings->value("firstdayofweek", Qt::Monday).toInt());
	m_dateFormat 		= m_settings->value("dateformat", "ddd. d MMMM yyyy").toString();
	m_timeout 			= m_settings->value("timeout", 20000).toInt();
	m_sound				= m_settings->value("sound", 1).toInt();
	m_rowCount			= m_settings->value("rowcount", 4).toInt();
}

void Organizer::saveSettings()
{
	m_settings->setValue("namedatabase", m_nameDatabase);
	m_settings->setValue("firstdayofweek", m_firstDayOfWeek);
	m_settings->setValue("dateformat", m_dateFormat);
	m_settings->setValue("timeout", m_timeout);
	m_settings->setValue("sound", (m_sound)?1:0);
}


void Organizer::addItems()
{
	m_mainWid = new QStackedWidget();
	setCentralWidget(m_mainWid);
	m_mainWid->addWidget(getCalendarPage());
	updateCalendar();
	m_mainWid->setCurrentIndex(0);
}

QWidget* Organizer::getCalendarPage()
{
	m_calendar = new CalendarWorkingHours();
	//
	if (!m_calendar->getDateSetSize())
		m_calendar->addWorkingHoursInYear(QDate(QDate::currentDate().year(), 1, 7), m_calendar->twoTurn42());
	//
	m_calendar->setGridVisible(true);
	m_calendar->setFirstDayOfWeek(Qt::DayOfWeek(m_firstDayOfWeek)); 
	m_calendar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
	QDate date = m_calendar->selectedDate();
	m_selDate = date.toString(m_dateFormat);
	QString Date = "<center><font size=\"4\">"+ tr("Schedule for ") + m_selDate + ":</font></center>";
	m_day = new QLabel(Date);
	connect(m_calendar, SIGNAL(selectionChanged()), this, SLOT(updateDay()));
	m_tableWid = new CQTableWidget();
	m_tableWid->setRowCount(m_rowCount);
	m_tableWid->setParent(this);
	m_tableWid->verticalHeader()->hide();
	m_tableWid->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	QStringList Clabels = QStringList() << tr("Event") << tr("From")<< tr("Until") << tr("Reminder");
	m_tableWid->setColumnCount(Clabels.size());
	m_tableWid ->setHorizontalHeaderLabels(Clabels); 
	//m_tableWid->horizontalHeader()->setStretchLastSection(true);
	m_tableWid->horizontalHeader()->setMovable(true);
	m_tableWid->setItemDelegate(&m_scheduleDelegate);
	m_tableWid->setObjectName("tableWid");
	QPushButton* newButton = new QPushButton(QIcon(QString(qtpanel_IMAGES_TARGET) + "/new-event.svg"), tr("New event"));
	newButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	connect(newButton, SIGNAL(clicked()), this, SLOT(insertRowToEnd()));
	QPushButton* deleteButton = new QPushButton(QIcon(QString(qtpanel_IMAGES_TARGET) + "/delete-event.svg"), tr("Delete event"));
	deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteRow()));
	m_journal = new CQTextEdit();
	m_journal->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	m_journal->setAcceptDrops(true);
	m_fontBox = new QFontComboBox();
	m_fontBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	m_fontBox->adjustSize();
	m_comboSize = new QComboBox();
	m_comboSize->setObjectName("comboSize");
	m_comboSize->setEditable(true);
	//Populate the font combo box
	QFontDatabase db;
	foreach(int size, db.standardSizes())
		m_comboSize->addItem(QString::number(size)); 
	//Set it to 12
	m_comboSize->setCurrentIndex(6);
	m_buttonBold = new QPushButton(QIcon(QString(qtpanel_IMAGES_TARGET) + "/format-text-bold.svg"),"");
	m_buttonItalic = new QPushButton(QIcon(QString(qtpanel_IMAGES_TARGET) + "/format-text-italic.svg"), "");
	m_buttonUnderline = new QPushButton(QIcon(QString(qtpanel_IMAGES_TARGET) + "/format-text-underline.svg"), "");
	m_buttonBold->setCheckable(true);
	m_buttonBold->setFlat(true);
	m_buttonUnderline->setCheckable(true);
	m_buttonUnderline->setFlat(true);
	m_buttonItalic->setCheckable(true);
	m_buttonItalic->setFlat(true);
	m_buttonBold->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	m_buttonItalic->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	m_buttonUnderline->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	//Set the selected Font
	setFont();  
	QString note = "<center><font size=\"4\">" + tr("Important notes, journal for ") + m_selDate + ":</font></center>";
	m_noteLabel = new QLabel(note); 
	QLabel* searchLabel = new QLabel(tr("Search:"));
	m_searchField = new LineEdit(this);
	QPushButton* searchPrevButton = new QPushButton(QIcon(QString(qtpanel_IMAGES_TARGET) + "/previous.svg"), tr("Previous"));
	searchPrevButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QPushButton* searchNextButton = new QPushButton(QIcon(QString(qtpanel_IMAGES_TARGET) + "/next.svg"), tr("Next"));
	searchNextButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QHBoxLayout* searchLayout = new QHBoxLayout;
	searchLayout->addWidget(searchLabel);
	searchLayout->addWidget(m_searchField);
	searchLayout->addWidget(searchPrevButton);
	searchLayout->addWidget(searchNextButton);
	m_searchCurrentIndex = -1;
	//-----------------------------connects-----------------------------
	//-------for error checking call remind if cell changed
	// carica il db prima di connettere remind
	loadSchedule();
	connect(m_tableWid,SIGNAL(cellChanged(int,int)), this, SLOT(remind()));
	//------Make sure that the new Font is set if the boxes change
	connect(m_fontBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(setFont()));
	connect(m_comboSize, SIGNAL(currentIndexChanged(int)), this, SLOT(setFont()));
	connect(m_buttonBold, SIGNAL(clicked()), this, SLOT(setFont()));
	connect(m_buttonUnderline, SIGNAL(clicked()), this, SLOT(setFont()));
	connect(m_buttonUnderline, SIGNAL(clicked()), this, SLOT(setFont())); 
	//------Change the font for every new character
	connect(m_journal, SIGNAL(textChanged()), this, SLOT(modifyText())); 
	//Update if calendar changed 
	connect(m_calendar, SIGNAL(selectionChanged()), this, SLOT(updateCalendar()));
	//Automatically Save after every change :P
	connect(m_tableWid, SIGNAL(focusLost()), this, SLOT(saveSchedule()));
	connect(m_journal, SIGNAL(focusLost()), this, SLOT(saveJournal()));
	//-----If an event gets deleted save
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(saveSchedule()));
	//------Search
	connect(searchPrevButton, SIGNAL(clicked()), this, SLOT(searchPrev()));
	connect(searchNextButton, SIGNAL(clicked()), this, SLOT(searchNext()));
	//-------------end connects---------
	QHBoxLayout* buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(newButton);
	buttonLayout->addWidget(deleteButton);
	QVBoxLayout* vl = new QVBoxLayout; //vertical layout
	vl->addWidget(m_calendar);
	vl->addWidget(m_day);
	vl->addWidget(m_tableWid);
	vl->addLayout(buttonLayout);
	vl->setSizeConstraint(QLayout::SetNoConstraint); 
	QHBoxLayout *settingsLayout = new QHBoxLayout; 
	settingsLayout->addWidget(m_fontBox);
	settingsLayout->addWidget(m_comboSize);
	settingsLayout->addWidget(m_buttonBold);
	settingsLayout->addWidget(m_buttonItalic);
	settingsLayout->addWidget(m_buttonUnderline);
	settingsLayout->setSizeConstraint(QLayout::SetNoConstraint);
	QVBoxLayout* textLayout = new QVBoxLayout;
	textLayout->addWidget(m_noteLabel);
	textLayout->addLayout(settingsLayout);
	textLayout->addWidget(m_journal);
	textLayout->addLayout(searchLayout);
	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->addLayout(vl);
	mainLayout->addLayout(textLayout);
	QWidget* CalendarPage = new QWidget; //Page that contains the calendar
	CalendarPage->setLayout(mainLayout);
	return CalendarPage;
}

void Organizer::emptyTable()
{
	m_tableWid->clearContents();  
	m_tableWid->setRowCount(m_rowCount);
}

bool Organizer::tableExistsDB(QString &tablename)
{ 
	QStringList tableList = m_db.tables(QSql::Tables);
	return (tableList.indexOf(tablename) != -1) ? true : false;
}
