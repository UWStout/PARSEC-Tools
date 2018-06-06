package edu.uwstout.berriers.PSHelper.Model;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

public class PSModelData {

	private long mFaceCount, mVertexCount;
	private File mZipFile;
	private String mMeshFilepath;
	private boolean mHasVtxColors, mHasUV;

	private Map<Integer, String> textureFiles;

	public PSModelData(File pZipFile) {
		mZipFile = pZipFile;
		
		mFaceCount = mVertexCount = -1;
		mHasVtxColors = mHasUV = false;
		mMeshFilepath = "";
		
		textureFiles = new HashMap<Integer, String>();
	}
	
	public void setFaceCount(long pFaceCount) { mFaceCount = pFaceCount; }
	public void setVertexCount(long pVertexCount) { mVertexCount = pVertexCount; }
	public void setHasVertexColors(boolean pHasVtxColors) { mHasVtxColors = pHasVtxColors; }
	public void setHasUV(boolean pHasUV) { mHasUV = pHasUV; }
	public void setMeshFilename(String pMeshFilepath) { mMeshFilepath = pMeshFilepath; }
	
	public void addTextureFile(int pId, String pFilepath) {
		if(textureFiles.containsKey(pId)) {
			System.err.println("Warning: possible texture ID collision (" + pId + " already exists)");
		}
		
		textureFiles.put(pId, pFilepath);
	}

	public File getArchiveFile() { return mZipFile; }

	public long getFaceCount() { return mFaceCount; }
	public long getVertexCount() { return mVertexCount; }
	public String getMeshFilename() { return mMeshFilepath; }
	public boolean hasVtxColors() { return mHasVtxColors; }
	public boolean hasUV() { return mHasUV; }

	public Map<Integer, String> getTextureFiles() { return textureFiles; }
	public String getTextureFile(int id) { return textureFiles.get(id); }
	
	public static PSModelData makeFromXML(XMLStreamReader reader, File pZipFile, PSChunkData pParent) throws XMLStreamException {
		// Sanity check
		if(reader.getEventType() == XMLStreamReader.START_DOCUMENT) { reader.nextTag(); }
		if(reader == null || reader.getLocalName() != "model" || !reader.isStartElement()) {
			return null;
		}

		// Make a new object
		PSModelData newModel = new PSModelData(pZipFile);

		// Parse the remaining XML data
		try {
			while(reader.hasNext()) {
			    reader.next();
			    // Tag open cases
			    if(reader.isStartElement()) {
			    	switch(reader.getLocalName()) {
			    	
			    		// Data inside the 'mesh' tag
			    		case "hasVertexColors": newModel.mHasVtxColors = reader.getElementText().equalsIgnoreCase("true"); break;
			    		case "hasUV": newModel.mHasVtxColors = reader.getElementText().equalsIgnoreCase("true"); break;
			    		case "faceCount": newModel.mFaceCount = Long.parseLong(reader.getElementText()); break;
			    		case "vertexCount": newModel.mVertexCount = Long.parseLong(reader.getElementText()); break;

			    		// Start of the mesh tag
			    		case "mesh":
			    			newModel.mMeshFilepath = reader.getAttributeValue(null, "path");
			    		break;

			    		// Texture tag
			    		case "texture":
			    		{
			    			String texPath = reader.getAttributeValue(null, "path");

			    			// Note, sometimes textures don't have an ID (if there's only one texture for instance)
			    			try {
			    				int texID = Integer.parseInt(reader.getAttributeValue(null, "id"));
			    				newModel.addTextureFile(texID, texPath);
			    			} catch(Exception e) { newModel.addTextureFile(0, texPath); }
			    		}
			    		break;

			    		// From inside the meta tag
            			case "property":
            			{
                        	String lPropertyName = reader.getAttributeValue(null, "name");
                        	String lPropertyValue = reader.getAttributeValue(null, "value");

                        	// Sometimes, this is the only way to get the face count (on older file versions)
                        	if(lPropertyName.contains("face_count") && newModel.mFaceCount <= 0) {
                        		newModel.mFaceCount = Long.parseLong(lPropertyValue);
                        	}
                        	
                        	pParent.parseChunkProperty(lPropertyName, lPropertyValue);
            			}
            			break;
			    	}
			    }

			    // Tag close cases
			    if(reader.isEndElement()) {
			    	if(reader.getLocalName().equalsIgnoreCase("model")) {
			    		return newModel;
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
