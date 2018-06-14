#include <exception>
#include "PSandPhotoScanner.h"

#include <QSettings>
#include <QtConcurrent>

#include "DirLister.h"
#include "PSSessionData.h"

const QStringList gIgnoreExceptions = { "_Finished", "_TouchUp", "_TouchedUpPleaseReview" };

PSandPhotoScanner::PSandPhotoScanner(QString pPath, int pMaxRecursionDepth) {
    mRootPath = QFileInfo(pPath);
    mMaxRecursionDepth = pMaxRecursionDepth;
    mDataScanned = false;

    // Open/Create the settings QFileInfo
    QString settingsFile = mRootPath.path() + "/PSHelperData.ini";
    mPSProjectInfoStore = initInfoStore(settingsFile);
}

bool PSandPhotoScanner::buildDirectoryList(QFileInfo pRoot) {
    // List and examine all directories
    DirLister lProjectLister(QDir(pRoot.path()), QStringList(), mMaxRecursionDepth, true);
    mProjectDirList = lProjectLister.getMatches();

    // Remove directories that start with '_'
    QFileInfoList lRemove;
    for(QFileInfo lDir : mProjectDirList) {
        if (lDir.filePath().contains("_")) {
            bool lIsException = false;
            for(QString lException : gIgnoreExceptions) {
                lIsException |= lDir.filePath().contains(lException);
            }
            if (!lIsException) {
                lRemove.append(lDir);
            }
        }
    }

    for(QFileInfo lDir : lRemove) {
        mProjectDirList.removeOne(lDir);
    }

    // Return success if we found at least one
    return (!mProjectDirList.empty());
}

QSettings* PSandPhotoScanner::initInfoStore(QString pSettingsFile) {
    QSettings* newSettings = new QSettings(pSettingsFile, QSettings::IniFormat);

    // Initialize the ini QFileInfo if it doesn't exist yet
    QFileInfo settings(pSettingsFile);
    if(!settings.exists()) {
        newSettings->beginGroup("Collection");
        newSettings->setValue("Name", "None");
        newSettings->setValue("Owner", "None");
        newSettings->endGroup();
    }

    return newSettings;
}

QSettings* PSandPhotoScanner::getInfoStore() const {
    return mPSProjectInfoStore;
}

bool PSandPhotoScanner::isReady() const { return mDataScanned; }
const QVector<PSSessionData*> PSandPhotoScanner::getPSProjectData() const { return mData; }

int PSandPhotoScanner::countUniqueDirs() const {
    QSet<QString> lDirSet;
    for(const PSSessionData* lData : mData) {
        QString lPath = lData->getPSProjectFolder().path();
        if (!lDirSet.contains(lPath)) {
            lDirSet.insert(lPath);
        }
    }

    return lDirSet.size();
}

int PSandPhotoScanner::countDirsWithoutProjects() const {
    int count = 0;
    for(PSSessionData* lProj : mData) {
        QFileInfo lFile = lProj->getPSProjectFile();
        if(lFile.filePath() == "") { count++; }
    }

    return count;
}

int PSandPhotoScanner::countDirsWithoutImageAlign() const {
    int count = 0;
    for(const PSSessionData* lData : mData) {
        if(lData->describeImageAlignPhase() == "N/A") {
            count++;
        }
    }

    return count;
}

int PSandPhotoScanner::countDirsWithoutDenseCloud() const {

    int count = 0;
    for(const PSSessionData* lData : mData) {
        if(lData->describeDenseCloudPhase() == "N/A") {
            count++;
        }
    }

    return count;
}

int PSandPhotoScanner::countDirsWithoutModels() const {
    int count = 0;
    for(const PSSessionData* lData : mData) {
        if(lData->describeModelGenPhase() == "N/A") {
            count++;
        }
    }

    return count;
}

PSSessionData* examineProjectFolder(QFileInfo pProjectFolder) {
    try {
        PSSessionData* lData = new PSSessionData(QDir(pProjectFolder.path()), NULL);
        if(lData == NULL || (lData->getPSProjectFile().filePath() == "" &&
           lData->getRawImageCount() == 0 && lData->getProcessedImageCount() == 0)) {
            throw new std::logic_error("No files in directory.");
        } else {
            return lData;
        }
    } catch(std::exception e) {
        qWarning("Error: unable to scan '%s'", pProjectFolder.filePath().toLocal8Bit().data());
        qWarning("Exception: %s", e.what());
        return NULL;
    }
}

QFuture<PSSessionData*> PSandPhotoScanner::startScanParallel() {
    buildDirectoryList(mRootPath);
    mFutureResults = QtConcurrent::mapped(mProjectDirList.begin(), mProjectDirList.end(), examineProjectFolder);
    return mFutureResults;
}

bool greaterThanPSSD(PSSessionData* A, PSSessionData* B) {
    return (A->compareTo(B) > 0);
}

void PSandPhotoScanner::finishDataParallel() {
    for(PSSessionData* lResult : mFutureResults) {
        if (lResult != NULL) {
            mData.append(lResult);
        }
    }

    std::sort(mData.begin(), mData.end(), greaterThanPSSD);
    for(PSSessionData* data : mData) {
        data->readGeneralSettings(mPSProjectInfoStore);
        data->readExposureSettings(mPSProjectInfoStore);
    }
    mDataScanned = true;
}
