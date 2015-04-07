#ifndef BACKLIGHTAPPLET_H
#define BACKLIGHTAPPLET_H

#include "applet.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QPushButton>
#include <QSlider>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>

class Ui_AppletBacklightSettings;

class BacklightApplet: public Applet
{
	Q_OBJECT
public:
	BacklightApplet(PanelWindow* panelWindow);
	~BacklightApplet();

	bool init();
	QSize desiredSize();
	void clicked();
	void showContextMenu(const QPoint& point);

public slots:
	void showConfigurationDialog();
	void valueChangedSlot(int value);
	void buttonIconClicked();

protected:
	void layoutChanged();
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	bool xmlRead();
	void xmlReadIcon();
	void xmlWrite(XmlConfigWriter* writer);
	void setGamma(double brightness);
	bool getGamma();
	int doubleToInt(double value, double max);
	double intToDouble(int value, double max);

private:
	QGraphicsPixmapItem* m_pixmapItem;
	QIcon m_icon;
	int m_delta;
	QSlider* m_slider;
	Display* m_display;
	int m_screen;
	XF86VidModeGamma m_gamma;
	Ui_AppletBacklightSettings *m_settingsUi;
};

#endif //BACKLIGHTAPPLET_H
