#include <exception>
#include "PSandPhotoScanner.h"

#include <QSettings>
#include <QtConcurrent>
#include <QDialogButtonBox>

#include "DirLister.h"
#include "PSSessionData.h"
#include "ImageProcessor.h"

#include "InitializeSessionDialog.h"

const QStringList gIgnoreExceptions = { "_Finished", "_TouchUp", "_TouchedUpPleaseReview" };

PSandPhotoScanner::PSandPhotoScanner(const QString& pPath, int pMaxRecursionDepth) {
    mRootPath = QFileInfo(pPath);
    mMaxRecursionDepth = pMaxRecursionDepth;
    mDataScanned = false;
}

bool PSandPhotoScanner::buildDirectoryList(QFileInfo pRoot) {
    // List and examine all directories
    DirLister lProjectLister(QDir(pRoot.filePath()), QStringList(), mMaxRecursionDepth, true);
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

const QVector<PSSessionData*> PSandPhotoScanner::getPSProjectData() const { return mData; }

int PSandPhotoScanner::countUniqueDirs() const {
    QSet<QString> lDirSet;
    for(const PSSessionData* lData : mData) {
        QString lPath = lData->getSessionFolder().path();
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
    PSSessionData* lData = new PSSessionData(QDir(pProjectFolder.filePath()));
    if(lData == nullptr || (lData->getPSProjectFile().filePath() == "" &&
       lData->getRawImageCount() == 0 && lData->getProcessedImageCount() == 0)) {
        qWarning("Error: unable to scan '%s'", pProjectFolder.filePath().toLocal8Bit().data());
        delete lData;
        return nullptr;
    } else {
        return lData;
    }
}

void syncOrInitializeSession(PSSessionData* pCurSession) {
    if (pCurSession->getExplicitlyIgnored()) {
        return;
    }

    if (!pCurSession->isInitialized()) {
        pCurSession->convertToPSSession();
    } else if (!pCurSession->isSynchronized()) {
        pCurSession->updateOutOfSyncSession();
    }

    if (pCurSession->getDateTimeCaptured().isNull() && pCurSession->getRawImageCount() > 0) {
        QFileInfo lRawFile = pCurSession->getRawFileList()[0];
        QDateTime lTimestamp = ImageProcessor::getDateFromMetadata(lRawFile);
        pCurSession->setDateTimeCaptured(lTimestamp);
        pCurSession->writeGeneralSettings();
    }
}

QFuture<PSSessionData*> PSandPhotoScanner::startScanParallel() {
    buildDirectoryList(mRootPath);
    // TODO: Remove when done debugging
//    QThreadPool::globalInstance()->setMaxThreadCount(1);
    mFutureScanResults = QtConcurrent::mapped(mProjectDirList.begin(), mProjectDirList.end(), examineProjectFolder);
    return mFutureScanResults;
}

QFuture<void> PSandPhotoScanner::startSyncAndInitParallel() {
    mFutureSyncAndInitResults = QtConcurrent::map(mData, syncOrInitializeSession);
    return mFutureSyncAndInitResults;
}

bool greaterThanPSSD(PSSessionData* A, PSSessionData* B) {
    return (A->compareTo(B) > 0);
}

void PSandPhotoScanner::finishScanParallel() {
    for(PSSessionData* lResult : mFutureScanResults) {
        if (lResult != nullptr) {
            mData.append(lResult);
        }
    }

    int length = PSSessionData::getNeedsApproval().length();
    if(length > 0) {
        int lRet;
        bool lApplyToAll = false;
        for(int i = 0; i < length; i++) {
            if(!lApplyToAll) {
                InitializeSessionDialog lInitSession;
                lInitSession.setTitle(PSSessionData::getNeedsApproval()[i]->getSessionFolder().dirName()+" is an uninitialized folder");
                lRet = lInitSession.exec();
                lApplyToAll = lInitSession.getApplyToAll();
            }

            switch (lRet) {
                case QDialogButtonBox::Yes:
//                    PSSessionData::getNeedsApproval()[i]->convertToPSSession();
                    PSSessionData::getNeedsApproval()[i]->setExplicitlyIgnored(false);
                    break;

                case QDialogButtonBox::Ignore:
                    PSSessionData::getNeedsApproval()[i]->setExplicitlyIgnored(true);
                    break;

                default:
                    break;
            }
        }
    }

    // Remove ignored session
    QVector<PSSessionData*> lRemove;
    for(PSSessionData* data : mData) {
        if (data->getExplicitlyIgnored()) {
            lRemove.push_back(data);
        }
    }

    for(PSSessionData* data : lRemove) {
        mData.removeOne(data);
    }
}

void PSandPhotoScanner::finishScanParallelForUninitSessions() {
    for(PSSessionData* lResult : mFutureScanResults) {
        if (lResult != nullptr && !lResult->iniFileExists()) {
            mData.append(lResult);
        }
    }

    int length = PSSessionData::getNeedsApproval().length();
    if(length > 0) {
        int lRet;
        bool lApplyToAll = false;
        for(int i = 0; i < length; i++) {
            if(!lApplyToAll) {
                InitializeSessionDialog lInitSession;
                lInitSession.setTitle(PSSessionData::getNeedsApproval()[i]->getSessionFolder().dirName()+" is an uninitialized folder");
                lRet = lInitSession.exec();
                lApplyToAll = lInitSession.getApplyToAll();
            }

            switch (lRet) {
                case QDialogButtonBox::Yes:
//                    PSSessionData::getNeedsApproval()[i]->convertToPSSession();
                    PSSessionData::getNeedsApproval()[i]->setExplicitlyIgnored(false);
                    break;

                case QDialogButtonBox::Ignore:
                    PSSessionData::getNeedsApproval()[i]->setExplicitlyIgnored(true);
                    break;

                default:
                    break;
            }
        }
    }

    // Remove ignored session
    QVector<PSSessionData*> lRemove;
    for(PSSessionData* data : mData) {
        if (data->getExplicitlyIgnored()) {
            lRemove.push_back(data);
        }
    }

    for(PSSessionData* data : lRemove) {
        mData.removeOne(data);
    }
}

void PSandPhotoScanner::finishSyncAndInitParallel() {

    if (mData.size() > 0) {
        std::sort(mData.begin(), mData.end(), greaterThanPSSD);
    }

    mDataScanned = true;
}
