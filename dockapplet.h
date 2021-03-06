#ifndef DOCKAPPLET_H
#define DOCKAPPLET_H

#include <QVector>
#include <QMap>
#include <QIcon>
#include <QGraphicsItem>
//
#include <QLabel>
//
#include "applet.h"

class QGraphicsPixmapItem;
class TextGraphicsItem;
class DockApplet;
class Client;

// Item di una finestra. Deve avere un Client*.
class DockItem: public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	DockItem(DockApplet* dockApplet, Client* client);
	~DockItem();

	void updateContent();
	void setTargetPosition(const QPoint& targetPosition);
	void setTargetSize(const QSize& targetSize);
	void moveInstantly();
	void startAnimation();
	// ritorna il puntatore al Client
	Client* client() {return m_client;}

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

public slots:
	void animate();
	void close();
	void maximize();
	void demaximize();

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
	void updateClientIconGeometry();
	bool isUrgent();

	QTimer* m_animationTimer;
	DockApplet* m_dockApplet;
	TextGraphicsItem* m_textItem;
	QGraphicsPixmapItem* m_iconItem;
	Client* m_client;
	QPoint m_position;
	QPoint m_targetPosition;
	QSize m_size;
	QSize m_targetSize;
	qreal m_highlightIntensity;
	qreal m_urgencyHighlightIntensity;
	bool m_dragging;
	QPointF m_mouseDownPosition;
	QPoint m_dragStartPosition;
};

// Used for tracking connected windows (X11 clients).
// Client may have it's DockItem, but not necessary (for example, special windows are not shown in dock).
class Client
{
public:
	Client(unsigned long handle);
	~Client();
	// ritorna l'handle alla finestra
	unsigned long handle() const {return m_handle;}
	// è visibile?
	bool isVisible() {return m_visible;}
	// ritorna il nome della finestra
	const QString& name() const {return m_name;}
	// ritorna l'icona della finestra;
	const QIcon& icon() const {return m_icon;}
	// è urgente?
	bool isUrgent() const {return m_isUrgent;}
	// è attiva?
	bool isActive();
	// ritorna il desktop della finestra
	int getDesktop();
	// attiva la finestra
	void activate();
	// gestisce i messaggi alla finestra
	void windowPropertyChanged(unsigned long atom);

private:
	void updateVisibility();
	void updateName();
	void updateIcon();
	void updateUrgency();
	unsigned long m_handle;
	QString m_name;
	QIcon m_icon;
	bool m_isUrgent;
	bool m_visible;
};

class DockApplet: public Applet
{
	Q_OBJECT
public:
	DockApplet(PanelWindow* panelWindow);
	~DockApplet();

	bool init();
	QSize desiredSize();
	void registerDockItem(DockItem* dockItem);
	void unregisterDockItem(DockItem* dockItem);
	void updateLayout();
	void draggingStarted();
	void draggingStopped();
	void moveItem(DockItem* dockItem, bool right);

protected slots:
	void layoutChanged();

private slots:
	void windowPropertyChanged(unsigned long window, unsigned long atom);

private:
	// aggiorna la lista delle finestre
	void updateClientList();
	// ritorna il puntatore al client della lista oppure NULL se non c'è
	Client* getClientFromDockItems(unsigned long handle);
	DockItem* getDockItem(unsigned long handle);
	// lista delle dockitem
	QVector<DockItem*> m_dockItems;
	bool m_dragging;
};

#endif
