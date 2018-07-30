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

// Values to control how containers of PSSessionData objects are sorted
PSSessionData::Field PSSessionData::mSortBy = PSSessionData::F_PROJECT_ID;
void PSSessionData::setSortBy(PSSessionData::Field pNewSortBy) {
    mSortBy = pNewSortBy;
}

// The next ID available that is guaranteed to be unique
uint64_t PSSessionData::mNextID = 0;

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

// Images sub-folder names
const QString PSSessionData::sRawFolderName = "Raw";
const QString PSSessionData::sProcessedFolderName = "Processed";
const QString PSSessionData::sMasksFolderName = "Masks";

PSSessionData::PSSessionData(QDir pPSProjectFolder)
    : mSettings(pPSProjectFolder.absolutePath() + QDir::separator() + "psh_meta.ini"),
      mExposure(ExposureSettings::DEFAULT_EXPOSURE) {
    // Fill everything with default values
    mSessionFolder = pPSProjectFolder;
    mStatus = PSS_UNKNOWN;
    mRawFileCount = mProcessedFileCount = mMaskFileCount = 0;

    isExternal = true;
    isConsistent = isSynchronized = false;

    mName = "";
    mNotes = QStringList();

    // Update project state and synchronize
    examineProject();
}

PSSessionData::~PSSessionData() {}

void PSSessionData::examineProject() {
    // Build folder names
    mRawFolder = QDir(mSessionFolder.absolutePath() + QDir::separator() + sRawFolderName);
    mProcessedFolder = QDir(mSessionFolder.absolutePath() + QDir::separator() + sProcessedFolderName);
    mMasksFolder = QDir(mSessionFolder.absolutePath() + QDir::separator() + sMasksFolderName);

    // Reset image counts
    mRawFileCount = mProcessedFileCount = mMaskFileCount = -1;

    // Preliminary examination of directory
    examineDirectory(mSessionFolder);

    // Is this an external session folder that needs to be fully examined (no INI file yet)
    if (!QFileInfo(mSettings).exists()) {
        // Try to guess some info from the folder name
        extractInfoFromFolderName(mSessionFolder.dirName());

        // Parse the PhotoScan XML file
        if(mPSProjectFile.filePath() != "") {
            qDebug() << "Working fine";
            mPSProject = new PSProjectFileData(mPSProjectFile);
        }

        // Ensure the image files lists are initialized and image counts are accurate
        getRawFileList(); getProcessedFileList(); getMaskFileList();

        // Update status and create initial INI file
        autoSetStatus();
        writeGeneralSettings();

        // Update state
        isExternal = false;
        isSynchronized = isConsistent = true;
    } else {
        // Read from INI file
        readGeneralSettings();
        readExposureSettings();

        // NOTE: image files lists will not be created until the first time
        //       their getters are called (to shorten loading time).

        // Update state
        isExternal = false;

        // TODO: Fix these so they actually check project filename and last modified timestamp
        isSynchronized = isConsistent = true;
    }
}

void PSSessionData::extractInfoFromFolderName(QString pFolderName) {
    // The original format: "[ID_as_integer] rest of folder name"
    QStringList parts = pFolderName.split(" ");
    if(parts.length() > 1) {
        // Get the ID
        setID(parts[0].toULongLong());

        // Build description
        parts.removeFirst();
        mName = parts.join(' ');
    } else {
        qWarning() << "Folder name not in correct format";
    }
}

int PSSessionData::compareTo(const PSSessionData* o) const {
    switch(mSortBy) {
        default:
        case F_PROJECT_FOLDER:
            return mSessionFolder.dirName().compare(o->mSessionFolder.dirName());

        case F_PROJECT_ID: return (mID - o->mID);
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

bool PSSessionData::examineDirectory(QDir pDirToExamine) {
    // Sanity check
    if(!pDirToExamine.exists()) {
        return false;
    }

    // Clear old data
    mPSProjectFile = QFileInfo();
    mDateTimeCaptured = QDateTime();

    // Ensure the image folders exist and files are copied into them
    initImageDir(mMasksFolder, gPSMaskFileExtensions, "Masks");
    initImageDir(mRawFolder, gRawFileExtensions, "Raw");
    initImageDir(mProcessedFolder, gPSImageFileExtensions, "Processed");

    // Find the project file
    QFileInfoList lProjectFiles = pDirToExamine.entryInfoList(
                gPSProjectFileExtensions, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    if(!lProjectFiles.isEmpty()) {
        mPSProjectFile = lProjectFiles[0];
        if(lProjectFiles.length() > 1) {
            qDebug("Session Warning: More then one project file. Will use the first one found only! (%s)",
                   mPSProjectFile.fileName().toLocal8Bit().data());
        }
    }

    // Return success
    return true;
}

inline void PSSessionData::initImageDir(const QDir &pDir, const QStringList& pFilter, const QString& pFolderName) {
    // Check to see if directory exists
    if(!pDir.exists()) {
        // If not, try to create that directory
        QString lNewDir = mSessionFolder.absolutePath() + QDir::separator() + pFolderName;
        if (!mSessionFolder.mkdir(pFolderName) || !pDir.exists()) {
            qDebug("Failed to create directory %s", lNewDir.toLocal8Bit().data());
        }
    }

    // Iterate through the all files that match the given filter in the session root dir
    QDirIterator lIt(mSessionFolder.absolutePath(), pFilter, QDir::Files);
    while (lIt.hasNext() && !lIt.next().isNull()) {
        if(lIt.fileName().isEmpty()) {
            continue;
        }

        // Move matched files into the given folder
        QString newName = pDir.absolutePath() + QDir::separator() + lIt.fileName();
    }
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

void PSSessionData::setID(uint64_t pID) {
    mID = pID;
    if(mID > mNextID) {
        mNextID = mID + 1;
    }
}

uint64_t PSSessionData::getID() const { return mID; }
uint64_t PSSessionData::getNextID() { return mNextID; }
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

int PSSessionData::getRawImageCount() const { return mRawFileCount; }
int PSSessionData::getProcessedImageCount() const { return mProcessedFileCount; }
int PSSessionData::getMaskImageCount() const { return mMaskFileCount; }

const QFileInfoList& PSSessionData::getRawFileList() const {
    if(mRawFileList.length() != mRawFileCount) {
        mRawFileList = mRawFolder.entryInfoList(gRawFileExtensions,
                           QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        mRawFileCount = mRawFileList.length();
    }
    return mRawFileList;
}

const QFileInfoList& PSSessionData::getProcessedFileList() const {
    if(mProcessedFileList.length() != mProcessedFileCount) {
        mProcessedFileList = mProcessedFolder.entryInfoList(gPSImageFileExtensions,
                                 QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        mProcessedFileCount = mProcessedFileList.length();
    }
    return mProcessedFileList;
}

const QFileInfoList& PSSessionData::getMaskFileList() const {
    if(mMaskFileList.length() != mMaskFileCount) {
        mMaskFileList = mMasksFolder.entryInfoList(gPSMaskFileExtensions,
                                 QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        mMaskFileCount = mMaskFileList.length();
    }
    return mMaskFileList;
}

QDateTime PSSessionData::getDateTimeCaptured() const { return mDateTimeCaptured; }
QStringList PSSessionData::getNotes() const { return mNotes; }
QString PSSessionData::getName() const { return mName; }
PSSessionData::Status PSSessionData::getStatus() const { return mStatus; }

void PSSessionData::initSettingsFile() {
}

void PSSessionData::writeGeneralSettings() {
    QSettings lSettings(mSettings, QSettings::IniFormat);
    lSettings.beginGroup("General");
    if (lSettings.status() != QSettings::NoError) {
        qDebug("Error with session ini file.");
    }

    // Write these only if they are not empty
    if(mID != NULL) { lSettings.setValue("ID", mID); }
    if(!mName.isEmpty()) { lSettings.setValue("Name", mName); }
    if(!mDescription.isEmpty()) { lSettings.setValue("Description", mDescription); }
    if(!mNotes.isEmpty()) {
        lSettings.beginWriteArray("Notes");
        for (int i = 0; i < mNotes.size(); i++) {
            lSettings.setArrayIndex(i);
            lSettings.setValue("note", mNotes[i]);
        }
        lSettings.endArray();
    }

    if(!mDateTimeCaptured.isNull()) { lSettings.setValue("DateTime", mDateTimeCaptured.toString()); }

    // Only write custom status states
    lSettings.setValue("Status", static_cast<int>(mStatus));
    lSettings.endGroup();

    // Store image counts so we can read them without scaning the image directories
    lSettings.beginGroup("Images");
    lSettings.setValue("RawImageCount", mRawFileCount);
    lSettings.setValue("ProcessedImageCount", mProcessedFileCount);
    lSettings.setValue("MaskImageCount", mMaskFileCount);
    lSettings.endGroup();
}

void PSSessionData::readGeneralSettings() {
    QSettings lSettings(mSettings, QSettings::IniFormat);
    lSettings.beginGroup("General");

    // Retrieve the general settings
    mID = lSettings.value("ID", mID).toULongLong();
    mName = lSettings.value("Name", mName).toString();
    mDescription = lSettings.value("Description", mDescription).toString();

    QStringList settingsNotes;
    int notesSize = lSettings.beginReadArray("Notes");
    for (int i = 0; i < notesSize; ++i) {
        lSettings.setArrayIndex(i);
        settingsNotes << lSettings.value("note").toString();
    }
    lSettings.endArray();

    mDateTimeCaptured = QDateTime::fromString(lSettings.value("DateTime").toString());

    // Only read these settings if they are not empty
    if(!settingsNotes.isEmpty()) { mNotes = settingsNotes; }

    // Read most recent status
    mStatus = static_cast<Status>(lSettings.value("Status", "0").toInt());

    lSettings.endGroup();
}

void PSSessionData::writeExposureSettings(ExposureSettings pExpSettings) {
    QSettings lSettings(mSettings, QSettings::IniFormat);

    // Write those to the settings file for the application
    lSettings.beginGroup("Exposure");

    lSettings.setValue("WhiteBalanceMode", static_cast<int>(pExpSettings.getWBMode()));
    lSettings.setValue("WhiteBalanceMode/R", pExpSettings.getWBCustom()[0]);
    lSettings.setValue("WhiteBalanceMode/G1", pExpSettings.getWBCustom()[1]);
    lSettings.setValue("WhiteBalanceMode/B", pExpSettings.getWBCustom()[2]);
    lSettings.setValue("WhiteBalanceMode/G2", pExpSettings.getWBCustom()[3]);

    lSettings.setValue("BrightnessMode", static_cast<int>(pExpSettings.getBrightMode()));
    lSettings.setValue("BrightnessMode/Scaler", pExpSettings.getBrightScale());

    // Close the group
    lSettings.endGroup();
}

ExposureSettings PSSessionData::readExposureSettings() const {
    ExposureSettings lDefSettings = ExposureSettings::DEFAULT_EXPOSURE;

    QSettings lSettings(mSettings, QSettings::IniFormat);
    lSettings.beginGroup("Exposure");

    int lWBOrdinal = lSettings.value("WhiteBalanceMode",
                                     static_cast<int>(lDefSettings.getWBMode())).toInt();
    double lWBCustom[4] = {
        lSettings.value("WhiteBalanceMode/R", lDefSettings.getWBCustom()[0]).toDouble(),
        lSettings.value("WhiteBalanceMode/G1", lDefSettings.getWBCustom()[1]).toDouble(),
        lSettings.value("WhiteBalanceMode/B", lDefSettings.getWBCustom()[2]).toDouble(),
        lSettings.value("WhiteBalanceMode/G2", lDefSettings.getWBCustom()[3]).toDouble()
    };

    int lBrightMOrdinal = lSettings.value("BrightnessMode", static_cast<int>(lDefSettings.getBrightMode())).toInt();
    double lBrightScaler = lSettings.value("BrightnessMode/Scaler", lDefSettings.getBrightScale()).toDouble();

    // Close the group and dispose of this object (cause you know, garbage collection)
    lSettings.endGroup();

    return ExposureSettings(
                static_cast<ExposureSettings::WhiteBalanceMode>(lWBOrdinal), lWBCustom,
                static_cast<ExposureSettings::BrightnessMode>(lBrightMOrdinal), lBrightScaler);
}

PSProjectFileData* PSSessionData::getProject() const {
    return mPSProject;
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
