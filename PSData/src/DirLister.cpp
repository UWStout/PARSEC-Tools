#include "include/DirLister.h"

DirLister::DirLister(QDir pRoot, QStringList pNameFilter) {
    mRoot = pRoot;
    mNameFilter = pNameFilter;
    examineDir(pRoot);
}

const QDir DirLister::getRoot() const { return mRoot; }
const QFileInfoList DirLister::getMatches() const { return mMatches; }
int DirLister::count() const { return mMatches.length(); }

void DirLister::examineDir(QDir pCur) {
    // Find matchs in the current directory
    mMatches.append(pCur.entryInfoList(mNameFilter, QDir::Files));

    // Recurse into any directories
    QFileInfoList lDirs = pCur.entryInfoList(QDir::Dirs | QDir::NoSymLinks);
    for (QFileInfo lNextDir : lDirs) {
        examineDir(lNextDir.absoluteDir());
    }
}
