#include "PSCameraData.h"

PSCameraData::PSCameraData(long pID) : ID(pID) {
    mLabel = "";
    mEnabled = false;

    mImageData = NULL;
    mSensorData = NULL;
}

PSCameraData::~PSCameraData() {

}

PSCameraData::makeFromXML(QXMLReader reader) {

    // TODO

//    // Sanity check
//    if(reader == null || reader.getLocalName() != "camera" || !reader.isStartElement()) {
//        return null;
//    }

//    // Build the basic Camera
//    PSCameraData newCamera = new PSCameraData(Long.parseLong(reader.getAttributeValue(null, "id")));
//    newCamera.mLabel = reader.getAttributeValue(null, "label");
//    newCamera.mEnabled = (reader.getAttributeValue(null, "enabled").equalsIgnoreCase("true"));
//    try { newCamera.mSensorID = Long.parseLong(reader.getAttributeValue(null, "sensor_id")); }
//    catch(Exception e) { newCamera.mSensorID = -1L; }

//    try {
//        while(reader.hasNext()) {
//            reader.next();
//            if(reader.isStartElement()) {
//                if(reader.getLocalName().equalsIgnoreCase("transform")) {
//                    String[] coeffs = reader.getElementText().split("\\s");
//                    newCamera.mTransform = new double[coeffs.length];
//                    for(int i=0; i<coeffs.length; i++) {
//                        newCamera.mTransform[i] = Double.parseDouble(coeffs[i]);
//                    }
//                }
//            }

//            if(reader.isEndElement()) {
//                if(reader.getLocalName().equalsIgnoreCase("camera")) {
//                    return newCamera;
//                }
//            }
//        }
//    } catch (XMLStreamException e) {
//        throw e;
//    }

//    // Should never reach this except when XML is malformed
//    return null;
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
