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
PSSessionData::PSSessionData(QDir pPSProjectFolder, QSettings* settings)
    : mExposure(ExposureSettings::DEFAULT_EXPOSURE) { // throws IOException {
    // Fill everything with default values
    mSessionFolder = pPSProjectFolder;
    mStatus = PSS_UNKNOWN;

    mImageCount_raw = 0;
    mImageCount_processed = 0;

    mName = "";
    mNotes = QStringList();

    // Build for the first time
    examineProject(settings);
}

PSSessionData::~PSSessionData() {}

void PSSessionData::examineProject(QSettings* settings) { // throws IOException, IllegalArgumentException {

    // Examine the directory for images and project files
    examineDirectory(mSessionFolder);
    examineImages();
    extractInfoFromFolderName(mSessionFolder.dirName());

    if(mPSProjectFile.fileName() == "") return;

    // Initialize the project
    mPSProject = new PSProjectFileData(mPSProjectFile, settings);

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
        case F_IMAGE_COUNT_REAL: return mImageCount_processed - o->mImageCount_processed;

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
    mImageCount_raw = mImageCount_processed = 0;

    // Build listers for the three types of files
    DirLister lProjectFileLister(pDirToExamine, gPSProjectFileExtensions);
    DirLister lImageFileLister(pDirToExamine, gPSImageFileExtensions);
    DirLister lRawFileLister(pDirToExamine, gRawFileExtensions);

    // Get info from the listers
    if(lProjectFileLister.getMatches().length() > 1) {
        qWarning() << "Warning: Multiple .psz files found. Using the first one.";
    }
    mPSProjectFile = lProjectFileLister.getMatches()[0];
    mImageCount_processed = lImageFileLister.count();
    mImageCount_raw = lRawFileLister.count();

    // Extract dates if there are raw files
    if(mImageCount_raw > 0) {
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

void PSSessionData::processImages(QDir& pDir, QFileInfoList& pImageList, const QStringList pFilter, const QString pFolderName) {
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
    processImages(mMasksFolder, mMaskFileList, gPSMaskFileExtensions, "Masks");
    processImages(mRawFolder, mRawFileList, gRawFileExtensions, "Raw");
    processImages(mProcessedFolder, mProcessedFileList, gPSImageFileExtensions, "Processed");
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

long PSSessionData::getRawImageCount() const { return mImageCount_raw; }
long PSSessionData::getProcessedImageCount() const { return mImageCount_processed; }
QFileInfoList PSSessionData::getRawFileList() const { return mRawFileList; }

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

void PSSessionData::writeGeneralSettings(QSettings* settings) const {
    if(settings == NULL) return;

    // We count on the folder name being unique inside this collection
    settings->beginGroup(mSessionFolder.dirName().replace(' ',  '_') + "_Settings");

    // Write these only if they are not empty
    if(!mID.isEmpty()) { settings->setValue("General/ID", mID); }
    if(!mName.isEmpty()) { settings->setValue("General/Name", mName); }
    if(!mNotes.isEmpty()) {
        settings->beginWriteArray("General/notes");
        for (int i = 0; i < mNotes.size(); i++) {
            settings->setArrayIndex(i);
            settings->setValue("note", mNotes[i]);
        }
        settings->endArray();
    }

    // Only write custom status states
    if(mStatus > PSS_TEXTURE_GEN_DONE) { settings->setValue("General/Status", (int)mStatus); }

    settings->endGroup();
}

void PSSessionData::readGeneralSettings(QSettings* settings) {
    if(settings == NULL) return;

    settings->beginGroup(mSessionFolder.dirName().replace(' ',  '_') + "_Settings");

    // Retrieve the general settings
    QString settingsID = settings->value("General/ID", mID).toString();
    QString settingsName = settings->value("General/Name", "").toString();
    if(settingsName.isEmpty()) { settingsName = settings->value("General/Description", "").toString(); }

    QStringList settingsNotes;
    int notesSize = settings->beginReadArray("General/notes");
    for (int i = 0; i < notesSize; ++i) {
        settings->setArrayIndex(i);
        settingsNotes << settings->value("note").toString();
    }
    settings->endArray();

    // Only read these settings if they are not empty
    if(!settingsID.isEmpty()) { mID = settingsID; }
    if(!settingsName.isEmpty()) { mName = settingsName; }
    if(!settingsNotes.isEmpty()) { mNotes = settingsNotes; }

    // Only read and accept custom status states
    int statVal = settings->value("General/Status", "0").toInt();
    if(statVal > PSS_TEXTURE_GEN_DONE) { mStatus = (Status)statVal; }

    settings->endGroup();
}

void PSSessionData::writeExposureSettings(ExposureSettings pExpSettings, QSettings* settings) const {
    // Write those to the settings file for the application
    if(settings == NULL) return;
    settings->beginGroup(mSessionFolder.dirName().replace(' ',  "_") + "_Settings");

    settings->setValue("WhiteBalanceMode", (int)pExpSettings.getWBMode());
    settings->setValue("WhiteBalanceMode/R", pExpSettings.getWBCustom()[0]);
    settings->setValue("WhiteBalanceMode/G1", pExpSettings.getWBCustom()[1]);
    settings->setValue("WhiteBalanceMode/B", pExpSettings.getWBCustom()[2]);
    settings->setValue("WhiteBalanceMode/G2", pExpSettings.getWBCustom()[3]);

    settings->setValue("BrightnessMode", (int)pExpSettings.getBrightMode());
    settings->setValue("BrightnessMode/Scaler", pExpSettings.getBrightScale());

    // Close the group
    settings->endGroup();
}

ExposureSettings PSSessionData::readExposureSettings(QSettings* settings) {
    if(settings == NULL) return ExposureSettings::DEFAULT_EXPOSURE;

    ExposureSettings lDefSettings = ExposureSettings::DEFAULT_EXPOSURE;

    settings->beginGroup(mSessionFolder.dirName().replace(' ',  "_") + "_Settings");

    int lWBOrdinal = settings->value("WhiteBalanceMode", (int)lDefSettings.getWBMode()).toInt();
    double lWBCustom[4] = {
        settings->value("WhiteBalanceMode/R", lDefSettings.getWBCustom()[0]).toDouble(),
        settings->value("WhiteBalanceMode/G1", lDefSettings.getWBCustom()[1]).toDouble(),
        settings->value("WhiteBalanceMode/B", lDefSettings.getWBCustom()[2]).toDouble(),
        settings->value("WhiteBalanceMode/G2", lDefSettings.getWBCustom()[3]).toDouble()
    };

    int lBrightMOrdinal = settings->value("BrightnessMode", (int)lDefSettings.getBrightMode()).toInt();
    double lBrightScaler = settings->value("BrightnessMode/Scaler", lDefSettings.getBrightScale()).toDouble();

    // Close the group and dispose of this object (cause you know, garbage collection)
    settings->endGroup();

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
    lDetails += "\n\tImages - " + QString::number(mImageCount_processed) + " in folder\n";
    if(mImageCount_raw > 0) {
        lDetails += "\t         " + QString::number(mImageCount_raw) + " raw images\n";
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
