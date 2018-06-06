package edu.uwstout.berriers.PSHelper.Model;

import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

public class PSCameraData {

	public final long ID;

	private String mLabel;
	private boolean mEnabled;
	private double[] mTransform;

	private Long mSensorID;
	private PSImageData mImageData;
	private PSSensorData mSensorData;
	
	PSCameraData(long pID) {
		ID = pID;		
		
		mLabel = "";
		mEnabled = false;
		
		mImageData = null;
		mSensorData = null;
	}

	public static PSCameraData makeFromXML(XMLStreamReader reader) throws XMLStreamException {
		// Sanity check
		if(reader == null || reader.getLocalName() != "camera" || !reader.isStartElement()) {
			return null;
		}

		// Build the basic Camera
		PSCameraData newCamera = new PSCameraData(Long.parseLong(reader.getAttributeValue(null, "id")));
		newCamera.mLabel = reader.getAttributeValue(null, "label");
		newCamera.mEnabled = (reader.getAttributeValue(null, "enabled").equalsIgnoreCase("true"));
		try { newCamera.mSensorID = Long.parseLong(reader.getAttributeValue(null, "sensor_id")); }
		catch(Exception e) { newCamera.mSensorID = -1L; }

		try {
			while(reader.hasNext()) {
			    reader.next();
			    if(reader.isStartElement()) {
			    	if(reader.getLocalName().equalsIgnoreCase("transform")) {
	    				String[] coeffs = reader.getElementText().split("\\s");
	    				newCamera.mTransform = new double[coeffs.length];
	    				for(int i=0; i<coeffs.length; i++) {
	    					newCamera.mTransform[i] = Double.parseDouble(coeffs[i]);
	    				}			    		
			    	}
			    }
			    
			    if(reader.isEndElement()) {
			    	if(reader.getLocalName().equalsIgnoreCase("camera")) {
			    		return newCamera;
			    	}
			    }
			}
		} catch (XMLStreamException e) {
			throw e;
		}

		// Should never reach this except when XML is malformed
		return null;
	}
	
	public String getLabel() { return mLabel; }
	public PSImageData getImageData() { return mImageData; }

	public long getSensorID() { return mSensorID; }
	public PSSensorData getSensorData() { return mSensorData; }

	public boolean isEnabled() { return mEnabled; }
	public boolean isAligned() { return mImageData != null; }

	public void setLabel(String pLabel) { mLabel = pLabel; }
	public void setImageData(PSImageData pImageData) { mImageData = pImageData; }
	public void setIsEnabled(boolean pEnabled) { mEnabled = pEnabled; }
	
	public void setSensorID(long pSensorID) { mSensorID = pSensorID; }
	public void setSensorData(PSSensorData pSensorData) { mSensorData = pSensorData; }	
}
