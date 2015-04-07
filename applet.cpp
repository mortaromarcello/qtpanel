#include "applet.h"

#include <QTimer>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
//
#include <QToolTip>
//
#include "panelwindow.h"
#include "animationutils.h"
#include "config.h"

qreal Applet::m_radiusInc = 0.0;
QColor Applet::m_highlightColor(255, 255, 255, 0);

Applet::Applet(PanelWindow* panelWindow)
	: m_panelWindow(panelWindow),
	m_position(QPoint()),
	m_size(QSize()),
	m_highlightIntensity(0.0),
	m_interactive(false),
	m_textToolTip(QString()),
	m_xmlConfigReader(qtpanel_APPLETS_FILE_CONFIG),
	m_configChanged(false)
{
	setZValue(-1.0);
	setAcceptedMouseButtons(Qt::RightButton);
	setParentItem(m_panelWindow->panelItem());
}

Applet::~Applet()
{
}

bool Applet::init()
{
	m_panelWindow->updateLayout();
	return true;
}

void Applet::setPosition(const QPoint& position)
{
	m_position = position;
	setPos(m_position);
}

void Applet::setSize(const QSize& size)
{
	m_size = size;
	layoutChanged();
}

void Applet::setInteractive(bool interactive)
{
	m_interactive = interactive;

	if(m_interactive)
	{
		setAcceptsHoverEvents(true);
		setAcceptedMouseButtons(Qt::RightButton | Qt::LeftButton);
	}
	else
	{
		setAcceptsHoverEvents(false);
		setAcceptedMouseButtons(Qt::RightButton);
	}
}

QRectF Applet::boundingRect() const
{
	return QRectF(0.0, 0.0, m_size.width(), m_size.height());
}

void Applet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	if(m_size.width() < 32)
		return; // Too small to draw a background (don't want to deal with weird corner cases).

	if(!m_interactive)
		return; // Currently, background is only used for highlight on interactive applets.

	painter->setPen(Qt::NoPen);
	//qreal radius = (m_size.width() * m_size.width() + m_size.height() * m_size.height()) / (4.0 * m_size.height());
	qreal radius = m_size.width() / 2.0;
	QPointF center(m_size.width() / 2.0, m_size.height() + radius - m_size.height() / 2.0);
	//static const qreal radiusInc = 10.0;
	//static const qreal radiusInc = 6.0;
	QRadialGradient gradient(center, radius + m_radiusInc, center);
	//
	//gradient.setSpread(QGradient::PadSpread);
	//gradient.setSpread(QGradient::ReflectSpread);
	//
	//QColor highlightColor(255, 255, 255, static_cast<int>(150 * m_highlightIntensity));
	QColor highlightColor(m_highlightColor.red(), m_highlightColor.green(), m_highlightColor.blue(), static_cast<int>(150 * m_highlightIntensity));
	//QColor highlightColor(200, 150, 20, static_cast<int>(150 * m_highlightIntensity));
	gradient.setColorAt(0.0, highlightColor);
	gradient.setColorAt((radius - m_size.height() / 2.0) / (radius + m_radiusInc), highlightColor);
	gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
	painter->setBrush(QBrush(gradient));
	painter->drawRect(boundingRect());
}

void Applet::animateHighlight()
{
	static const qreal highlightAnimationSpeed = 0.15;
	qreal targetIntensity = isHighlighted() ? 1.0 : 0.0;
	bool needAnotherStep = false;
	m_highlightIntensity = AnimationUtils::animate(m_highlightIntensity, targetIntensity, highlightAnimationSpeed, needAnotherStep);
	if(needAnotherStep)
		QTimer::singleShot(20, this, SLOT(animateHighlight()));
	update();
}

void Applet::clicked()
{
}

void Applet::layoutChanged()
{
}

QPoint Applet::localToScreen(const QPoint& point)
{
	return m_panelWindow->pos() + m_position + point;
}

bool Applet::isHighlighted()
{
	return isUnderMouse();
}
//
void Applet::showToolTip()
{
	QToolTip::showText(localToScreen(QPoint(0, (m_panelWindow->verticalAnchor() == PanelWindow::Bottom) ? 0 : 16)), m_textToolTip);
}

void Applet::hideToolTip()
{
	QToolTip::hideText();
}
//
void Applet::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	showToolTip();
	animateHighlight();
}

void Applet::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	hideToolTip();
	animateHighlight();
}

void Applet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void Applet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(isUnderMouse())
	{
		if(event->button() == Qt::LeftButton)
		{
			// FIXME: Workaround.
			// For some weird reason, if clicked() function is called directly, and menu is opened,
			// this item will receive hover enter event on menu close. But it shouldn't (mouse is outside).
			// Probably somehow related to taking a mouse grab when one is already active.
			QTimer::singleShot(1, this, SLOT(clicked()));
		}
		if(event->button() == Qt::RightButton)
		{
			m_panelWindow->showPanelContextMenu(m_position + QPoint(static_cast<int>(event->pos().x()), static_cast<int>(event->pos().y())));
		}
	}
}
