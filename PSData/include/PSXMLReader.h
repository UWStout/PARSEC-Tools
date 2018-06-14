#ifndef PS_XML_READER_H
#define PS_XML_READER_H

#include "psdata_global.h"

class QFile;
class QXmlStreamReader;

#include <QStack>
#include <QString>
#include <QFileInfo>

class PSDATASHARED_EXPORT PSXMLReader {
public:
    static QXmlStreamReader* explodeTag(QXmlStreamReader* reader, QStack<QFileInfo> currentFileStack);
    static QFileInfo checkForAndUpdatePath(QXmlStreamReader* reader, QFileInfo currentFile);
    static QXmlStreamReader* getXMLStreamFromFile(QFileInfo pFile);

    void readElementArray(QXmlStreamReader* reader, QString arrayName, QString elementName);
    virtual void processArrayElement(QXmlStreamReader* reader, QString elementName);
    virtual void parseProperty(const QString& pPropN, const QString& pPropV);
};

#endif
