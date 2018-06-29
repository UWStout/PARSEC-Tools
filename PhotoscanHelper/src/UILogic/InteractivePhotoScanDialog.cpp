#include "InteractivePhotoScanDialog.h"
#include "ui_InteractivePhotoScanDialog.h"

#include <QDebug>
#include <QTextStream>

InteractivePhotoScanDialog::InteractivePhotoScanDialog(QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::InteractivePhotoScanDialog();
    mGUI->setupUi(this);
    mGUI->photoScanPythonConsole->setLocalEchoEnabled(true);
    startPhotoScan();
}

InteractivePhotoScanDialog::~InteractivePhotoScanDialog() {
    mPSProc.close();
    delete mGUI;
}

void InteractivePhotoScanDialog::startPhotoScan() {
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
//    mPSProc.setWorkingDirectory("C:/Program Files/Agisoft/PhotoScan Pro/");
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
//    mPSProc.setWorkingDirectory("/Applications/PhotoScan Pro/PhotoScanPro.app/Contents/MacOS/");
#endif

    for(auto val: lEnv.toStringList()) {
        qInfo() << "===============================";
        qInfo() << val;
    }
    qInfo() << "===============================";

    // Create and start the other process
    mPSProc.setProcessEnvironment(lEnv);
    mPSProc.start(lCommand, lArgs);
    connect(&mPSProc, &QProcess::started, this, &InteractivePhotoScanDialog::onStarted);
    connect(&mPSProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &InteractivePhotoScanDialog::onFinished);
    connect(&mPSProc, &QProcess::errorOccurred, this, &InteractivePhotoScanDialog::onErrorOccurred);

    connect(&mPSProc, &QProcess::readyReadStandardOutput, this, &InteractivePhotoScanDialog::inputFromPS);
    connect(&mPSProc, &QProcess::readyReadStandardError, this, &InteractivePhotoScanDialog::errorFromPS);
    connect(mGUI->photoScanPythonConsole, &QtConsole::getData, this, &InteractivePhotoScanDialog::onSendOutput);
}

void InteractivePhotoScanDialog::inputFromPS() {
//    qInfo() << "Standard output received from PhotoScan" << endl;
    mGUI->photoScanPythonConsole->putData(
        mPSProc.readAllStandardOutput());
}

void InteractivePhotoScanDialog::errorFromPS() {
//    qInfo() << "Standard error received from PhotoScan" << endl;
    mGUI->photoScanPythonConsole->putData(
        mPSProc.readAllStandardError());
}

void InteractivePhotoScanDialog::onSendOutput(const QByteArray &data) {
    mPSProc.write(data);
}

void InteractivePhotoScanDialog::onStarted() {
    qInfo() << "Interactive PhotoScan Started" << endl;
    mGUI->photoScanPythonConsole->putData("");
}

void InteractivePhotoScanDialog::onFinished(int pExitCode, QProcess::ExitStatus pExitStatus) {
    qInfo() << "PhotoScan Finished: exit code '" << pExitCode << "', exit status '" << pExitStatus << "'" << endl;
}

void InteractivePhotoScanDialog::onErrorOccurred(QProcess::ProcessError pError) {
    qInfo() << "PhotoScan Error: error code '" << pError << "'" << endl;
}
