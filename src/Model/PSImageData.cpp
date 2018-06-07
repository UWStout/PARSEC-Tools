#include "PSImageData.h"

PSImageData::PSImageData(long pCamID) {
    mCamID = pCamID;
    mFilePath = "";
    mProperties = new QMap<QString, QString>();
    mCameraData = NULL;
}

PSImageData::~PSImageData() {

}

void PSImageData::addProperty(QString key, QString value) {
    mProperties.insert(key, value);
}

void PSImageData::setCameraData(PSCameraData pCameraData) { mCameraData = pCameraData; }

long PSImageData::getCamID() { return mCamID; }
QString PSImageData::getFilePath() { return mFilePath; }
QString PSImageData::getProperty(QString key) { return mProperties.find(key).value(); }
PSCameraData PSImageData::getCameraData() { return mCameraData; }

PSImageData PSImageData::makeFromXML(QXMLReader reader) { // TODO
//		// Sanity check
//		if(reader == null || reader.getLocalName() != "camera" || !reader.isStartElement()) {
//			return null;
//		}

//		// In some older file formats, the ID is not there
//		long camID = -1L;
//		try {
//			camID = Long.parseLong(reader.getAttributeValue(null, "camera_id"));
//		} catch(Exception e) {}

//		// Make a new object
//		PSImageData newImage = new PSImageData(camID);

//		// Parse the remaining XML data
//		try {
//			while(reader.hasNext()) {
//			    reader.next();
//			    if(reader.isStartElement()) {
//			    	switch(reader.getLocalName()) {
//			    		case "photo":
//				    		newImage.mFilePath = reader.getAttributeValue(null, "path");
//			    		break;

//			    		case "property":
//			    		{
//                        	String lPropertyName = reader.getAttributeValue(null, "name");
//                        	String lPropertyValue = reader.getAttributeValue(null, "value");
//                        	newImage.mProperties.put(lPropertyName, lPropertyValue);
//			    		}
//			    		break;
//			    	}
//			    }

//			    if(reader.isEndElement()) {
//			    	if(reader.getLocalName().equalsIgnoreCase("camera")) {
//			    		return newImage;
//			    	}
//			    }
//			}
//		} catch (XMLStreamException e) {
//			throw e;
//		}

//		// Should never reach this except when XML is malformed
//		return null;
}
