#ifndef SCRIPTEDPHASE1DIALOG_H
#define SCRIPTEDPHASE1DIALOG_H

#include <QDialog>
#include <QProcess>
#include <QRegularExpression>

class PSSessionData;

namespace Ui {
class ScriptedPhase1Dialog;
}

class ScriptedPhase1Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptedPhase1Dialog(PSSessionData* pSessionData,
                                  QString pProjectName,
                                  QString pAccuracy,
                                  QString pGenericPreselection,
                                  QString pReferencePreselection,
                                  QString pKeyPointLimit,
                                  QString pTiePointLimit,
                                  QString pFilterByMasks,
                                  QString pTolerance,
                                  QString pAdaptiveCamera,
                                  QString pBuildDenseCloud,
                                  QString pQuality,
                                  QString pDepthFiltering,
                                  QString pPointColors,
                                  QWidget *parent = 0);
    ~ScriptedPhase1Dialog();

private:
    Ui::ScriptedPhase1Dialog *mGUI;
    QProcess mPSProc;
    PSSessionData* mSessionData;
    QString mProjectName;
    QString mAccuracy;
    QString mGenericPreselection;
    QString mReferencePreselection;
    QString mKeyPointLimit;
    QString mTiePointLimit;
    QString mFilterByMasks;
    QString mTolerance;
    QString mAdaptiveCamera;
    QString mBuildDenseCloud;
    QString mQuality;
    QString mDepthFiltering;
    QString mPointColors;

    QRegularExpression mRegEx, mRegExOverall;

    void startPhotoScan();
    void inputFromPS();
    void updateStatusLine(QByteArray pData);
    void onStarted();
    void onFinished(int pExitCode, QProcess::ExitStatus pExitStatus);
    void onErrorOccurred(QProcess::ProcessError pError);


};

#endif // SCRIPTEDPHASE1DIALOG_H
