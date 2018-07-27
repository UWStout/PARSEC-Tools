#include "PSXMLReader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

#include <quazip/quazipfile.h>

// This looks for a 'path' attribute in the current element.
// Sometimes a portion of the XML file is stripped out and placed
// in it's own file under the .files directory. This function will
// look for that attribute and follow it if found.
QXmlStreamReader* PSXMLReader::explodeTag(QXmlStreamReader* reader, QStack<QFileInfo> currentFileStack) {
    // Is there a path tag we need to follow?
    if(!reader->attributes().value("", "path").isEmpty()) {
        // Construct the absolute path to the new QFile and let it become the current QFile
        QFileInfo newXMLFile = checkForAndUpdatePath(reader, currentFileStack.top());
        currentFileStack.push(newXMLFile);        

        // Turn into an input stream
        QXmlStreamReader* newReader = getXMLStreamFromFile(newXMLFile);
        return newReader;
    }

    // Otherwise, stick with the passed in reader
    return reader;
}

// This looks for a 'path' attribute in the current element and extracts
// a proper QFile string so it may be followed and opened.
QFileInfo PSXMLReader::checkForAndUpdatePath(QXmlStreamReader* reader, QFileInfo currentFile) {
    // Is there a path tag we need to follow?
    if(!reader->attributes().value("", "path").isEmpty()) {

        // Retrieve relative path
        QString newFilePath = reader->attributes().value("", "path").toString();

        qDebug("Opening %s with parent %s", newFilePath.toLocal8Bit().data(), currentFile.filePath().toLocal8Bit().data());

        // Fill in project name if needed
        if(newFilePath.contains("{projectname}")) {
            QString baseFilename = QFileInfo(currentFile).baseName();
            newFilePath = newFilePath.replace("{projectname}", baseFilename);
        }

        // Make into an absolute QFile Path object
        newFilePath = QFileInfo(currentFile.filePath()).absolutePath() + QDir::separator() + newFilePath;
        return QFileInfo(newFilePath);
    }

    // Nothing to follow, so file stays the same
    return currentFile;
}

// Examine the given PhotoScan file and create a reasonable XML stream reader for it.
// Will examine the file to find the XML data given:
// - A psz or zip file (will look for doc.xml inside the file)
// - OR a psx file accompanied by a .files directory
QXmlStreamReader* PSXMLReader::getXMLStreamFromFile(QFileInfo pFile) {

    // The XML FileStream source
    QXmlStreamReader* lXMLFileStream = NULL;
    QString ext = QFileInfo(pFile.fileName()).completeSuffix();

    // Zip files with the XML inside them as doc.xml
    if(ext == "psz" || ext == "zip")
    {
        QuaZipFile* lInsideFile = new QuaZipFile(pFile.filePath(), "doc.xml");
        if(!lInsideFile->open(QIODevice::ReadOnly)) {
            qWarning("Failed to open zip file: %d.", lInsideFile->getZipError());
            delete lInsideFile;
        } else if(lInsideFile->pos() >= 0) {
            lXMLFileStream = new QXmlStreamReader(lInsideFile);
        }
    } else if(ext == "psx" || ext == "xml") {
        // A raw PS xml QFile (probably to be accompanied by a .files directory)
        // Note: This also happens during testing
        QFile* lXMLFile = new QFile(pFile.absoluteFilePath());
        lXMLFile->open(QIODevice::ReadOnly);
        lXMLFileStream = new QXmlStreamReader(lXMLFile);
    } else {
        // Something else that's not supported
        qWarning("Unsupported file type pased to PSXMLReader: '%s'\n",
                 pFile.filePath().toLocal8Bit().data());
    }

    // Might be NULL
    return lXMLFileStream;
}

// Function to read arrays encoded in XML (a la Agisoft's XML File format)
void PSXMLReader::readElementArray(QXmlStreamReader* reader, QString arrayName, QString elementName) {
    // Loop over all elements till the end of the array is reached
    bool readingElements = true;
    while(readingElements && !reader->atEnd()) {
        // Move to the next tag element (START or END)
        reader->readNext();

        // Parse array element (preserving old reader, just in case)
        if(reader->isStartElement() && reader->name() == elementName) {
            QXmlStreamReader* oldReader = reader;
            processArrayElement(reader, elementName);
            reader = oldReader;
        }

        // Parse property tags inside of arrays (happens with depth maps)
        else if (reader->isStartElement() && reader->name() == "property") {
            QString lPropertyName = reader->attributes().value(NULL, "name").toString();
            QString lPropertyValue = reader->attributes().value(NULL, "value").toString();
            parseProperty(lPropertyName, lPropertyValue);
        }

        // Check for end of array
        else if(reader->isEndElement() && reader->name() == arrayName) {
            readingElements = false;
        }
    }
}

void PSXMLReader::parseProperty(const QString& pPropN, const QString& pPropV) {
    (void)pPropV;
    qWarning("Request to process property '%s' not handled.\n", pPropN.toLocal8Bit().data());
}

void PSXMLReader::processArrayElement(QXmlStreamReader* reader, QString elementName) {
    (void)reader;
    qWarning("Request to process array element '%s' not handled.\n", elementName.toLocal8Bit().data());
}
