#ifndef DOCKLAUNCHERAPPLET_H
#define DOCKLAUNCHERAPPLET_H

#include <QVector>
#include <QMap>
#include <QIcon>
#include <QGraphicsItem>
//
#include <QLabel>
#include <QProcess>
//
#include "applet.h"
class Ui_LauncherSettings;

class QGraphicsPixmapItem;
class TextGraphicsItem;
class DockLauncherApplet;
class Launcher;

// Item di una finestra. Deve avere un Launcher*.
class DockLauncherItem: public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	DockLauncherItem(DockLauncherApplet* dockLauncherApplet, Launcher* launcher);
	~DockLauncherItem();

	void updateContent();
	void setTargetPosition(const QPoint& targetPosition);
	void setTargetSize(const QSize& targetSize);
	void moveInstantly();
	void startAnimation();
	// ritorna il puntatore al Launcher
	Launcher* launcher() {return m_launcher;}
	const QSize& size() const {return m_size;}

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

public slots:
	void animate();
	//void close();
	void buttonIconClicked();
	void iconChanged(QString nameicon);
	void buttonCommandClicked();
	void commandChanged(QString command);
	void configure();
	void add();
	void remove();

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
	//void updateLauncherIconGeometry();
	//bool isUrgent();

	QTimer* m_animationTimer;
	DockLauncherApplet* m_dockLauncherApplet;
	//TextGraphicsItem* m_textItem;
	QGraphicsPixmapItem* m_iconItem;
	Launcher* m_launcher;
	QPoint m_position;
	QPoint m_targetPosition;
	QSize m_size;
	QSize m_targetSize;
	qreal m_highlightIntensity;
	qreal m_urgencyHighlightIntensity;
	bool m_dragging;
	QPointF m_mouseDownPosition;
	QPoint m_dragStartPosition;
	Ui_LauncherSettings *m_settingsUi;
};

// Used for tracking connected windows (X11 clients).
// Launcher may have it's DockLauncherItem, but not necessary (for example, special windows are not shown in dock).
class Launcher
{
public:
	Launcher(const QIcon& icon, const QString& command, const QString& tooltip);
	Launcher() {Launcher(QIcon(), QString(), QString());}
	~Launcher();
	const QString& command() const {return m_command;}
	void setCommand(const QString& command) {m_command = command;}
	// ritorna l'icona della finestra;
	const QIcon& icon() const {return m_icon;}
	void setIcon(const QIcon& icon) {m_icon = icon;}
	const QString& tooltip() {return m_tooltip;}
	void setTooltip(const QString& tooltip) {m_tooltip = tooltip;}
	const QString& iconNameFile() const {return m_iconNameFile;}
	void setIconNameFile(const QString& iconNameFile) {m_iconNameFile = iconNameFile;}
	void runCommand() {QProcess::startDetached(m_command);}

private:
	QString m_command;
	QIcon m_icon;
	QString m_tooltip;
	QString m_iconNameFile;
};

class DockLauncherApplet: public Applet
{
	Q_OBJECT
public:
	DockLauncherApplet(PanelWindow* panelWindow);
	~DockLauncherApplet();

	bool init();
	QSize desiredSize();
	void registerDockLauncherItem(DockLauncherItem* dockLauncherItem, DockLauncherItem* prev = 0);
	void unregisterDockLauncherItem(DockLauncherItem* dockLauncherItem);
	void updateLayout();
	void draggingStarted();
	void draggingStopped();
	void moveItem(DockLauncherItem* dockLauncherItem, bool right);
	void xmlWrite(XmlConfigWriter* writer);

protected slots:
	void layoutChanged();

protected:
	bool xmlRead();
	void xmlReadIcon(DockLauncherItem* item);
	void xmlReadCommand(DockLauncherItem* item);
	void xmlReadTooltip(DockLauncherItem* item);

private slots:
	//void windowPropertyChanged(unsigned long window, unsigned long atom);

private:
	// aggiorna la lista delle finestre
	//void updateLauncherList();
	// ritorna il puntatore al client della lista oppure NULL se non c'è
	Launcher* getLauncherFromDockLauncherItems(int handle);
	DockLauncherItem* getDockLauncherItem(int handle);
	// lista delle dockitem
	QVector<DockLauncherItem*> m_dockLauncherItems;
	bool m_dragging;
};

#endif // DOCKLAUNCHERAPPLET_H
