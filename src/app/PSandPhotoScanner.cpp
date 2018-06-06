package edu.uwstout.berriers.PSHelper.app;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;

import com.trolltech.qt.core.QFutureVoid;
import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.core.QtConcurrent;

import edu.uwstout.berriers.PSHelper.Model.PSSessionData;

public class PSandPhotoScanner implements QtConcurrent.MapFunctor<File> {

	private File mRootPath;
	private int mMaxRecursionDepth;
	private QSettings mPSProjectInfoStore;
	
	private boolean mDataScanned;
	private ArrayList<PSSessionData> mData;
	
	private ArrayList<File> mProjectFileList;
	
	private static final String[] IGNORE_EXCEPTIONS = { "_Finished", "_TouchUp", "_TouchedUpPleaseReview" };
		
	private static final ExtensionFileFilter mDirectoryFilter =
			new ExtensionFileFilter(true);
	
	public PSandPhotoScanner(String pPath, int pMaxRecursionDepth) throws IOException {
		mRootPath = new File(pPath);
		if(!mRootPath.exists()) { throw new IOException("Path does not exist."); }
		
		mProjectFileList = new ArrayList<File>();
		mRootPath = mRootPath.getCanonicalFile();
		mMaxRecursionDepth = pMaxRecursionDepth;
		mData = new ArrayList<PSSessionData>();
		mDataScanned = false;

		// Open/Create the settings file
		String settingsFile = mRootPath.getPath() + "/PSHelperData.ini";
		mPSProjectInfoStore = initInfoStore(settingsFile);
	}

	private static QSettings initInfoStore(String pSettingsFile)
	{		
		QSettings newSettings = new QSettings(pSettingsFile, QSettings.Format.IniFormat);
		
		// Initialize the ini file if it doesn't exist yet
		File settings = new File(pSettingsFile);
		if(!settings.exists())
		{
			newSettings.beginGroup("Collection");
			newSettings.setValue("Name", "None");
			newSettings.setValue("Owner", "None");
			newSettings.endGroup();
		}
		
		return newSettings;
	}
	
	public QSettings getInfoStore()
	{
		return mPSProjectInfoStore;
	}
	
	public boolean isReady() { return mDataScanned; }
	public ArrayList<PSSessionData> getPSProjectData() { return mData; }
	
	public int countUniqueDirs() {
		HashSet<String> lDirSet = new HashSet<String>();
		for(final PSSessionData lData : mData) {
			lDirSet.add(lData.getPSProjectFolder().getName());
		}
		
		return lDirSet.size();
	}
	
	public int countDirsWithoutProjects() {
		int count = 0;
		for(PSSessionData lProj : mData) {
			File lFile = lProj.getPSProjectFile();
			if(lFile == null) { count++; }
		}

		return count;
	}

	public int countDirsWithoutImageAlign() {
		int count = 0;
		for(final PSSessionData lData : mData) {
			if(lData.describeImageAlignPhase().equals("N/A")) {
				count++;
			}
		}
		
		return count;
	}

	public int countDirsWithoutDenseCloud() {

		int count = 0;
		for(final PSSessionData lData : mData) {
			if(lData.describeDenseCloudPhase().equals("N/A")) {
				count++;
			}
		}
		
		return count;
	}

	public int countDirsWithoutModels() {
		int count = 0;
		for(final PSSessionData lData : mData) {
			if(lData.describeModelGenPhase().equals("N/A")) {
				count++;
			}
		}
		
		return count;
	}
	
	public QFutureVoid startScanParallel() {
		buildProjectListRecursive(mRootPath, 0);
		return QtConcurrent.map(mProjectFileList, this);
	}

	public void finishDataParallel() {
		mData.sort(null);
		for(PSSessionData data : mData) {
			data.readGeneralSettings(mPSProjectInfoStore);
			data.readExposureSettings(mPSProjectInfoStore);
		}
		mDataScanned = true;
	}
	
	private boolean buildProjectListRecursive(File pFile, int depth) {
		if(depth == mMaxRecursionDepth) {
			mProjectFileList.add(pFile);
			return true;
		}
		
		// List and examine all directories
		File[] lDirs = pFile.listFiles(mDirectoryFilter);
		boolean foundOne = false;
		for(final File lFileEntry : lDirs) {
			
			// Ignore folders that start with '_' (other than some exceptions)
			if(lFileEntry.getName().startsWith("_")) {
				if(Arrays.binarySearch(IGNORE_EXCEPTIONS, lFileEntry.getName()) >= 0) {
					// Move down into a '_' exception without increasing depth
					foundOne |= buildProjectListRecursive(lFileEntry, depth);
				} else {
					System.out.printf("Skipping '%s'\n", lFileEntry.getName());
				}
			} else {
				foundOne |= buildProjectListRecursive(lFileEntry, depth+1);
			}
		}
		
		return foundOne;
	}

	@Override
	public void map(File pProjectFolder) {
		try {
			PSSessionData lData = new PSSessionData(pProjectFolder.getPath(), null);
			if(lData == null || (lData.getPSProjectFile() == null &&
			   lData.getRawImageCount() == 0 && lData.getProcessedImageCount() == 0)) {
				throw new IOException("No files in directory.");
			} else {
				mData.add(lData);
			}
		} catch(Exception e) {
			System.out.println("Error: unable to scan '" + pProjectFolder.getName() + "'");
			e.printStackTrace();
		}
	}
}
