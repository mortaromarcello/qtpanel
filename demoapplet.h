#ifndef DEMOAPPLET_H
#define DEMOAPPLET_H

#include "applet.h"

class Ui_AppletDemoSettings;

class QGraphicsRectItem;

class DemoApplet: public Applet
{
	Q_OBJECT
public:
	DemoApplet(PanelWindow* panelWindow);
	~DemoApplet();
	bool init();
	void xmlWrite(XmlConfigWriter* writer);
	void showContextMenu(const QPoint& point);
	QSize desiredSize();

public slots:
	void showConfigurationDialog();

protected:
	void layoutChanged();
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	bool xmlRead();
	void xmlReadColor();

private:
	QGraphicsRectItem* m_rectItem;
	QColor m_color;
	Ui_AppletDemoSettings* m_settingsUi;
};

#endif // DEMOAPPLET_H
