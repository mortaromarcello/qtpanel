#include <QDialog>
#include <QGraphicsScene>
#include <QMenu>
#include <QTimer>
#include <QX11Info>
#include <math.h>
#include <QFileDialog>
#include "panelwindow.h"
#include "dpisupport.h"
#include "panelapplication.h"
#include "ui_appletbacklightsettings.h"
#include "backlightapplet.h"


#define BRIGHTNESS_MAX  2.0

BacklightApplet::BacklightApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_pixmapItem(new QGraphicsPixmapItem(this)),
	m_delta(adjustHardcodedPixelSize(4)),
	m_slider(new QSlider(Qt::Vertical)),
	m_display(QX11Info::display()),
	m_screen(QX11Info::appScreen()),
	m_settingsUi(new Ui::AppletBacklightSettings())
{
	m_slider->resize(QSize(19, 88));
	m_slider->setWindowFlags(Qt::FramelessWindowHint);
	m_slider->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
	m_slider->setStyleSheet("border: 1px solid black");
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	if (m_icon.name().isEmpty())
		m_icon = QIcon::fromTheme("none");
	m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	getGamma();
	connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSlot(int)));
}

BacklightApplet::~BacklightApplet()
{
	delete m_pixmapItem;
	delete m_settingsUi;
}

bool BacklightApplet::init()
{
	setInteractive(true);
	m_slider->setValue(doubleToInt(m_gamma.red, BRIGHTNESS_MAX));
	return true;
}

void BacklightApplet::layoutChanged()
{
	m_pixmapItem->setOffset(m_delta, m_delta);
}

QSize BacklightApplet::desiredSize()
{
	return QSize(adjustHardcodedPixelSize(32), adjustHardcodedPixelSize(32));
}

void BacklightApplet::clicked()
{
	static bool showned = false;
	if (!showned) {
		if (m_panelWindow->verticalAnchor() == PanelWindow::Bottom) //Bottom
		{
#ifdef __DEBUG__
			MyDBG << m_slider->rect() << m_slider->pos();
#endif
			m_slider->move(localToScreen(QPoint(m_size.width()/2-m_slider->width()/2, - m_slider->height())));
		}
		else
			m_slider->move(localToScreen(QPoint(m_size.width()/2-m_slider->width()/2, m_size.height())));
		m_slider->show();
		showned = true;
	}
	else {
		m_slider->hide();
		showned = false;
	}
}

void BacklightApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}

void BacklightApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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
			//QProcess process;
			//process.startDetached(m_mixer);
			showContextMenu(localToScreen(QPoint(0, 0)));
		}
	}
}

void BacklightApplet::showConfigurationDialog()
{
	QDialog dialog;
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->buttonIcon->setIcon(m_icon);
	m_settingsUi->buttonIcon->setText(m_icon.name());
	QObject::connect(m_settingsUi->buttonIcon, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
	if(dialog.exec() == QDialog::Accepted) {
		m_icon = m_settingsUi->buttonIcon->icon();
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
		m_configChanged = true;
	}
}

void BacklightApplet::valueChangedSlot(int value)
{
	double d_value = intToDouble(value, BRIGHTNESS_MAX);
	setGamma(d_value);
}

void BacklightApplet::buttonIconClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QString name;
		name = QFileInfo(namefile).baseName();
		QIcon icon = QIcon::fromTheme(name);
		m_settingsUi->buttonIcon->setIcon(icon);
		m_settingsUi->buttonIcon->setText(icon.name());
	}
}

bool BacklightApplet::xmlRead()
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
									if (m_xmlConfigReader.name() == "icon")
										xmlReadIcon();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void BacklightApplet::xmlReadIcon()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_icon = QIcon::fromTheme(m_xmlConfigReader.readElementText());
}

void BacklightApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("icon", m_icon.name());
	writer->writeEndElement();
}

bool BacklightApplet::getGamma()
{
	if (!XF86VidModeGetGamma(m_display, m_screen, &m_gamma)) {
		qDebug("backlight: can't get gamma.");
		return false;
	}
	return true;
}

void BacklightApplet::setGamma(double brightness)
{
	m_gamma.red = brightness;
	m_gamma.green = m_gamma.red;
	m_gamma.blue = m_gamma.red;
	
	if (!XF86VidModeSetGamma(m_display, m_screen, &m_gamma)) {
		qDebug("backlight: can't set gamma.");
		return;
	}
	//qDebug("backlight: gamma.red=%f, gamma.green=%f, gamma.blue=%f\n", m_gamma.red, m_gamma.green, m_gamma.blue);
}

int BacklightApplet::doubleToInt(double value, double max)
{
	return (int) (value * 100 / max)+((fmodf(value*100, max) > 0.5) ? 1 : 0);
}

double BacklightApplet::intToDouble(int value, double max)
{
	return (((double) value) * max / 100);
}
