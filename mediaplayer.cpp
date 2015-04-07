#include "mediaplayer.h"

#include <QFileDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QVBoxLayout>

MediaVideoWidget::MediaVideoWidget(MediaPlayer *player, QWidget *parent) :
	Phonon::VideoWidget(parent), m_player(player), m_action(this), m_step(2000)
{
	m_action.setCheckable(true);
	m_action.setChecked(false);
	m_action.setShortcut(QKeySequence( Qt::AltModifier + Qt::Key_Return));
	m_action.setShortcutContext(Qt::WindowShortcut);
	connect(&m_action, SIGNAL(toggled(bool)), SLOT(setFullScreen(bool)));
	addAction(&m_action);
	setAcceptDrops(true);
}

void MediaVideoWidget::setFullScreen(bool enabled)
{
	Phonon::VideoWidget::setFullScreen(enabled);
	if (isFullScreen())
		Phonon::VideoWidget::activateWindow();
	emit fullScreenChanged(enabled);
}

void MediaVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	setFullScreen(!isFullScreen());
	Phonon::VideoWidget::mouseDoubleClickEvent(e);
}

void MediaVideoWidget::keyPressEvent(QKeyEvent *e)
{
	if(!e->modifiers()) {
		if (e->key() == Qt::Key_Space || e->key() == Qt::Key_0) {
			if (m_player->isPlaying())
				m_player->pause();
			else
				m_player->play();
			e->accept();
			return;
		}
		else if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Backspace) {
			setFullScreen(false);
			e->accept();
			return;
		}
		else if (e->key() == Qt::Key_Right) {
			if (m_player->currentTime()+m_step <= m_player->totalTime())
				m_player->seek(m_player->currentTime()+m_step);
			else
				m_player->seek(0);
		}
		else if (e->key() == Qt::Key_Left) {
			if (m_player->currentTime()-m_step >= 0)
				m_player->seek(m_player->currentTime()-m_step);
			else
				m_player->seek(m_player->totalTime());
		}
		else if (e->key() == Qt::Key_PageDown) {
			qint64 step = m_player->totalTime() / 10;
			if (m_player->currentTime()+step <= m_player->totalTime())
				m_player->seek(m_player->currentTime()+step);
			else
				m_player->seek(m_player->totalTime());
		}
		else if (e->key() == Qt::Key_PageUp) {
			qint64 step = m_player->totalTime() / 10;
			if (m_player->currentTime()-step >= 0)
				m_player->seek(m_player->currentTime()-step);
			else
				m_player->seek(0);
		}
	}
	Phonon::VideoWidget::keyPressEvent(e);
}

bool MediaVideoWidget::event(QEvent *e)
{
	switch(e->type())
	{
	case QEvent::Close:
		e->ignore();
		return true;
	case QEvent::MouseMove:
#ifndef QT_NO_CURSOR
		unsetCursor();
#endif
		//fall through
	case QEvent::WindowStateChange:
		{
			m_action.setChecked(windowState() & Qt::WindowFullScreen);
			const Qt::WindowFlags flags = m_player->windowFlags();
			if (windowState() & Qt::WindowFullScreen) {
				m_timer.start(1000, this);
			} else {
				m_timer.stop();
#ifndef QT_NO_CURSOR
				unsetCursor();
#endif
			}
		}
		break;
	default:
		break;
	}
	return Phonon::VideoWidget::event(e);
}

void MediaVideoWidget::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == m_timer.timerId()) {
		//let's store the cursor shape
#ifndef QT_NO_CURSOR
		setCursor(Qt::BlankCursor);
#endif
	}
	Phonon::VideoWidget::timerEvent(e);
}

MediaPlayer::MediaPlayer()
	:m_AudioOutput(Phonon::VideoCategory), m_videoWidget(new MediaVideoWidget(this))
{
	setWindowTitle(tr("Media Player"));
	setWindowIcon(QIcon::fromTheme("multimedia-player"));
	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(0, 0, 0, 0);
	initVideoWindow();
	m_slider = new Phonon::SeekSlider(this);
	m_slider->setMediaObject(&m_MediaObject);
	vLayout->addWidget(&m_videoWindow);
	QWidget *sliderPanel = new QWidget(this);
	QHBoxLayout *sliderLayout = new QHBoxLayout();
	sliderLayout->addWidget(m_slider);
	sliderLayout->setContentsMargins(0, 0, 0, 0);
	sliderPanel->setLayout(sliderLayout);
	sliderPanel->setFixedHeight(sliderPanel->sizeHint().height()); // fix height
	vLayout->addWidget(sliderPanel);
	setLayout(vLayout);
	m_audioOutputPath = Phonon::createPath(&m_MediaObject, &m_AudioOutput);
	Phonon::createPath(&m_MediaObject, m_videoWidget);
	resize(minimumSizeHint());
}

void MediaPlayer::initVideoWindow()
{
	QVBoxLayout *videoLayout = new QVBoxLayout();
	videoLayout->addWidget(m_videoWidget);
	videoLayout->setContentsMargins(0, 0, 0, 0);
	m_videoWindow.setLayout(videoLayout);
	m_videoWindow.setMinimumSize(100, 100);
}

void MediaPlayer::loadFile(const QString & file)
{
	m_MediaObject.setCurrentSource(Phonon::MediaSource(file));
}

Phonon::MediaSource MediaPlayer::currentSource() const
{
	return m_MediaObject.currentSource();
}

Phonon::State MediaPlayer::state() const
{
	return m_MediaObject.state();
}

qint64 MediaPlayer::currentTime() const
{
	return m_MediaObject.currentTime();
}

qint64 MediaPlayer::totalTime() const
{
	return m_MediaObject.totalTime();
}

QString MediaPlayer::currentFile() const
{
	return currentSource().fileName();
}

bool MediaPlayer::isSeekable() const
{
	return m_MediaObject.isSeekable();
}

QStringList MediaPlayer::metaData(const QString & key) const
{
	return m_MediaObject.metaData(key);
}

QStringList MediaPlayer::metaData(Phonon::MetaData key) const
{
	return m_MediaObject.metaData(key);
}

bool MediaPlayer::hasVideo() const
{
	return m_MediaObject.hasVideo();
}

bool MediaPlayer::isPlaying() const
{
	return (m_MediaObject.state() == Phonon::PlayingState);
}

bool MediaPlayer::isPaused() const
{
	return (m_MediaObject.state() == Phonon::PausedState);
}

bool MediaPlayer::isStopped() const
{
	return (m_MediaObject.state() == Phonon::StoppedState);
}

bool MediaPlayer::isError() const
{
	return (m_MediaObject.state() == Phonon::ErrorState);
}

bool MediaPlayer::isBuffering() const
{
	return (m_MediaObject.state() == Phonon::BufferingState);
}

bool MediaPlayer::isLoading() const
{
	return (m_MediaObject.state() == Phonon::LoadingState);
}

void MediaPlayer::clear()
{
	m_MediaObject.clearQueue();
	m_MediaObject.setCurrentSource(Phonon::MediaSource());
}

void MediaPlayer::play()
{
	m_MediaObject.play();
}

void MediaPlayer::pause()
{
	m_MediaObject.pause();
}

void MediaPlayer::stop()
{
	m_MediaObject.stop();
}

void MediaPlayer::seek(qint64 time)
{
	m_MediaObject.seek(time);
}

void MediaPlayer::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Space || e->key() == Qt::Key_0) {
		if (isPlaying())
			pause();
		else
			play();
	}
}
