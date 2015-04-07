#ifndef PANELWINDOW_H
#define PANELWINDOW_H

#include <QVector>
#include <QWidget>
#include <QGraphicsItem>
#include <QSettings>
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"

class QFont;
class QGraphicsScene;
class QGraphicsView;
class Applet;
class PanelWindow;

class PanelWindowGraphicsItem: public QGraphicsItem
{
public:
	PanelWindowGraphicsItem(PanelWindow* panelWindow);
	~PanelWindowGraphicsItem();

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
private:
	PanelWindow* m_panelWindow;
};

class PanelWindow: public QWidget
{
	Q_OBJECT
public:
	PanelWindow();
	~PanelWindow();

	enum Anchor
	{
		Top,
		Bottom
	};

	enum LayoutPolicy
	{
		Normal,
		AutoSize,
		FillSpace
	};

	bool init();

	bool dockMode() const
	{
		return m_dockMode;
	}

	void setDockMode(bool dockMode);
	int screen() const {return m_screen;}
	void setScreen(int screen);
	QSettings* trolltechConf() const {return m_trolltechConf;}
	Anchor verticalAnchor() const {return m_verticalAnchor;}
	void setVerticalAnchor(Anchor verticalAnchor);
	LayoutPolicy layoutPolicy() const {return m_layoutPolicy;}
	void setLayoutPolicy(LayoutPolicy layoutPolicy);
	void updatePosition();
	const QFont& font() const;
	int textBaseLine();
	PanelWindowGraphicsItem* panelItem() {return m_panelItem;}
	void resizeEvent(QResizeEvent* event);
	void updateLayout();
	void showPanelContextMenu(const QPoint& point);
	bool xmlRead();
	void xmlReadType();
	void xmlWrite();
	bool isConfigAppletsChanged();

public slots:
	void showConfigurationDialog();

protected:
	XmlConfigWriter m_xmlConfigWriter;
private:
	bool m_dockMode;
	int m_screen;
	Anchor m_verticalAnchor;
	LayoutPolicy m_layoutPolicy;

	QGraphicsScene* m_scene;
	QGraphicsView* m_view;
	PanelWindowGraphicsItem* m_panelItem;
	QVector<Applet*> m_applets;
	XmlConfigReader m_xmlConfigReader;
	//
	QSettings* m_trolltechConf;
	//
};

#endif
