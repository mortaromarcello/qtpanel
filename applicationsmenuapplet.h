#ifndef APPLICATIONSMENUAPPLET_H
#define APPLICATIONSMENUAPPLET_H

#include <QList>
#include <QMap>
#include <QAction>
#include <QIcon>
#include <QCleanlooksStyle>
#include "applet.h"

class Ui_AppletApplicationsMenuSettings;

class ApplicationsMenuStyle: public QCleanlooksStyle
{
	Q_OBJECT
public:
	int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const;
};

class SubMenu
{
public:
	SubMenu()
	{
	}

	SubMenu(QMenu* parent, const QString& title, const QString& category, const QString& icon);

	QMenu* menu()
	{
		return m_menu;
	}

	const QString& category() const
	{
		return m_category;
	}

private:
	QMenu* m_menu;
	QString m_category;
	QIcon m_icon;
};

class TextGraphicsItem;
class DesktopApplication;

class ApplicationsMenuApplet: public Applet
{
	Q_OBJECT
public:
	ApplicationsMenuApplet(PanelWindow* panelWindow);
	~ApplicationsMenuApplet();

	bool init();
	QSize desiredSize();
	void clicked();
	void xmlWrite(XmlConfigWriter* writer);
	void showContextMenu(const QPoint& point);

public slots:
	void showConfigurationDialog();

protected:
	void layoutChanged();
	bool isHighlighted();
	bool xmlRead();
	void xmlReadMenuIcon();
	void xmlReadOpacity();
	void xmlReadBackgroundMenuColor();
	void xmlReadTextMenuColor();
	void xmlReadSelectedMenuColor();
	void xmlReadBackgroundSelectedMenuColor();
    void xmlReadMenuBorderRadiusTopLeft();
    void xmlReadMenuBorderRadiusTopRight();
    void xmlReadMenuBorderRadiusBottomLeft();
    void xmlReadMenuBorderRadiusBottomRight();
    void xmlReadMenuBorderWidth();
    void xmlReadMenuBorderColor();
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void setOpacity(qreal opacity);

private slots:
	void actionTriggered();
	void applicationUpdated(const DesktopApplication& app);
	void applicationRemoved(const QString& path);
	void buttonIconClicked();

private:
	ApplicationsMenuStyle m_style;
	TextGraphicsItem* m_textItem;
	bool m_menuOpened;
	QMenu* m_menu;
	QList<SubMenu> m_subMenus;
	QMap<QString, QAction*> m_actions;
	//
	QGraphicsPixmapItem* m_pixmapItem;
	QIcon m_icon;
	qreal m_opacity;
	QColor m_backgroundMenuColor;
	QColor m_textMenuColor;
	QColor m_selectedMenuColor;
	QColor m_backgroundSelectedMenuColor;
    QColor m_menuBorderColor;
    int m_menuBorderWidth;
    int m_menuBorderRadiusTopLeft;
    int m_menuBorderRadiusTopRight;
    int m_menuBorderRadiusBottomLeft;
    int m_menuBorderRadiusBottomRight;
	Ui_AppletApplicationsMenuSettings *m_settingsUi;
	//
};

#endif
