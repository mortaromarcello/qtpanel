#include "launcherapplet.h"

#include <QSettings>
#include <QGraphicsSceneMouseEvent>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QToolTip>
#include <QMenu>
#include "panelwindow.h"
#include "dpisupport.h"
#include "ui_appletlaunchersettings.h"
#include "mydebug.h"
#include "config.h"
#include "panelapplication.h"

static unsigned int instance = 0;	// Numero di instanza dell'applet

LauncherApplet::LauncherApplet(PanelWindow* panelWindow)
	: Applet(panelWindow),
	m_pixmapItem(new QGraphicsPixmapItem(this)),
	m_delta(adjustHardcodedPixelSize(4)),
	m_icon(QIcon()),
	m_iconNameFile(QString()),
	m_instance(instance),
	m_settingsUi(new Ui::AppletLauncherSettings())
{
	++instance;
#ifdef __DEBUG__
	MyDBG << "instance =" << instance;
#endif
	if (!xmlRead()) {
		qWarning("Don't read configuration applet file.");
	}
}

LauncherApplet::~LauncherApplet()
{
	delete m_pixmapItem;
	delete m_settingsUi;
	instance = 0;
}

bool LauncherApplet::init()
{
	setInteractive(true);
	m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
	return true;
}

const unsigned int LauncherApplet::getLastInstance()
{
	return instance;
}

void LauncherApplet::layoutChanged()
{
	m_pixmapItem->setOffset(m_delta, m_delta);
}

QSize LauncherApplet::desiredSize()
{
	return QSize(adjustHardcodedPixelSize(32), adjustHardcodedPixelSize(32));
}

void LauncherApplet::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
#ifdef __DEBUG__
	MyDBG << "MousePressEvent";
#endif
	if (event->buttons() == Qt::RightButton)
		showContextMenu(localToScreen(QPoint(0, 0)));
	if (event->buttons() == Qt::LeftButton) {
		m_pixmapItem->setOffset(m_delta + 1, m_delta +1);
		QProcess::startDetached(m_command);
	}
}

void LauncherApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
#ifdef __DEBUG__
	MyDBG << "MouseReleaseEvent";
#endif
	m_pixmapItem->setOffset(m_delta, m_delta);
}

bool LauncherApplet::xmlRead()
{
	int _instance = 0;
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
								while (m_xmlConfigReader.readNextStartElement()) {
									if (m_xmlConfigReader.name() == "instance")
										_instance = xmlReadInstance();
									if (_instance == instance) {
										if(m_xmlConfigReader.name() == "icon")
											xmlReadIcon();
										else if (m_xmlConfigReader.name() == "command")
											xmlReadCommand();
										else if (m_xmlConfigReader.name() == "tooltip")
											xmlReadTooltip();
										m_instance = _instance;
									}
								}
	}
	m_xmlConfigReader.xmlClose();
	return true;
}

void LauncherApplet::showConfigurationDialog()
{
	QDialog dialog;
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->command->setText(m_command);
	QObject::connect(m_settingsUi->command, SIGNAL(textChanged(QString)), this, SLOT(commandChanged(QString)));
	m_settingsUi->buttonCommand->setIcon(QIcon::fromTheme("document-open"));
	QObject::connect(m_settingsUi->buttonCommand, SIGNAL(clicked()), this, SLOT(buttonCommandClicked()));
	m_settingsUi->tooltip->setText(m_textToolTip);
	if (!m_icon.name().isEmpty())
		m_settingsUi->icon->setText(m_icon.name());
	else {
		if (!m_iconNameFile.isEmpty())
			m_settingsUi->icon->setText(m_iconNameFile);
	}
	QObject::connect(m_settingsUi->icon, SIGNAL(textChanged(QString)), this, SLOT(iconChanged(QString)));
	m_settingsUi->buttonIcon->setIcon(m_icon);
	m_settingsUi->buttonIcon->setText(m_icon.name());
	QObject::connect(m_settingsUi->buttonIcon, SIGNAL(clicked()), this, SLOT(buttonIconClicked()));
	if(dialog.exec() == QDialog::Accepted) {
		m_command = m_settingsUi->command->text();
		m_icon = m_settingsUi->buttonIcon->icon();
		m_textToolTip = m_settingsUi->tooltip->text();
		m_pixmapItem->setPixmap(m_icon.pixmap(adjustHardcodedPixelSize(24), adjustHardcodedPixelSize(24)));
		m_configChanged = true;
	}
}

void LauncherApplet::buttonIconClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load Icon"), "/usr/share/icons/" + m_icon.themeName(), tr("Icons (*.png *.svg *.xpm *.ico)"));
	if (!namefile.isNull()) {
		QString name;
		QIcon icon;
		name = QFileInfo(namefile).baseName();
		if (QIcon::hasThemeIcon(name)) {
			icon = QIcon::fromTheme(name);
			m_settingsUi->icon->setText(name);
			m_iconNameFile = QString();
		}
		else {
			icon = QIcon(QPixmap(namefile));
			m_settingsUi->icon->setText(namefile);
			m_iconNameFile = namefile;
		}
		m_settingsUi->buttonIcon->setIcon(icon);
		m_settingsUi->buttonIcon->setText(icon.name());
	}
#ifdef __DEBUG__
	MyDBG << "Name icon:"<< namefile;
#endif
}

void LauncherApplet::iconChanged(QString nameicon)
{
	QIcon icon;
	if (QIcon::hasThemeIcon(nameicon)) {
		icon = QIcon::fromTheme(nameicon);
		m_iconNameFile = QString();
	}
	else if (QFileInfo(nameicon).exists()) {
		icon = QIcon(QPixmap(nameicon));
		m_iconNameFile = nameicon;
	}
	else
		icon = QIcon::fromTheme("none");
	m_settingsUi->buttonIcon->setIcon(icon);
}

void LauncherApplet::buttonCommandClicked()
{
	QString namefile = QFileDialog::getOpenFileName(NULL, tr("Load command"), QDir::currentPath(), tr("Commands (*)"));
	if (!namefile.isNull()) {
		m_settingsUi->command->setText(namefile);
	}
#ifdef __DEBUG__
	MyDBG << "Command:"<< namefile;
#endif
}

void LauncherApplet::commandChanged(QString command)
{
	QIcon icon = QIcon::fromTheme(QFileInfo(command).fileName(), QIcon::fromTheme("none"));
	m_settingsUi->buttonIcon->setIcon(icon);
	//m_settingsUi->buttonIcon->setText(icon.name());
	m_settingsUi->icon->setText(icon.name());
	
}

int LauncherApplet::xmlReadInstance()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "instance");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	return m_xmlConfigReader.readElementText().toInt();
}

void LauncherApplet::xmlReadIcon()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "icon");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	QString nameIcon = m_xmlConfigReader.readElementText();
	if (QIcon::hasThemeIcon(nameIcon))
		m_icon = QIcon::fromTheme(nameIcon);
	else {
		m_iconNameFile = nameIcon;
		m_icon = QIcon(QPixmap(m_iconNameFile));
	}
#ifdef __DEBUG__
	MyDBG << m_icon.name();
#endif
}

void LauncherApplet::xmlReadCommand()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "command");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_command = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_command;
#endif
}

void LauncherApplet::xmlReadTooltip()
{
	Q_ASSERT(m_xmlConfigReader.isStartElement() && m_xmlConfigReader.name == "tooltip");
#ifdef __DEBUG__
	MyDBG << m_xmlConfigReader.name().toString();
#endif
	m_textToolTip = m_xmlConfigReader.readElementText();
#ifdef __DEBUG__
	MyDBG << m_textToolTip;
#endif
}

void LauncherApplet::xmlWrite(XmlConfigWriter* writer)
{
	writer->writeStartElement("config");
	writer->writeTextElement("instance", QString::number(m_instance));
	if (!m_icon.name().isEmpty())
		writer->writeTextElement("icon", m_icon.name());
	else
		writer->writeTextElement("icon", m_iconNameFile);
	writer->writeTextElement("command", m_command);
	writer->writeTextElement("tooltip", m_textToolTip);
	writer->writeEndElement();
}

void  LauncherApplet::showContextMenu(const QPoint& point)
{
	QMenu menu;
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure..."), this, SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-desktop"), tr("Configure Panel"), PanelApplication::instance(), SLOT(showConfigurationDialog()));
	menu.addAction(QIcon::fromTheme("preferences-other"), tr("Configure applets"), m_panelWindow, SLOT(showConfigurationDialog()));
	menu.exec(point);
}
