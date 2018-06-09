#ifndef DIRLISTER_H
#define DIRLISTER_H

#include <QDir>

class DirLister
{
public:
    DirLister(QDir pRoot, QStringList pNamefilter);

    const QDir getRoot() const;
    const QFileInfoList getMatches() const;
    int count() const;

private:
    void examineDir(QDir pCur);

    QDir mRoot;
    QStringList mNameFilter;
    QFileInfoList mMatches;
};

#endif // DIRLISTER_H
