#ifndef PAGERAPPLET_H
#define PAGERAPPLET_H

#include <QDesktopWidget>
#include <QGraphicsSceneMouseEvent>
#include "mydebug.h"
#include "applet.h"
#include "xfitman.h"

class PagerApplet: public Applet
{
	Q_OBJECT

public:
	PagerApplet(PanelWindow* panelWindow);
	~PagerApplet();
	bool init();
	void xmlWrite(XmlConfigWriter* writer);
	void showContextMenu(const QPoint& point);
	QSize desiredSize();

protected:
	void reset();
	void layoutChanged();
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	bool xmlRead();
	void xmlReadTimeout();
	void xmlReadUsePixmaps();

private slots:
	void showConfigurationDialog();
	void update();

private:
	void initializePixmapDesktops();
	int m_numDesktop;
	QList <QGraphicsPixmapItem*> m_listPixmapDesktops;
	int m_currentDesktop;
	QTimer* m_timer;
	Pixmap m_rootmap;
	int m_timeout;
	bool m_usePixmaps;
};

#endif // PAGERAPPLET_H
