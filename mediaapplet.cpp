#include "mediaapplet.h"
#include "config.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QDir>
#include <QFileDialog>
#include <QKeyEvent>
#include "panelwindow.h"
#include "panelapplication.h"
#include "mediaplayer.h"

MediaApplet::MediaApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_currentPath(QDir::homePath()),				// path corrente
	m_listMediaFile(QMap<QString, qint64>()),		// lista dei media
	m_listMediaAction(QMap<QString, QAction*>()),	// lista delle QAction
	m_player(new MediaPlayer()),					// media player
	m_fileMenu(new QMenu()),						// menù file
	m_prev(new QGraphicsPixmapItem(this)),			// icona prev
	m_playPause(new QGraphicsPixmapItem(this)),		// icona playPause
	m_stop(new QGraphicsPixmapItem(this)),			// icona stop
	m_next(new QGraphicsPixmapItem(this)),			// icona next
	m_currentMetaArtist(QStringList()),
	m_currentMetaAlbum(QStringList()),
	m_currentMetaTitle(QStringList()),
	m_currentMetaDate(QStringList()),
	m_currentMetaGenre(QStringList()),
	m_currentMetaTrack(QStringList()),
	m_currentMetaComment(QStringList()),
	m_useInternalIcon(true),
	m_lastCurrentMediaPosition(0),
	m_memorizePositionMediaFiles(true),
	m_reloadLastSession(true),
	m_loopMedia(false)
{
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	// segnale di hasVideo  (indica se il media ha lo stream video) cambiato
	connect(m_player->mediaObject(), SIGNAL(hasVideoChanged(bool)), this, SLOT(hasVideoChanged(bool)));
	// segnale di media giunto alla fine
	connect(m_player->mediaObject(), SIGNAL(finished()), this, SLOT(finished()));
	// segnale di stato del media cambiato (palying, stop, etc.)
	connect(m_player->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(stateChanged()));
	// segnala di metaData cambiato
	connect(m_player->mediaObject(), SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
	// segnale di media corrente cambiato
	connect(m_player->mediaObject(), SIGNAL(currentSourceChanged(const Phonon::MediaSource&)), this, SLOT(metaDataChanged()));
	m_prev->setOffset(2, 4);
	m_playPause->setOffset(24 + 2, 4);
	m_stop->setOffset(48 + 2, 4);
	m_next->setOffset(72 + 2, 4);
	createFileMenu();
}

MediaApplet::~MediaApplet()
{
	delete m_prev;
	delete m_playPause;
	delete m_stop;
	delete m_next;
	delete m_player;
	delete m_fileMenu;
}

bool MediaApplet::init()
{
	setInteractive(true);
	if (m_reloadLastSession) {
		if (!m_player->currentFile().isEmpty()) {
			m_player->play();
			if (m_memorizePositionMediaFiles)
				m_player->seek(m_lastCurrentMediaPosition);
			m_addFileAction->setEnabled(true);
		}
		if (m_memorizePositionMediaFiles) {
			QMap<QString, qint64>::iterator i = m_listMediaFile.begin();
			while(i != m_listMediaFile.end()) {
				QIcon icon;
				if ((QFileInfo(i.key()).suffix().toLower() == "mp3") ||
					(QFileInfo(i.key()).suffix().toLower() == "ogg"))
					icon = QIcon::fromTheme("audio-x-generic");
				else if ((QFileInfo(i.key()).suffix().toLower() == "avi") ||
					(QFileInfo(i.key()).suffix().toLower() == "mp4") ||
					(QFileInfo(i.key()).suffix().toLower() == "mkv"))
					icon = QIcon::fromTheme("video-x-generic");
				else icon = QIcon::fromTheme("emblem-multimedia");
				QAction* action = new QAction(icon, QFileInfo(i.key()).fileName(), this);
				action->setData(i.key());
				m_fileMenu->addAction(action);
				m_listMediaAction[i.key()] = action;
				++i;
			}
			if (!m_listMediaFile.isEmpty()) m_clearMenuFilesAction->setEnabled(true);
		}
	}
	setIcons();
	m_configChanged = true;
	return true;
}

void MediaApplet::playMediaFromMenu(QAction *action)
{
	if ((action->data().toString().isEmpty()) || (action->data().toString() == m_player->currentFile()))
		return;
#ifdef __DEBUG__
	MyDBG << "playMediaFromMenu" << action->data().toString();
#endif
	m_player->stop();
	m_player->loadFile(action->data().toString());
	setIcons();
	playPause();
	//m_player->play();
	if (m_memorizePositionMediaFiles) {
		m_player->seek(m_listMediaFile[m_player->currentFile()]);
	}
	//
	//
}

void MediaApplet::memorizeToggled(bool checked)
{
#ifdef __DEBUG__
	MyDBG << "memorizeToggled:" << checked;
#endif
	m_memorizePositionMediaFiles = checked;
}

void MediaApplet::reloadToggled(bool checked)
{
#ifdef __DEBUG__
	MyDBG << "reloadToggled:" << checked;
#endif
	m_reloadLastSession = checked;
}

void MediaApplet::clearMenuFiles()
{
	stop();
	createFileMenu();
	m_player->clear();
	m_listMediaFile.clear();
	m_listMediaAction.clear();
	setIcons();
}

void MediaApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("memorizepos", (m_memorizePositionMediaFiles) ? "true" : "false");
	writer->writeTextElement("reloadlastsession", (m_reloadLastSession) ? "true" : "false");
	writer->writeTextElement("loopmedia", (m_loopMedia) ? "true" : "false");
	if (m_memorizePositionMediaFiles) {
		if (!m_listMediaFile.isEmpty()) {
			QMap<QString, qint64>::iterator i = m_listMediaFile.begin();
			while(i != m_listMediaFile.end()) {
				if (!i.key().isEmpty()) {
					writer->writeTextElement("mediafile", i.key() + "," + QString("%1").arg(i.value()));
					++i;
				}
			}
		}
		if (!m_player->currentFile().isEmpty()) {
			if (m_player->isPlaying()) {
				m_lastCurrentMediaPosition = m_player->currentTime();
			}
			writer->writeTextElement("currentmedia", m_player->currentFile() + "," + QString("%1").arg(m_lastCurrentMediaPosition));
		}
	}
	writer->writeEndElement();
}

void MediaApplet::showConfigurationDialog() {}

void MediaApplet::openFile()
{
	QStringList fileNames;
	fileNames = QFileDialog::getOpenFileNames(m_player, tr("Open Media"), m_currentPath, tr("Audio/Video (*.mp3 *.ogg *.avi *.mp4);;All (*.*)"));
	if (fileNames.isEmpty())
		return;
	clearMenuFiles();
	foreach(QString str, fileNames) {
		m_listMediaFile[str] = 0;
	}
	if (!m_listMediaFile.isEmpty()) {
		m_addFileAction->setEnabled(true);
		m_clearMenuFilesAction->setEnabled(true);
		QMap<QString, qint64>::iterator i = m_listMediaFile.begin();
		while(i != m_listMediaFile.end()) {
			QIcon icon;
			if ((QFileInfo(i.key()).suffix().toLower() == "mp3") ||
				(QFileInfo(i.key()).suffix().toLower() == "ogg"))
				icon = QIcon::fromTheme("audio-x-generic");
			else if ((QFileInfo(i.key()).suffix().toLower() == "avi") ||
				(QFileInfo(i.key()).suffix().toLower() == "mp4") ||
				(QFileInfo(i.key()).suffix().toLower() == "mkv"))
				icon = QIcon::fromTheme("video-x-generic");
			else icon = QIcon::fromTheme("emblem-multimedia");
			QAction* action = new QAction(icon, QFileInfo(i.key()).fileName(), this);
			action->setData(i.key());
			m_fileMenu->addAction(action);
			m_listMediaAction[i.key()] = action;
			++i;
		}
		m_player->loadFile(m_listMediaFile.begin().key());
		setCurrentMetaData();
		if (!m_player->hasVideo())
			m_player->hide();
		m_player->play();
		setIcons();
		m_currentPath = QDir(m_player->currentFile()).path();
#ifdef __DEBUG__
		MyDBG << m_player->currentFile();
#endif
	}
}

void MediaApplet::addFile()
{
	if (m_listMediaFile.isEmpty() && m_player->currentFile().isEmpty())
		return;
	QStringList fileNames;
	QMap<QString, qint64> tempList;
	fileNames = QFileDialog::getOpenFileNames(m_player, tr("Add Media"), m_currentPath, tr("Audio/Video (*.mp3 *.ogg *.avi *.mp4 *.mkv);;All (*.*)"));
	if (fileNames.isEmpty())
		return;
	foreach(QString str, fileNames) {
		if (!m_listMediaFile.contains(str)) {
			tempList[str] = 0;
			m_listMediaFile[str] = 0;
		}
	}
	QMap<QString, qint64>::iterator i = tempList.begin();
	while(i != tempList.end()) {
		QIcon icon;
		if ((QFileInfo(i.key()).suffix().toLower() == "mp3") ||
			(QFileInfo(i.key()).suffix().toLower() == "ogg"))
			icon = QIcon::fromTheme("audio-x-generic");
		else if ((QFileInfo(i.key()).suffix().toLower() == "avi") ||
			(QFileInfo(i.key()).suffix().toLower() == "mp4") ||
			(QFileInfo(i.key()).suffix().toLower() == "mkv"))
			icon = QIcon::fromTheme("video-x-generic");
		else icon = QIcon::fromTheme("emblem-multimedia");
		QAction* action = new QAction(icon, QFileInfo(i.key()).fileName(), this);
		action->setData(i.key());
		m_fileMenu->addAction(action);
		m_listMediaAction[i.key()] = action;
		++i;
	}
	setIcons();
	m_clearMenuFilesAction->setEnabled(true);
	m_currentPath = QDir(m_listMediaFile.begin().key()).path();
}

void MediaApplet::hasVideoChanged(bool changed)
{
#ifdef __DEBUG__
	MyDBG << "hasVideo changed:" << changed;
#endif
	if (changed) {
		m_player->resize(m_player->videoWidget()->sizeHint());
		m_player->show();
	}
	if (!changed)
		m_player->hide();
}

void MediaApplet::finished()
{
#ifdef __DEBUG__
	MyDBG << "MediaApplet finished";
#endif
	if ((m_loopMedia ? nextMedia(false) : nextMedia(true))) {
		// se nextMedia è riuscito ad aprire il prossime media (m_player->currentFile() non è vuoto)
		m_player->play();
		if (m_memorizePositionMediaFiles) {
			// se m_memorizePositionMediaFiles=true va alla posizione memorizzata
			m_player->seek(m_listMediaFile[m_player->currentFile()]);
		}
	}
	else {
		if (!m_loopMedia) {
			// altrimenti
			if (!m_player->currentFile().isEmpty())
				// se currentFile non è vuoto
				m_player->stop();
			else {
				// se currentFile è vuoto
				if (m_listMediaFile.isEmpty()) {
					// se la lista è vuota
					m_addFileAction->setEnabled(false);
					m_clearMenuFilesAction->setEnabled(false);
				}
			}
			m_player->hide();
			if (m_player->videoWidget()->isFullScreen())
				// se è a schermo intero
				m_player->videoWidget()->exitFullScreen();
		}
		else {
			m_player->loadFile(m_listMediaFile.begin().key());
			m_player->play();
		}
	}
	setIcons();
}

void MediaApplet::stateChanged()
{
	Phonon::State state = m_player->state();
#ifdef __DEBUG__
	MyDBG << "State changed:" << state;
#endif
	switch(state) {
		case Phonon::LoadingState:
			break;
		case Phonon::StoppedState:
			setIcon(m_stop, "media-playback-stop", QIcon::Disabled);
			(m_player->currentFile().isEmpty()) ? setIcon(m_playPause, "media-playback-start", QIcon::Disabled) : setIcon(m_playPause, "media-playback-start");
			break;
		case Phonon::PlayingState:
			setIcon(m_playPause, "media-playback-pause");
			setIcon(m_stop, "media-playback-stop");
			break;
		case Phonon::BufferingState:
			break;
		case Phonon::PausedState:
			setIcon(m_playPause, "media-playback-start");
			break;
		case Phonon::ErrorState:
			m_player->clear();
			qWarning(QString(tr("Error in media player\n")).toAscii());
			break;
		default:
			break;
	}
}

void MediaApplet::metaDataChanged()
{
#ifdef __DEBUG__
	MyDBG << "metaData changed" << m_player->currentFile();
#endif
	setCurrentMetaData();
}

bool MediaApplet::nextMedia(bool remove)
{
#ifdef __DEBUG__
	MyDBG << "nextMedia";
#endif
	if (!m_listMediaFile.isEmpty() && !m_player->currentFile().isEmpty()) {
		// se la lista non è vuota
		QMap<QString, qint64>::iterator i = m_listMediaFile.find(m_player->currentFile());
		// se per qualche motivo currentFile non è nella lista ritorna false
		if (i == m_listMediaFile.end()) return false;
		if (i != --m_listMediaFile.end()) {
			// se i è diverso dall'ultimo media (QMap().end() non è l'ultimo elemento ma un elemento vuoto inserito dopo l'ultimo)
			if (/*!m_player->currentFile().isEmpty() && */ m_memorizePositionMediaFiles) {
				// se il file corrente non è vuoto e m_memorizePositionMediaFile = true
				m_listMediaFile[i.key()] = m_player->currentTime();
			}
			//
			if (remove && (m_player->currentTime() == 0)) {
				// se remove=true e il file corrente è all'inizio cancella dalla lista il file
				m_listMediaFile.erase(i);
				m_fileMenu->removeAction(m_listMediaAction[m_player->currentFile()]);
				m_listMediaAction.remove(m_player->currentFile());
				m_player->loadFile((++i).key());
			}
			//
			else {
				// altrimenti carica solo il file
				m_player->loadFile((++i).key());
			}
			if (!m_player->currentFile().isEmpty())
				return true;
		}
		else {
			// se i è l'ultimo elemento
			if (remove && !m_player->currentFile().isEmpty()) {
				// se remove=true e il file corrente è vuoto
				m_listMediaFile.erase(i);
				m_fileMenu->removeAction(m_listMediaAction[m_player->currentFile()]);
				m_listMediaAction.remove(m_player->currentFile());
				m_player->clear();
			}
		}
	}
	// se la lista (m_listMediaFile) e se currentFile sono vuoti ritorna false
	return false;
}

bool MediaApplet::prevMedia(bool erase)
{
#ifdef __DEBUG__
	MyDBG << "prevMedia";
#endif
	if (!m_listMediaFile.isEmpty() && !m_player->currentFile().isEmpty()) {
		// se la lista e currentFile non sono vuoti
		QMap<QString, qint64>::iterator i = m_listMediaFile.find(m_player->currentFile());
		// se per qualche motivo currentFile non è nella lista ritorna false
		if (i == m_listMediaFile.end()) return false;
		if (i != m_listMediaFile.begin()) {
			// se i è diverso dal primo media della lista
			if (/*!m_player->currentFile().isEmpty() &&*/ m_memorizePositionMediaFiles) {
				// se il file corrente non è vuoto e m_memorizePositionMediaFile = true
				m_listMediaFile[i.key()] = m_player->currentTime();
			}
			//
			if (remove && (m_player->currentTime() == 0)) {
				// se remove=true e il file corrente è all'inizio
				m_listMediaFile.erase(i);
				m_fileMenu->removeAction(m_listMediaAction[m_player->currentFile()]);
				m_listMediaAction.remove(m_player->currentFile());
				if (!m_listMediaFile.isEmpty())
					m_player->loadFile((--i).key());
				else m_player->clear();
			}
			//
			else
				m_player->loadFile((--i).key());
			if (!m_player->currentFile().isEmpty())
				return true;
		}
		else {
			// se i è il primo elemento
			if (remove && !m_player->currentFile().isEmpty()) {
				// se remove=true e il file corrente è vuoto
				m_listMediaFile.erase(i);
				m_fileMenu->removeAction(m_listMediaAction[m_player->currentFile()]);
				m_listMediaAction.remove(m_player->currentFile());
				m_player->clear();
			}
		}
	}
	// se la lista (m_listMediaFile) e se currentFile sono vuoti ritorna false
	return false;
}

QString MediaApplet::getNextMedia()
{
	if (m_player->currentFile().isEmpty() || m_listMediaFile.isEmpty())
		return QString();
	QMap<QString, qint64>::iterator i = m_listMediaFile.find(m_player->currentFile());
	if (i != --m_listMediaFile.end() && i != m_listMediaFile.end())
		return (++i).key();
	return QString();
}

QString MediaApplet::getPrevMedia()
{
	if (m_player->currentFile().isEmpty() || m_listMediaFile.isEmpty())
		return QString();
	QMap<QString, qint64>::iterator i = m_listMediaFile.find(m_player->currentFile());
	if (i != m_listMediaFile.begin())
		return (--i).key();
	return QString();
}

void MediaApplet::createFileMenu()
{
	m_fileMenu->clear();	// // cancella il menù
	m_openFileAction = m_fileMenu->addAction(QIcon::fromTheme("stock_open"), tr("Open &Files..."), this, SLOT(openFile()));
	m_addFileAction = m_fileMenu->addAction(QIcon::fromTheme("add"), tr("Add &Files..."), this, SLOT(addFile()));
	m_addFileAction->setEnabled(false);
	QAction* memorizePositionMediaFileAction = new QAction(tr("Memorize position Media"), m_fileMenu);
	memorizePositionMediaFileAction->setCheckable(true);
	memorizePositionMediaFileAction->setChecked(m_memorizePositionMediaFiles);
	m_fileMenu->addAction(memorizePositionMediaFileAction);
	connect(memorizePositionMediaFileAction, SIGNAL(toggled(bool)), this, SLOT(memorizeToggled(bool)));
	QAction* reloadLastSessionAction = new QAction(tr("Reload last session"), m_fileMenu);
	reloadLastSessionAction->setCheckable(true);
	reloadLastSessionAction->setChecked(m_reloadLastSession);
	m_fileMenu->addAction(reloadLastSessionAction);
	connect(reloadLastSessionAction, SIGNAL(toggled(bool)), this, SLOT(reloadToggled(bool)));
	m_fileMenu->addSeparator();
	m_clearMenuFilesAction = m_fileMenu->addAction(QIcon::fromTheme("edit-clear"), tr("&Clear Files..."), this, SLOT(clearMenuFiles()));
	m_clearMenuFilesAction->setEnabled(false);
	connect(m_fileMenu, SIGNAL(triggered(QAction*)), this, SLOT(playMediaFromMenu(QAction*)));
	//
	QAction* loopMediaAction = new QAction(tr("Loop List Media"), m_fileMenu);
	loopMediaAction->setCheckable(true);
	loopMediaAction->setChecked(m_loopMedia);
	m_fileMenu->addAction(loopMediaAction);
	connect(loopMediaAction, SIGNAL(toggled(bool)), this, SLOT(loopToggled(bool)));
}

void MediaApplet::setCurrentMetaData()
{
	if (!m_player->currentFile().isEmpty()) {
		m_currentMetaArtist = m_player->metaData("ARTIST");
		m_currentMetaAlbum = m_player->metaData("ALBUM");
		m_currentMetaTitle = m_player->metaData("TITLE");
		m_currentMetaDate = m_player->metaData("DATE");
		m_currentMetaGenre = m_player->metaData("GENRE");
		m_currentMetaTrack = m_player->metaData("TRACKNUMBER");
		m_currentMetaComment = m_player->metaData("DESCRIPTION");
	}
	else
		clearCurrentMetaData();
}

QSize MediaApplet::desiredSize()
{
	return QSize(102, 64);
}

void MediaApplet::prev()
{
	if (prevMedia()) {
		if (!m_player->isHidden() && !m_player->hasVideo())
			m_player->hide();
		m_player->play();
		if (m_memorizePositionMediaFiles) {
			m_player->seek(m_listMediaFile[m_player->currentFile()]);
		}
		setIcons();
#ifdef __DEBUG__
		MyDBG << m_player->currentFile();
#endif
	}
}

void MediaApplet::playPause()
{ 
	if (!m_player->isHidden() && !m_player->hasVideo())
		m_player->hide();
	if (m_player->isPlaying()) {
		m_lastCurrentMediaPosition = m_player->currentTime();
		if (m_listMediaFile.contains(m_player->currentFile()))
			m_listMediaFile[m_player->currentFile()] = m_lastCurrentMediaPosition;
		m_player->pause();
	}
	else {
		if (m_player->isHidden() && m_player->hasVideo())
			m_player->show();
		if (m_player->isMinimized() && m_player->hasVideo())
			m_player->showNormal();
		if (!m_player->mediaObject()->currentSource().fileName().isEmpty()) {
			m_player->play();
		}
	}
}

void MediaApplet::stop()
{
	if (!m_player->isStopped()) {
		m_lastCurrentMediaPosition = m_player->currentTime();
		if (m_listMediaFile.contains(m_player->currentFile()))
			m_listMediaFile[m_player->currentFile()] = m_lastCurrentMediaPosition;
		m_player->stop();
		m_player->hide();
	}
}

void MediaApplet::next()
{
	if (nextMedia()) {
		if (!m_player->isHidden() && !m_player->hasVideo())
			m_player->hide();
		m_player->play();
		if (m_memorizePositionMediaFiles) {
			m_player->seek(m_listMediaFile[m_player->currentFile()]);
		}
		setIcons();
#ifdef __DEBUG__
		MyDBG << m_player->currentFile();
#endif
	}
}

void MediaApplet::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	if ((event->scenePos().x() > m_prev->scenePos().x()) && (event->scenePos().x() < (m_prev->scenePos().x() + m_prev->offset().x() + m_prev->pixmap().width()))) {
		m_textToolTip = tr("Previous: ") + (getPrevMedia().isEmpty() ? tr("No Media") : QFileInfo(getPrevMedia()).fileName());
	}
	else if ((event->scenePos().x() > m_playPause->scenePos().x()) && (event->scenePos().x() < (m_playPause->scenePos().x() + m_playPause->offset().x() + m_playPause->pixmap().width()))) {
		m_textToolTip = getCurrentMetaData().isEmpty() ? tr("No Media") : getCurrentMetaData();
	}
	else if ((event->scenePos().x() > m_stop->scenePos().x()) && (event->scenePos().x() < (m_stop->scenePos().x() + m_stop->offset().x() + m_stop->pixmap().width()))) {
		m_textToolTip = getCurrentMetaData().isEmpty() ? tr("No Media") : getCurrentMetaData();
	}
	else if ((event->scenePos().x() > m_next->scenePos().x()) && (event->scenePos().x() < (m_next->scenePos().x() + m_next->offset().x() + m_next->pixmap().width()))) {
		m_textToolTip = tr("Next: ") + (getNextMedia().isEmpty() ? tr("No Media") : QFileInfo(getNextMedia()).fileName());
	}
	Applet::hoverEnterEvent(event);
}

void MediaApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->buttons() == Qt::RightButton) {
		m_fileMenu->exec(localToScreen(QPoint(0, 0)));
	}
	if (event->buttons() == Qt::LeftButton) {
		if (m_prev->contains(event->pos())) {
#ifdef __DEBUG__
			MyDBG << "Prev pressed";
#endif
			prev();
		}
		if (m_playPause->contains(event->pos())) {
#ifdef __DEBUG__
			MyDBG << "PlayPause pressed";
#endif
			playPause();
		}
		if (m_stop->contains(event->pos())) {
#ifdef __DEBUG__
			MyDBG << "Stop pressed";
#endif
			stop();
		}
		if (m_next->contains(event->pos())) {
#ifdef __DEBUG__
			MyDBG << "Next pressed";
#endif
			next();
		}
	}
}

void MediaApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	//if(isUnderMouse())
	//	showContextMenu(localToScreen(QPoint(0, 0)));
}

bool MediaApplet::xmlRead()
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
									if (m_xmlConfigReader.name() == "memorizepos")
										xmlReadMemorizedPos();
									else if (m_xmlConfigReader.name() == "reloadlastsession")
										xmlReadReloadLastSession();
									else if (m_xmlConfigReader.name() == "currentmedia")
										xmlReadCurrentMedia();
									else if (m_xmlConfigReader.name() == "mediafile")
										xmlReadMediaFile();
									else if (m_xmlConfigReader.name() == "loopmedia")
										xmlReadLoopMedia();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void MediaApplet::xmlReadMemorizedPos()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "memorizepos");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	m_memorizePositionMediaFiles = (m_xmlConfigReader.readElementText() == "true") ? true : false;
}

void MediaApplet::xmlReadReloadLastSession()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "reloadlastsession");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	m_reloadLastSession = (m_xmlConfigReader.readElementText() == "true") ? true : false;
}

void MediaApplet::xmlReadLoopMedia()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "loopmedia");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	m_loopMedia = (m_xmlConfigReader.readElementText() == "true") ? true : false;
}

void MediaApplet::xmlReadCurrentMedia()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "currentmedia");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	QString text = m_xmlConfigReader.readElementText();
	QString nameFile = text.split(",")[0];
	m_lastCurrentMediaPosition = text.split(",")[1].toInt();
	if (m_reloadLastSession) {
		if (!nameFile.isEmpty()) {
			m_listMediaFile[nameFile] = m_lastCurrentMediaPosition;
			m_player->loadFile(nameFile);
		}
	}
}

void MediaApplet::xmlReadMediaFile()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "mediafile");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	QString text = m_xmlConfigReader.readElementText();
	QString nameFile = text.split(",")[0];
	qint64 position = text.split(",")[1].toInt();
	if (m_memorizePositionMediaFiles)
		if (!m_listMediaFile.contains(nameFile)) {
			m_listMediaFile[nameFile] = position;
		}
}

const QString MediaApplet::getCurrentMetaData() const
{
	QStringList temp;
	if (!m_currentMetaArtist.join("").isEmpty())
		temp << (tr("Artist: ") + m_currentMetaArtist.join(""));
	if (!m_currentMetaAlbum.join("").isEmpty())
		temp << (tr("Album: ") + m_currentMetaAlbum.join(""));
	if (!m_currentMetaTitle.join("").isEmpty())
		temp << (tr("Title: ") + m_currentMetaTitle.join(""));
	if (!m_currentMetaDate.join("").isEmpty())
		temp << (tr("Date: ") + m_currentMetaDate.join(""));
	if (!m_currentMetaGenre.join("").isEmpty())
		temp << (tr("Genre: ") + m_currentMetaGenre.join(""));
	if (!m_currentMetaTrack.join("").isEmpty())
		temp << (tr("Track: ") + m_currentMetaTrack.join(""));
	if (!m_currentMetaComment.join("").isEmpty())
		temp << (tr("Comment: ") + m_currentMetaComment.join(""));
	return temp.join("\n");
}

void MediaApplet::setIcon(QGraphicsPixmapItem* item, const QString &icon, QIcon::Mode mode, int extent, const QString &ext)
{
	QString temp = ext;
	if (temp.isEmpty())
		temp = "svg";
	else {
		if (temp[0] == '.')
			temp = temp.remove(0,1);
		}
	if (!m_useInternalIcon)
		QIcon::hasThemeIcon(icon) ? item->setPixmap(QIcon::fromTheme(icon).pixmap(extent, mode)) : item->setPixmap(QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + icon + "." + temp).pixmap(extent, mode));
	else
		item->setPixmap(QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + icon + "." + temp).pixmap(extent, mode));
}

void MediaApplet::setIcons()
{
	if (m_listMediaFile.isEmpty() || m_listMediaFile.size() == 1 || m_player->currentFile().isEmpty()) {
		// se la lista è vuota o la lista=1 o il file corrente è vuoto
		setIcon(m_prev, "media-skip-backward", QIcon::Disabled);
		setIcon(m_playPause, "media-playback-start", QIcon::Disabled);
		setIcon(m_stop, "media-playback-stop", QIcon::Disabled);
		setIcon(m_next, "media-skip-forward", QIcon::Disabled);
		return;
	}
	if (!m_listMediaFile.isEmpty()) {
		// se la lista non è vuota
		QMap<QString, qint64>::iterator i = m_listMediaFile.find(m_player->currentFile());
		if (i == m_listMediaFile.begin()) {
			// se i è il primo
			setIcon(m_prev, "media-skip-backward", QIcon::Disabled);
			if (m_listMediaFile.size() > 1)
				setIcon(m_next, "media-skip-forward");
			else setIcon(m_next, "media-skip-forward", QIcon::Disabled);
		}
		if (i == --m_listMediaFile.end()) {
			// se i è l'ultimo
			setIcon(m_next, "media-skip-forward", QIcon::Disabled);
			if (m_listMediaFile.size() > 1)
				setIcon(m_prev, "media-skip-backward");
			else setIcon(m_next, "media-skip-forward", QIcon::Disabled);
		}
		if(i != m_listMediaFile.begin() && i != --m_listMediaFile.end()) {
			// se i non è il primo né l'ultimo
			setIcon(m_prev, "media-skip-backward");
			setIcon(m_next, "media-skip-forward");
		}
	}
}

void MediaApplet::clearCurrentMetaData()
{
	m_currentMetaArtist.clear();
	m_currentMetaAlbum.clear();
	m_currentMetaTitle.clear();
	m_currentMetaDate.clear();
	m_currentMetaGenre.clear();
	m_currentMetaTrack.clear();
	m_currentMetaComment.clear();
}
