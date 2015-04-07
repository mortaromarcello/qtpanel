#ifndef BATTERYAPPLET_H
#define BATTERYAPPLET_H

#include <QTimer>
#include <QIcon>
#include "applet.h"
#include "panelwindow.h"
#include "battery.h"
#include "config.h"

class Ui_AppletBatterySettings;

class BatteryApplet: public Applet, public Battery
{
	Q_OBJECT
public:
	BatteryApplet(PanelWindow *panelWindow);
	~BatteryApplet();
	bool init();
	void xmlWrite(XmlConfigWriter* writer);
	void showContextMenu(const QPoint& point);
	QSize desiredSize();

public slots:
	void update();
	void showConfigurationDialog();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void setIcon(int percentage);
	void layoutChanged();
	bool xmlRead();
    void xmlReadIconMissing();
    void xmlReadIcons();
    void xmlReadIconsCharging();
    void xmlReadPercBeforeHalt();

private:
	QGraphicsPixmapItem* m_pixmapItem;
	QIcon m_icon;
	QTimer m_timer;
	int m_percBeforeHalt;
	int m_delta;
	QStringList batteryIcons, batteryIconsCharging;
	QString batteryIconMissing;
	Ui_AppletBatterySettings* m_settingsUi;
};

#endif // BATTERYAPPLET_H
