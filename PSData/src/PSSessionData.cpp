#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
using namespace std;

#include "PSSessionData.h"

#include <QSettings>
#include <QDebug>
#include <QDirIterator>

#include "PSProjectFileData.h"
#include "PSChunkData.h"
#include "PSModelData.h"
#include "DirLister.h"

DEFINE_ENUM(Field, FIELDS_ENUM, PSSessionData)

// The number of fields shown for base and extended modes
const char PSSessionData::BASE_LENGTH = 6;
const char PSSessionData::EXTENDED_LENGTH = 11;

PSSessionData::Field PSSessionData::mSortBy = PSSessionData::F_PROJECT_ID;
void PSSessionData::setSortBy(PSSessionData::Field pNewSortBy) {
    mSortBy = pNewSortBy;
}

int PSSessionData::mNextID = 0;

// File name filters
// Note: raw file extensions from https://en.wikipedia.org/wiki/Raw_image_format
const QStringList gPSProjectFileExtensions = { "*.psz", "*.psx" };
const QStringList gPSImageFileExtensions = {
    "*.jpg", "*.jpeg",
    "*.tif", "*.tiff",
    "*.pgm", "*.ppm",
    "*.png",
    "*.bmp",
    "*.exr"
};
const QStringList gPSMaskFileExtensions = {
    "*_mask.jpg", "*_mask.jpeg",
    "*_mask.tif", "*_mask.tiff",
    "*_mask.pgm", "*_mask.ppm",
    "*_mask.png",
    "*_mask.bmp",
    "*_mask.exr"
};
const QStringList gRawFileExtensions = {
    "*.3fr",
    "*.ari","*.arw",
    "*.bay",
    "*.crw","*.cr2","*.cr3",
    "*.cap",
    "*.data","*.dcs","*.dcr","*.dng",
    "*.drf",
    "*.eip","*.erf",
    "*.fff",
    "*.gpr",
    "*.iiq",
    "*.k25","*.kdc",
    "*.mdc","*.mef","*.mos","*.mrw",
    "*.nef","*.nrw",
    "*.obm","*.orf",
    "*.pef","*.ptx","*.pxn",
    "*.r3d","*.raf","*.raw","*.rwl","*.rw2","*.rwz",
    "*.sr2","*.srf","*.srw",
    "*.x3f"
};

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
PSSessionData::PSSessionData(QDir pPSProjectFolder)
    : mExposure(ExposureSettings::DEFAULT_EXPOSURE),
      mSettings(pPSProjectFolder.absolutePath() + QDir::separator() + "psh_meta.ini", QSettings::IniFormat) {
    // Fill everything with default values
    mSessionFolder = pPSProjectFolder;
    mStatus = PSS_UNKNOWN;

    mName = "";
    mNotes = QStringList();

    // Initialize settings
    initSettingsFile();

    // Build for the first time
    examineProject();
}

PSSessionData::~PSSessionData() {}

void PSSessionData::examineProject() {

    // Examine the directory for images and project files
    examineDirectory(mSessionFolder);
    examineImages();
    extractInfoFromFolderName(mSessionFolder.dirName());

    if(mPSProjectFile.fileName() == "") return;

    // Initialize the project
    mPSProject = new PSProjectFileData(mPSProjectFile);

    autoSetStatus();
}

void PSSessionData::extractInfoFromFolderName(QString pFolderName) {
    // The original format: "[ID_as_integer] rest of folder name"
    QStringList parts = pFolderName.split("\\s");
    if(parts.length() > 1) {
        // Get the ID
        setID(parts[0]);

        // Build description
        parts.removeFirst();
        mName = parts.join(' ');
    } else {
        // MIA format: "[Date as YYMMDD]_[accession number/ID w/ characters]_[name/description]"
//        parts = pFolderName.split("_");
//        mID = ""; int endOfID = 1;
//        if(parts != NULL && parts.length > 1) {
//            try {
//                while(endOfID<parts.length) {
//                    if(endOfID > 1 && !Character.isDigit(parts[endOfID].charAt(0))) {
//                        break;
//                    } else {
//                        if(endOfID == 1) { mID = parts[endOfID]; }
//                        else { mID += "_" + parts[endOfID]; }
//                    }
//                    endOfID++;
//                }
//            } catch(Exception e) { mID = "-1"; }

//            // Try to parse out the date
//            SimpleDateFormat dateFormatter = new SimpleDateFormat("yyMMdd");
//            try {
//                mDateTakenStart = dateFormatter.parse(parts[0]);
//                mDateTakenFinish = mDateTakenStart;
//            } catch (...) {
//                mDateTakenStart = mDateTakenFinish = QDateTime();
//            }

//            // Build description
//            StringJoiner joiner = new StringJoiner(" ");
//            for(int i=endOfID; i<parts.length; i++) {
//                joiner.add(parts[i]);
//            }

//            mName = joiner.toString();
//        }
    }
}

int PSSessionData::compareTo(const PSSessionData* o) const {
    switch(mSortBy) {
        default:
        case F_PROJECT_FOLDER:
            return mSessionFolder.dirName().compare(o->mSessionFolder.dirName());

        case F_PROJECT_ID: return mID.compare(o->mID);
        case F_PROJECT_NAME: return getName().compare(o->getName());
        case F_PHOTO_DATE:
        {
            if(mDateTimeCaptured.isNull()) return -1;
            if(o->mDateTimeCaptured.isNull()) return 1;
            return o->mDateTimeCaptured.daysTo(mDateTimeCaptured);
        }
        case F_IMAGE_COUNT_REAL: return mProcessedFileList.length() - o->mProcessedFileList.length();

        case F_PROJECT_STATUS: return (int)(mStatus) - (int)(o->mStatus);
        case F_IMAGE_ALIGN_LEVEL: return describeImageAlignPhase().compare(o->describeImageAlignPhase());
        case F_DENSE_CLOUD_LEVEL: return (int)(getDenseCloudPhaseStatus()) - (int)(o->getDenseCloudPhaseStatus());
        case F_MODEL_GEN_LEVEL: return (int)(getModelFaceCount()) - (int)(o->getModelFaceCount());
        case F_TEXTURE_GEN_LEVEL: return describeTextureGenPhase().compare(o->describeTextureGenPhase());
    }
}

bool compareFileInfo(const QFileInfo& A, const QFileInfo& B) {
    return (A.fileName().compare(B.fileName()) < 0);
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
bool PSSessionData::examineDirectory(QDir pDirToExamine) {
    // Sanity check
    if(!pDirToExamine.exists()) {
        return false;
    }

    // Clear old data
    mPSProjectFile = QFileInfo();
    mDateTimeCaptured = QDateTime();

    // Build listers for the three types of files
    DirLister lProjectFileLister(pDirToExamine, gPSProjectFileExtensions);
    DirLister lImageFileLister(pDirToExamine, gPSImageFileExtensions);
    DirLister lRawFileLister(pDirToExamine, gRawFileExtensions);

    // Get info from the listers
    if(lProjectFileLister.getMatches().length() > 1) {
        qWarning() << "Warning: Multiple .psz files found. Using the first one.";
    }
    mPSProjectFile = lProjectFileLister.getMatches()[0];

    // Extract dates if there are raw files
    if(mRawFileList.length() > 0) {
        // Just assume first and last files are the earliest and latest files
        std::sort(mRawFileList.begin(), mRawFileList.end(), compareFileInfo);
        try {
//            mDateTakenStart = ImageProcessorIM4J.getDateFromMetadata(mRawFileList[0]);
//            mDateTakenFinish = ImageProcessorIM4J.getDateFromMetadata(mRawFileList[mRawFileList.length-1]);
        } catch (...) {
            qWarning("Error: failed to extract dates from raw images\n");
            mDateTimeCaptured = QDateTime();
        }
    } else {
        mDateTimeCaptured = QDateTime();
    }

    // Return success
    return true;
}

void PSSessionData::setImages(QDir& pDir, QFileInfoList& pImageList, const QStringList pFilter, const QString pFolderName) {
    // Create QDir
    pDir = QDir(mSessionFolder.absolutePath() + QDir::separator() + pFolderName);

    // Check to see if it exists
    if(!pDir.exists()) {
        // If not, create that directory
        qDebug() << pDir.mkdir(mSessionFolder.absolutePath() + QDir::separator() + pFolderName);
    }

    // Iterate through the QDir
    QDirIterator lIt(mSessionFolder.absolutePath(), pFilter, QDir::Files);
    while (lIt.hasNext() && !lIt.next().isNull()) {
        if(lIt.fileName().isEmpty()) {
            continue;
        }
        // Move files into proper folders
        QString newName = mSessionFolder.absolutePath() + QDir::separator() + pFolderName + QDir::separator() + lIt.fileName();
        qDebug() << QFile::rename(lIt.filePath(), newName);
        qDebug() << lIt.filePath() << " <--> " << newName;
    }

    // Store files in QFileInfoList
    // NOTE: May need to include symlinks at a later time
    pImageList = pDir.entryInfoList(pFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    qDebug() << pFolderName << " length: " << pImageList.length();
}

void PSSessionData::examineImages() {
    setImages(mMasksFolder, mMaskFileList, gPSMaskFileExtensions, "Masks");
    setImages(mRawFolder, mRawFileList, gRawFileExtensions, "Raw");
    setImages(mProcessedFolder, mProcessedFileList, gPSImageFileExtensions, "Processed");
}

void PSSessionData::autoSetStatus() {
    // Pick one of the auto-status phases
    if(getProcessedImageCount() == 0) {
        mStatus = PSS_UNPROCESSSED;
    } else if(describeImageAlignPhase() == "N/A") {
        mStatus = PSS_RAW_PROCESSING_DONE;
    } else if(describeDenseCloudPhase() == "N/A") {
        mStatus = PSS_ALIGNMENT_DONE;
    } else if(describeModelGenPhase() == "N/A") {
        mStatus = PSS_POINT_CLOUD_DONE;
    } else if(describeTextureGenPhase() == "N/A") {
        mStatus = PSS_MODEL_GEN_DONE;
    } else {
        mStatus = PSS_TEXTURE_GEN_DONE;
    }
}

void PSSessionData::setCustomStatus(int statusIndex) {
    int ordinal = PSS_TEXTURE_GEN_DONE + statusIndex;
    if(ordinal > PSS_TEXTURE_GEN_DONE && ordinal <= PSS_FINAL_APPROVAL) {
        mStatus = (Status)ordinal;
    } else {
        autoSetStatus();
    }
}

void PSSessionData::setID(QString pID) {
    mID = pID;
    if(mID.toInt() > mNextID) {
        mNextID = mID.toInt() + 1;
    }
}

QString PSSessionData::getID() const { return mID; }
int PSSessionData::getNextID() { return mNextID; }
void PSSessionData::addNotes(QString pNotes) { mNotes.append(pNotes); }
void PSSessionData::setName(QString pName) { mName = pName; }

QFileInfo PSSessionData::getPSProjectFile() const {
    return mPSProjectFile;
}

QDir PSSessionData::getSessionFolder() const {
    return mSessionFolder;
}

PSModelData* PSSessionData::getModelData() const {
    return mPSProject->getModelData();
}

QFileInfo PSSessionData::getModelArchiveFile() const {
    return mPSProject->getModelArchiveFile();
}

size_t PSSessionData::getRawImageCount() const { return mRawFileList.length(); }
size_t PSSessionData::getProcessedImageCount() const { return mProcessedFileList.length(); }
size_t PSSessionData::getMaskImageCount() const { return mMaskFileList.length(); }
QFileInfoList PSSessionData::getRawFileList() const { return mRawFileList; }
QFileInfoList PSSessionData::getProcessedFileList() const { return mProcessedFileList; }
QFileInfoList PSSessionData::getMaskFileList() const { return mMaskFileList; }

// TODO: Implement this vvv
bool PSSessionData::isImageExposureKnown() const { return true; }

const double* PSSessionData::getWhiteBalanceMultipliers() const { return mExposure.getWBCustom(); }
double PSSessionData::getBrightnessMultiplier() const { return mExposure.getBrightScale(); }
QDateTime PSSessionData::getDateTimeCaptured() const { return mDateTimeCaptured; }
QStringList PSSessionData::getNotes() const { return mNotes; }

QString PSSessionData::getName() const {
    if(mName.isNull() || mName.isEmpty()) {
        return mSessionFolder.dirName();
    }
    return mName;
}

QString PSSessionData::getNameStrict() const { return mName; }

PSSessionData::Status PSSessionData::getStatus() const { return mStatus; }

void PSSessionData::initSettingsFile() {
    if (mSettings.status() != QSettings::NoError) {
        qDebug("Error with session ini file.");
        return;
    }

    readGeneralSettings();
    readExposureSettings();
}

void PSSessionData::writeGeneralSettings() {
    mSettings.beginGroup("General");

    // Write these only if they are not empty
    if(!mID.isEmpty()) { mSettings.setValue("ID", mID); }
    if(!mName.isEmpty()) { mSettings.setValue("Name", mName); }
    if(!mDescription.isEmpty()) { mSettings.setValue("Description", mDescription); }
    if(!mNotes.isEmpty()) {
        mSettings.beginWriteArray("Notes");
        for (int i = 0; i < mNotes.size(); i++) {
            mSettings.setArrayIndex(i);
            mSettings.setValue("note", mNotes[i]);
        }
        mSettings.endArray();
    }

    // TODO: Write date and time of capture to settings

    // Only write custom status states
    // TODO: Consider storing ALL statuses, not just custom ones
    if(mStatus > PSS_TEXTURE_GEN_DONE) { mSettings.setValue("Status", (int)mStatus); }

    mSettings.endGroup();
}

void PSSessionData::readGeneralSettings() {
    mSettings.beginGroup("General");

    // Retrieve the general settings
    mID = mSettings.value("ID", mID).toString();
    mName = mSettings.value("Name", mName).toString();
    mDescription = mSettings.value("Description", mDescription).toString();

    QStringList settingsNotes;
    int notesSize = mSettings.beginReadArray("Notes");
    for (int i = 0; i < notesSize; ++i) {
        mSettings.setArrayIndex(i);
        settingsNotes << mSettings.value("note").toString();
    }
    mSettings.endArray();

    // TODO: Read date and time of capture

    // Only read these settings if they are not empty
    if(!settingsNotes.isEmpty()) { mNotes = settingsNotes; }

    // Only read and accept custom status states
    int statVal = mSettings.value("Status", "0").toInt();
    if(statVal > PSS_TEXTURE_GEN_DONE) { mStatus = (Status)statVal; }

    mSettings.endGroup();
}

void PSSessionData::writeExposureSettings(ExposureSettings pExpSettings) {
    // Write those to the settings file for the application
    mSettings.beginGroup("Exposure");

    mSettings.setValue("WhiteBalanceMode", (int)pExpSettings.getWBMode());
    mSettings.setValue("WhiteBalanceMode/R", pExpSettings.getWBCustom()[0]);
    mSettings.setValue("WhiteBalanceMode/G1", pExpSettings.getWBCustom()[1]);
    mSettings.setValue("WhiteBalanceMode/B", pExpSettings.getWBCustom()[2]);
    mSettings.setValue("WhiteBalanceMode/G2", pExpSettings.getWBCustom()[3]);

    mSettings.setValue("BrightnessMode", (int)pExpSettings.getBrightMode());
    mSettings.setValue("BrightnessMode/Scaler", pExpSettings.getBrightScale());

    // Close the group
    mSettings.endGroup();
}

ExposureSettings PSSessionData::readExposureSettings() {
    if(mSettings.status() != QSettings::NoError) return ExposureSettings::DEFAULT_EXPOSURE;

    ExposureSettings lDefSettings = ExposureSettings::DEFAULT_EXPOSURE;

    mSettings.beginGroup("Exposure");

    int lWBOrdinal = mSettings.value("WhiteBalanceMode", (int)lDefSettings.getWBMode()).toInt();
    double lWBCustom[4] = {
        mSettings.value("WhiteBalanceMode/R", lDefSettings.getWBCustom()[0]).toDouble(),
        mSettings.value("WhiteBalanceMode/G1", lDefSettings.getWBCustom()[1]).toDouble(),
        mSettings.value("WhiteBalanceMode/B", lDefSettings.getWBCustom()[2]).toDouble(),
        mSettings.value("WhiteBalanceMode/G2", lDefSettings.getWBCustom()[3]).toDouble()
    };

    int lBrightMOrdinal = mSettings.value("BrightnessMode", (int)lDefSettings.getBrightMode()).toInt();
    double lBrightScaler = mSettings.value("BrightnessMode/Scaler", lDefSettings.getBrightScale()).toDouble();

    // Close the group and dispose of this object (cause you know, garbage collection)
    mSettings.endGroup();

    return ExposureSettings((ExposureSettings::WhiteBalanceMode)lWBOrdinal, lWBCustom,
                            (ExposureSettings::BrightnessMode)lBrightMOrdinal, lBrightScaler);
}

/**
 * Overloading of the standard string conversion function.  Describes all parts
 * of this object in a string for printing to the screen.  NOT for serialization.
 * @see java.lang.Object#toString()
 */
QString PSSessionData::toString() const {
    // General information
    QString lDetails = "Session: " + mSessionFolder.dirName();
    lDetails += "\n";

    lDetails += "\tName: ";
    if(mName != NULL && mName != "") { lDetails += mName; }
    else { lDetails += "[none]"; }
    lDetails += "\n";

    lDetails += "\tNotes: ";
    if(!mNotes.isEmpty()) { lDetails += mNotes.join("; "); }
    else { lDetails += "[none]"; }
    lDetails += "\n";

    // Image data details
    lDetails += "\n\tImages - " + QString::number(mProcessedFileList.length()) + " in folder\n";
    if(mRawFileList.length() > 0) {
        lDetails += "\t         " + QString::number(mRawFileList.length()) + " raw images\n";
    }

    if(isImageExposureKnown()) {
        const double* lMult = mExposure.getWBCustom();
        lDetails += "\tTaken on " + mDateTimeCaptured.toString();
        lDetails += "\tConverted as - ";
        for(int i=0; i<4; i++) {
            lDetails.append(QString::number(lMult[i]));
            if (i<3) { lDetails += ", "; }
            else { lDetails += "\n"; }
        }

        double lScaler = mExposure.getBrightScale();
        if(fabs(lScaler - 1.0) > 1e-8) {
            lDetails += "\t               " + QString::number(lScaler) + " adjustment\n";
        }
    }

    return lDetails;
}

PSProjectFileData* PSSessionData::getActiveProject() const {
    return mPSProject;
}

int PSSessionData::getChunkCount() const {
    return mPSProject->getChunkCount();
}

int PSSessionData::getActiveChunkIndex() const {
    return mPSProject->getActiveChunkIndex();
}

PSChunkData* PSSessionData::getChunk(int index) const {
    return mPSProject->getChunk(index);
}

PSChunkData* PSSessionData::getActiveChunk() const {
    return mPSProject->getActiveChunk();
}

QString PSSessionData::describeImageAlignPhase() const {
    return mPSProject->describeImageAlignPhase();
}

char PSSessionData::getAlignPhaseStatus() const {
    return mPSProject->getAlignPhaseStatus();
}

QString PSSessionData::describeDenseCloudPhase() const {
    return mPSProject->describeDenseCloudPhase();
}

int PSSessionData::getDenseCloudDepthImages() const {
    return mPSProject->getDenseCloudDepthImages();
}

char PSSessionData::getDenseCloudPhaseStatus() const {
    return mPSProject->getDenseCloudPhaseStatus();
}

QString PSSessionData::describeModelGenPhase() const {
    return mPSProject->describeModelGenPhase();
}

char PSSessionData::getModelGenPhaseStatus() const {
    return mPSProject->getModelGenPhaseStatus();
}

long PSSessionData::getModelFaceCount() const {
    return mPSProject->getModelFaceCount();
}

long PSSessionData::getModelVertexCount() const {
    return mPSProject->getModelVertexCount();
}

QString PSSessionData::describeTextureGenPhase() const {
    return mPSProject->describeTextureGenPhase();
}

char PSSessionData::getTextureGenPhaseStatus() const {
    return mPSProject->getTextureGenPhaseStatus();
}
