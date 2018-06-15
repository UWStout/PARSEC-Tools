#include "include/DirLister.h"

DirLister::DirLister(QDir pRoot, QStringList pNameFilter, int pMaxDepth, bool pDirsOnly) {
    mRoot = pRoot;
    mNameFilter = pNameFilter;
    mMaxDepth = pMaxDepth;
    mDirsOnly = pDirsOnly;
    examineDir(pRoot);
}

const QDir DirLister::getRoot() const { return mRoot; }
const QFileInfoList DirLister::getMatches() const { return mMatches; }
int DirLister::count() const { return mMatches.length(); }

void DirLister::examineDir(QDir pCur, int pDepth) {
    // Find matchs in the current directory
    if (mDirsOnly) {
        mMatches.append(pCur.entryInfoList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot));
    } else {
        mMatches.append(pCur.entryInfoList(mNameFilter, QDir::Files));
    }

    // Recurse into any directories
    if (pDepth < mMaxDepth || mMaxDepth < 0) {
        QFileInfoList lDirs = pCur.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (QFileInfo lNextDir : lDirs) {
            examineDir(QDir(lNextDir.filePath()), pDepth+1);
        }
    }
}
