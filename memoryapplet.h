#ifndef MEMORYAPPLET_H
#define MEMORYAPPLET_H

#include <QTimer>
#include "applet.h"

class QGraphicsRectItem;
class Ui_AppletMemorySettings;
class TextGraphicsItem;

class SysInfo
{
public:
	SysInfo();
	~SysInfo();
	unsigned long totalRam() {return m_totalRam;}
	unsigned long freeRam() {return m_freeRam;}
	unsigned long sharedRam() {return m_sharedRam;}
	unsigned long bufferRam() {return m_bufferRam;}
	unsigned long totalSwap() {return m_totalSwap;}
	unsigned long freeSwap() {return m_freeSwap;}
	unsigned long totalHigh() {return m_totalHigh;}
	unsigned long freeHigh() {return m_freeHigh;}
private:
	unsigned long m_totalRam;
	unsigned long m_freeRam;
	unsigned long m_sharedRam;
	unsigned long m_bufferRam;
	unsigned long m_totalSwap;
	unsigned long m_freeSwap;
	unsigned long m_totalHigh;
	unsigned long m_freeHigh;
};

class MemoryApplet: public Applet
{
	Q_OBJECT
public:
	MemoryApplet(PanelWindow* panelWindow);
	~MemoryApplet();
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

private slots:
	void update();

private:
	TextGraphicsItem* m_textItem;
	QColor m_color;
	QTimer m_timer;
	Ui_AppletMemorySettings* m_settingsUi;
};

#endif // MEMORYAPPLET_H
