#ifndef DIR_LISTER_H
#define DIR_LISTER_H

#include "psdata_global.h"

#include <QDir>

class PSDATASHARED_EXPORT DirLister {
public:
    DirLister(QDir pRoot, QStringList pNamefilter, int pMaxDepth = -1, bool pDirsOnly = false);

    const QDir getRoot() const;
    const QFileInfoList getMatches() const;
    int count() const;

private:
    void examineDir(QDir pCur, int pDepth = 0);

    int mMaxDepth;
    QDir mRoot;
    QStringList mNameFilter;
    QFileInfoList mMatches;
    bool mDirsOnly;
};

#endif // DIRLISTER_H
