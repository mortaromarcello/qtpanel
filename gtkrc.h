#ifndef GTKRC_H
#define GTKRC_H

#include <QFile>
#include <QMap>
#include <QList>
#include <QTextStream>
#include <QBuffer>
#include <QSettings>

class GtkRc
{
public:
	GtkRc();
	~GtkRc();
	enum GtkVersion {
		Gtk2, Gtk3
	};

	QString getGtkRcThemeName(GtkVersion version) {
		switch(version) {
			case Gtk2: return getGtk2RcThemeName();
			case Gtk3: return getGtk3RcThemeName();
		}
	}
	QString getGtkRcIconThemeName(GtkVersion version) {
		switch(version) {
			case Gtk2: return getGtk2RcIconThemeName();
			case Gtk3: return getGtk3RcIconThemeName();
		}
	}
	QString getGtkRcFontName(GtkVersion version) {
		switch(version) {
			case Gtk2: return getGtk2RcFontName();
			case Gtk3: return getGtk3RcFontName();
		}
	}
	void setGtkRcThemeName(GtkVersion version, QString value) {
		switch(version) {
			case Gtk2: setGtk2RcThemeName(value); break;
			case Gtk3: setGtk3RcThemeName(value); break;
			default: break;
		}
	}
	void setGtkRcIconThemeName(GtkVersion version, QString value) {
		switch(version) {
			case Gtk2: setGtk2RcIconThemeName(value); break;
			case Gtk3: setGtk3RcIconThemeName(value); break;
			default: break;
		}
	}
	void setGtkRcFontName(GtkVersion version, QString value) {
		switch(version) {
			case Gtk2: setGtk2RcFontName(value); break;
			case Gtk3: setGtk3RcFontName(value); break;
			default: break;
		}
	}
	const QMap<QString, QByteArray>& gtk2RcOptions() const {return m_gtk2RcOptions;}
	const QByteArray& modifiedGtk2RcData() const {return m_modifiedGtk2RcData;}
	bool modified() {return m_modifiedGtk2RcData.isEmpty();}
	QStringList getListGtkTheme(GtkVersion version);

protected:
	QList<QByteArray> parserGtk2RcLine(QByteArray line);
	void parserGtk2Rc();
	QByteArray getModifiedGtk2RcData();
	QString getGtk2RcThemeName();
	void setGtk2RcThemeName(QString value);
	QString getGtk3RcThemeName();
	void setGtk3RcThemeName(QString value);
	QString getGtk2RcIconThemeName();
	void setGtk2RcIconThemeName(QString value);
	QString getGtk3RcIconThemeName();
	void setGtk3RcIconThemeName(QString value);
	QString getGtk2RcFontName();
	void setGtk2RcFontName(QString value);
	QString getGtk3RcFontName();
	void setGtk3RcFontName(QString value);
	void saveModifiedGtk2RcFile();
private:
	QString m_nameGtk2RcFile;
	QString m_nameGtk3RcFile;
	QSettings m_gtk3Settings;
	QByteArray m_gtk2RcData;
	QByteArray m_modifiedGtk2RcData;
	QList<QByteArray> m_gtk2RcLines;
	QMap<QString, QByteArray> m_gtk2RcOptions;
};

#endif // GTKRC_H
