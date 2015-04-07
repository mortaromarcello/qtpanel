#ifndef MEDIAAPPLET_H
#define MEDIAAPPLET_H

#include "applet.h"
#include <QIcon>

class MediaPlayer;
class QGraphicsPixmapItem;
class QAction;

class MediaApplet: public Applet
{
	Q_OBJECT
public:
	MediaApplet(PanelWindow* panelWindow);
	~MediaApplet();
	bool init();
	void xmlWrite(XmlConfigWriter* writer);
	void showContextMenu(const QPoint& point);
	QSize desiredSize();
	void prev();
	void playPause();
	void stop();
	void next();

public slots:
	void showConfigurationDialog();
	void openFile();
	void addFile();
	void hasVideoChanged(bool changed);
	void finished();
	void stateChanged();
	void metaDataChanged();
	void playMediaFromMenu(QAction* action);
	void memorizeToggled(bool checked);
	void reloadToggled(bool checked);
	void loopToggled(bool checked) {m_loopMedia = checked;}
	void clearMenuFiles();

protected:
	bool nextMedia(bool remove = false);
	bool prevMedia(bool remove = false);
	QString getNextMedia();
	QString getPrevMedia();
	void createFileMenu();
	void setCurrentMetaData();
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	bool xmlRead();
	void xmlReadMemorizedPos();
	void xmlReadCurrentMedia();
	void xmlReadMediaFile();
	void xmlReadReloadLastSession();
	void xmlReadLoopMedia();

private:
	const QString getCurrentMetaData() const;
	void setIcon(QGraphicsPixmapItem* item, const QString &icon, QIcon::Mode mode = QIcon::Normal, int extent = 24, const QString &ext = "");
	void setIcons();
	void clearCurrentMetaData();
	QMenu* m_fileMenu;
	QAction* m_openFileAction;
	QAction* m_addFileAction;
	QAction* m_clearMenuFilesAction;
	QGraphicsPixmapItem* m_prev;
	QGraphicsPixmapItem* m_playPause;
	QGraphicsPixmapItem* m_stop;
	QGraphicsPixmapItem* m_next;
	MediaPlayer* m_player;
	QString m_currentPath;
	QStringList m_currentMetaArtist;
	QStringList m_currentMetaAlbum;
	QStringList m_currentMetaTitle;
	QStringList m_currentMetaDate;
	QStringList m_currentMetaGenre;
	QStringList m_currentMetaTrack;
	QStringList m_currentMetaComment;
	QMap<QString, qint64> m_listMediaFile;
	QMap<QString, QAction*> m_listMediaAction;
	bool m_useInternalIcon;
	bool m_memorizePositionMediaFiles;
	bool m_reloadLastSession;
	qint64 m_lastCurrentMediaPosition;
	bool m_loopMedia;
};

#endif // MEDIAAPPLET_H
