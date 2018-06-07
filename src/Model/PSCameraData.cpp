#include <exception>

#include "PSCameraData.h"
#include "PSSensorData.h"
#include "PSImageData.h"

#include <QXmlStreamReader>

PSCameraData::PSCameraData(long pID) : ID(pID) {
    mLabel = "";
    mEnabled = false;

    mTransform = NULL;
    mImageData = NULL;
    mSensorData = NULL;
}

PSCameraData::~PSCameraData() {}

PSCameraData* PSCameraData::makeFromXML(QXmlStreamReader* reader) {
    // Sanity check
    if(reader == NULL || reader->name() != "camera" || !reader->isStartElement()) {
        return NULL;
    }

    // Build the basic Camera
    PSCameraData* newCamera = new PSCameraData(reader->attributes().value("", "id").toLong());
    newCamera->mLabel = reader->attributes().value("", "label").toString();
    newCamera->mEnabled = (reader->attributes().value("", "enabled") == "true");
    try { newCamera->mSensorID = reader->attributes().value("", "sensor_id").toLong(); }
    catch(...) { newCamera->mSensorID = -1L; }

    try {
        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement()) {
                if(reader->name() == "transform") {
                    QVector<QStringRef> coeffs = reader->readElementText().splitRef("\\s");
                    newCamera->mTransform = new double[coeffs.length()];
                    for(int i=0; i<coeffs.length(); i++) {
                        newCamera->mTransform[i] = coeffs[i].toDouble();
                    }
                }
            }

            // Reader might advance to the end element so this should not be mutually exclusive
            if(reader->isEndElement()) {
                if(reader->name() == "camera") {
                    return newCamera;
                }
            }
        }
    } catch (...) {
        delete newCamera;
        throw new std::logic_error("Error parsing camera XML tag");
    }

    // Should never reach this except when XML is malformed
    return NULL;
}

QString PSCameraData::getLabel() { return mLabel; }
PSImageData *PSCameraData::getImageData() { return mImageData; }

long PSCameraData::getSensorID() { return mSensorID; }
PSSensorData *PSCameraData::getSensorData() { return mSensorData; }

bool PSCameraData::isEnabled() { return mEnabled; }
bool PSCameraData::isAligned() { return (mImageData != NULL); }

void PSCameraData::setLabel(QString pLabel) { mLabel = pLabel; }
void PSCameraData::setImageData(PSImageData *pImageData) { mImageData = pImageData; }
void PSCameraData::setIsEnabled(bool pEnabled) { mEnabled = pEnabled; }

void PSCameraData::setSensoID(long pSensorID) { mSensorID = pSensorID; }
void PSCameraData::setSensorData(PSSensorData *pSensorData) { mSensorData = pSensorData; }
