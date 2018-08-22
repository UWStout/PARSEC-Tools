#ifndef PS_AND_PHOTO_SCANNER_H
#define PS_AND_PHOTO_SCANNER_H

#include <QString>
#include <QFileInfo>
#include <QVector>
#include <QFuture>

class PSSessionData;
class QSettings;

class PSandPhotoScanner {
public:
    PSandPhotoScanner(const QString &pPath, int pMaxRecursionDepth);
    const QVector<PSSessionData*> getPSProjectData() const;

    const QString getRootPath() const { return mRootPath.absoluteFilePath(); }

    int countUniqueDirs() const;
    int countDirsWithoutProjects() const;
    int countDirsWithoutImageAlign() const;
    int countDirsWithoutDenseCloud() const;
    int countDirsWithoutModels() const;

    QFuture<PSSessionData*> startScanParallel();
    void finishScanParallel();
    void finishScanParallelForUninitSessions();

    QFuture<void> startSyncAndInitParallel();
    void finishSyncAndInitParallel();

private:
    QFileInfo mRootPath;
    int mMaxRecursionDepth;

    QFuture<PSSessionData*> mFutureScanResults;
    QFuture<void> mFutureSyncAndInitResults;

    bool mDataScanned;
    QVector<PSSessionData*> mData;
    QFileInfoList mProjectDirList;

    bool buildDirectoryList(QFileInfo pRoot);
    static QSettings* initInfoStore(QString pSettingsFile);
};

#endif
