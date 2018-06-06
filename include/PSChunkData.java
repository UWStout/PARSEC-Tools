package edu.uwstout.berriers.PSHelper.Model;

import java.io.File;
import java.io.IOException;
import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;
import java.util.StringTokenizer;

import javax.xml.stream.XMLStreamConstants;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

public class PSChunkData extends PSXMLReader implements PSStatusDescribable {

	/**
	 * @author berriers
	 * Set of fixed levels of detail for the PhotoScan Image Alignment phase.
	 * These match values found in the standard PhotoScan XML file.
	 */
	public enum ImageAlignmentDetail {
		UNKNOWN ("Unknown Detail"),
		HIGH ("High Detail"),				// Check
		MEDIUM ("Medium Detail"),
		LOW ("Low Detail");
		
		public final String name;
		
		ImageAlignmentDetail(String name) {
			this.name = name;
		}
	}
	
	/**
	 * @author berriers
	 * Set of fixed levels of detail for the PhotoScan Dense Cloud phase.
	 * These match values found in the standard PhotoScan XML file.
	 */
	public enum DenseCloudDetail {
		UNKNOWN ("Unknown Detail"),
		ULTRA_HIGH ("Ultra High Detail"),	// Check
		HIGH ("High Detail"),
		MEDIUM ("Medium Detail"),
		LOW ("Low Detail"),
		LOWEST ("Lowest Detail");
		
		public final String name;
		
		DenseCloudDetail(String name) {
			this.name = name;
		}
	}

	/**
	 * @author berriers
	 * Set of fixed levels of filtering for the PhotoScan Dense Cloud phase.
	 * These match values found in the standard PhotoScan XML file.
	 */
	public enum DenseCloudFilter {
		UNKNOWN ("Unknown Filter"),
		DISABLED ("Filter Disabled"),
		AGGRESSIVE ("Aggressive Filter"),	// Check
		MODERATE ("Moderate Filter"),
		MILD ("Mild Filter");
		
		public final String name;
		
		DenseCloudFilter(String name) {
			this.name = name;
		}
	}

	/**
	 * @author berriers
	 * Set of fixed levels of detail for the PhotoScan Mesh Generation phase.
	 * These match values found in the standard PhotoScan XML file.
	 */
	public enum ModelGenerationDetail {
		UNKNOWN ("Unknown Detail"),
		LOW ("Low Detail"),
		MEDIUM ("Medium Detail"),
		HIGH ("High Detail"),
		CUSTOM ("Custom Detail");
		
		public final String name;
		
		ModelGenerationDetail(String name) {
			this.name = name;
		}
	}
	
	// What files was this chunk data read from
	File mSourceFile, mFrameFile;
	
	// General Information (from 'chunk' tag attributes)
	private long mID;
	private String mLabel;
	private boolean mEnabled;
	private boolean mInsideFrame;
	
	// Sensors in this chunk 
	private long mSensorCount_inChunk;
	
	// Markers & Scalebars
	private long mMarkerCount, mScalebarCount;	
		
	// Model data
	private PSModelData mModelData;
	
	// Image Alignment phase details
	private double mImageAlignment_matchDurationSeconds;
	private double mImageAlignment_alignDurationSeconds;
	private byte mImageAlignment_Level;
	private boolean mImageAlignment_Masked;
	private long mImageAlignment_featureLimit;
	private long mImageAlignment_tiePointLimit;
	
	// Camera Optimization Phase
	private double mOptimize_durationSeconds;
	private boolean mOptimize_aspect;
	private boolean mOptimize_f;
	private boolean mOptimize_cx;
	private boolean mOptimize_cy;
	private boolean mOptimize_b1;
	private boolean mOptimize_b2;
	private boolean mOptimize_p1;
	private boolean mOptimize_p2;
	private boolean mOptimize_p3;
	private boolean mOptimize_p4;
	private boolean mOptimize_k1;
	private boolean mOptimize_k2;
	private boolean mOptimize_k3;
	private boolean mOptimize_k4;
	private boolean mOptimize_skew;

	// Dense cloud generation phase
	private double mDenseCloud_durationSeconds;
	private byte mDenseCloud_level;
	private byte mDenseCloud_filterLevel;
	private byte mDenseCloud_imagesUsed;

	// Model Generation phase
	private double mModelGeneration_resolution;
	private double mModelGeneration_durationSeconds;
	private long mModelGeneration_faceCount;
	private boolean mModelGeneration_denseSource;
	private boolean mModelGeneration_interpolationEnabled;

	// Texture Generation phase
	private double mTextureGeneration_blendDuration;
	private double mTextureGeneration_uvGenDuration;
	private byte mTextureGeneration_mappingMode;
	private byte mTextureGeneration_blendMode;
	private byte mTextureGeneration_count;
	private int mTextureGeneration_width;
	private int mTextureGeneration_height;
	
	// Camera and Image data
	private Map<Long, PSSensorData> mSensors;
	private Map<Long, PSCameraData> mCameras;
	private ArrayList<PSImageData> mImages;
	
	// Stack used to track descending file structures
	Stack<File> mTempFileStack;	
	
	public PSChunkData(File pSourceFile, XMLStreamReader reader) throws IOException {
		this(pSourceFile, reader, null);
	}
	
	public PSChunkData(File pSourceFile, XMLStreamReader reader, Stack<File> pFileStack) throws IOException {
		mSourceFile = pSourceFile;		
		mTempFileStack = pFileStack;
		
		mID = 0;
		mLabel = "";
		mEnabled = false;
		mInsideFrame = false;
		
		mModelData = null;		
		mCameras = new HashMap<Long, PSCameraData>();
		mSensors = new HashMap<Long, PSSensorData>();
		mImages = new ArrayList<PSImageData>();
		
		mMarkerCount = mScalebarCount = 0;
		
		mOptimize_aspect = mOptimize_f = mOptimize_cx = mOptimize_cy = mOptimize_b1 = mOptimize_b2 = false;
		mOptimize_p1 = mOptimize_p2 = mOptimize_k1 = mOptimize_k2 = mOptimize_k3 = false;
		mOptimize_p3 = mOptimize_p4 = mOptimize_k4 = mOptimize_skew = false;
		
		mImageAlignment_Level = 0; mImageAlignment_Masked = false;
		mImageAlignment_featureLimit = mImageAlignment_tiePointLimit = 0;
		mDenseCloud_level = mDenseCloud_filterLevel = mDenseCloud_imagesUsed = 0;

		mModelGeneration_resolution = 0.0;
		mModelGeneration_faceCount = 0;
		mModelGeneration_denseSource = false;
		mModelGeneration_interpolationEnabled = false;
		
		mTextureGeneration_uvGenDuration = 0.0;
		mTextureGeneration_blendDuration = 0.0;
		mTextureGeneration_count = 0;
		mTextureGeneration_width = mTextureGeneration_height = 0;
		
		mImageAlignment_matchDurationSeconds = mImageAlignment_alignDurationSeconds = 0;
		mOptimize_durationSeconds = mDenseCloud_durationSeconds = 0;
		mModelGeneration_durationSeconds = 0;
		
		parseXMLChunk(reader);
	}

	public void parseXMLChunk(XMLStreamReader reader) throws IOException {

		// Sanity checks
		if(reader == null || reader.getLocalName() != "chunk") {
			return;
		}
		
		if(mTempFileStack == null) {
			mTempFileStack = new Stack<File>();
			mTempFileStack.push(mSourceFile);
		}
		
		try { reader = explodeTag(reader, mTempFileStack); }
		catch (XMLStreamException e) {
			throw new IOException("Error exploding tag", e);
		}
		
	    try {
	        while(reader.hasNext()) {
	            int event = reader.next();
	            switch(event) {
	            	case XMLStreamConstants.START_ELEMENT:
	            		switch(reader.getLocalName()) {
	            			case "sensors":
	            				readElementArray(reader, "sensors", "sensor");
	    					break;
	    					
	            			case "cameras":
	            				readElementArray(reader, "cameras", "camera");
	    					break;

	            			case "markers":
	            				readElementArray(reader, "markers", "marker");
        					break;
	            			
	            			case "scalebars":
	            				readElementArray(reader, "scalebars", "scalebar");
        					break;
	            			
	            			case "frames":
	            				readElementArray(reader, "frames", "frame");
	            			break;
        					
	            			case "property":
	            			{
	                        	String lPropertyName = reader.getAttributeValue(null, "name");
	                        	String lPropertyValue = reader.getAttributeValue(null, "value");	                        	
	                        	parseChunkProperty(lPropertyName, lPropertyValue);
	            			}
	            			break;
	            		}
	            	break;
	            }
	            
	        }
	    } catch (XMLStreamException e) {        	
	    	throw new IOException("XML parsing error.");
	    }  
	}
	
	@Override
	public void processArrayElement(XMLStreamReader reader, String elementName) {
		switch(elementName) {
			case "sensor":
			{
				try {
					PSSensorData lNewSensor = PSSensorData.makeFromXML(reader);
					mSensors.put(lNewSensor.ID, lNewSensor);
				} catch (XMLStreamException e) {
					System.err.println("Error while parsing XML sensor tag from chunk.");
					e.printStackTrace();
				}
			}
			break;
				
			case "camera":
			{
				if(mInsideFrame) {
					try {
						PSImageData lNewImage = PSImageData.makeFromXML(reader);
						if(mCameras.containsKey(lNewImage.getCamID())) {
							lNewImage.setCameraData(mCameras.get(lNewImage.getCamID()));
						}
						mImages.add(lNewImage);
					} catch (XMLStreamException e) {
						System.err.println("Error while parsing XML camera tag from frame.");
						e.printStackTrace();
					}					
				} else {
					try {
						PSCameraData lNewCamera = PSCameraData.makeFromXML(reader);
						if(mSensors.containsKey(lNewCamera.getSensorID())) {
							lNewCamera.setSensorData(mSensors.get(lNewCamera.getSensorID()));
						}
						mCameras.put(lNewCamera.ID, lNewCamera);
					} catch (XMLStreamException e) {
						System.err.println("Error while parsing XML camera tag from chunk.");
						e.printStackTrace();
					}
				}
			}
			break;
			
			case "depth_map":
				addDepthImage();
			break;
			
			case "marker":
				mMarkerCount++;
			break;

			case "scalebar":
				mScalebarCount++;
			break;
			
			case "frame":
				try { parseXMLFrame(reader); }
				catch(IOException e) { 
					System.err.println("Error while parsing XML frame tag from chunk.");
					e.printStackTrace();
				}
			break;

			// Call super version by default
			default: super.processArrayElement(reader, elementName); break;
		}
	}
	
	private void parseXMLFrame(XMLStreamReader reader) throws IOException {
		
		// Sanity checks
		if(reader == null || reader.getLocalName() != "frame") {
			return;
		}

		// Follow any path attrib if needed
		XMLStreamReader preFrameReader = reader;
		try {
			reader = explodeTag(reader, mTempFileStack);
			mFrameFile = mTempFileStack.peek();
		} catch (XMLStreamException e) {
			throw new IOException("Error exploding tag", e);
		}

		mInsideFrame = true;

		// Parse the elements within the frame tag
	    try {
	        while(reader.hasNext()) {
	            int event = reader.next();
	            switch(event) {
	            	case XMLStreamConstants.START_ELEMENT:
	            		switch(reader.getLocalName()) {
	            			case "cameras":
	            				readElementArray(reader, "cameras", "camera");
	            			break;
	            			
	            			case "markers":
		            			// Ignore all the markers inside the frame tag
		            			while(!reader.isEndElement() || !reader.getLocalName().equals("markers")) {
		            				event = reader.next();
		            			}
		            		break;
		            		
	            			case "depth_maps":
	            				readElementArray(reader, "depth_maps", "depth_map");
	            			break;
	            			
	            			case "thumbnails": break;
	            			
	            			case "point_cloud": break;
	            			case "dense_cloud": break;
	            			
	            			case "model":
	            			{
	            				// Dive into the model tag
	            				XMLStreamReader preModelReader = reader;
	            				try { reader = explodeTag(reader, mTempFileStack); }
	            				catch (XMLStreamException e) {
	            					throw new IOException("Error exploding tag", e);
	            				}
	            				
	            				// Build the model object
	            				mModelData = PSModelData.makeFromXML(reader, mTempFileStack.peek(), this);
	            				
	            				// Return to old stream
	            				if(reader != preModelReader) {
	            					mTempFileStack.pop();
	            					reader.close();
		            				reader = preModelReader;
	            				}
	            			}
	            			break;

	            			case "property":
	            			{
	                        	String lPropertyName = reader.getAttributeValue(null, "name");
	                        	String lPropertyValue = reader.getAttributeValue(null, "value");	                        	
	                        	parseChunkProperty(lPropertyName, lPropertyValue);
	            			}
	            			break;
	            		}
	            	break;
	            }
	        }
	        
		    if(preFrameReader != reader) {
				mTempFileStack.pop();
				reader.close();
				reader = preFrameReader;
		    }
	    } catch (XMLStreamException e) {        	
			mInsideFrame = false;
	    	throw new IOException("XML parsing error.");
	    }	    

	    mInsideFrame = false;
	}

	// Read a property tag that's inside a chunk (1 or MORE levels below)
    public void parseChunkProperty(String pPropertyName, String pPropertyValueStr) {

    	double lPropertyValue = 0;
    	try { lPropertyValue = Double.parseDouble(pPropertyValueStr); }
    	catch(Exception e) {}
    	
    	switch(pPropertyName) {

    		// Texture Generation Properties
    		case "atlas/atlas_blend_mode": this.setTextureGeneration_blendMode((byte)lPropertyValue); break;
    		case "atlas/atlas_count": this.setTextureGeneration_count((byte)lPropertyValue); break;
    		case "atlas/atlas_height": this.setTextureGeneration_height((int)lPropertyValue); break;
    		case "atlas/atlas_mapping_mode": this.setTextureGeneration_mappingMode((byte)lPropertyValue); break;
    		case "atlas/atlas_width": this.setTextureGeneration_width((int)lPropertyValue); break;
    		    		
    		// Model Generation Properties
    		case "model/depth_downscale": break;
    		case "model/depth_filter_mode": break; //value="2"/>
    		case "model/mesh_face_count": this.setModelGeneration_faceCount((long)lPropertyValue); break;
    		case "model/mesh_interpolation": this.setModelGeneration_interpolationEnabled((lPropertyValue)==1); break;
    		case "model/mesh_object_type": break; //value="0"/>
    		case "model/mesh_source_data": this.setModelGeneration_denseSource((lPropertyValue)==1); break;
    		case "model/resolution": this.setModelGeneration_resolution(lPropertyValue); break;
    	
    		// Dense cloud properties
			case "dense_cloud/depth_downscale": this.setDenseCloud_level((byte)lPropertyValue); break;
			case "dense_cloud/depth_filter_mode": this.setDenseCloud_filterLevel((byte)lPropertyValue); break;
			case "dense_cloud/density": break;
			case "dense_cloud/resolution": break;
	
			// Image Alignment properties
			case "match/match_downscale": this.setImageAlignment_Level((byte)lPropertyValue); break;
			case "match/match_filter_mask": this.setImageAlignment_Masked((lPropertyValue)==0?false:true); break;
			case "match/match_point_limit": this.setImageAlignment_featureLimit((long)lPropertyValue); break;
			case "match/match_tiepoint_limit": this.setImageAlignment_tiePointLimit((long)lPropertyValue); break;
			case "match/match_select_pairs": break;
			
			// Duration properties
			case "match/duration": this.setImageAlignment_matchDurationSeconds(lPropertyValue); break;
			case "optimize/duration": this.setOptimize_durationSeconds(lPropertyValue); break;
			case "dense_cloud/duration": this.setDenseCloud_durationSeconds(lPropertyValue); break;
			case "align/duration": this.setImageAlignment_alignDurationSeconds(lPropertyValue); break;
    		case "model/duration": this.setModelGeneration_durationSeconds(lPropertyValue); break;
    		case "atlas/duration_blend": this.setTextureGeneration_blendDuration(lPropertyValue); break;
    		case "atlas/duration_uv": this.setTextureGeneration_uvGenDuration(lPropertyValue); break;
			
			// Fitting properties
    		case "optimize/fit_flags":
    			StringTokenizer tok = new StringTokenizer(pPropertyValueStr, " ");
    			while(tok.hasMoreTokens()) {
    				switch(tok.nextToken()) {
    					case "f": this.setOptimize_f(true); break;
    					case "cx": this.setOptimize_cx(true); break;
    					case "cy": this.setOptimize_cy(true); break;
    					case "b1": this.setOptimize_b1(true); break;
    					case "b2": this.setOptimize_b2(true); break;
    					case "k1": this.setOptimize_k1(true); break;
    					case "k2": this.setOptimize_k2(true); break;
    					case "k3": this.setOptimize_k3(true); break;
    					case "k4": this.setOptimize_k4(true); break;
    					case "p1": this.setOptimize_p1(true); break;
    					case "p2": this.setOptimize_p2(true); break;
    					case "p3": this.setOptimize_p3(true); break;
    					case "p4": this.setOptimize_p4(true); break;
    				}
    			}
    		break;
    			
			case "optimize/fit_aspect": this.setOptimize_aspect((lPropertyValue==0?false:true)); break;
			
			case "optimize/fit_cxcy":
				this.setOptimize_cx((lPropertyValue==0?false:true));
				this.setOptimize_cy((lPropertyValue==0?false:true)); 
			break;
			
			case "optimize/fit_f": this.setOptimize_f((lPropertyValue==0?false:true)); break;
			
			case "optimize/fit_k1k2k3":
				this.setOptimize_k1((lPropertyValue==0?false:true));
				this.setOptimize_k2((lPropertyValue==0?false:true));
				this.setOptimize_k3((lPropertyValue==0?false:true));
			break;
			
			case "optimize/fit_p1p2":
				this.setOptimize_p1((lPropertyValue==0?false:true));
				this.setOptimize_p2((lPropertyValue==0?false:true));
			break;
			
			case "optimize/fit_skew": this.setOptimize_skew((lPropertyValue==0?false:true)); break;
			case "optimize/fit_k4": this.setOptimize_k4((lPropertyValue==0?false:true)); break;

    		// Accuracy properties
    		case "accuracy_tiepoints": break;
    		case "accuracy_cameras": break;
    		case "accuracy_cameras_ypr": break;
    		case "accuracy_markers": break;
    		case "accuracy_scalebars": break;
    		case "accuracy_projections": break;
    	}
    }
    
    @Override
    public String toString() {
    	
    	// General details
    	String lDetails = "\tChunk ID: " + mID + ", Label: ";
    	if(mLabel != null && mLabel != "") {
    		lDetails += mLabel + "\n";
    	} else {
    		lDetails += "[none]\n";
    	}
    	
		lDetails += "\t" + getImageCount() + " image(s), "
			     + mSensorCount_inChunk + " sensor(s), "
				 + getDenseCloud_imagesUsed() + " depth map(s)\n";

    	if(!mEnabled) { lDetails += "\tDISABLED\n"; }

    	// Image alignment details
    	if(mImageAlignment_Level == 0) {
    		lDetails += "\n\tImage Align - no data\n";    		
    	} else {
			long lMillisecs = (long)((mImageAlignment_matchDurationSeconds + mImageAlignment_alignDurationSeconds)*1000);
			Duration lAlignTime = Duration.ofMillis(lMillisecs);
	 
			lDetails += "\n\tImage Align";
			if(lAlignTime != Duration.ZERO) {	lDetails += " - " + formatDuration(lAlignTime); }
			lDetails += "\n";
			
			
			lDetails += "\t              " + ImageAlignmentDetail.values()[mImageAlignment_Level].name;
			if(mImageAlignment_Masked) { lDetails += ", MASKED"; }
			lDetails += "\n";
			
			lDetails += "\t              " + mImageAlignment_featureLimit + " key point limit\n";
			lDetails += "\t              " + mImageAlignment_tiePointLimit + " tie point limit\n";
    	}

		// Optimization details
		Duration lOptimizeTime = Duration.ofMillis((long)(mOptimize_durationSeconds*1000));
		if(lOptimizeTime != Duration.ZERO) {		
			lDetails += "\n\tOptimization - " + formatDuration(lOptimizeTime);
			
			int lCount = 0;
			if(mOptimize_aspect) { lDetails += addOptimizeElement("aspect", lCount++); }
			if(mOptimize_f) { lDetails += addOptimizeElement("f", lCount++); }
			if(mOptimize_cx) { lDetails += addOptimizeElement("Cx", lCount++); }
			if(mOptimize_cy) { lDetails += addOptimizeElement("Cy", lCount++); }
			if(mOptimize_b1) { lDetails += addOptimizeElement("B1", lCount++); }
			if(mOptimize_b2) { lDetails += addOptimizeElement("B2", lCount++); }
			if(mOptimize_p1) { lDetails += addOptimizeElement("P1", lCount++); }
			if(mOptimize_p2) { lDetails += addOptimizeElement("P2", lCount++); }
			if(mOptimize_p3) { lDetails += addOptimizeElement("P3", lCount++); }
			if(mOptimize_p4) { lDetails += addOptimizeElement("P4", lCount++); }
			if(mOptimize_k1) { lDetails += addOptimizeElement("k1", lCount++); }
			if(mOptimize_k2) { lDetails += addOptimizeElement("k2", lCount++); }
			if(mOptimize_k3) { lDetails += addOptimizeElement("k3", lCount++); }
			if(mOptimize_k4) { lDetails += addOptimizeElement("k4", lCount++); }
			if(mOptimize_skew) { lDetails += addOptimizeElement("skew", lCount++); }
			if(lCount > 0) { lDetails = lDetails.substring(0,  lDetails.length()-2); }
			lDetails += "\n";
		}
		
		// Dense cloud details
		if(mDenseCloud_filterLevel == 0) {
			lDetails += "\n\tDense Cloud - no data\n";
		} else {
			Duration lDenseCloudTime = Duration.ofMillis((long)(mDenseCloud_durationSeconds*1000));
			lDetails += "\n\tDense Cloud";
			if(lDenseCloudTime != Duration.ZERO) {	lDetails += " - " + formatDuration(lDenseCloudTime); }
			lDetails += "\n";

			lDetails += "\t              " + DenseCloudDetail.values()[mDenseCloud_level].name;
			lDetails += ", " + DenseCloudFilter.values()[mDenseCloud_filterLevel].name + "\n";
		}
		
		// Model generation details
		if(mModelData == null) {
			lDetails += "\n\tModel Generation - no data\n";
		} else {			
			Duration lModelGenTime = Duration.ofMillis((long)(mModelGeneration_durationSeconds*1000));
			lDetails += "\n\tModel Generation";
			if(lModelGenTime != Duration.ZERO) {	lDetails += " - " + formatDuration(lModelGenTime); }
			lDetails += "\n";
			
			lDetails += "\t                   " + mModelData.getFaceCount() + " faces\n";
		}
		
		return lDetails;
    }

	/**
	 * A helper for adding the various 'optimization' options to the main string
	 * returned by toString().
	 * 
	 * @param pName The name of the optimization (will be added to the string)
	 * @param pCount How many optimizations have been added already (will break at 4)
	 * @return A string to be appended to the toString() results
	 */
	private String addOptimizeElement(String pName, int pCount)
	{
		String result = "";
		if(pCount%4 == 0) { result += "\n\t               "; }
		result += pName + ", ";
		return result;
	}
	
	/**
	 * Format a java.time.Duration as a more human readable string than
	 * Duration.toString() gives.
	 * 
	 * @param pTime The duration object you want to format as a string.
	 * @return The duration object expressed as a human readable string.
	 */
	private String formatDuration(Duration pTime) {
		String result = pTime.toDays() + "d ";
		pTime = pTime.minusDays(pTime.toDays());
		
		result += pTime.toHours() + "h ";
		pTime = pTime.minusHours(pTime.toHours());

		result += pTime.toMinutes() + "m ";
		pTime = pTime.minusMinutes(pTime.toMinutes());
		
		result += String.format("%.2f", pTime.toNanos()*1.0e-9) + "s";

		return result;
	}
	
	public String getLabel() { return mLabel; }
	public void addMarker() { mMarkerCount++; }
	public void addScalebar() { mScalebarCount++; }	
	public long getMarkerCount() { return mMarkerCount; }
	public long getScalebarCount() { return mScalebarCount; }
	
	public void addImage(PSImageData pImage) { mImages.add(pImage); }	
	public int getImageCount() { return mImages.size(); }
	
	public int getCameraCount() { return mCameras.size(); }

	public void addSensor() { mSensorCount_inChunk++; }
	public long getSensorCount() {
		return mSensorCount_inChunk;
	}

	public void setModelData(PSModelData pModelData) {
		mModelData = pModelData;
	}
		
	public double getImageAlignment_matchDurationSeconds() {
		return mImageAlignment_matchDurationSeconds;
	}

	public void setImageAlignment_matchDurationSeconds(
			double mImageAlignment_matchDurationSeconds) {
		this.mImageAlignment_matchDurationSeconds = mImageAlignment_matchDurationSeconds;
	}

	public double getImageAlignment_alignDurationSeconds() {
		return mImageAlignment_alignDurationSeconds;
	}

	public void setImageAlignment_alignDurationSeconds(
			double mImageAlignment_alignDurationSeconds) {
		this.mImageAlignment_alignDurationSeconds = mImageAlignment_alignDurationSeconds;
	}

	public String getImageAlignment_durationString() {
		long lMillisecs = (long)((mImageAlignment_matchDurationSeconds + mImageAlignment_alignDurationSeconds)*1000);
		Duration lAlignTime = Duration.ofMillis(lMillisecs);
		return formatDuration(lAlignTime);
	}
	
	public byte getImageAlignment_Level() {
		return mImageAlignment_Level;
	}

	public String getImageAlignment_LevelString() {
		return ImageAlignmentDetail.values()[mImageAlignment_Level].name;
	}

	public void setImageAlignment_Level(byte mImageAlignment_Level) {
		if(mImageAlignment_Level < 0 || mImageAlignment_Level >= ImageAlignmentDetail.values().length) {
			System.err.println("ERROR: bad image alignment detail value (" + mImageAlignment_Level + ")");
			return;
		}
		this.mImageAlignment_Level = mImageAlignment_Level;
	}

	public boolean getImageAlignment_Masked() {
		return mImageAlignment_Masked;
	}

	public void setImageAlignment_Masked(boolean mImageAlignment_Masked) {
		this.mImageAlignment_Masked = mImageAlignment_Masked;
	}

	public long getImageAlignment_featureLimit() {
		return mImageAlignment_featureLimit;
	}

	public void setImageAlignment_featureLimit(long mImageAlignment_featureLimit) {
		this.mImageAlignment_featureLimit = mImageAlignment_featureLimit;
	}

	public long getImageAlignment_tiePointLimit() {
		return mImageAlignment_tiePointLimit;
	}

	public void setImageAlignment_tiePointLimit(long mImageAlignment_tiePointLimit) {
		this.mImageAlignment_tiePointLimit = mImageAlignment_tiePointLimit;
	}

	public String getOptimizeString() {
		String lDesc = "";

		if(mOptimize_aspect) { lDesc += "aspect, "; }
		if(mOptimize_f) { lDesc += "fv, "; }
		if(mOptimize_cx) { lDesc += "Cx, "; }
		if(mOptimize_cy) { lDesc += "Cy, "; }
		if(mOptimize_b1) { lDesc += "B1, "; }
		if(mOptimize_b2) { lDesc += "B2, "; }
		if(mOptimize_p1) { lDesc += "P1, "; }
		if(mOptimize_p2) { lDesc += "P2, "; }
		if(mOptimize_p3) { lDesc += "P3, "; }
		if(mOptimize_p4) { lDesc += "P4, "; }
		if(mOptimize_k1) { lDesc += "k1, ";  }
		if(mOptimize_k2) { lDesc += "k2, ";  }
		if(mOptimize_k3) { lDesc += "k3, ";  }
		if(mOptimize_k4) { lDesc += "k4, "; }
		if(mOptimize_skew) { lDesc += "skew, "; }
		
		if(!lDesc.isEmpty()) { lDesc = lDesc.substring(0, lDesc.length()-2); }		
		return lDesc;
	}
	
	public double getOptimize_durationSeconds() {
		return mOptimize_durationSeconds;
	}

	public String getOptimize_durationString() {
		long lMillisecs = (long)(mOptimize_durationSeconds*1000);
		Duration lOptimizeTime = Duration.ofMillis(lMillisecs);
		return formatDuration(lOptimizeTime);		
	}
	
	public void setOptimize_durationSeconds(double mOptimize_durationSeconds) {
		this.mOptimize_durationSeconds = mOptimize_durationSeconds;
	}

	public boolean getOptimize_aspect() {
		return mOptimize_aspect;
	}

	public void setOptimize_aspect(boolean mOptimize_aspect) {
		this.mOptimize_aspect = mOptimize_aspect;
	}

	public boolean getOptimize_f() {
		return mOptimize_f;
	}

	public void setOptimize_f(boolean mOptimize_f) {
		this.mOptimize_f = mOptimize_f;
	}

	public boolean getOptimize_cx() {
		return mOptimize_cx;
	}

	public boolean getOptimize_cy() {
		return mOptimize_cy;
	}

	public void setOptimize_cx(boolean mOptimize_cx) {
		this.mOptimize_cx = mOptimize_cx;
	}

	public void setOptimize_cy(boolean mOptimize_cy) {
		this.mOptimize_cy = mOptimize_cy;
	}

	public boolean getOptimize_p1() {
		return mOptimize_p1;
	}

	public boolean getOptimize_p2() {
		return mOptimize_p2;
	}

	public boolean getOptimize_p3() {
		return mOptimize_p3;
	}

	public boolean getOptimize_p4() {
		return mOptimize_p4;
	}

	public void setOptimize_p1(boolean mOptimize_p1) {
		this.mOptimize_p1 = mOptimize_p1;
	}

	public void setOptimize_p2(boolean mOptimize_p2) {
		this.mOptimize_p2 = mOptimize_p2;
	}
	
	public void setOptimize_p3(boolean mOptimize_p1) {
		this.mOptimize_p1 = mOptimize_p1;
	}

	public void setOptimize_p4(boolean mOptimize_p2) {
		this.mOptimize_p2 = mOptimize_p2;
	}

	public boolean getOptimize_b1() {
		return mOptimize_b1;
	}

	public boolean getOptimize_b2() {
		return mOptimize_b2;
	}

	public void setOptimize_b1(boolean mOptimize_b1) {
		this.mOptimize_b1 = mOptimize_b1;
	}

	public void setOptimize_b2(boolean mOptimize_b2) {
		this.mOptimize_b2 = mOptimize_b2;
	}
	
	public boolean getOptimize_k1() {
		return mOptimize_k1;
	}

	public boolean getOptimize_k2() {
		return mOptimize_k2;
	}

	public boolean getOptimize_k3() {
		return mOptimize_k3;
	}

	public boolean getOptimize_k4() {
		return mOptimize_k4;
	}

	public void setOptimize_k1(boolean mOptimize_k1) {
		this.mOptimize_k1 = mOptimize_k1;
	}

	public void setOptimize_k2(boolean mOptimize_k2) {
		this.mOptimize_k2 = mOptimize_k2;
	}

	public void setOptimize_k3(boolean mOptimize_k3) {
		this.mOptimize_k3 = mOptimize_k3;
	}

	public void setOptimize_k4(boolean mOptimize_k4) {
		this.mOptimize_k4 = mOptimize_k4;
	}

	public boolean getOptimize_skew() {
		return mOptimize_skew;
	}

	public void setOptimize_skew(boolean mOptimize_skew) {
		this.mOptimize_skew = mOptimize_skew;
	}

	public double getDenseCloud_durationSeconds() {
		return mDenseCloud_durationSeconds;
	}

	public String getDenseCloud_durationString() {
		long lMillisecs = (long)(mDenseCloud_durationSeconds*1000);
		Duration lDenseCloudTime = Duration.ofMillis(lMillisecs);
		return formatDuration(lDenseCloudTime);
	}

	public void setDenseCloud_durationSeconds(double mDenseCloud_durationSeconds) {
		this.mDenseCloud_durationSeconds = mDenseCloud_durationSeconds;
	}

	public byte getDenseCloud_level() {
		return mDenseCloud_level;
	}

	public String getDenseCloud_levelString() {
		return DenseCloudDetail.values()[mDenseCloud_level].name;
	}
	
	public void setDenseCloud_level(byte mDenseCloud_level) {
		if(mDenseCloud_level < 0 || mDenseCloud_level >= DenseCloudDetail.values().length) {
			System.err.println("ERROR: bad dense cloud detail value (" + mDenseCloud_level + ")");
			return;
		}
		this.mDenseCloud_level = mDenseCloud_level;
	}

	public int getDenseCloud_imagesUsed() {
		return mDenseCloud_imagesUsed;
	}

	public void addDepthImage() {
		mDenseCloud_imagesUsed++;
	}
	
	public byte getDenseCloud_filterLevel() {
		return mDenseCloud_filterLevel;
	}

	public String getDenseCloud_filterLevelString() {
		return DenseCloudFilter.values()[mDenseCloud_filterLevel].name;
	}
	
	public void setDenseCloud_filterLevel(byte mDenseCloud_filterLevel) {
		if(mDenseCloud_filterLevel < 0 || mDenseCloud_filterLevel >= DenseCloudFilter.values().length) {
			System.err.println("ERROR: bad dense cloud filter value (" + mDenseCloud_filterLevel + ")");
			return;
		}
		this.mDenseCloud_filterLevel = mDenseCloud_filterLevel;
	}

	public double getModelGeneration_resolution() {
		return mModelGeneration_resolution;
	}

	public void setModelGeneration_resolution(double mModelGeneration_resolution) {
		this.mModelGeneration_resolution = mModelGeneration_resolution;
	}

	public double getModelGeneration_durationSeconds() {
		return mModelGeneration_durationSeconds;
	}

	public String getModelGeneration_durationString() {
		long lMillisecs = (long)(mModelGeneration_durationSeconds*1000);
		Duration lModelGenTime = Duration.ofMillis(lMillisecs);
		return formatDuration(lModelGenTime);		
	}
	
	public void setModelGeneration_durationSeconds(
			double mModelGeneration_durationSeconds) {
		this.mModelGeneration_durationSeconds = mModelGeneration_durationSeconds;
	}

	public PSModelData getModelData() {
		return mModelData;
	}
	

	public long getModelGeneration_faceCount() {
		return mModelGeneration_faceCount;
	}

	public void setModelGeneration_faceCount(long mModelGeneration_faceCount) {
		this.mModelGeneration_faceCount = mModelGeneration_faceCount;
	}

	public boolean getModelGeneration_denseSource() {
		return mModelGeneration_denseSource;
	}

	public void setModelGeneration_denseSource(boolean mModelGeneration_denseSource) {
		this.mModelGeneration_denseSource = mModelGeneration_denseSource;
	}

	public boolean getModelGeneration_interpolationEnabled() {
		return mModelGeneration_interpolationEnabled;
	}

	public void setModelGeneration_interpolationEnabled(boolean mModelGeneration_interpolationEnabled) {
		this.mModelGeneration_interpolationEnabled = mModelGeneration_interpolationEnabled;
	}

	public double getTextureGeneration_blendDuration() {
		return mTextureGeneration_blendDuration;
	}

	public void setTextureGeneration_blendDuration(double mTextureGeneration_blendDuration) {
		this.mTextureGeneration_blendDuration = mTextureGeneration_blendDuration;
	}

	public double getTextureGeneration_uvGenDuration() {
		return mTextureGeneration_uvGenDuration;
	}
	
	public void setTextureGeneration_uvGenDuration(double mTextureGeneration_uvGenDuration) {
		this.mTextureGeneration_uvGenDuration = mTextureGeneration_uvGenDuration;
	}

	public String getTextureGeneration_durationString() {
		long lMillisecs = (long)((mTextureGeneration_blendDuration+mTextureGeneration_uvGenDuration)*1000);
		Duration lTexGenTime = Duration.ofMillis(lMillisecs);
		return formatDuration(lTexGenTime);
	}
	
	public byte getTextureGeneration_mappingMode() {
		return mTextureGeneration_mappingMode;
	}

	public void setTextureGeneration_mappingMode(byte mTextureGeneration_mappingMode) {
		this.mTextureGeneration_mappingMode = mTextureGeneration_mappingMode;
	}

	public byte getTextureGeneration_blendMode() {
		return mTextureGeneration_blendMode;
	}

	public void setTextureGeneration_blendMode(byte mTextureGeneration_blendMode) {
		this.mTextureGeneration_blendMode = mTextureGeneration_blendMode;
	}

	public byte getTextureGeneration_count() {
		return mTextureGeneration_count;
	}

	public void setTextureGeneration_count(byte mTextureGeneration_count) {
		this.mTextureGeneration_count = mTextureGeneration_count;
	}

	public int getTextureGeneration_width() {
		return mTextureGeneration_width;
	}

	public void setTextureGeneration_width(int mTextureGeneration_width) {
		this.mTextureGeneration_width = mTextureGeneration_width;
	}

	public int getTextureGeneration_height() {
		return mTextureGeneration_height;
	}

	public void setTextureGeneration_height(int mTextureGeneration_height) {
		this.mTextureGeneration_height = mTextureGeneration_height;
	}
	
	// Output string of the form ([num aligned images] - [feature limit]/[tie point limit])
	@Override
	public String describeImageAlignPhase() {
		String lData = getImageAlignment_LevelString();
		long featLimit = getImageAlignment_featureLimit()/1000;
		long tieLimit = getImageAlignment_tiePointLimit()/1000;
		lData += " (" + mImages.size() + " - " + featLimit + "k/" + tieLimit + "k)";
		return lData;
	}

	// Compute ratio of total images to aligned images and return status
	@Override
	public byte getAlignPhaseStatus() {
		long allImages = mCameras.size();
		long alignedImages = mImages.size();
		double ratio = alignedImages/(double)allImages;
		
		if(alignedImages == 0 || describeImageAlignPhase() == "N/A") {
			return 5;
		} else if(ratio < .100) {
			return 4;
		} else if(ratio < .3333) {
			return 3;
		} else if(ratio < .6667) {
			return 2;
		} else if(ratio < .950) {
			return 1;
		} else {
			return 0;
		}
	}
	
	@Override
	public String describeDenseCloudPhase() {
		String lData = getDenseCloud_levelString();
		lData += " (" + getDenseCloud_imagesUsed() + ")";
		return lData;
	}
	
	@Override
	public int getDenseCloudDepthImages() {
		return getDenseCloud_imagesUsed();
	}
	
	@Override
	public byte getDenseCloudPhaseStatus() {
		long projectImages = mCameras.size();
		long depthImages = mDenseCloud_imagesUsed;
		double ratio = depthImages/(double)projectImages;		
		if(depthImages == 0) {
			if(describeDenseCloudPhase() == "N/A") {
				return 5;
			} else {
				return 3;
			}
		} else if(ratio < .100) {
			return 4;
		} else if(ratio < .3333) {
			return 3;
		} else if(ratio < .6667) {
			return 2;
		} else if(ratio < .950) {
			return 1;
		} else {
			return 0;
		}
	}

	@Override
	public String describeModelGenPhase() {
		double faces = getModelFaceCount();
		if(faces < 0) {
			if(hasMesh()) { return "?"; }
			else { return "N/A"; }
		}
		
		faces /= 1000;
		if(faces >= 1000) {
			return String.format("%.1fM faces", faces/1000);
		} else {
			return String.format("%.1fK faces", faces);
		}
	}

	public boolean hasMesh() {
		return mModelData != null;
	}
	
	public File getModelArchiveFile() {
		if(mModelData != null) {
			return mModelData.getArchiveFile();
		}
		
		return null;
	}
	
	@Override
	public long getModelFaceCount() {	
		if(!hasMesh()) return -1L;
		return mModelData.getFaceCount();
	}

	@Override
	public long getModelVertexCount() {
		if(!hasMesh()) return -1L;
		return mModelData.getVertexCount();
	}

	@Override
	public byte getModelGenPhaseStatus() {
		long faceCount = getModelFaceCount();
		
		// Examine the model resolution
		if(faceCount < 0) {
			return 5;
		} else if(faceCount < 5000) {
			return 4;
		} else if(faceCount < 10000) {
			return 3;
		} else if(faceCount < 50000) {
			return 2;
		} else if(faceCount < 1000000) {
			return 1;	
		} else {
			return 0;
		}
	}

	@Override
	public String describeTextureGenPhase() {
		if(getTextureGeneration_count() != 0) {
			return String.format("%d @ (%d x %d)", getTextureGeneration_count(),
					getTextureGeneration_width(), getTextureGeneration_height());
		}
		
		return "N/A";
	}
	
	@Override
	public byte getTextureGenPhaseStatus() {
		
		// Examine the texture resolution
		if(getTextureGeneration_width() == 0 || getTextureGeneration_height() == 0) {
			return 5;
		} else if(getTextureGeneration_width() < 1024 || getTextureGeneration_height() < 1024) {
			return 4;
		} else if(getTextureGeneration_width() < 2048 || getTextureGeneration_height() < 2048) {
			return 3;
		} else if(getTextureGeneration_width() < 3072 || getTextureGeneration_height() < 3072) {
			return 2;
		} else if(getTextureGeneration_width() < 4096 || getTextureGeneration_height() < 4069) {
			return 1;	
		} else {
			return 0;
		}
	}
}
