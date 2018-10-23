#include "ScriptedPhotoScanDialog.h"
#include "ui_ScriptedPhotoScanDialog.h"

#include <QDebug>
#include <QPushButton>
#include <QThread>
#include <QRegularExpressionMatch>

#include <PSSessionData.h>

ScriptedPhotoScanDialog::ScriptedPhotoScanDialog(QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::ScriptedPhotoScanDialog();
    mGUI->setupUi(this);
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    mTextureSize = 0;
    mTolerance = 0;
    mSession = nullptr;
    mRegEx = QRegularExpression("^(.*) progress: ([0-9]+\\.[0-9][0-9])%");
    mRegExOverall = QRegularExpression("Stage ([0-9]) of ([0-9])");
}

ScriptedPhotoScanDialog::ScriptedPhotoScanDialog(PSSessionData* pSession, QFileInfo pMaskDir,
                                                 int pTextureSize, int pTolerance, QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::ScriptedPhotoScanDialog();
    mGUI->setupUi(this);
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    mMaskDir = pMaskDir;
    mTextureSize = pTextureSize;
    mTolerance = pTolerance;
    mSession = pSession;
    mRegEx = QRegularExpression("^(.*) progress: ([0-9]+\\.[0-9][0-9])%");
    mRegExOverall = QRegularExpression("Stage ([0-9]) of ([0-9])");
    startPhotoScan();
}

ScriptedPhotoScanDialog::~ScriptedPhotoScanDialog() {
    mPSProc.close();
    delete mGUI;
}

void ScriptedPhotoScanDialog::startPhotoScan() {
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
    lArgs << "-r" << "QuickPreview.py" << mSession->getProcessedFolder().absolutePath();
    lArgs << mMaskDir.filePath();

    (mMaskDir.filePath() == "" ? lArgs << "False" : lArgs << "True");
    lArgs << QString::number(mTolerance);
    (mTextureSize <= 0 ? lArgs << "False" : lArgs << "True");
    lArgs << QString::number(mTextureSize);

    // Connect signals and slots
    connect(&mPSProc, &QProcess::started, this, &ScriptedPhotoScanDialog::onStarted);
    connect(&mPSProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ScriptedPhotoScanDialog::onFinished);

    connect(&mPSProc, &QProcess::errorOccurred, this, &ScriptedPhotoScanDialog::onErrorOccurred);
    connect(&mPSProc, &QProcess::readyReadStandardOutput, this, &ScriptedPhotoScanDialog::inputFromPS);
    connect(&mPSProc, &QProcess::readyReadStandardError, this, &ScriptedPhotoScanDialog::inputFromPS);

    // Create and start the other process
    mPSProc.setProcessEnvironment(lEnv);
    mPSProc.start(lCommand, lArgs);
}

void ScriptedPhotoScanDialog::inputFromPS() {
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
            mGUI->psStatusLineEdit->setText(QString(stdErr));
        }
    } else if(!stdOut.isEmpty()) {
        // Trim to one line of output
        stdOut = stdOut.trimmed();
        int lLastLineIdx = stdOut.lastIndexOf("\n");
        stdOut = stdOut.right(stdOut.length() - lLastLineIdx - 1);

        mGUI->psStatusLineEdit->setText(QString(stdOut));
    }
}

void ScriptedPhotoScanDialog::updateStatusLine(QByteArray pData) {
    pData = pData.trimmed();
    int lLastLineIdx = pData.lastIndexOf("\n");
    QByteArray lLastLine = pData.right(pData.length() - lLastLineIdx);
    mGUI->psStatusLineEdit->setText(QString(lLastLine));
}

void ScriptedPhotoScanDialog::onStarted() {
    qInfo() << "Scripted PhotoScan Started";
}

void ScriptedPhotoScanDialog::onFinished(int pExitCode, QProcess::ExitStatus pExitStatus) {
    qInfo() << "PhotoScan Finished: exit code '" << pExitCode << "', exit status '" << pExitStatus << "'";

    mGUI->psStatusLineEdit->setText("[PhotoScan Ended]");
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    mGUI->stageProgressBar->setRange(0, 1);
    mGUI->overallProgressBar->setRange(0, 1);
    mGUI->stageProgressBar->setValue(1);
    mGUI->overallProgressBar->setValue(1);
}

void ScriptedPhotoScanDialog::onErrorOccurred(QProcess::ProcessError pError) {
    qInfo() << "PhotoScan Error: error code '" << pError << "'";
}
