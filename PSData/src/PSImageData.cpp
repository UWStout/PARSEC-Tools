#include <exception>

#include "PSImageData.h"
#include "PSCameraData.h"

#include <QXmlStreamReader>

PSImageData::PSImageData(long pCamID, QString pFilePath) {
    mCamID = pCamID;
    mFilePath = pFilePath;
    mCameraData = nullptr;
}

PSImageData::~PSImageData() {}

void PSImageData::addProperty(QString key, QString value) {
    mProperties.insert(key, value);
}

void PSImageData::setCameraData(PSCameraData* pCameraData) { mCameraData = pCameraData; }

long PSImageData::getCamID() { return mCamID; }
QString PSImageData::getFilePath() { return mFilePath; }
PSCameraData* PSImageData::getCameraData() { return mCameraData; }

QString PSImageData::getProperty(QString key) { return mProperties.value(key); }
QStringList PSImageData::getPropertyKeys() const { return mProperties.keys(); }
int PSImageData::getPropertyCount() const { return mProperties.size(); }

PSImageData* PSImageData::makeFromXML(QXmlStreamReader* reader) {
    // If this is a fresh XML doc, push to first non-document tag.
    while(reader->tokenType() == QXmlStreamReader::NoToken ||
          reader->tokenType() == QXmlStreamReader::StartDocument ||
          reader->name() == "document") {
        reader->readNextStartElement();
    }

    // Sanity check
    if(reader == nullptr || reader->name() != "camera" || !reader->isStartElement()) {
        return nullptr;
    }

    // Note, in some older file formats, the ID is not there
    long camID = -1L;
    if (reader->attributes().hasAttribute("", "camera_id")) {
        camID = reader->attributes().value("", "camera_id").toLong();
    }

    // Make a new object
    PSImageData* newImage = new PSImageData(camID);

    // Parse the remaining XML data
    try {
        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement()) {
                if(reader->name() == "photo") {
                    newImage->mFilePath = reader->attributes().value("", "path").toString();
                } else if(reader->name() == "property") {
                    QString lPropertyName = reader->attributes().value("", "name").toString();
                    QString lPropertyValue = reader->attributes().value("", "value").toString();
                    newImage->mProperties.insert(lPropertyName, lPropertyValue);
                }
            }

            // Reader might advance to the end element so this should not be mutually exclusive
            if(reader->isEndElement()) {
                if(reader->name() == "camera") {
                    return newImage;
                }
            }
        }
    } catch (...) {
        throw new std::logic_error("Error parsing Camera XLM tag to PSImageData");
    }

    // Should never reach this except when XML is malformed
    return nullptr;
}
