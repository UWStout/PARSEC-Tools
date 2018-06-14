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
    PSandPhotoScanner(QString pPath, int pMaxRecursionDepth);
    QSettings* getInfoStore() const;
    bool isReady() const;
    const QVector<PSSessionData*> getPSProjectData() const;

    int countUniqueDirs() const;
    int countDirsWithoutProjects() const;
    int countDirsWithoutImageAlign() const;
    int countDirsWithoutDenseCloud() const;
    int countDirsWithoutModels() const;

    QFuture<PSSessionData*> startScanParallel();
    void finishDataParallel();

private:
    QFileInfo mRootPath;
    int mMaxRecursionDepth;
    QSettings* mPSProjectInfoStore;
    QFuture<PSSessionData*> mFutureResults;

    bool mDataScanned;
    QVector<PSSessionData*> mData;
    QFileInfoList mProjectDirList;

    bool buildDirectoryList(QFileInfo pRoot);

//    static const QString[] IGNORE_EXCEPTIONS = { "_Finished", "_TouchUp", "_TouchedUpPleaseReview" };

//    static const ExtensionFileFilter mDirectoryFilter =
//			new ExtensionFileFilter(true);

    static QSettings* initInfoStore(QString pSettingsFile);
};

#endif
