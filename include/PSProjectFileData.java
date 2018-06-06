package edu.uwstout.berriers.PSHelper.Model;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Stack;

import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

import com.trolltech.qt.core.QSettings;

public class PSProjectFileData extends PSXMLReader implements PSStatusDescribable {

	private File mPSProjectFile;
	private Stack<File> mPathStack;
	private String mPSVersion;
	
	// Project Chunks
	private ArrayList<PSChunkData> mChunks;
	private int mActiveChunk;

	public PSProjectFileData(File pPSProjectFile, QSettings settings) throws IOException {
		// Clear out any old chunks by re-initializing the array
		mChunks = new ArrayList<PSChunkData>();
		mActiveChunk = -1;

		if(pPSProjectFile != null) {
			mPSProjectFile = pPSProjectFile;
			mPSProjectFile = mPSProjectFile.getCanonicalFile();
			parseProjectFile();
			mActiveChunk = 0;
		}
	}

	public File getPSProjectFile() { return mPSProjectFile; }
	public String getPSVersion() { return mPSVersion; }

	public boolean parseProjectFile() throws IOException {
		// Sanity Checks
		if(mPSProjectFile == null || !mPSProjectFile.exists() || !mPSProjectFile.isFile()) {
			return false;
		}
			
	    XMLStreamReader reader = null;
	    mPathStack = new Stack<File>();
	    mPathStack.push(mPSProjectFile);
	    
	    try {
			// Get the starting XML stream
			InputStream lXMLFileStream = getXMLStreamFromFile(mPSProjectFile);
	    	reader = XML_FACTORY.createXMLStreamReader(lXMLFileStream);
	    	
	    	// Loop over all element tags
	        while (reader.hasNext()) {
	        	reader.next();
	            if(reader.isStartElement())
	            {
                    switch (reader.getLocalName()) {
                    
                    	// The document tag contains the PS XML version and possibly a path
                        case "document":
                        	// Extract PhotoScan file version
                        	mPSVersion = reader.getAttributeValue(null, "version");
                        	
                        	// Attempt to descend
                        	XMLStreamReader oldReader = reader;
                        	reader = explodeTag(reader, mPathStack);
                        	if(oldReader != reader) { oldReader.close(); }
                        break;
                    
                        // The chunks tag is an array of chunk tags
                        case "chunks":
                        	readElementArray(reader, "chunks", "chunk");
                        break;	                        
                    }
            	}	            
            }
	    } catch (XMLStreamException e) {
	    	throw new IOException("XML parsing error.", e);
	    } finally {
			try { if(reader != null) reader.close(); }
			catch (XMLStreamException e) {}
	    }
		
		return true;
	}
	
	@Override
	public void processArrayElement(XMLStreamReader reader, String elementName) {
		switch(elementName) {
			case "chunk":
				// Process this chunk tag
				PSChunkData lNewChunk = null;				
				try {
					lNewChunk = new PSChunkData(mPSProjectFile, reader, mPathStack);
				} catch (IOException e) {
					System.err.println("Error parsing chunk");
					e.printStackTrace();
					return;
				}
				
				// Save the results
				if(lNewChunk != null) {
					mChunks.add(lNewChunk);
				}
			break;
				
			default: super.processArrayElement(reader, elementName); break;
		}
	}

	public int getChunkCount() {
		return mChunks.size();
	}
	
	public int getActiveChunkIndex() {
		return mActiveChunk;
	}

	public PSChunkData getActiveChunk() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk);
		}

		return null;
	}

	public PSChunkData getChunk(int index) {
		if(index >= 0 && index < mChunks.size()) {
			return mChunks.get(index);
		}

		return null;
	}

	public File getModelArchiveFile() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getModelArchiveFile();
		}

		return null;
	}
	
	public PSModelData getModelData() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getModelData();
		}
		
		return null;
	}
	
	@Override
	public String describeImageAlignPhase() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).describeImageAlignPhase();
		}

		return "N/A";
	}

	@Override
	public byte getAlignPhaseStatus() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getAlignPhaseStatus();			
		}
		
		return 0;
	}

	@Override
	public String describeDenseCloudPhase() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).describeDenseCloudPhase();			
		}
		
		return "N/A";
	}

	@Override
	public int getDenseCloudDepthImages() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getDenseCloudDepthImages();			
		}
		
		return 0;
	}
	
	@Override
	public byte getDenseCloudPhaseStatus() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getDenseCloudPhaseStatus();			
		}
		
		return 0;		
	}

	@Override
	public String describeModelGenPhase() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).describeModelGenPhase();			
		}
		
		return "N/A";
	}

	@Override
	public byte getModelGenPhaseStatus() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getModelGenPhaseStatus();			
		}
		
		return 0;
	}

	@Override
	public long getModelFaceCount() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getModelFaceCount();			
		}
		
		return 0;
	}

	@Override
	public long getModelVertexCount() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getModelVertexCount();			
		}
		
		return 0;
	}

	@Override
	public String describeTextureGenPhase() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).describeTextureGenPhase();			
		}
		
		return "N/A";
	}

	@Override
	public byte getTextureGenPhaseStatus() {
		if(mActiveChunk >= 0 && mActiveChunk < mChunks.size()) {
			return mChunks.get(mActiveChunk).getTextureGenPhaseStatus();			
		}
		
		return 0;
	}
}
