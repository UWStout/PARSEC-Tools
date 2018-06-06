package edu.uwstout.berriers.PSHelper.Model;

import java.util.HashMap;
import java.util.Map;

import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

public class PSImageData {

	private long mCamID;
	private String mFilePath;
	private Map<String, String> mProperties;
	private PSCameraData mCameraData;
	
	public PSImageData(long pCamID) {
		mCamID = pCamID;
		mFilePath = "";
		mProperties = new HashMap<String, String>();
		mCameraData = null;
	}
	
	public void addProperty(String key, String value) {
		mProperties.put(key, value);
	}

	public void setCameraData(PSCameraData pCameraData) { mCameraData = pCameraData; }

	public long getCamID() { return mCamID; }
	public String getFilePath() { return mFilePath; }
	public String getProperty(String key) { return mProperties.get(key); }	
	public PSCameraData getCameraData() { return mCameraData; }
	
	public static PSImageData makeFromXML(XMLStreamReader reader) throws XMLStreamException {
		// Sanity check
		if(reader == null || reader.getLocalName() != "camera" || !reader.isStartElement()) {
			return null;
		}

		// In some older file formats, the ID is not there
		long camID = -1L;
		try {
			camID = Long.parseLong(reader.getAttributeValue(null, "camera_id"));
		} catch(Exception e) {}

		// Make a new object
		PSImageData newImage = new PSImageData(camID);

		// Parse the remaining XML data
		try {
			while(reader.hasNext()) {
			    reader.next();
			    if(reader.isStartElement()) {
			    	switch(reader.getLocalName()) {
			    		case "photo":
				    		newImage.mFilePath = reader.getAttributeValue(null, "path");
			    		break;
			    		
			    		case "property":
			    		{
                        	String lPropertyName = reader.getAttributeValue(null, "name");
                        	String lPropertyValue = reader.getAttributeValue(null, "value");	                        	
                        	newImage.mProperties.put(lPropertyName, lPropertyValue);
			    		}
			    		break;
			    	}
			    }
			    
			    if(reader.isEndElement()) {
			    	if(reader.getLocalName().equalsIgnoreCase("camera")) {
			    		return newImage;
			    	}
			    }
			}
		} catch (XMLStreamException e) {
			throw e;
		}

		// Should never reach this except when XML is malformed
		return null;
	}	
}
