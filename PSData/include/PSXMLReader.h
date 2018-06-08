#ifndef PS_XML_READER_H
#define PS_XML_READER_H

#include "psdata_global.h"

class QFile;
class QXmlStreamReader;

#include <QStack>
#include <QString>

class PSDATASHARED_EXPORT PSXMLReader {
public:
    static QXmlStreamReader* explodeTag(QXmlStreamReader* reader, QStack<QFile*> currentFileStack);
    static QFile* checkForAndUpdatePath(QXmlStreamReader* reader, QFile* currentFile);
    static QXmlStreamReader* getXMLStreamFromFile(QFile* pFile);

    void readElementArray(QXmlStreamReader* reader, QString arrayName, QString elementName);
    virtual void processArrayElement(QXmlStreamReader* reader, QString elementName);
};

#endif
