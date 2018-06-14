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
        mMatches.append(pCur.entryInfoList(QStringList(), QDir::Dirs));
    } else {
        mMatches.append(pCur.entryInfoList(mNameFilter, QDir::Files));
    }

    // Recurse into any directories
    if (mMaxDepth >= 0 && pDepth < mMaxDepth) {
        QFileInfoList lDirs = pCur.entryInfoList(QDir::Dirs | QDir::NoSymLinks);
        for (QFileInfo lNextDir : lDirs) {
            examineDir(lNextDir.absoluteDir(), pDepth+1);
        }
    }
}
