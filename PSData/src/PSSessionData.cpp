#include "PSSessionData.h"

#include <QFile>
#include <QSettings>

#include "PSProjectFileData.h"
#include "PSChunkData.h"
#include "PSModelData.h"

DEFINE_ENUM(Field, FIELDS_ENUM, PSSessionData)

// The number of fields shown for base and extended modes
const char PSSessionData::BASE_LENGTH = 6;
const char PSSessionData::EXTENDED_LENGTH = 11;

PSSessionData::Field mSortBy = PSSessionData::F_PROJECT_ID;
void PSSessionData::setSortBy(PSSessionData::Field pNewSortBy) {
    mSortBy = pNewSortBy;
}

int PSSessionData::mNextID = 0;

//// QFile filter setup
//static const ExtensionFileFilter mDirectoryFilter = new ExtensionFileFilter(true);

//static const QString* mPSProjectFileExtensions = { ".psz", ".psx" };
//static const ExtensionFileFilter mPSProjectFilter =
//        new ExtensionFileFilter(mPSProjectFileExtensions);

//// Most of the image types supported by PhotoScan (we leave out ones we can't support)
//static const QString* mPSImageFileExtensions = {
//    ".jpg", ".jpeg", ".tif", ".tiff", ".pgm", ".ppm",
//    ".png", ".bmp", ".exr"
//};

//static const ExtensionFileFilter mPSImageFileFilter =
//        new ExtensionFileFilter(mPSImageFileExtensions);

//// Extension list from https://en.wikipedia.org/wiki/Raw_image_format
//static const QString* mRawFileExtensions = {
//        ".3fr",
//        ".ari",".arw",
//        ".bay",
//        ".crw",".cr2",
//        ".cap",
//        ".dcs",".dcr",".dng",
//        ".drf",
//        ".eip",".erf",
//        ".fff",
//        ".iiq",
//        ".k25",".kdc",
//        ".mdc",".mef",".mos",".mrw",
//        ".nef",".nrw",
//        ".obm",".orf",
//        ".pef",".ptx",".pxn",
//        ".r3d",".raf",".raw",".rwl",".rw2",".rwz",
//        ".sr2",".srf",".srw",
//        ".x3f"
//    };

//static const ExtensionFileFilter mRawFileFilter =
//        new ExtensionFileFilter(mRawFileExtensions);

/**
 * Construct a new PSProjectData that points to the PSZ project file
 * indicated by the path string.  The folder that contains the PSZ
 * file (as well as all sub-folders) will be examined for images.
 *
 * @param pPSProjectFilename A path string the indicates an existing PSZ file.
 * will be used with java.io.QFile to process the PhotoScan project.
 * @throws IOException see readPSProjectFile()
 * @see readPSProjectFile
 */
PSSessionData::PSSessionData(QString pPSProjectFolder, QSettings* settings)
    : mExposure(ExposureSettings::DEFAULT_EXPOSURE) { // throws IOException {
    // Fill everything with default values
    mPSProjectFolder = new QFile(pPSProjectFolder);
    mStatus = PSS_UNKNOWN;

    mImageCount_raw = 0;
    mImageCount_processed = 0;

    mResultsApproved = false;
    mSpecialNotes = mName = "";

    mActiveProject = -1;
    mPSProjectFileList = NULL;
    mPSProjectList = NULL;

    // Build for the first time
    examineProjects(settings);
}

void PSSessionData::examineProjects(QSettings* settings) { // throws IOException, IllegalArgumentException {

    // Examine the directory for images and project files
    examineDirectory(mPSProjectFolder);
    extractInfoFromFolderName(mPSProjectFolder.getName());

    if(mPSProjectFileList.length == 0) return;

    // initialize the project list
    mPSProjectList = new PSProjectFileData*[mPSProjectFileList.length];

    for(int i=0; i<mPSProjectFileList.length; i++) {
        mPSProjectList[i] = new PSProjectFileData(mPSProjectFileList[i], settings);
    }

    mActiveProject = mPSProjectFileList.length-1;
    autoSetStatus();
}

void PSSessionData::extractInfoFromFolderName(QString pFolderName) {
    // The original format: "[ID_as_integer] rest of folder name"
    QString* parts = pFolderName.split("\\s");
    if(parts != NULL && parts.length > 1) {
        try {
            mID = parts[0];
            if(Integer.parseInt(mID) >= mNextID) { mNextID = Integer.parseInt(mID)+1; }
        } catch(...) { mID = "-1"; }

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
        if(parts != NULL && parts.length > 1) {
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
            } catch (...) {
                mDateTakenStart = mDateTakenFinish = QDate();
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

int PSSessionData::compareTo(const PSSessionData* o) {
    switch(mSortBy) {
        default:
        case F_PROJECT_FOLDER:
        {
            if(mPSProjectFolder == NULL) return -1;
            if(o.mPSProjectFolder == NULL) return 1;
            return mPSProjectFolder.getName().compareTo(o.mPSProjectFolder.getName());
        }

        case F_PROJECT_ID: return mID.compareTo(o.mID);
        case F_PROJECT_NAME: return getName().compareTo(o.getName());
        case F_PHOTO_DATE:
        {
            if(mDateTakenStart == NULL) return -1;
            if(o.mDateTakenStart == NULL) return 1;
            return mDateTakenStart.compareTo(o.mDateTakenStart);
        }
        case F_IMAGE_COUNT_REAL: return Long.compare(mImageCount_processed, o.mImageCount_processed);

        case F_PROJECT_STATUS: return Integer.compare(mStatus.ordinal(), o.mStatus.ordinal());
        case F_IMAGE_ALIGN_LEVEL: return describeImageAlignPhase().compareTo(o.describeImageAlignPhase());
        case F_DENSE_CLOUD_LEVEL: return Byte.compare(getDenseCloudPhaseStatus(), o.getDenseCloudPhaseStatus());
        case F_MODEL_GEN_LEVEL: return Long.compare(getModelFaceCount(), o.getModelFaceCount());
        case F_TEXTURE_GEN_LEVEL: return describeTextureGenPhase().compareTo(o.describeTextureGenPhase());
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
bool PSSessionData::examineDirectory(QFile* pDirToExamine) {
    mRawFileList = mPSProjectFileList = NULL;
    if(pDirToExamine == NULL || !pDirToExamine.isDirectory()) {
        mImageCount_raw = mImageCount_processed = 0;
        return false;
    }

    mPSProjectFileList = listFilesIn(pDirToExamine, mPSProjectFilter);
    mImageCount_processed = countFilesIn(pDirToExamine, mPSImageFileFilter);
    mImageCount_raw = countFilesIn(pDirToExamine, mRawFileFilter);

    if(mImageCount_raw > 0) {
        // Get all the raw files
        mRawFileList = listFilesIn(pDirToExamine, mRawFileFilter);
        if(mRawFileList != NULL && mRawFileList.length > 0) {
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
 * @param pDir A QFile object that points to a directory.
 * @param pFilter A valid FileFilter to limit the files that are counted (pass NULL to count all files)
 * @return The number of files in the directory pDir or any of its sub-directories that pass pFilter.
 */
long PSSessionData::countFilesIn(QFile* pDir, FileFilter pFilter) {
    if(!pDir.isDirectory()) return 0;

    // Count files that pass the filter
    long count = pDir.listFiles(pFilter).length;

    // Recurse into any sub-directories and add their count
    for (const QFile* lSubDir : pDir.listFiles(mDirectoryFilter)) {
        count += countFilesIn(lSubDir, pFilter);
    }

    // Return total count
    return count;
}

/**
 * Internal helper function to recursively move through a directory tree that starts at the
 * given directory (pDir) and retrieve QFile objects of all files that pass the given filter.
 * @param pDir A QFile object that points to a directory.
 * @param pFilter A valid FileFilter to limit the files that are counted (pass NULL to count all files)
 * @return An array of all the Files in the directory pDir or any of its sub-directories that pass pFilter.
 */
QFile* PSSessionData::listFilesIn(QFile* pDir, FileFilter pFilter) {
    if(!pDir.isDirectory()) return NULL;

    // Count files that pass the filter
    QFile* lFiles = pDir.listFiles(pFilter);

    // Recurse into any sub-directories and add their count
    ArrayList<QFile*> lMoreFiles = new ArrayList<QFile*>();
    lMoreFiles.add(lFiles);
    for (const QFile* lSubDir : pDir.listFiles(mDirectoryFilter)) {
        lMoreFiles.add(listFilesIn(lSubDir, pFilter));
    }

    // Return total count
    return mergeArrays(lMoreFiles);
}

QFile* PSSessionData::mergeArrays(ArrayList<QFile*> pArrays) {
    int count = 0;
    for(const QFile* lArray : pArrays) {
        count += lArray.length;
    }

    QFile* lMerged = new QFile[count];
    int dest=0;
    for(const QFile* lArray : pArrays) {
        for(int i=0; i<lArray.length; i++, dest++) {
            lMerged[dest] = lArray[i];
        }
    }

    return lMerged;
}

void PSSessionData::autoSetStatus() {
    // Pick one of the auto-status phases
    if(getProcessedImageCount() == 0) {
        mStatus = PSS_UNPROCESSSED;
    } else if(describeImageAlignPhase().equals("N/A")) {
        mStatus = PSS_RAW_PROCESSING_DONE;
    } else if(describeDenseCloudPhase().equals("N/A")) {
        mStatus = PSS_ALIGNMENT_DONE;
    } else if(describeModelGenPhase().equals("N/A")) {
        mStatus = PSS_POINT_CLOUD_DONE;
    } else if(describeTextureGenPhase().equals("N/A")) {
        mStatus = PSS_MODEL_GEN_DONE;
    } else {
        mStatus = PSS_TEXTURE_GEN_DONE;
    }
}

void PSSessionData::setCustomStatus(int statusIndex) {
    int ordinal = Status.TEXTURE_GEN_DONE.ordinal() + statusIndex;
    if(ordinal > Status.TEXTURE_GEN_DONE.ordinal() && ordinal < Status.values().length) {
        mStatus = Status.values()[ordinal];
    } else {
        autoSetStatus();
    }
}

QString PSSessionData::getID() {
    return mID;
}

static int PSSessionData::getNextID() {
    return mNextID;
}

void PSSessionData::setID(QString mID) {
    this.mID = mID;
    try {
        if(Integer.parseInt(mID) > mNextID) { mNextID = Integer.parseInt(mID)+1; }
    } catch(Exception e) {}
}

void PSSessionData::setSpecialNotes(QString pSpecialNotes) {
    this.mSpecialNotes = pSpecialNotes;
}

void PSSessionData::setName(QString pName) {
    this.mName = pName;
}

QFile* PSSessionData::getPSProjectFile(int which) {
    if(which >= 0 && which < mPSProjectFileList.length) {
        return mPSProjectFileList[which];
    }

    return NULL;
}

QFile* PSSessionData::getPSProjectFile() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectFileList.length) {
        return mPSProjectFileList[mActiveProject];
    }

    return NULL;
}

QFile* PSSessionData::getPSProjectFolder() {
    return mPSProjectFolder;
}

PSModelData* PSSessionData::getModelData() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        if(mPSProjectList[mActiveProject].getModelArchiveFile() != NULL) {
            return mPSProjectList[mActiveProject].getModelData();
        }
    }

    return NULL;
}

QFile* PSSessionData::getModelArchiveFile() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        if(mPSProjectList[mActiveProject].getModelArchiveFile() != NULL) {
            return mPSProjectList[mActiveProject].getModelArchiveFile();
        }
    }

    // Fall back to the current project file if any of the above fails
    return getPSProjectFile();
}

long PSSessionData::getRawImageCount() {
    return mImageCount_raw;
}

long PSSessionData::getProcessedImageCount() {
    return mImageCount_processed;
}

QFile* PSSessionData::getRawFileList() {
    return mRawFileList;
}

bool PSSessionData::isImageExposureKnown() {
    return (mExposure != NULL);
}

double* PSSessionData::getWhiteBalanceMultipliers() {
    if(isImageExposureKnown()) { return mExposure.getWBCustom(); }
    else { return NULL; }
}

double PSSessionData::getBrightnessMultiplier() {
    if(isImageExposureKnown()) { return mExposure.getBrightScale(); }
    else { return 1.0; }
}

Date PSSessionData::getDateTakenStart() {
    return mDateTakenStart;
}

Date PSSessionData::getDateTakenFinish() {
    return mDateTakenFinish;
}

bool PSSessionData::areResultsApproved() {
    return mResultsApproved;
}

QString PSSessionData::getSpecialNotes() {
    return mSpecialNotes;
}

QString PSSessionData::getName() {
    if(mName == NULL || mName.isEmpty()) {
        return mPSProjectFolder.getName();
    }
    return mName;
}

QString PSSessionData::getNameStrict() {
    return mName;
}

PSSessionData::Status PSSessionData::getStatus() {
    return mStatus;
}

void PSSessionData::writeGeneralSettings(QSettings* settings) {
    if(settings == NULL) return;

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

void PSSessionData::readGeneralSettings(QSettings* settings) {
    if(settings == NULL) return;

    settings.beginGroup(mPSProjectFolder.getName().replace(' ',  '_') + "_Settings");

    // Retrieve the general settings
    QString settingsID = settings.value("General/ID", mID).toString();
    QString settingsName = settings.value("General/Name", "").toString();
    if(settingsName.isEmpty()) { settingsName = settings.value("General/Description", "").toString(); }
    QString settingsSpecialNotes = settings.value("General/SpecialNotes", "").toString();

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

void PSSessionData::writeExposureSettings(ExposureSettings* pExpSettings, QSettings* settings) {
    // Try to get some non-NULL exposure settings
    ExposureSettings* lExpSettings = pExpSettings;
    if(lExpSettings == NULL) { lExpSettings = mExposure; }
    if(lExpSettings == NULL) return;

    // Write those to the settings file for the application
    if(settings == NULL) return;
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

ExposureSettings* PSSessionData::readExposureSettings(QSettings* settings) {
    if(settings == NULL) return ExposureSettings.DEFAULT_EXPOSURE;

    ExposureSettings lDefSettings = ExposureSettings.DEFAULT_EXPOSURE;

    settings.beginGroup(mPSProjectFolder.getName().replace(' ',  '_') + "_Settings");

    int lWBOrdinal = Integer.parseInt(settings.value("WhiteBalanceMode", lDefSettings.getWBMode().ordinal()).toString());
    double* lWBCustom = {
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
QString PSSessionData::toString()
{
    // General information
    QString lDetails = "Session: " + mPSProjectFolder.getName();
    if(mResultsApproved) lDetails += " (APPROVED)";
    lDetails += "\n";

    lDetails += "\tName: ";
    if(mName != NULL && mName != "") { lDetails += mName; }
    else { lDetails += "[none]"; }
    lDetails += "\n";

    lDetails += "\tNotes: ";
    if(mSpecialNotes != NULL && mSpecialNotes != "") { lDetails += mSpecialNotes; }
    else { lDetails += "[none]"; }
    lDetails += "\n";

    // Image data details
    lDetails += "\n\tImages - " + mImageCount_processed + " in folder\n";
    if(mImageCount_raw > 0) { lDetails += "\t         " + mImageCount_raw + " raw images\n"; }

    if(isImageExposureKnown()) {
        double* lMult = mExposure.getWBCustom();
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

int PSSessionData::getProjectCount() {
    return mPSProjectList.length;
}

int PSSessionData::getActiveProjectIndex() {
    return mActiveProject;
}

PSProjectFileData* PSSessionData::getActiveProject() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject];
    }

    return NULL;
}

int PSSessionData::getChunkCount() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getChunkCount();
    }

    return 0;
}

int PSSessionData::getActiveChunkIndex() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getActiveChunkIndex();
    }

    return -1;
}

PSChunkData* PSSessionData::getChunk(int index) {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getChunk(index);
    }

    return NULL;
}

PSChunkData* PSSessionData::getActiveChunk() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getActiveChunk();
    }

    return NULL;
}

QString PSSessionData::describeImageAlignPhase() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].describeImageAlignPhase();
    }

    return "N/A";
}

char PSSessionData::getAlignPhaseStatus() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getAlignPhaseStatus();
    }

    return 0;
}

QString PSSessionData::describeDenseCloudPhase() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].describeDenseCloudPhase();
    }

    return "N/A";
}

int PSSessionData::getDenseCloudDepthImages() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getDenseCloudDepthImages();
    }

    return 0;
}

char PSSessionData::getDenseCloudPhaseStatus() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getDenseCloudPhaseStatus();
    }

    return 0;
}

QString PSSessionData::describeModelGenPhase() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].describeModelGenPhase();
    }

    return "N/A";
}

char PSSessionData::getModelGenPhaseStatus() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getModelGenPhaseStatus();
    }

    return 0;
}

long PSSessionData::getModelFaceCount() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getModelFaceCount();
    }

    return 0;
}

long PSSessionData::getModelVertexCount() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getModelVertexCount();
    }

    return 0;
}

QString PSSessionData::describeTextureGenPhase() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].describeTextureGenPhase();
    }

    return "N/A";
}

char PSSessionData::getTextureGenPhaseStatus() {
    if(mActiveProject >= 0 && mActiveProject < mPSProjectList.length) {
        return mPSProjectList[mActiveProject].getTextureGenPhaseStatus();
    }

    return 0;
}
