#ifndef PANELAPPLICATION_H
#define PANELAPPLICATION_H

#include <QApplication>
#include <QFont>
#include "panelwindow.h"
#include "compmgr.h"
#include "gtkrc.h"

class IconLoader;
class X11Support;
class DesktopApplications;
class Ui_PanelApplicationSettings;

class PanelApplication: public QApplication
{
	Q_OBJECT
public:
	PanelApplication(int& argc, char** argv);
	~PanelApplication();
	// ritorna la istanza di PanelApplication (m_instance è settato a this oppure a NULL)
	static PanelApplication* instance() {return m_instance;}
	bool x11EventFilter(XEvent* event);
	void init();
	void reset();
	bool notify(QObject* receiver, QEvent* e);
	void setFontName(const QString& fontName);
	void setIconThemeName(const QString& iconThemeName);
	const QFont& panelFont() const {return m_panelFont;}
	const QString getApplicationName() const {return applicationName();}
//
	QString getColorApplication() {return m_colorApplication;}
	int getTrasparency() {return m_trasparency;}
	bool getCompositeManager() {return m_compositeManager;}
//

public slots:
	void showConfigurationDialog();
	void highlightColorChanged();

private slots:
	void reinit();

private:
	static PanelApplication* m_instance;
	IconLoader* m_iconLoader;
	X11Support* m_x11support;
	DesktopApplications* m_desktopApplications;
	QString m_fontName;
	//
	int m_fontSize;
	bool m_fontItalic, m_fontBold;
	QString m_colorApplication;
	int m_trasparency;
	bool m_compositeManager;
	CompMgr m_compMgr;
	//
	QString m_iconThemeName;
	PanelWindow::Anchor m_verticalAnchor;
	QString m_defaultIconThemeName;
	QFont m_panelFont;
	PanelWindow* m_panelWindow;
	Ui_PanelApplicationSettings* m_settingsUi;
	GtkRc m_gtkrc;
	bool m_applyFontToGtk;
};

#endif
