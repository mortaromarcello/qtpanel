#ifndef SESSIONAPPLET_H
#define SESSIONAPPLET_H

#include <QIcon>
#include <QMenu>
#include "applet.h"

class Ui_AppletSessionSettings;

class QGraphicsRectItem;

class SessionApplet: public Applet
{
	Q_OBJECT
public:
	SessionApplet(PanelWindow* panelWindow);
	~SessionApplet();
	bool init();
	void clicked();
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
	void xmlReadIcon();
	void xmlReadIconLogout();
	void xmlReadIconHalt();
	void xmlReadIconReboot();
	void xmlReadIconHibernate();
	void xmlReadIconSuspend();
	void xmlReadCommandLogout();
	void xmlReadCommandHalt();
	void xmlReadCommandReboot();
	void xmlReadCommandHibernate();
	void xmlReadCommandSuspend();
	void xmlReadColorText();
	void xmlReadColorBackground();
	void xmlReadColorSelectedText();
	void xmlReadColorSelectedBackground();
	void xmlReadColorBorder();
	void xmlReadRadiusBorderTopLeft();
	void xmlReadRadiusBorderTopRight();
	void xmlReadRadiusBorderBottomLeft();
	void xmlReadRadiusBorderBottomRight();
	void xmlReadHeightBorder();

private slots:
	void logout();
	void halt();
	void reboot();
	void hibernate();
	void suspend();
	void buttonIconClicked();
	void buttonIconLogoutClicked();
	void buttonIconRebootClicked();
	void buttonIconHaltClicked();
	void buttonIconHibernateClicked();
	void buttonIconSuspendClicked();

private:
	void createActions();
	void createMenu();
	QGraphicsPixmapItem* m_pixmapItem;
	bool m_menuOpened;
	QColor m_colorText;
	QColor m_colorBackground;
	QColor m_colorSelectedText;
	QColor m_colorSelectedBackground;
	QColor m_colorBorder;
	QIcon m_icon;
	QIcon m_iconLogout;
	QIcon m_iconHalt;
	QIcon m_iconReboot;
	QIcon m_iconHibernate;
	QIcon m_iconSuspend;
	QString m_commandLogout;
	QString m_commandHalt;
	QString m_commandReboot;
	QString m_commandHibernate;
	QString m_commandSuspend;
	QMenu* m_menu;
	QAction* m_logout;
	QAction* m_halt;
	QAction* m_reboot;
	QAction* m_hibernate;
	QAction* m_suspend;
	QAction* m_separator1;
	QAction* m_separator2;
	int m_radiusBorderTopLeft;
	int m_radiusBorderTopRight;
	int m_radiusBorderBottomLeft;
	int m_radiusBorderBottomRight;
	int m_heightBorder;
	Ui_AppletSessionSettings* m_settingsUi;
};

#endif // SESSIONAPPLET_H
