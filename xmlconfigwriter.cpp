#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include "xmlconfigwriter.h"

XmlConfigWriter::XmlConfigWriter()
{
	XmlConfigWriter("config.xml");
}

XmlConfigWriter::XmlConfigWriter(const QString& namefile)
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

XmlConfigWriter::~XmlConfigWriter()
{
	xmlClose();
}

bool XmlConfigWriter::xmlOpen()
{
	bool ret = m_xmlFile.open(QIODevice::WriteOnly | QIODevice::Text);
	if (ret) {
#ifdef __DEBUG__
		MyDBG << "Open" << m_xmlFile.fileName() << "OK";
#endif
		setDevice(&m_xmlFile);
		setAutoFormatting(true);
		setAutoFormattingIndent(2);
	}
	return ret;
}

void XmlConfigWriter::xmlClose()
{
	m_xmlFile.close();
}
