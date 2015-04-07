#include "gtkrc.h"
#include "mydebug.h"

#include <iostream>
#include <QDir>

void removeCharsAtBeginningLine(char ch, QByteArray& line)
{
	for (int i = 0; i < line.size(); i++) {
		if (line[i] == ch) line.remove(i, 1);
		else break;
	}
}

void removeCharsAtEndingLine(char ch, QByteArray& line)
{
	for (int i = line.size()-1; i >= 0; i--) {
		if (line[i] == ch) line.remove(i, 1);
		else break;
	}
}

bool isComment(QByteArray line)
{
	for (int i = 0; i < line.size(); i++) {
		if (line[i] == ' ') continue;
		if (line[i] == '#') return true;
	}
	return false;
}

GtkRc::GtkRc()
	:m_nameGtk2RcFile(QDir::homePath()+QDir::separator()+".gtkrc-2.0"),
	m_nameGtk3RcFile(QDir::homePath()+QDir::separator()+".config"+QDir::separator()+"gtk-3.0"+QDir::separator()+"settings.ini"),
	m_gtk3Settings(m_nameGtk3RcFile, QSettings::IniFormat)
{
	m_gtk3Settings.beginGroup("Settings");
	QFile file(m_nameGtk2RcFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		std::cerr << "Cannot open file for reading: " << qPrintable(file.errorString()) << std::endl;
		return;
	}
	m_gtk2RcData = file.readAll();
	file.close();
	m_gtk2RcLines = m_gtk2RcData.split('\n');
	parserGtk2Rc();
}

GtkRc::~GtkRc()
{
}

QString GtkRc::getGtk2RcThemeName()
{
	return QString(m_gtk2RcOptions["gtk-theme-name"]).remove("\"");
}

void GtkRc::setGtk2RcThemeName(QString value)
{
	if (!value.contains("\"")) {
		value.insert(0, '"');
		int last_char = value.size();
		value.insert(last_char, '"');
	}
	m_gtk2RcOptions["gtk-theme-name"] = value.toAscii();
	m_modifiedGtk2RcData = getModifiedGtk2RcData();
	saveModifiedGtk2RcFile();
}

QString GtkRc::getGtk3RcThemeName()
{
	return m_gtk3Settings.value("gtk-theme-name").toString();
}

void GtkRc::setGtk3RcThemeName(QString value)
{
	m_gtk3Settings.setValue("gtk-theme-name", value);
}

QString GtkRc::getGtk2RcIconThemeName()
{
	return QString(m_gtk2RcOptions["gtk-icon-theme-name"]).remove("\"");
}

void GtkRc::setGtk2RcIconThemeName(QString value)
{
	if (!value.contains("\"")) {
		value.insert(0, '"');
		int last_char = value.size();
		value.insert(last_char, '"');
	}
	m_gtk2RcOptions["gtk-icon-theme-name"] = value.toAscii();
	m_modifiedGtk2RcData = getModifiedGtk2RcData();
	saveModifiedGtk2RcFile();
}

QString GtkRc::getGtk3RcIconThemeName()
{
	return m_gtk3Settings.value("gtk-icon-theme-name").toString();
}

void GtkRc::setGtk3RcIconThemeName(QString value)
{
	m_gtk3Settings.setValue("gtk-icon-theme-name", value);
}

QString GtkRc::getGtk2RcFontName()
{
	return QString(m_gtk2RcOptions["gtk-font-name"]).remove("\"");
}

void GtkRc::setGtk2RcFontName(QString value)
{
	if (!value.contains("\"")) {
		value.insert(0, '"');
		int last_char = value.size();
		value.insert(last_char, '"');
	}
	m_gtk2RcOptions["gtk-font-name"] = value.toAscii();
	m_modifiedGtk2RcData = getModifiedGtk2RcData();
	saveModifiedGtk2RcFile();
}

QString GtkRc::getGtk3RcFontName()
{
	return m_gtk3Settings.value("gtk-font-name").toString();
}

void GtkRc::setGtk3RcFontName(QString value)
{
	m_gtk3Settings.setValue("gtk-font-name", value);
}

void GtkRc::saveModifiedGtk2RcFile()
{
	QFile file(m_nameGtk2RcFile);
	file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
	file.write(m_modifiedGtk2RcData);
	file.close();
}

// utilità per leggere i settaggi di gtk2/3
// torna la lista dei temi
QStringList GtkRc::getListGtkTheme(GtkVersion version)
{
	QStringList themes;
	QList<QDir> dirs = QList<QDir>() << QDir("/usr/share/themes") << QDir("/usr/local/share/themes") << QDir(QDir::homePath()+QDir::separator()+".themes");
	for (int i = 0; i < dirs.size(); i++) {
		QStringList temp = dirs[i].entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (int j = 0; j < temp.size(); j++) {
			QStringList temp2 = QDir(dirs[i].absolutePath()+QDir::separator()+temp[j]).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
			switch(version) {
				case Gtk2:
				if (temp2.contains("gtk-2.0", Qt::CaseInsensitive))
					themes << temp[j]; break;
				case Gtk3:
				if (temp2.contains("gtk-3.0", Qt::CaseInsensitive))
					themes << temp[j]; break;
				default:;
			}
		}
	}
	themes.removeDuplicates();
	return themes;
}

QByteArray GtkRc::getModifiedGtk2RcData()
{
	QByteArray result;
	for (int i = 0; i < m_gtk2RcLines.size(); i++) {
		if (m_gtk2RcLines[i].isEmpty()) continue;
		QList<QByteArray> option = parserGtk2RcLine(m_gtk2RcLines[i]);
		if (option.isEmpty()) {
			result += m_gtk2RcLines[i]+'\n';
			continue;
		}
		if (m_gtk2RcOptions[option[0]] != QString()) {
			result += option[0]+'='+m_gtk2RcOptions[option[0]]+'\n';
		}
		else result += m_gtk2RcLines[i]+'\n';
	}
	return result;
}

QList<QByteArray> GtkRc::parserGtk2RcLine(QByteArray line)
{
	QList<QByteArray> option;
	if (!line.contains('=')) return QList<QByteArray>();
	option = line.split('=');
	if (option.size() != 2)
		return QList<QByteArray>();
	removeCharsAtBeginningLine(' ', option[0]);
	removeCharsAtEndingLine(' ', option[0]);
	removeCharsAtBeginningLine(' ', option[1]);
	removeCharsAtEndingLine(' ', option[1]);
	return option;
}

void GtkRc::parserGtk2Rc()
{
	QList<QByteArray> option;
	for (int i = 0; i < m_gtk2RcLines.size(); i++) {
		if (m_gtk2RcLines[i].isEmpty())
			continue;
		option = parserGtk2RcLine(m_gtk2RcLines[i]);
		if (option.isEmpty())
			continue;
		m_gtk2RcOptions.insert(QString(option[0]), option[1]);
	};
}
