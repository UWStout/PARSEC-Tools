#include "PSXMLReader.h"

#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

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

        // Fill in project name if needed
        if(newFilePath.contains("{projectname}")) {
            QString baseFilename = QFileInfo(newFilePath).baseName();
            newFilePath = newFilePath.replace("{projectname}", baseFilename);
        }

        // Make into an absolute QFile Path object
        newFilePath = QFileInfo(currentFile.fileName()).canonicalPath() + newFilePath;
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
//        QuaZipFile* lInsideFile = new QuaZipFile(pFile->fileName(), "doc.xml", lXMLFileStream);
//        if(lInsideFile.pos() < 0) {
//            lXMLFileStream = new QXmlStreamReader(lInsideFile);
//            lInsideFile.setParent(lXMLFileStream);
//        }
    } else if(ext == "psx") {
        // A raw PS xml QFile (probably to be accompanied by a .files directory)
        QFile* lXMLFile = new QFile(pFile.absoluteFilePath());
        lXMLFileStream = new QXmlStreamReader(lXMLFile);
    } else {
        // Something else that's not supported
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

        // Check for end of array
        else if(reader->isEndElement() && reader->name() == arrayName) {
            readingElements = false;
        }
    }
}

void PSXMLReader::processArrayElement(QXmlStreamReader* reader, QString elementName) {
    (void)reader;
    qWarning("Request to process array element '%s' not handled.\n", elementName.toLocal8Bit().data());
}
