#ifndef LAUNCHERAPPLET_H
#define LAUNCHERAPPLET_H

#include <QGraphicsPixmapItem>
#include <QIcon>
#include <QXmlStreamReader>
#include "applet.h"
class Ui_AppletLauncherSettings;

class LauncherApplet: public Applet
{
	Q_OBJECT
public:
	LauncherApplet(PanelWindow* panelWindow);
	~LauncherApplet();
	bool init();
	const unsigned int getLastInstance();
	int getInstance() {return m_instance;}
	void setInstance(unsigned int instance) {m_instance = instance; m_configChanged = true;}
	const QString getCommand() const {return m_command;}
	void xmlWrite(XmlConfigWriter* writer);
	void showContextMenu(const QPoint& point);
	QSize desiredSize();

public slots:
	void showConfigurationDialog();
	void buttonIconClicked();
	void iconChanged(QString nameicon);
	void buttonCommandClicked();
	void commandChanged(QString command);

protected:
	void layoutChanged();
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	bool xmlRead();
	int xmlReadInstance();
	void xmlReadIcon();
	void xmlReadCommand();
	void xmlReadTooltip();

private:
	QGraphicsPixmapItem* m_pixmapItem;
	QIcon m_icon;
	QString m_iconNameFile;
	int m_delta;
	QString m_command;
	int m_instance;
	Ui_AppletLauncherSettings *m_settingsUi;
};

#endif
