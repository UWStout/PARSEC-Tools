#ifndef SCRIPTEDPHOTOSCANDIALOG_H
#define SCRIPTEDPHOTOSCANDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QFileInfo>
#include <QRegularExpression>

class PSSessionData;

namespace Ui {
    class ScriptedPhotoScanDialog;
}

class ScriptedPhotoScanDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScriptedPhotoScanDialog(QWidget *parent = nullptr);
    ScriptedPhotoScanDialog(PSSessionData* pSession, QFileInfo pMaskDir = QFileInfo(),
                            int pTextureSize = 0, int pTolerance = 0, QWidget *parent = nullptr);
    ~ScriptedPhotoScanDialog();

private:
    Ui::ScriptedPhotoScanDialog *mGUI;
    QProcess mPSProc;
    PSSessionData* mSession;
    QFileInfo mMaskDir;
    int mTextureSize;
    int mTolerance;
    QRegularExpression mRegEx, mRegExOverall;

    void startPhotoScan();
    void updateStatusLine(QByteArray pData);

protected slots:
    void inputFromPS();
    void onStarted();
    void onFinished(int pExitCode, QProcess::ExitStatus pExitStatus);
    void onErrorOccurred(QProcess::ProcessError pError);
};

#endif // SCRIPTEDPHOTOSCANDIALOG_H
