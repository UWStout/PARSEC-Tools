#include "include\ScriptedPhase1Dialog.h"
#include "ui_ScriptedPhase1Dialog.h"

#include <QDebug>

#include "PSSessionData.h"

ScriptedPhase1Dialog::ScriptedPhase1Dialog(PSSessionData* pSessionData,
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
                                           QWidget *parent) : QDialog(parent), mGUI(new Ui::ScriptedPhase1Dialog) {
    mGUI->setupUi(this);
    mSessionData = pSessionData;
    mProjectName = mSessionData->getSessionFolder().absolutePath() + QDir::separator() + pProjectName;
    mAccuracy = pAccuracy;
    mGenericPreselection = pGenericPreselection;
    mReferencePreselection = pReferencePreselection;
    mKeyPointLimit = pKeyPointLimit;
    mTiePointLimit = pTiePointLimit;
    mFilterByMasks = pFilterByMasks;
    mTolerance = pTolerance;
    mAdaptiveCamera = pAdaptiveCamera;
    mBuildDenseCloud = pBuildDenseCloud;
    mQuality = pQuality;
    mDepthFiltering = pDepthFiltering;
    mPointColors = pPointColors;

    startPhotoScan();
}

ScriptedPhase1Dialog::~ScriptedPhase1Dialog() {
    mPSProc.close();
    delete mGUI;
}


void ScriptedPhase1Dialog::startPhotoScan() {
    // Command and arguments;
    QString lCommand;
    QStringList lArgs;
    QProcessEnvironment lEnv = QProcessEnvironment::systemEnvironment();

#ifdef Q_OS_WIN32
    // Command to run
    lCommand = "C:/Program Files/Agisoft/PhotoScan Pro/PhotoScan.exe";

    // Setup execution environment
    lEnv.remove("QTDIR");
    lEnv.insert("QT_PLUGIN_PATH", "C:/Program Files/Agisoft/PhotoScan Pro/plugins");
#elif defined(Q_OS_MAC)
    // Command to run
    lCommand = "/Applications/PhotoScan Pro/PhotoScanPro.app/Contents/MacOS/PhotoScanPro";

    // Setup execution environment
    lEnv.remove("DYLD_FRAMEWORK_PATH");
    lEnv.remove("DYLD_LIBRARY_PATH");
    lEnv.remove("QTDIR");

    lEnv.insert("QT_PLUGIN_PATH", "/Applications/PhotoScan Pro/PhotoScanPro.app/Contents/PlugIns");
    mPSProc.setProcessEnvironment(lEnv);
#endif

    // Command arguments
    lArgs << "-r" << "PhotoScanPhase1.py" << mSessionData->getProcessedFolder().absolutePath();
    lArgs << mSessionData->getMasksFolder().absolutePath();
    lArgs << mProjectName;
    lArgs << mAccuracy;
    lArgs << mGenericPreselection;
    lArgs << mReferencePreselection;
    lArgs << mKeyPointLimit;
    lArgs << mTiePointLimit;
    lArgs << mFilterByMasks;
    lArgs << mTolerance;
    lArgs << mAdaptiveCamera;
    lArgs << mBuildDenseCloud;
    lArgs << mQuality;
    lArgs << mDepthFiltering;
    lArgs << mPointColors;

    // Connect signals and slots
    connect(&mPSProc, &QProcess::started, this, &ScriptedPhase1Dialog::onStarted);
    connect(&mPSProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ScriptedPhase1Dialog::onFinished);

    connect(&mPSProc, &QProcess::errorOccurred, this, &ScriptedPhase1Dialog::onErrorOccurred);
    connect(&mPSProc, &QProcess::readyReadStandardOutput, this, &ScriptedPhase1Dialog::inputFromPS);
    connect(&mPSProc, &QProcess::readyReadStandardError, this, &ScriptedPhase1Dialog::inputFromPS);

    // Create and start the other process
    mPSProc.setProcessEnvironment(lEnv);
    mPSProc.start(lCommand, lArgs);
}

void ScriptedPhase1Dialog::inputFromPS() {
    QByteArray stdOut = mPSProc.readAllStandardOutput();
    QByteArray stdErr = mPSProc.readAllStandardError();

    // Debugging
    qDebug("%s", stdOut.data());
    qDebug("%s", stdErr.data());

    if (!stdErr.isEmpty()) {
        // Trim to one line of output
        stdErr = stdErr.trimmed();
        int lLastLineIdx = stdErr.lastIndexOf("\n");
        stdErr = stdErr.right(stdErr.length() - lLastLineIdx - 1);

        // Check for RegEx matches
        QRegularExpressionMatch match = mRegEx.match(stdErr);
        QRegularExpressionMatch match2 = mRegExOverall.match(stdErr);
        if(match.hasMatch()) {
            mGUI->stageProgressBar->setValue(match.captured(2).toFloat());
            mGUI->stageProgressLabel->setText(QString("Stage progress: %1").arg(match.captured(1)));
        } else if(match2.hasMatch()) {
            mGUI->overallProgressBar->setRange(1, match2.captured(2).toInt());
            mGUI->overallProgressBar->setValue(match2.captured(1).toInt());
            mGUI->overallProgressLabel->setText(QString("Overall progress: %1 of %2")
                                                        .arg(match2.captured(1))
                                                        .arg(match2.captured(2)));
        } else {
            if (stdErr.contains("progress") || stdErr.contains("Stage")) {
                qInfo("Line not matched '%s'", stdErr.data());
            }
            mGUI->statusLineEdit->setText(QString(stdErr));
        }
    } else if(!stdOut.isEmpty()) {
        // Trim to one line of output
        stdOut = stdOut.trimmed();
        int lLastLineIdx = stdOut.lastIndexOf("\n");
        stdOut = stdOut.right(stdOut.length() - lLastLineIdx - 1);

        mGUI->statusLineEdit->setText(QString(stdOut));
    }
}

void ScriptedPhase1Dialog::updateStatusLine(QByteArray pData) {
    pData = pData.trimmed();
    int lLastLineIdx = pData.lastIndexOf("\n");
    QByteArray lLastLine = pData.right(pData.length() - lLastLineIdx);
    mGUI->statusLineEdit->setText(QString(lLastLine));
}

void ScriptedPhase1Dialog::onStarted() {
    qInfo() << "Scripted Phase 1 Started";
}

void ScriptedPhase1Dialog::onFinished(int pExitCode, QProcess::ExitStatus pExitStatus) {
    qInfo() << "PhotoScan Finished: exit code '" << pExitCode << "', exit status '" << pExitStatus << "'";

    mGUI->statusLineEdit->setText("[PhotoScan Ended]");
    //mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    mGUI->stageProgressBar->setRange(0, 1);
    mGUI->overallProgressBar->setRange(0, 1);
    mGUI->stageProgressBar->setValue(1);
    mGUI->overallProgressBar->setValue(1);

    mSessionData->updateOutOfSyncSession();
}

void ScriptedPhase1Dialog::onErrorOccurred(QProcess::ProcessError pError) {
    qInfo() << "PhotoScan Error: error code '" << pError << "'";
}
