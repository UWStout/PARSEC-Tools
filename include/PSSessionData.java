package edu.uwstout.berriers.PSHelper.Model;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.StringJoiner;

import com.trolltech.qt.core.QSettings;

import edu.uwstout.berriers.PSHelper.app.ExposureSettings;
import edu.uwstout.berriers.PSHelper.app.ExtensionFileFilter;
import edu.uwstout.berriers.PSHelper.app.ImageProcessorIM4J;
import edu.uwstout.berriers.PSHelper.app.ImageProcessorIM4J.*;

public class PSSessionData implements Comparable<PSSessionData>, PSStatusDescribable {

	public enum Columns {
		
		// Reduced columns
		PROJECT_ID("ID", "Unique identifier parsed from the containing folder"),
		PROJECT_NAME("Name", "Short name of the object being photographed"),
		PHOTO_DATE("Date Shot", "The date the photographs were taken"),
		ACTIVE_VERSION("Version", "Which project file is currently active (if more than one found)"),
		ACTIVE_CHUNK("Chunk", "Which chunk is currently active (if more than one found)"),
		IMAGE_COUNT_REAL("Image Count", "The number of images in the project"),

		// Extended columns
		PROJECT_STATUS("Status", "Status of this model in the processing pipeline" ),
		IMAGE_ALIGN_LEVEL("Image Align", "Details from the image alignment phase of PhotoScan"),
		DENSE_CLOUD_LEVEL("Dense Cloud", "Details from the dense cloud generation phase of PhotoScan"),
		MODEL_GEN_LEVEL("Model", "Details from the model generation phase of PhotoScan"),
		TEXTURE_GEN_LEVEL("Texture", "Details from the texture generation phase of PhotoScan"),
		
		// Other
		PROJECT_FOLDER("Folder", "The folder that holds the data for this project"),
		PROJECT_NOTE("Note", "A custom note for this project");

		// The number of fields shown for base and extended modes
		public static final byte BASE_LENGTH = 6;
		public static final byte EXTENDED_LENGTH = 11;

		// Info for each enum entry
		public final String name;
		public final String tooltip;

		// Gather data from each enum entry
		private Columns(String name, String tooltip) {
			this.name = name;
			this.tooltip = tooltip;
		}
		
		public String toString() {
			return name;
		}
	}
	
	private static Columns mSortBy = Columns.PROJECT_ID;
	private static int mNextID = 0;
	
	// Path to the project file
	private File mPSProjectFolder;
	
	// Information about images and sensors
	private long mImageCount_raw, mImageCount_processed;
	private File[] mRawFileList;

	// General PS Project information
	private String mID;

	// Image exposure with dcraw
	private ExposureSettings mExposure;
	private Date mDateTakenStart, mDateTakenFinish;

	// Custom data for this project
	private boolean mResultsApproved;
	private String mSpecialNotes;
	private String mName;
	private Status mStatus;
	
	// The list of project files in the directory
	private File[] mPSProjectFileList;
	private PSProjectFileData[] mPSProjectList;
	private int mActiveProject;
	
	// File filter setup
	private static final ExtensionFileFilter mDirectoryFilter = new ExtensionFileFilter(true);

	private static final String[] mPSProjectFileExtensions = { ".psz", ".psx" };
	private static final ExtensionFileFilter mPSProjectFilter =
			new ExtensionFileFilter(mPSProjectFileExtensions);

	// Most of the image types supported by PhotoScan (we leave out ones we can't support)
	private static final String[] mPSImageFileExtensions = {
		".jpg", ".jpeg", ".tif", ".tiff", ".pgm", ".ppm",
		".png", ".bmp", ".exr"
	};
	
	private static final ExtensionFileFilter mPSImageFileFilter = 
			new ExtensionFileFilter(mPSImageFileExtensions);

	// Extension list from https://en.wikipedia.org/wiki/Raw_image_format
	private static final String[] mRawFileExtensions = {
			".3fr",
			".ari",".arw",
			".bay",
			".crw",".cr2",
			".cap",
			".dcs",".dcr",".dng",
			".drf",
			".eip",".erf",
			".fff",
			".iiq",
			".k25",".kdc",
			".mdc",".mef",".mos",".mrw",
			".nef",".nrw",
			".obm",".orf",
			".pef",".ptx",".pxn",
			".r3d",".raf",".raw",".rwl",".rw2",".rwz",
			".sr2",".srf",".srw",
			".x3f"
		};
	
	private static final ExtensionFileFilter mRawFileFilter =
			new ExtensionFileFilter(mRawFileExtensions);
	
	/**
	 * Construct a new PSProjectData that points to the PSZ project file
	 * indicated by the path string.  The folder that contains the PSZ
	 * file (as well as all sub-folders) will be examined for images.
	 * 
	 * @param pPSProjectFilename A path string the indicates an existing PSZ file.
	 * will be used with java.io.File to process the PhotoScan project.
	 * @throws IOException see readPSProjectFile()
	 * @see readPSProjectFile
	 */
	public PSSessionData(String pPSProjectFolder, QSettings settings) throws IOException {
		// Fill everything with default values
		mPSProjectFolder = new File(pPSProjectFolder);
		mDateTakenStart = mDateTakenFinish = null;
		mStatus = Status.UNKNOWN;

		mImageCount_raw = 0;
		mImageCount_processed = 0;

		mExposure = null;
		mResultsApproved = false;	
		mSpecialNotes = mName = "";
		
		mActiveProject = -1;
		mPSProjectFileList = null;
		mPSProjectList = null;
				
		// Build for the first time
		examineProjects(settings);
	}

	public void examineProjects(QSettings settings) throws IOException, IllegalArgumentException {
		
		// Examine the directory for images and project files
		examineDirectory(mPSProjectFolder);
		extractInfoFromFolderName(mPSProjectFolder.getName());
		
		if(mPSProjectFileList.length == 0) return;
		
		// initialize the project list
		mPSProjectList = new PSProjectFileData[mPSProjectFileList.length];

		for(int i=0; i<mPSProjectFileList.length; i++) {
			mPSProjectList[i] = new PSProjectFileData(mPSProjectFileList[i], settings);
		}
		
		mActiveProject = mPSProjectFileList.length-1;
		autoSetStatus();
	}
	
	private void extractInfoFromFolderName(String pFolderName) {
		// The original format: "[ID_as_integer] rest of folder name"
		String[] parts = pFolderName.split("\\s");
		if(parts != null && parts.length > 1) {
			try {
				mID = parts[0];
				if(Integer.parseInt(mID) >= mNextID) { mNextID = Integer.parseInt(mID)+1; }
			} catch(Exception e) { mID = "-1"; }
			
			// Build description
			StringJoiner joiner = new StringJoiner(" ");
			for(int i=1; i<parts.length; i++) {
				joiner.add(parts[i]);
			}			
			mName = joiner.toString();
		} else {
			// MIA format: "[Date as YYMMDD]_[accession number/ID w/ characters]_[name/description]"
			parts = pFolderName.split("_");
			mID = ""; int endOfID = 1;
			if(parts != null && parts.length > 1) {
				try {
					while(endOfID<parts.length) {
						if(endOfID > 1 && !Character.isDigit(parts[endOfID].charAt(0))) {
							break;
						} else {
							if(endOfID == 1) { mID = parts[endOfID]; }
							else { mID += "_" + parts[endOfID]; }
						}
						endOfID++;
					}
				} catch(Exception e) { mID = "-1"; }
				
				// Try to parse out the date
				SimpleDateFormat dateFormatter = new SimpleDateFormat("yyMMdd");
				try {
					mDateTakenStart = dateFormatter.parse(parts[0]);
					mDateTakenFinish = mDateTakenStart;
				} catch (ParseException e) {
					mDateTakenStart = mDateTakenFinish = null;
				}
				
				// Build description
				StringJoiner joiner = new StringJoiner(" ");
				for(int i=endOfID; i<parts.length; i++) {
					joiner.add(parts[i]);
				}
				
				mName = joiner.toString();
			}
		}
	}
	
	public static void setSortBy(Columns pNewSortBy) {
		mSortBy = pNewSortBy;
	}
	
	@Override
	public int compareTo(PSSessionData o) {
		switch(mSortBy) {
			default:
			case PROJECT_FOLDER:
			{
				if(mPSProjectFolder == null) return -1;
				if(o.mPSProjectFolder == null) return 1;
				return mPSProjectFolder.getName().compareTo(o.mPSProjectFolder.getName());
			}
			
			case PROJECT_ID: return mID.compareTo(o.mID);
			case PROJECT_NAME: return getName().compareTo(o.getName());
			case PHOTO_DATE: 
			{
				if(mDateTakenStart == null) return -1;
				if(o.mDateTakenStart == null) return 1;
				return mDateTakenStart.compareTo(o.mDateTakenStart);
			}			
			case IMAGE_COUNT_REAL: return Long.compare(mImageCount_processed, o.mImageCount_processed);

			case PROJECT_STATUS: return Integer.compare(mStatus.ordinal(), o.mStatus.ordinal());
			case IMAGE_ALIGN_LEVEL: return describeImageAlignPhase().compareTo(o.describeImageAlignPhase());
			case DENSE_CLOUD_LEVEL: return Byte.compare(getDenseCloudPhaseStatus(), o.getDenseCloudPhaseStatus());
			case MODEL_GEN_LEVEL: return Long.compare(getModelFaceCount(), o.getModelFaceCount());
			case TEXTURE_GEN_LEVEL: return describeTextureGenPhase().compareTo(o.describeTextureGenPhase());
		}
	}

	/**
	 * Examine the files in the given directory and count any normal images and raw
	 * images that are found there OR in any of the sub-directories.  This is intended
	 * to be used in a folder that contains a PSZ file as a 'side-car' file for all
	 * the source images used in that same PSZ file.  Afterwards, mImageCount_* will
	 * be set to the numbers of images found.  If the function fails (return false)
	 * the mImageCount properties are set to 0.
	 * @return True if the directory was successfully examined, false on failure.
	 * @see countfilesIn
	 */
	private boolean examineDirectory(File pDirToExamine) {
		mRawFileList = mPSProjectFileList = null;
		if(pDirToExamine == null || !pDirToExamine.isDirectory()) {
			mImageCount_raw = mImageCount_processed = 0;
			return false;
		}

		mPSProjectFileList = listFilesIn(pDirToExamine, mPSProjectFilter);
		mImageCount_processed = countFilesIn(pDirToExamine, mPSImageFileFilter);
		mImageCount_raw = countFilesIn(pDirToExamine, mRawFileFilter);

		if(mImageCount_raw > 0) {
			// Get all the raw files
			mRawFileList = listFilesIn(pDirToExamine, mRawFileFilter);
			if(mRawFileList != null && mRawFileList.length > 0) {
				Arrays.sort(mRawFileList);
	
				// Just assume first and last files are the earliest and latest files
				try {
					mDateTakenStart = ImageProcessorIM4J.getDateFromMetadata(mRawFileList[0]);
					mDateTakenFinish = ImageProcessorIM4J.getDateFromMetadata(mRawFileList[mRawFileList.length-1]);
				} catch (Exception e) {
					System.err.println("Error: " + e.getMessage());
					mDateTakenStart = new Date(0);
					mDateTakenFinish = new Date(0);
				}
			}
		} else {
			mDateTakenStart = new Date(0);
			mDateTakenFinish = new Date(0);
		}
		
		return true;
	}

	// Recursively count all files that pass the given filter
	/**
	 * Internal helper function to recursively move through a directory tree that starts
	 * at the given directory (pDir) and counts all files that pass the given filter.
	 * @param pDir A File object that points to a directory.
	 * @param pFilter A valid FileFilter to limit the files that are counted (pass null to count all files)
	 * @return The number of files in the directory pDir or any of its sub-directories that pass pFilter.
	 */
	public long countFilesIn(File pDir, FileFilter pFilter) {
		if(!pDir.isDirectory()) return 0;

		// Count files that pass the filter
		long count = pDir.listFiles(pFilter).length;

		// Recurse into any sub-directories and add their count
		for (final File lSubDir : pDir.listFiles(mDirectoryFilter)) {
        	count += countFilesIn(lSubDir, pFilter);
	    }

		// Return total count
		return count;
	}

	/**
	 * Internal helper function to recursively move through a directory tree that starts at the
	 * given directory (pDir) and retrieve File objects of all files that pass the given filter.
	 * @param pDir A File object that points to a directory.
	 * @param pFilter A valid FileFilter to limit the files that are counted (pass null to count all files)
	 * @return An array of all the Files in the directory pDir or any of its sub-directories that pass pFilter.
	 */
	public File[] listFilesIn(File pDir, FileFilter pFilter) {
		if(!pDir.isDirectory()) return null;

		// Count files that pass the filter
		File[] lFiles = pDir.listFiles(pFilter);

		// Recurse into any sub-directories and add their count
		ArrayList<File[]> lMoreFiles = new ArrayList<File[]>();
		lMoreFiles.add(lFiles);
		for (final File lSubDir : pDir.listFiles(mDirectoryFilter)) {
			lMoreFiles.add(listFilesIn(lSubDir, pFilter));
	    }

		// Return total count
		return mergeArrays(lMoreFiles);
	}

	private File[] mergeArrays(ArrayList<File[]> pArrays) {
		int count = 0;
		for(final File[] lArray : pArrays) {
			count += lArray.length;
		}
		
		File[] lMerged = new File[count];
		int dest=0;
		for(final File[] lArray : pArrays) {
			for(int i=0; i<lArray.length; i++, dest++) {
				lMerged[dest] = lArray[i];
			}
		}
		
		return lMerged;
	}
	
	private void autoSetStatus() {
		// Pick one of the auto-status phases
		if(getProcessedImageCount() == 0) {
			mStatus = Status.UNPROCESSSED;
		} else if(describeImageAlignPhase().equals("N/A")) {
			mStatus = Status.RAW_PROCESSING_DONE;			
		} else if(describeDenseCloudPhase().equals("N/A")) {
			mStatus = Status.ALIGNMENT_DONE;
		} else if(describeModelGenPhase().equals("N/A")) {
			mStatus = Status.POINT_CLOUD_DONE;
		} else if(describeTextureGenPhase().equals("N/A")) {
			mStatus = Status.MODEL_GEN_DONE;			
		} else {
			mStatus = Status.TEXTURE_GEN_DONE;
		}
	}
	
	public void setCustomStatus(int statusIndex) {
		int ordinal = Status.TEXTURE_GEN_DONE.ordinal() + statusIndex;
		if(ordinal > Status.TEXTURE_GEN_DONE.ordinal() && ordinal < Status.values().length) {
			mStatus = Status.values()[ordinal];
		} else {
			autoSetStatus();
		}
	}
	
	public String getID() {
		return mID;
	}
	
	public static int getNextID() {
		return mNextID;
	}

	public void setID(String mID) {
		this.mID = mID;
		try {
			if(Integer.parseInt(mID) > mNextID) { mNextID = Integer.parseInt(mID)+1; }
		} catch(Exception e) {}
	}

	public void setSpecialNotes(String pSpecialNotes) {
		this.mSpecialNotes = pSpecialNotes;
	}

	public void setName(String pName) {
		this.mName = pName;
	}

	public File getPSProjectFile(int which) {
		if(which >= 0 && which < mPSProjectFileList.length) {
			return mPSProjectFileList[which];
		}
		
		return null;
	}

	public File getPSProjectFile() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectFileList.length) {
			return mPSProjectFileList[mActiveProject];
		}
		
		return null;
	}
	
	public File getPSProjectFolder() {
		return mPSProjectFolder;
	}

	public PSModelData getModelData() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			if(mPSProjectList[mActiveProject].getModelArchiveFile() != null) {
				return mPSProjectList[mActiveProject].getModelData();
			}
		}
		
		return null;		
	}
	
	public File getModelArchiveFile() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			if(mPSProjectList[mActiveProject].getModelArchiveFile() != null) {
				return mPSProjectList[mActiveProject].getModelArchiveFile();
			}
		}
		
		// Fall back to the current project file if any of the above fails
		return getPSProjectFile();
	}
	
	public long getRawImageCount() {
		return mImageCount_raw;
	}
	
	public long getProcessedImageCount() {
		return mImageCount_processed;
	}
	
	public File[] getRawFileList() {
		return mRawFileList;
	}
	
	public boolean isImageExposureKnown() {
		return (mExposure != null);
	}
	
	public double[] getWhiteBalanceMultipliers() {
		if(isImageExposureKnown()) { return mExposure.getWBCustom(); }
		else { return null; }
	}
	
	public double getBrightnessMultiplier() {
		if(isImageExposureKnown()) { return mExposure.getBrightScale(); }
		else { return 1.0; }
	}
	
	public Date getDateTakenStart() {
		return mDateTakenStart;
	}
	
	public Date getDateTakenFinish() {
		return mDateTakenFinish;
	}
	
	public boolean areResultsApproved() {
		return mResultsApproved;
	}
	
	public String getSpecialNotes() {
		return mSpecialNotes;
	}
	
	public String getName() {
		if(mName == null || mName.isEmpty()) {
			return mPSProjectFolder.getName();
		}
		return mName;
	}
	
	public String getNameStrict() {
		return mName;
	}

	public Status getStatus() {
		return mStatus;
	}

	public void writeGeneralSettings(QSettings settings) {
		if(settings == null) return;
		
		// We count on the folder name being unique inside this collection
	    settings.beginGroup(mPSProjectFolder.getName().replace(' ',  '_') + "_Settings");

	    // Write these only if they are not empty
	    if(!mID.isEmpty()) { settings.setValue("General/ID", mID); }
	    if(!mName.isEmpty()) { settings.setValue("General/Name", mName); }
	    if(!mSpecialNotes.isEmpty()) { settings.setValue("General/SpecialNotes", mSpecialNotes); }

	    // Only write custom status states
	    if(mStatus.ordinal() > Status.TEXTURE_GEN_DONE.ordinal()) {
	    	settings.setValue("General/Status", mStatus.ordinal());
	    }
	    
	    settings.endGroup();
	}

	public void readGeneralSettings(QSettings settings) {
		if(settings == null) return;
		
	    settings.beginGroup(mPSProjectFolder.getName().replace(' ',  '_') + "_Settings");

	    // Retrieve the general settings
	    String settingsID = settings.value("General/ID", mID).toString();
	    String settingsName = settings.value("General/Name", "").toString();
	    if(settingsName.isEmpty()) { settingsName = settings.value("General/Description", "").toString(); }
	    String settingsSpecialNotes = settings.value("General/SpecialNotes", "").toString();

	    // Only read these settings if they are not empty
	    if(!settingsID.isEmpty()) { mID = settingsID; }	    
	    if(!settingsName.isEmpty()) { mName = settingsName; }
	    if(!settingsSpecialNotes.isEmpty()) { mSpecialNotes = settingsSpecialNotes; }
	    
	    // Only read and accept custom status states
	    int statVal = Integer.parseInt(settings.value("General/Status", "0").toString());
	    if(statVal > Status.TEXTURE_GEN_DONE.ordinal()) {
	    	mStatus = Status.values()[statVal];
	    }
	    
	    settings.endGroup();
	}
	
	public void writeExposureSettings(ExposureSettings pExpSettings, QSettings settings) {
		// Try to get some non-null exposure settings
		ExposureSettings lExpSettings = pExpSettings;
		if(lExpSettings == null) { lExpSettings = mExposure; }
		if(lExpSettings == null) return;

		// Write those to the settings file for the application
		if(settings == null) return;
	    settings.beginGroup(mPSProjectFolder.getName().replace(' ',  '_') + "_Settings");

	    settings.setValue("WhiteBalanceMode", pExpSettings.getWBMode().ordinal());
	    settings.setValue("WhiteBalanceMode/R", pExpSettings.getWBCustom()[0]);
	    settings.setValue("WhiteBalanceMode/G1", pExpSettings.getWBCustom()[1]);
	    settings.setValue("WhiteBalanceMode/B", pExpSettings.getWBCustom()[2]);
	    settings.setValue("WhiteBalanceMode/G2", pExpSettings.getWBCustom()[3]);
	    
	    settings.setValue("BrightnessMode", pExpSettings.getBrightMode().ordinal());
	    settings.setValue("BrightnessMode/Scaler", pExpSettings.getBrightScale());

	    // Close the group and dispose of this object (cause you know, garbage collection)
	    settings.endGroup();
	}

	public ExposureSettings readExposureSettings(QSettings settings) {
		if(settings == null) return ExposureSettings.DEFAULT_EXPOSURE;
		
	    ExposureSettings lDefSettings = ExposureSettings.DEFAULT_EXPOSURE;

	    settings.beginGroup(mPSProjectFolder.getName().replace(' ',  '_') + "_Settings");

	    int lWBOrdinal = Integer.parseInt(settings.value("WhiteBalanceMode", lDefSettings.getWBMode().ordinal()).toString());
	    double[] lWBCustom = {
	    		Double.parseDouble(settings.value("WhiteBalanceMode/R", lDefSettings.getWBCustom()[0]).toString()),
	    		Double.parseDouble(settings.value("WhiteBalanceMode/G1", lDefSettings.getWBCustom()[1]).toString()),
	    		Double.parseDouble(settings.value("WhiteBalanceMode/B", lDefSettings.getWBCustom()[2]).toString()),
	    		Double.parseDouble(settings.value("WhiteBalanceMode/G2", lDefSettings.getWBCustom()[3]).toString())
	    };
	    
	    int lBrightMOrdinal = Integer.parseInt(settings.value("BrightnessMode", lDefSettings.getBrightMode().ordinal()).toString());
	    double lBrightScaler = Double.parseDouble(settings.value("BrightnessMode/Scaler", lDefSettings.getBrightScale()).toString());	    

	    // Close the group and dispose of this object (cause you know, garbage collection)
	    settings.endGroup();

	    return (new ExposureSettings(WhiteBalanceMode.values()[lWBOrdinal], lWBCustom,
	    				BrightnessMode.values()[lBrightMOrdinal], lBrightScaler));
	}

	/**
	 * Overloading of the standard string conversion function.  Describes all parts
	 * of this object in a string for printing to the screen.  NOT for serialization.
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString()
	{
		// General information
		String lDetails = "Session: " + mPSProjectFolder.getName();
		if(mResultsApproved) lDetails += " (APPROVED)";
		lDetails += "\n";

		lDetails += "\tName: ";
		if(mName != null && mName != "") { lDetails += mName; }
		else { lDetails += "[none]"; }
		lDetails += "\n";

		lDetails += "\tNotes: ";
		if(mSpecialNotes != null && mSpecialNotes != "") { lDetails += mSpecialNotes; }
		else { lDetails += "[none]"; }
		lDetails += "\n";
		
		// Image data details
		lDetails += "\n\tImages - " + mImageCount_processed + " in folder\n";
		if(mImageCount_raw > 0) { lDetails += "\t         " + mImageCount_raw + " raw images\n"; }
		
		if(isImageExposureKnown()) {
			double[] lMult = mExposure.getWBCustom();
			lDetails += "\tTaken from " + mDateTakenStart + " to " + mDateTakenFinish + "\n";
			lDetails += "\tConverted as - " + lMult[0] + ", " + lMult[1] + ", "
											+ lMult[2] + ", " + lMult[3] + "\n";
			
			double lScaler = mExposure.getBrightScale();
			if(Math.abs(lScaler - 1.0) > 1e-8) {
				lDetails += "\t               " + lScaler + " adjustment\n";
			}
		}

		// Add project info
		lDetails += "\nPhotoScan Project: ";
		if(mActiveProject == -1) {
			lDetails += "none\n";
		} else {
			lDetails += mPSProjectList[mActiveProject].toString();
		}
		
		return lDetails;
	}

	public int getProjectCount() {
		return mPSProjectList.length;
	}

	public int getActiveProjectIndex() {
		return mActiveProject;
	}

	public PSProjectFileData getActiveProject() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject];
		}

		return null;
	}

	public int getChunkCount() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getChunkCount();
		}

		return 0;		
	}

	public int getActiveChunkIndex() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getActiveChunkIndex();
		}

		return -1;		
	}
	
	public PSChunkData getChunk(int index) {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getChunk(index);
		}

		return null;
	}

	public PSChunkData getActiveChunk() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getActiveChunk();
		}

		return null;
	}

	@Override
	public String describeImageAlignPhase() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].describeImageAlignPhase();
		}

		return "N/A";
	}

	@Override
	public byte getAlignPhaseStatus() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getAlignPhaseStatus();			
		}
		
		return 0;
	}

	@Override
	public String describeDenseCloudPhase() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].describeDenseCloudPhase();			
		}
		
		return "N/A";
	}

	@Override
	public int getDenseCloudDepthImages() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getDenseCloudDepthImages();			
		}
		
		return 0;
	}
	
	@Override
	public byte getDenseCloudPhaseStatus() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getDenseCloudPhaseStatus();			
		}
		
		return 0;		
	}

	@Override
	public String describeModelGenPhase() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].describeModelGenPhase();			
		}
		
		return "N/A";
	}

	@Override
	public byte getModelGenPhaseStatus() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getModelGenPhaseStatus();			
		}
		
		return 0;
	}

	@Override
	public long getModelFaceCount() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getModelFaceCount();			
		}
		
		return 0;
	}

	@Override
	public long getModelVertexCount() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getModelVertexCount();			
		}
		
		return 0;
	}

	@Override
	public String describeTextureGenPhase() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].describeTextureGenPhase();			
		}
		
		return "N/A";
	}

	@Override
	public byte getTextureGenPhaseStatus() {
		if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
			return mPSProjectList[mActiveProject].getTextureGenPhaseStatus();			
		}
		
		return 0;
	}
}
