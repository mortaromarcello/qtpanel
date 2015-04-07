#ifndef XMLCONFIGREADER_H
#define XMLCONFIGREADER_H

#include <QXmlStreamReader>
#include <QString>
#include <QFile>
#include "mydebug.h"

class XmlConfigReader: public QXmlStreamReader
{
public:
	XmlConfigReader();
	XmlConfigReader(const QString& namefile);
	~XmlConfigReader();
	bool xmlOpen();
	void xmlClose();
	QString xmlErrorString() const;
	void setFileName(const QString& fileName) {m_xmlFile.setFileName(fileName);}

private:
	QString m_namefile;
	QFile m_xmlFile;
};

#endif //XMLCONFIGREADER_H
