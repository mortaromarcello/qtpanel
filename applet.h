#ifndef APPLET_H
#define APPLET_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QGraphicsItem>
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"

class PanelWindow;

class Applet: public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	Applet(PanelWindow* panelWindow);
	~Applet();

	virtual bool init();

	void setPosition(const QPoint& position);
	void setSize(const QSize& size);

	const QSize& size() const
	{
		return m_size;
	}

	virtual QSize desiredSize() = 0;

	PanelWindow* panelWindow()
	{
		return m_panelWindow;
	}

	void setInteractive(bool interactive);

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	static int position;
	//
	virtual void xmlWrite(XmlConfigWriter*){}
	virtual void showContextMenu() {}
	bool isConfigChanged() const {return m_configChanged;}
	QString getNameApplet() const {return this->metaObject()->className();}
	static QColor highlightColor() {return m_highlightColor;}
	static qreal radiusInc() {return m_radiusInc;}
	static void setHighlightColor(QColor color) {m_highlightColor = color;}
	static void setRadiusInc(qreal radius) {m_radiusInc = radius;}
	const bool configChanged() const {return m_configChanged;}
	void setConfigChanged(bool configChanged) {m_configChanged = configChanged;}
	//

public slots:
	void animateHighlight();
	virtual void clicked();
	virtual void showConfigurationDialog() {}

protected:
	virtual void layoutChanged();
	QPoint localToScreen(const QPoint& point);

	virtual bool isHighlighted();

//
	void showToolTip();
	void hideToolTip();
//
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

	PanelWindow* m_panelWindow;
	QPoint m_position;
	QSize m_size;
	bool m_interactive;
	qreal m_highlightIntensity;
	//
	QString m_textToolTip;
	XmlConfigReader m_xmlConfigReader;
	bool m_configChanged;
	static QColor m_highlightColor;
	static qreal m_radiusInc;
	//
};

#endif
