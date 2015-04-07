#ifndef VOLUMEAPPLET_H
#define VOLUMEAPPLET_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QSlider>
#include <alsa/asoundlib.h>
#include "applet.h"

class Ui_AppletVolumeSettings;

class VolumeApplet: public Applet
{
	Q_OBJECT
public:
	VolumeApplet(PanelWindow* panelWindow);
	~VolumeApplet();

	bool init();
	QSize desiredSize();
	void clicked();
	void showContextMenu(const QPoint& point);

public slots:
	void showConfigurationDialog();
	void valueChangedSlot(int value);
	void launchMixer();
	void buttonIconClicked();
	void mixerChanged(QString mixer);

protected:
	void layoutChanged();
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	bool asoundInitialize();
	bool asoundFindElement(const char * ename);
	void updateIcon();
	bool xmlRead();
	void xmlReadMixer();
	void xmlReadIconMixer();
	void xmlReadVolume();
	void xmlWrite(XmlConfigWriter* writer);

private:
	void setIcon(QGraphicsPixmapItem* item, const QString &icon, QIcon::Mode mode = QIcon::Normal, int extent = 24, const QString &ext = "");
	QGraphicsPixmapItem* m_pixmapItem;
	//QIcon m_icon;
	int m_delta;
	QSlider* m_slider;
	snd_mixer_t* m_handle;
	snd_mixer_elem_t* m_elem;
	snd_mixer_selem_id_t* m_sid;
	QString m_mixer;
	QIcon m_icon_mixer;
	Ui_AppletVolumeSettings* m_settingsUi;
	bool m_useInternalIcon;
	long m_volume;
};

#endif //VOLUMEAPPLET_H
