#ifndef CLOCKAPPLET_H
#define CLOCKAPPLET_H

#include "applet.h"
#include <QGraphicsSceneMouseEvent>

class Ui_AppletClockSettings;

class Organizer;

class QTimer;
class TextGraphicsItem;

class ClockApplet: public Applet
{
	Q_OBJECT
public:
	ClockApplet(PanelWindow* panelWindow);
	~ClockApplet();
	void xmlWrite(XmlConfigWriter* writer);
	void showContextMenu(const QPoint& point);

	bool init();
	QSize desiredSize();

public slots:
	void showConfigurationDialog();

protected:
	void layoutChanged();
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	bool xmlRead();
	void xmlReadUseOrganizer();
	void xmlReadTimeFormat();

private slots:
	void updateContent();

private:
	void scheduleUpdate();

	QTimer* m_timer;
	QString m_text;
	TextGraphicsItem* m_textItem;
	Organizer* m_organizer;
	bool m_useOrganizer;
	QString m_timeFormat;
	Ui_AppletClockSettings *m_settingsUi;
};

#endif
