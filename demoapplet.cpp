#include "demoapplet.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include "panelwindow.h"
#include "ui_appletdemosettings.h"
#include "panelapplication.h"

DemoApplet::DemoApplet(PanelWindow* panelWindow)
	: Applet(panelWindow), m_color(Qt::blue),
	m_rectItem(new QGraphicsRectItem(this)),
	m_settingsUi(new Ui::AppletDemoSettings())
{
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
}

DemoApplet::~DemoApplet()
{
	delete m_settingsUi;
}

bool DemoApplet::init()
{
	m_rectItem->setPen(QPen(Qt::NoPen));
	m_rectItem->setBrush(QBrush(m_color));
	setInteractive(true);
	return true;
}

void DemoApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("color", m_color.name());
	writer->writeEndElement();
}

void DemoApplet::showConfigurationDialog()
{
	QDialog dialog;
	QStringList colors = QColor::colorNames();
	QList <QColor> colorList;
	for (int i = 0; i < colors.size(); i++)
		colorList << QColor(colors.at(i));
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->color->addItems(colors);
	m_settingsUi->color->setCurrentIndex(colorList.indexOf(m_color));
	if(dialog.exec() == QDialog::Accepted) {
		m_color = QColor(m_settingsUi->color->currentText());
		m_rectItem->setBrush(QBrush(m_color));
		m_configChanged = true;
	}
}

void DemoApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}

void DemoApplet::layoutChanged()
{
	static const int delta = 8;
	m_rectItem->setRect(delta, delta, m_size.width() - 2*delta, m_size.height() - 2*delta);

}

QSize DemoApplet::desiredSize()
{
	return QSize(64, 64);
}

void DemoApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void DemoApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(isUnderMouse())
		showContextMenu(localToScreen(QPoint(0, 0)));
}

bool DemoApplet::xmlRead()
{
	if (!m_xmlConfigReader.xmlOpen()) {
#ifdef __DEBUG__
		MyDBG << "Error opening file.";
#else
		qDebug("Error opening file.");
#endif
		return false;
	}
	while (!m_xmlConfigReader.atEnd()) {
		if (m_xmlConfigReader.hasError()) {
			m_xmlConfigReader.xmlErrorString();
			m_xmlConfigReader.xmlClose();
			return false;
		}
		while (m_xmlConfigReader.readNextStartElement())
			if (m_xmlConfigReader.name() == "applet")
			while (m_xmlConfigReader.readNextStartElement())
				if (m_xmlConfigReader.name() == "type") 
					if (m_xmlConfigReader.readElementText() == getNameApplet())
						while (m_xmlConfigReader.readNextStartElement())
							if (m_xmlConfigReader.name() == "config")
								while (m_xmlConfigReader.readNextStartElement())
									if (m_xmlConfigReader.name() == "color")
										xmlReadColor();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void DemoApplet::xmlReadColor()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "color");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	m_color.setNamedColor(m_xmlConfigReader.readElementText());
}
