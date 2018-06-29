#include "InteractivePhotoScanDialog.h"
#include "ui_InteractivePhotoScanDialog.h"

#include <QProcess>
#include <QTextStream>

InteractivePhotoScanDialog::InteractivePhotoScanDialog(QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::InteractivePhotoScanDialog();
    mGUI->setupUi(this);
}

InteractivePhotoScanDialog::~InteractivePhotoScanDialog() {
    delete mGUI;
}

void InteractivePhotoScanDialog::startPhotoScan() {
    // Command and arguments;
    QString lCommand;
    QStringList lArgs;

    // Command to run
//    lCommand = "C:/Program Files/Agisoft/PhotoScan Pro/PhotoScan.exe";
    lCommand = "/Applications/PhotoScan Pro/PhotoScanPro.app/Contents/MacOS/PhotoScanPro";

    // Command arguments
    lArgs << "-i";

    // Create and start the other process
    mPSProc = new QProcess(this);
    mPSProc->start(lCommand, lArgs);
    connect(mPSProc, &QProcess::readyReadStandardOutput, this, &InteractivePhotoScanDialog::inputFromPS);
    connect(mPSProc, &QProcess::readyReadStandardError, this, &InteractivePhotoScanDialog::errorFromPS);
}

void InteractivePhotoScanDialog::inputFromPS() {
    mGUI->photoScanPythonConsole->putData(
        mPSProc->readAllStandardOutput());
}

void InteractivePhotoScanDialog::errorFromPS() {
    mGUI->photoScanPythonConsole->putData(
        mPSProc->readAllStandardError());
}
