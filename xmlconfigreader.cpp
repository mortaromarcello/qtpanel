#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include "xmlconfigreader.h"

XmlConfigReader::XmlConfigReader()
{
	XmlConfigReader("config.xml");
}

XmlConfigReader::XmlConfigReader(const QString& namefile)
{
	m_namefile = namefile;
#ifdef __DEBUG__
	MyDBG << m_namefile;
#endif
	QSettings settings;
	QFileInfo fileInfo(settings.fileName());
	QDir dir(fileInfo.absoluteDir());
	QString xml_path_file(dir.absolutePath() + QDir::separator() + m_namefile);
#ifdef __DEBUG__
	MyDBG << xml_path_file;
#endif
	m_xmlFile.setFileName(xml_path_file);
}

XmlConfigReader::~XmlConfigReader()
{
	xmlClose();
}

bool XmlConfigReader::xmlOpen()
{
	bool ret = m_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
	if (ret)
		setDevice(&m_xmlFile);
	return ret;
}

void XmlConfigReader::xmlClose()
{
	m_xmlFile.close();
}

QString XmlConfigReader::xmlErrorString() const
{
	return QObject::tr("%1\nLine %2, column %3, Type Error %4")
					.arg(errorString())
					.arg(lineNumber())
					.arg(columnNumber())
					.arg(error());
}
