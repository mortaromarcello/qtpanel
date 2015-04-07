#include "volumeapplet.h"
#include <QtGui/QGraphicsScene>
#include <QSettings>
#include <QTimer>
#include <QProcess>
#include <QMenu>
#include <QFileDialog>
#include "panelwindow.h"
#include "dpisupport.h"
#include "mydebug.h"
#include "config.h"
#include "panelapplication.h"
#include "ui_appletvolumesettings.h"

VolumeApplet::VolumeApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_pixmapItem(new QGraphicsPixmapItem(this)),
	m_delta(adjustHardcodedPixelSize(4)),
	m_slider(new QSlider(Qt::Vertical)),
	m_mixer("x-terminal-emulator -e alsamixer"),
	m_useInternalIcon(true),
	m_settingsUi(new Ui::AppletVolumeSettings()),
	m_volume(100)
{
	m_slider->resize(QSize(19, 88));
	m_slider->setWindowFlags(Qt::FramelessWindowHint);
	m_slider->setStyleSheet("border: 1px solid black");
	m_slider->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
	QObject::connect(m_slider,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSlot(int)));
}

VolumeApplet::~VolumeApplet()
{
	delete m_pixmapItem;
	delete m_settingsUi;
}

bool VolumeApplet::init()
{
	setInteractive(true);
	if (!asoundInitialize()) {
#ifdef __DEBUG__
		MyDBG << "Error initialize sound";
#else
		qDebug("Error initialize sound");
#endif
	}
	if (snd_mixer_selem_set_playback_volume_all(m_elem, m_volume) < 0) {
#ifdef __DEBUG
		MyDBG << "can't get volume";
#else
		qDebug("can't get volume");
#endif
	}
	m_slider->setValue(m_volume);
	updateIcon();
	m_configChanged = true;
	return true;
}

void VolumeApplet::layoutChanged()
{
	m_pixmapItem->setOffset(m_delta, m_delta);
}

QSize VolumeApplet::desiredSize()
{
	return QSize(adjustHardcodedPixelSize(32), adjustHardcodedPixelSize(32));
}

void VolumeApplet::clicked()
{
	if (!m_slider->isVisible()) {
		if (m_panelWindow->verticalAnchor() == PanelWindow::Bottom)
		{
#ifdef __DEBUG__
			MyDBG << m_slider->rect() << m_slider->pos();
#endif
			m_slider->move(localToScreen(QPoint(m_size.width()/2-m_slider->width()/2, - m_slider->height())));
		}
		else
			m_slider->move(localToScreen(QPoint(m_size.width()/2-m_slider->width()/2, m_size.height())));
		m_slider->show();
	}
	else {
		m_slider->hide();
	}
#ifdef __DEBUG__
	MyDBG << "clicked, value=" << m_slider->value();
#endif
}

void VolumeApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.addAction(m_icon_mixer, tr("Launch mixer"), this, SLOT(launchMixer()));
	menu.exec(point);
}

void VolumeApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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
			showContextMenu(localToScreen(QPoint(0, 0)));
		}
	}
}

void VolumeApplet::showConfigurationDialog()
{
	QDialog dialog;
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->mixer->setText(m_mixer);
	QObject::connect(m_settingsUi->mixer, SIGNAL(textChanged(QString)), this, SLOT(mixerChanged(QString)));
	m_settingsUi->icon->setIcon(m_icon_mixer);
	m_settingsUi->icon->setText(m_icon_mixer.name());
	QObject::connect(m_settingsUi->icon, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
	if(dialog.exec() == QDialog::Accepted) {
		m_mixer = m_settingsUi->mixer->text();
		m_icon_mixer = m_settingsUi->icon->icon();
		m_configChanged = true;
	}
}

void VolumeApplet::valueChangedSlot(int value)
{
	if (snd_mixer_selem_set_playback_volume_all(m_elem, value) < 0)
#ifdef __DEBUG__
		MyDBG << "don't set volume";
#else
		qDebug("don't set volume");
#endif
	updateIcon();
}

void VolumeApplet::launchMixer()
{
	QProcess::startDetached(m_mixer);
}

void VolumeApplet::buttonIconClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/"+QIcon::themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QString name;
		name = QFileInfo(namefile).baseName();
		QIcon icon = QIcon::fromTheme(name);
		m_settingsUi->icon->setIcon(icon);
		m_settingsUi->icon->setText(icon.name());
	}
}

void VolumeApplet::mixerChanged(QString mixer)
{
	QIcon icon = QIcon::fromTheme(mixer, QIcon::fromTheme("none"));
	m_settingsUi->icon->setIcon(icon);
	m_settingsUi->icon->setText(icon.name());
}


bool VolumeApplet::asoundInitialize()
{
	/* Access the "default" device. */
	snd_mixer_selem_id_alloca(&m_sid);
	snd_mixer_open(&m_handle, 0);
	snd_mixer_attach(m_handle, "default");
	snd_mixer_selem_register(m_handle, NULL, NULL);
	snd_mixer_load(m_handle);

	/* Find Master element, or Front element, or PCM element, or LineOut element.
	* If one of these succeeds, master_element is valid. */
	if ( ! asoundFindElement("Master"))
		if ( ! asoundFindElement("Front"))
			if ( ! asoundFindElement("PCM"))
				if ( ! asoundFindElement("LineOut"))
					return FALSE;

	/* Set the playback volume range as we wish it. */
	snd_mixer_selem_set_playback_volume_range(m_elem, 0, 100);
	return TRUE;
}

bool VolumeApplet::asoundFindElement(const char * ename)
{
	for (
		m_elem = snd_mixer_first_elem(m_handle);
		m_elem != NULL;
		m_elem = snd_mixer_elem_next(m_elem))
	{
		snd_mixer_selem_get_id(m_elem, m_sid);
		if ((snd_mixer_selem_is_active(m_elem))
		&& (strcmp(ename, snd_mixer_selem_id_get_name(m_sid)) == 0))
			return TRUE;
	}
	return FALSE;
}

void VolumeApplet::updateIcon()
{
	m_volume = m_slider->value();
	if (m_volume == 0) {
		setIcon(m_pixmapItem, "audio-volume-muted");
	}
	if (m_volume >= 1 && m_volume <= 33) {
		setIcon(m_pixmapItem, "audio-volume-low");
	}
	if (m_volume >= 34 && m_volume <= 66) {
		setIcon(m_pixmapItem, "audio-volume-medium");
	}
	if (m_volume >= 67) {
		setIcon(m_pixmapItem, "audio-volume-high");
	}
}

bool VolumeApplet::xmlRead()
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
									if (m_xmlConfigReader.name() == "mixer")
										xmlReadMixer();
									else if (m_xmlConfigReader.name() == "icon-mixer")
										xmlReadIconMixer();
									else if (m_xmlConfigReader.name() == "volume")
										xmlReadVolume();
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void VolumeApplet::xmlReadMixer()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "mixer");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_mixer = m_xmlConfigReader.readElementText();
}

void VolumeApplet::xmlReadIconMixer()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon-mixer");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_icon_mixer = QIcon::fromTheme(m_xmlConfigReader.readElementText(), QIcon::fromTheme("none"));
}

void VolumeApplet::xmlReadVolume()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "volume");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_volume = m_xmlConfigReader.readElementText().toLong();
}

void VolumeApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("mixer", m_mixer);
	writer->writeTextElement("icon-mixer", m_icon_mixer.name());
	writer->writeTextElement("volume", QString("%1").arg(m_volume));
	writer->writeEndElement();
}

void VolumeApplet::setIcon(QGraphicsPixmapItem* item, const QString &icon, QIcon::Mode mode, int extent, const QString &ext)
{
	QString temp = ext;
	if (temp.isEmpty())
		temp = "svg";
	else {
		if (temp[0] == '.')
			temp = temp.remove(0,1);
		}
	if (!m_useInternalIcon)
		QIcon::hasThemeIcon(icon) ? item->setPixmap(QIcon::fromTheme(icon).pixmap(extent, mode)) : item->setPixmap(QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + icon + "." + temp).pixmap(extent, mode));
	else
		item->setPixmap(QIcon(QString(qtpanel_IMAGES_TARGET) + "/" + icon + "." + temp).pixmap(extent, mode));
}
