#include "ScriptedPhotoScanDialog.h"
#include "ui_ScriptedPhotoScanDialog.h"

#include <QDebug>
#include <QPushButton>
#include <QThread>

#include <PSSessionData.h>

ScriptedPhotoScanDialog::ScriptedPhotoScanDialog(QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::ScriptedPhotoScanDialog();
    mGUI->setupUi(this);
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    mGUI->progressBar->setRange(0, 0);
    mGUI->progressBar_2->setRange(0, 0);

    mTextureSize = 0;
    mSession = NULL;
}

ScriptedPhotoScanDialog::ScriptedPhotoScanDialog(PSSessionData* pSession, QFileInfo pMaskDir,
                                                 int pTextureSize, QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::ScriptedPhotoScanDialog();
    mGUI->setupUi(this);
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    mGUI->progressBar->setRange(0, 0);
    mGUI->progressBar_2->setRange(0, 0);

    mMaskDir = pMaskDir;
    mTextureSize = pTextureSize;
    mSession = pSession;
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
    lArgs << "-r" << "QuickPreview.py" << mSession->getPSProjectFolder().absolutePath();
    lArgs << mMaskDir.filePath();

    (mMaskDir.filePath() == "" ? lArgs << "False" : lArgs << "True");
    (mTextureSize <= 0 ? lArgs << "False" : lArgs << "True");

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

    if (!stdErr.isEmpty()) {
        qWarning() << stdErr;
        updateStatusLine(stdErr);
    } else if(!stdOut.isEmpty()) {
        qInfo() << stdOut;
        updateStatusLine(stdOut);
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
    mGUI->progressBar->setRange(0, 1);
    mGUI->progressBar_2->setRange(0, 1);
    mGUI->progressBar->setValue(1);
    mGUI->progressBar_2->setValue(1);
}

void ScriptedPhotoScanDialog::onErrorOccurred(QProcess::ProcessError pError) {
    qInfo() << "PhotoScan Error: error code '" << pError << "'";
}
