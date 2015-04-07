#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <phonon>
#include <QBasicTimer>
#include <QAction>

class MediaPlayer;

class MediaVideoWidget : public Phonon::VideoWidget
{
	Q_OBJECT
public:
	MediaVideoWidget(MediaPlayer *player, QWidget *parent = 0);
	void setStep(qint64 step) {m_step = step;}
	qint64 step() const {return m_step;}

public slots:
	void setFullScreen(bool);

signals:
	void fullScreenChanged(bool);

protected:
	void mouseDoubleClickEvent(QMouseEvent *e);
	void keyPressEvent(QKeyEvent *e);
	bool event(QEvent *e);
	void timerEvent(QTimerEvent *e);
	qint64 m_step;

private:
	MediaPlayer *m_player;
	QBasicTimer m_timer;
	QAction m_action;
};

class MediaPlayer: public QWidget
{
	Q_OBJECT
public:
	MediaPlayer();
	void initVideoWindow();
	void loadFile(const QString & file);
	Phonon::MediaSource currentSource() const;
	Phonon::State state() const;
	qint64 currentTime() const;
	qint64 totalTime() const;
	QString currentFile() const;
	bool isSeekable() const;
	QStringList metaData(const QString & key) const;
	QStringList metaData(Phonon::MetaData key) const;
	const Phonon::MediaObject* mediaObject() const {return &m_MediaObject;}
	Phonon::VideoWidget* videoWidget() const {return m_videoWidget;}
	bool hasVideo() const;
	bool isPlaying() const;
	bool isPaused() const;
	bool isStopped() const;
	bool isError() const;
	bool isBuffering() const;
	bool isLoading() const;
	void clear();

public slots:
	void play();
	void pause();
	void stop();
	void seek(qint64 time);

protected:
	void keyPressEvent(QKeyEvent *e);

private:
	QWidget m_videoWindow;
	Phonon::SeekSlider* m_slider;
	Phonon::MediaObject m_MediaObject;
	Phonon::AudioOutput m_AudioOutput;
	MediaVideoWidget* m_videoWidget;
	Phonon::Path m_audioOutputPath;
};

#endif // MEDIAPLAYER_H
