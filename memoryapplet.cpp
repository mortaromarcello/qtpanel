#include "memoryapplet.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <sys/sysinfo.h>
#include "textgraphicsitem.h"
#include "panelwindow.h"
#include "ui_appletmemorysettings.h"
#include "panelapplication.h"

SysInfo::SysInfo()
{
	struct sysinfo info;
	if (!sysinfo(&info)) {
		m_totalRam = info.totalram * (unsigned long long) info.mem_unit / 1024;
		m_freeRam = info.freeram * (unsigned long long) info.mem_unit / 1024;
		m_sharedRam = info.sharedram * (unsigned long long) info.mem_unit / 1024;
		m_bufferRam = info.bufferram * (unsigned long long) info.mem_unit / 1024;
		m_totalSwap = info.totalswap * (unsigned long long) info.mem_unit / 1024;
		m_freeSwap = info.freeswap * (unsigned long long) info.mem_unit / 1024;
		m_totalHigh = info.totalhigh * (unsigned long long) info.mem_unit / 1024;
		m_freeHigh = info.freehigh * (unsigned long long) info.mem_unit / 1024;
	}
	else qWarning("error reading sysinfo");
}

SysInfo::~SysInfo()
{
}

MemoryApplet::MemoryApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_color(Qt::blue),
	m_textItem(new TextGraphicsItem(this)),
	m_settingsUi(new Ui::AppletMemorySettings())
{
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
}

MemoryApplet::~MemoryApplet()
{
	delete m_textItem;
	delete m_settingsUi;
}

bool MemoryApplet::init()
{
	setInteractive(true);
	m_textItem->setText(QString("%1M").arg(SysInfo().freeRam()/1024));
	m_textItem->setColor(m_color);
	m_textItem->setFont(PanelApplication::instance()->panelFont());
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
	m_timer.start(1000);
	return true;
}

void MemoryApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("color", m_color.name());
	writer->writeEndElement();
}

void MemoryApplet::showConfigurationDialog()
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
		m_textItem->setColor(m_color);
		m_configChanged = true;
	}
}

void MemoryApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}

void MemoryApplet::layoutChanged()
{
	m_textItem->setPos((m_size.width() - m_textItem->boundingRect().size().width())/2.0, m_panelWindow->textBaseLine());
	update();
}

QSize MemoryApplet::desiredSize()
{
	return QSize(m_textItem->boundingRect().width() + 8, m_textItem->boundingRect().height() + 8);
	//return QSize(64, 32);
}

void MemoryApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void MemoryApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (isUnderMouse())
		showContextMenu(localToScreen(QPoint(0, 0)));
}

bool MemoryApplet::xmlRead()
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

void MemoryApplet::xmlReadColor()
{
	Q_ASSERT(m_xmlReader.isStartElement() && m_xmlReader.name() == "color");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name();
#endif
	m_color.setNamedColor(m_xmlConfigReader.readElementText());
}

void MemoryApplet::update()
{
	SysInfo info;
	m_textItem->setText(QString("%1M").arg(info.freeRam()/1024));
	m_textToolTip = QString(tr(	"Total Ram:%1\n"
								"Free Ram:%2\n"
								"Shared Ram:%3\n"
								"Buffer Ram:%4\n"
								"Total Swap:%5\n"
								"Free Swap:%6\n"
								"Total High:%7\n"
								"Free High:%8"))
								.arg(info.totalRam()/1024)
								.arg(info.freeRam()/1024)
								.arg(info.sharedRam()/1024)
								.arg(info.bufferRam()/1024)
								.arg(info.totalSwap()/1024)
								.arg(info.freeSwap()/1024)
								.arg(info.totalHigh()/1024)
								.arg(info.freeHigh()/1024);
}
