#ifndef XMLCONFIGWRITER_H
#define XMLCONFIGWRITER_H

#include <QXmlStreamWriter>
#include <QString>
#include <QFile>
#include "mydebug.h"

class XmlConfigWriter: public QXmlStreamWriter
{
public:
	XmlConfigWriter();
	XmlConfigWriter(const QString& namefile);
	~XmlConfigWriter();
	bool xmlOpen();
	void xmlClose();
	void setFileName(const QString& fileName) {m_xmlFile.setFileName(fileName);}

private:
	QString m_namefile;
	QFile m_xmlFile;
};

#endif //XMLCONFIGWRITER_H
