#include "config.h"
#include "panelapplication.h"

#include <QSettings>
#include <QTimer>
#include <QtDebug>
#include <QDir>
#include <QColorDialog>
#include <QStyleFactory>
#include "iconloader.h"
#include "x11support.h"
#include "dpisupport.h"
#include "desktopapplications.h"
#include "applet.h"
#include "ui_panelapplicationsettings.h"

// setta m_instance a NULL
PanelApplication* PanelApplication::m_instance = NULL;

PanelApplication::PanelApplication(int& argc, char** argv)
	: QApplication(argc, argv),
	m_defaultIconThemeName(QIcon::themeName()),
	m_iconLoader(new IconLoader()),
	m_x11support(new X11Support()),
	m_desktopApplications(new DesktopApplications()),
	m_settingsUi(new Ui::PanelApplicationSettings()),
	m_applyFontToGtk(false)
{
	m_instance = this;
	setOrganizationName(qtpanel_ORGANIZATION);
	setApplicationName(qtpanel_APPLICATION);
	if (!QIcon::themeSearchPaths().contains("/usr/share/pixmaps"))
		QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << "/usr/share/pixmaps");
	QSettings settings;
	m_compositeManager = (settings.value("compositeManager", "false").toString() == "true") ? true : false;
	if (m_compositeManager)
		if (QX11Info::isCompositingManagerRunning())
			qDebug("A Composite manager is running");
		else
			m_compMgr.start();
	//MyDBG << m_gtk2rc.options();
	//MyDBG << m_gtk2rc.getGtkThemeName();
	//m_gtk2rc.setGtkThemeName("\"Clearlooks\"");
	//MyDBG << m_gtk2rc.getGtkIconThemeName();
	//MyDBG << m_gtk2rc.modifiedData();
}

PanelApplication::~PanelApplication()
{
	reset();
	delete m_desktopApplications;
	delete m_x11support;
	delete m_iconLoader;
	m_instance = NULL;
	m_compMgr.stop();
	m_compMgr.quit();
}

bool PanelApplication::x11EventFilter(XEvent* event)
{
	m_x11support->onX11Event(event);
	return false;
}

bool PanelApplication::notify(QObject* receiver, QEvent* e)
{
	try {
		return QApplication::notify(receiver, e);
	} catch ( std::exception& e ) {
		qDebug() << tr("Arrrgggghhhh\n");
		return false;
	}
}

void PanelApplication::showConfigurationDialog()
{
	QDialog dialog;
	QPixmap pixmap(24, 24);
	pixmap.fill(Applet::highlightColor());
	QStringList colors = QColor::colorNames();
	QList <QColor> colorList;
	QStringList themeIcons, pathsThemeIcons;
	pathsThemeIcons = QIcon::themeSearchPaths();
	for (int i = 0; i< pathsThemeIcons.size(); i++)
		themeIcons << QDir(pathsThemeIcons[i]).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	QStringList qtStyles = QStyleFactory::keys();
	QStringList gtk3Themes = m_gtkrc.getListGtkTheme(GtkRc::Gtk3);
	QStringList gtk2Themes = m_gtkrc.getListGtkTheme(GtkRc::Gtk2);
	for (int i = 0; i < colors.size(); i++)
		colorList << QColor(colors.at(i));
	m_settingsUi->setupUi(&dialog);
	m_settingsUi->fontName->setCurrentFont(QFont(m_fontName));
	m_settingsUi->fontSize->setValue(m_fontSize);
	m_settingsUi->fontSize->setMinimum(6);
	m_settingsUi->fontSize->setMaximum(16);
	m_settingsUi->fontItalic->setChecked(m_fontItalic);
	m_settingsUi->fontBold->setChecked(m_fontBold);
	m_settingsUi->applyFontToGtk->setChecked(m_applyFontToGtk);
	// qtstyle settings
	m_settingsUi->qtStyle->addItems(qtStyles);
	m_settingsUi->qtStyle->setCurrentIndex(qtStyles.indexOf(m_panelWindow->trolltechConf()->value("style").toString()));
	// gtk2 themes
	m_settingsUi->gtk2Theme->addItems(gtk2Themes);
	m_settingsUi->gtk2Theme->setCurrentIndex(gtk2Themes.indexOf(m_gtkrc.getGtkRcThemeName(GtkRc::Gtk2)));
	// gtk3 themes
	m_settingsUi->gtk3Theme->addItems(gtk3Themes);
	m_settingsUi->gtk3Theme->setCurrentIndex(gtk3Themes.indexOf(m_gtkrc.getGtkRcThemeName(GtkRc::Gtk3)));
	// themes icon
	m_settingsUi->iconThemeName->addItems(themeIcons);
	m_settingsUi->iconThemeName->setCurrentIndex(themeIcons.indexOf(m_iconThemeName));
	//
	m_settingsUi->colorApplication->addItems(colors);
	m_settingsUi->verticalPosition->setCurrentIndex(m_verticalAnchor == PanelWindow::Bottom ? 1 : 0);
	m_settingsUi->colorApplication->setCurrentIndex(colors.indexOf(m_colorApplication));
	m_settingsUi->trasparency->setMinimum(0);
	m_settingsUi->trasparency->setMaximum(255);
	m_settingsUi->trasparency->setValue(m_trasparency);
	m_settingsUi->trasparency->setEnabled(m_compositeManager);
	m_settingsUi->compositeManager->setChecked(m_compositeManager);

	m_settingsUi->highlight_toolbutton->setIcon(QIcon(pixmap));
	connect(m_settingsUi->highlight_toolbutton, SIGNAL(pressed()), this, SLOT(highlightColorChanged()));
	m_settingsUi->radiusinc_doublespinbox->setRange(0.0, 20.0);
	m_settingsUi->radiusinc_doublespinbox->setValue(Applet::radiusInc());

	if(dialog.exec() == QDialog::Accepted)
	{
		QSettings settings;
		settings.setValue("fontName", m_settingsUi->fontName->currentFont().family());
		settings.setValue("fontSize", m_settingsUi->fontSize->value());
		settings.setValue("fontItalic", (m_settingsUi->fontItalic->isChecked() ? "true":"false"));
		settings.setValue("fontBold", (m_settingsUi->fontBold->isChecked() ? "true":"false"));
		settings.setValue("fontBold", (m_settingsUi->fontBold->isChecked() ? "true":"false"));
		settings.setValue("applyfonttogtk", (m_settingsUi->applyFontToGtk->isChecked() ? "true":"false"));
		if (m_settingsUi->applyFontToGtk->isChecked() == true) {
			QString font = m_settingsUi->fontName->currentFont().family() + (m_settingsUi->fontBold->isChecked() ? " Bold":"") + (m_settingsUi->fontItalic->isChecked() ? " Italic":"") + " " + QString("%1").arg(m_settingsUi->fontSize->value());
			m_gtkrc.setGtkRcFontName(GtkRc::Gtk2, font);
			m_gtkrc.setGtkRcFontName(GtkRc::Gtk3, font);
		}
		//
		settings.setValue("iconThemeName", m_settingsUi->iconThemeName->currentText());
		if (m_gtkrc.getGtkRcIconThemeName(GtkRc::Gtk2) != m_settingsUi->iconThemeName->currentText()) {
			m_gtkrc.setGtkRcIconThemeName(GtkRc::Gtk2, m_settingsUi->iconThemeName->currentText());
		}
		m_gtkrc.setGtkRcIconThemeName(GtkRc::Gtk3, m_settingsUi->iconThemeName->currentText());
		//
		settings.setValue("verticalPosition", m_settingsUi->verticalPosition->currentText());
		settings.setValue("colorApplication", m_settingsUi->colorApplication->currentText());
		settings.setValue("trasparency", m_settingsUi->trasparency->value());
		settings.setValue("compositeManager", (m_settingsUi->compositeManager->isChecked() ? "true" : "false"));
		settings.setValue("highlightcolor", Applet::highlightColor().name());
		Applet::setRadiusInc(m_settingsUi->radiusinc_doublespinbox->value());
		settings.setValue("radiusinc", Applet::radiusInc());
		// setting qt style
		m_panelWindow->trolltechConf()->setValue("style", m_settingsUi->qtStyle->currentText());
		if (m_gtkrc.getGtkRcThemeName(GtkRc::Gtk2) != m_settingsUi->gtk2Theme->currentText()) {
			m_gtkrc.setGtkRcThemeName(GtkRc::Gtk2, m_settingsUi->gtk2Theme->currentText());
		}
		// setting gtk 3 theme
		m_gtkrc.setGtkRcThemeName(GtkRc::Gtk3, m_settingsUi->gtk3Theme->currentText());
		//
		qApp->setStyle(m_panelWindow->trolltechConf()->value("style").toString());
		//
		m_compositeManager = m_settingsUi->compositeManager->isChecked();
		// Don't want to delete objects right now (because we're called from those objects), schedule it for later.
		QTimer::singleShot(1, this, SLOT(reinit()));
	}
}

void PanelApplication::highlightColorChanged()
{
	QPixmap pixmap(24, 24);
	QColorDialog dialog(Applet::highlightColor());
	if (dialog.exec() == QDialog::Accepted) {
		Applet::setHighlightColor(dialog.currentColor());
		pixmap.fill(Applet::highlightColor());
		m_settingsUi->highlight_toolbutton->setIcon(pixmap);
	}
}

void PanelApplication::reinit()
{
	reset();
	init();
}

void PanelApplication::init()
{
	QSettings settings;
	//
	m_fontSize = settings.value("fontSize", QApplication::font().pointSize()).toInt();
	m_fontItalic = (settings.value("fontItalic", "false").toString() == "true") ? true : false;
	m_fontBold = (settings.value("fontBold", "false").toString() == "true") ? true : false;
	m_applyFontToGtk = (settings.value("applyfonttogtk", "false").toString() == "true") ? true : false;
	setFontName(settings.value("fontName", QApplication::font().family()).toString());
	//
	setIconThemeName(settings.value("iconThemeName", "default").toString());
	QString verticalPosition = settings.value("verticalPosition", tr("Top")).toString();
	//
	m_colorApplication = settings.value("colorApplication", "trasparent").toString();
	m_trasparency = settings.value("trasparency", 128).toInt();
	//
	QColor color;
	color.setNamedColor(settings.value("highlightcolor", "#FFFFFF").toString());
	Applet::setHighlightColor(color);
	Applet::setRadiusInc(settings.value("radiusinc", 10.0).toDouble());
	//
	if(verticalPosition == tr("Top"))
		m_verticalAnchor = PanelWindow::Top;
	else if(verticalPosition == tr("Bottom"))
		m_verticalAnchor = PanelWindow::Bottom;

	m_panelWindow = new PanelWindow();
	m_panelWindow->resize(adjustHardcodedPixelSize(128), adjustHardcodedPixelSize(32));
	m_panelWindow->setLayoutPolicy(PanelWindow::FillSpace);
	m_panelWindow->setVerticalAnchor(m_verticalAnchor);
	m_panelWindow->setDockMode(true);
	m_panelWindow->init();
	m_panelWindow->show();
}

void PanelApplication::reset()
{
	delete m_panelWindow;
}

void PanelApplication::setFontName(const QString& fontName)
{
	m_fontName = fontName;
	if(m_fontName != QApplication::font().family()) {
		m_panelFont = QFont(m_fontName, m_fontSize);
		m_panelFont.setItalic(m_fontItalic);
		m_panelFont.setBold(m_fontBold);
	}
	else
		m_panelFont = QApplication::font();
}

void PanelApplication::setIconThemeName(const QString& iconThemeName)
{
	m_iconThemeName = iconThemeName;
	if(m_iconThemeName != "default")
		QIcon::setThemeName(m_iconThemeName);
	else
		QIcon::setThemeName(m_defaultIconThemeName);
}
