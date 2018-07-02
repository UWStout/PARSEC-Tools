#include "InteractivePhotoScanDialog.h"
#include "ui_InteractivePhotoScanDialog.h"

#include <QDebug>
#include <QTextStream>
#include <QPushButton>
#include <QThread>

InteractivePhotoScanDialog::InteractivePhotoScanDialog(QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::InteractivePhotoScanDialog();
    mGUI->setupUi(this);
    mGUI->photoScanPythonConsole->setLocalEchoEnabled(true);
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    mIgnoreStdErr = 0;
    startPhotoScan();
}

InteractivePhotoScanDialog::InteractivePhotoScanDialog(const QFileInfo &pProjectFile, QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::InteractivePhotoScanDialog();
    mGUI->setupUi(this);
    mGUI->photoScanPythonConsole->setLocalEchoEnabled(true);
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    mIgnoreStdErr = 0;
    startPhotoScan(QFileInfo(pProjectFile));
}

InteractivePhotoScanDialog::~InteractivePhotoScanDialog() {
    mPSProc.close();
    delete mGUI;
}

void InteractivePhotoScanDialog::startPhotoScan(QFileInfo pProjectFile) {
    // Initialize members
    mActiveProjctFile = pProjectFile;

    // Command and arguments;
    QString lCommand;
    QStringList lArgs;
    QProcessEnvironment lEnv = QProcessEnvironment::systemEnvironment();

#ifdef Q_OS_WIN32
    // Command to run
    lCommand = "C:/Program Files/Agisoft/PhotoScan Pro/PhotoScan.exe";

    // Command arguments
    lArgs << "-i";

    // Setup execution environment
    lEnv.remove("QTDIR");
    lEnv.insert("QT_PLUGIN_PATH", "C:/Program Files/Agisoft/PhotoScan Pro/plugins");
#elif defined(Q_OS_MAC)
    // Command to run
    lCommand = "/Applications/PhotoScan Pro/PhotoScanPro.app/Contents/MacOS/PhotoScanPro";

    // Command arguments
    lArgs << "-i";

    // Setup execution environment
    lEnv.remove("DYLD_FRAMEWORK_PATH");
    lEnv.remove("DYLD_LIBRARY_PATH");
    lEnv.remove("QTDIR");

    lEnv.insert("QT_PLUGIN_PATH", "/Applications/PhotoScan Pro/PhotoScanPro.app/Contents/PlugIns");
    mPSProc.setProcessEnvironment(lEnv);
#endif

    // Create and start the other process
    mPSProc.setProcessEnvironment(lEnv);
    mPSProc.start(lCommand, lArgs);
    connect(&mPSProc, &QProcess::started, this, &InteractivePhotoScanDialog::onStarted);
    connect(&mPSProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &InteractivePhotoScanDialog::onFinished);

    connect(&mPSProc, &QProcess::errorOccurred, this, &InteractivePhotoScanDialog::onErrorOccurred);

    connect(&mPSProc, &QProcess::readyReadStandardOutput, this, &InteractivePhotoScanDialog::inputFromPS);
    connect(&mPSProc, &QProcess::readyReadStandardError, this, &InteractivePhotoScanDialog::inputFromPS);
    connect(mGUI->photoScanPythonConsole, &QtConsole::getData, this, &InteractivePhotoScanDialog::runCommand);
}

void InteractivePhotoScanDialog::inputFromPS() {
    mGUI->photoScanPythonConsole->putData(mPSProc.readAllStandardOutput());

    QByteArray stdErr = mPSProc.readAllStandardError();
    if (!stdErr.isEmpty()) {
        if (mIgnoreStdErr <= 0) {
            mGUI->photoScanPythonConsole->putSecondaryData(stdErr);
        } else {
            mIgnoreStdErr--;
        }
    }
}

void InteractivePhotoScanDialog::runCommand(const QByteArray &data) {
    mPSProc.write(data);
    mPSProc.write("\n");
}

void InteractivePhotoScanDialog::onStarted() {
    qInfo() << "Interactive PhotoScan Started";
    if (mActiveProjctFile.filePath() != "") {
        runCommand("doc = PhotoScan.app.document");
        QString lOpenCMD = QString::asprintf("doc.open('%s')", mActiveProjctFile.filePath().toLocal8Bit().data());
        runCommand(lOpenCMD.toLocal8Bit());
        mIgnoreStdErr++;
    }
}

void InteractivePhotoScanDialog::onFinished(int pExitCode, QProcess::ExitStatus pExitStatus) {
    qInfo() << "PhotoScan Finished: exit code '" << pExitCode << "', exit status '" << pExitStatus << "'";
    mGUI->photoScanPythonConsole->putData("\n[Process Ended]");
    mGUI->photoScanPythonConsole->setEnabled(false);
    mGUI->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void InteractivePhotoScanDialog::onErrorOccurred(QProcess::ProcessError pError) {
    qInfo() << "PhotoScan Error: error code '" << pError << "'";
}
