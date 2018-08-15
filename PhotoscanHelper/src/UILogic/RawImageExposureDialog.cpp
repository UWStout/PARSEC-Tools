#include "RawImageExposureDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QFutureWatcher>
#include <QAbstractButton>
#include <QSettings>
#include <QtConcurrent>

#include "ui_RawImageExposureDialog.h"
#include "ImageProcessor.h"
#include <PSSessionData.h>
#include <ExposureSettings.h>
#include <libraw/libraw.h>

RawImageExposureDialog::RawImageExposureDialog(QWidget *parent) : QDialog(parent) {
    mDefaultSettings = new ExposureSettings(ExposureSettings::DEFAULT_EXPOSURE);

    setUpGUI();

    mPreviewFileWatcher = new QFutureWatcher<QFileInfo>(this);
    connect(mPreviewFileWatcher, &QFutureWatcher<QFileInfo>::finished,
            this, &RawImageExposureDialog::previewReady);
}

RawImageExposureDialog::~RawImageExposureDialog() {
    delete mPreviewFileWatcher;
    qDebug() << "Destructor called";
}

void RawImageExposureDialog::setUpGUI() {
    mProjectData = nullptr;
    mBlockUpdateFromGUI = false;
    mEnqueueMode = false;

    mGUI = new Ui::RawImageExposureDialog();
    mGUI->setupUi(this);
    mGUI->BrightScaleSpinBox->setEnabled(false);

    setWBCustomEnabled(false);
    setEnqueueMode(false);
    applySettings(mDefaultSettings);
}

void RawImageExposureDialog::setEnqueueMode(bool pEnabled) {
    mEnqueueMode = pEnabled;
    if(mEnqueueMode) {
        mGUI->buttonBox->button(QDialogButtonBox::Ok)->setText("Enqueue");
    } else {
        mGUI->buttonBox->button(QDialogButtonBox::Ok)->setText("Expose");
    }
}

void RawImageExposureDialog::setProjectData(PSSessionData* pData) {
    mProjectData = pData;
    try {
        mGUI->DestinationDirLineEdit->setText(mProjectData->getSessionFolder().canonicalPath() + QDir::separator() + "Processed");
    } catch(...) {}
    projectDataChanged();
}

void RawImageExposureDialog::applySettings(const ExposureSettings* pSettings) {
    mBlockUpdateFromGUI = true;
    mGUI->WBModeComboBox->setCurrentIndex((int)pSettings->getWBMode());
    mGUI->RSpinBox->setValue(pSettings->getWBCustom()[0]);
    mGUI->G1SpinBox->setValue(pSettings->getWBCustom()[1]);
    mGUI->BSpinBox->setValue(pSettings->getWBCustom()[2]);
    mGUI->G2SpinBox->setValue(pSettings->getWBCustom()[3]);

    mGUI->BrightModeComboBox->setCurrentIndex((int)pSettings->getBrightMode());
    mGUI->BrightScaleSpinBox->setValue(pSettings->getBrightScale());
    mBlockUpdateFromGUI = false;

    asyncGeneratePreview();
}

void RawImageExposureDialog::on_PreviewButton_clicked() {
    asyncGeneratePreview();
}

void RawImageExposureDialog::on_WBModeComboBox_currentIndexChanged(int pNewIndex) {
    setWBCustomEnabled(pNewIndex == 3);
}

void RawImageExposureDialog::on_BrightModeComboBox_currentIndexChanged(int pNewIndex) {
    mGUI->BrightScaleSpinBox->setEnabled(pNewIndex == 2);
}

void RawImageExposureDialog::on_DestinationBrowseButton_clicked() {
    QString lDestination = QFileDialog::getExistingDirectory(this, "Select Destination",
                                                             mProjectData->getSessionFolder().path());
    if(!lDestination.isNull() && !lDestination.isEmpty()) {
        mGUI->DestinationDirLineEdit->setText(lDestination);
    }
}

void RawImageExposureDialog::on_buttonBox_clicked(QAbstractButton* pButton) {
    switch(mGUI->buttonBox->standardButton(pButton)) {
        case QDialogButtonBox::RestoreDefaults:
            applySettings(mDefaultSettings);
            break;

        case QDialogButtonBox::Ok: {
            int lResult = QMessageBox::Cancel;
            if(mEnqueueMode) {
                lResult = QMessageBox::question(this, "Add to queue?", "Add to processing queue?", QMessageBox::Cancel | QMessageBox::Yes);
            } else {
                lResult = QMessageBox::question(this, "Start Processing?",
                            "Begin processing raw images?\n\n(Note: Processing may take a long time and use significant resources)",
                            QMessageBox::Cancel | QMessageBox::Yes);
            }

            if(lResult == QMessageBox::Yes) {
                qDebug("Writing exposure settings for session %s", mProjectData->getName().toLocal8Bit().data());
                mProjectData->writeExposureSettings(*getExposureSettings());
                this->accept();
            }
        } break;

        case QDialogButtonBox::Cancel: this->reject(); break;

        default: break;
    }
}

void RawImageExposureDialog::setWBCustomEnabled(bool pEnable) {
    mGUI->RSpinBox->setEnabled(pEnable);
    mGUI->G1SpinBox->setEnabled(pEnable);
    mGUI->BSpinBox->setEnabled(pEnable);
    mGUI->G2SpinBox->setEnabled(pEnable);
}

QFileInfo RawImageExposureDialog::getDestinationPath() const {
    return QFileInfo(mGUI->DestinationDirLineEdit->text());
}

void RawImageExposureDialog::updateFromGUI() {
    if(isVisible() && !mBlockUpdateFromGUI) {
        asyncGeneratePreview();
    }
}

// TODO: Update to use libraw exposure
void RawImageExposureDialog::asyncGeneratePreview() {
    if(mProjectData == nullptr) return;

    qInfo() << "Generating preview.";
    mGUI->PreviewButton->setEnabled(false);
    mGUI->PreviewButton->setText("Wait...");
    ExposureSettings lSettings(*getExposureSettings());

    int lIndex = mGUI->PreviewImageComboBox->currentIndex();
    if(lIndex >= 0 && lIndex < mProjectData->getRawFileList().length()) {
        QFileInfo lRawFile = mProjectData->getRawFileList()[lIndex];

        try {
            QFuture<QFileInfo> lPreviewResult = QtConcurrent::run(&ImageProcessor::developRawImage, lRawFile, lSettings, true);
            mPreviewFileWatcher->setFuture(lPreviewResult);
        } catch(std::exception e) {
            qWarning("Exception occured: %s", e.what());
        }
    }

//    qInfo() << "Generating preview.";
//    mGUI->PreviewButton->setEnabled(false);
//    mGUI->PreviewButton->setText("Wait...");
//    ExposureSettings lSettings(*getExposureSettings());

//    int lIndex = mGUI->PreviewImageComboBox->currentIndex();
//    if(lIndex >= 0 && lIndex < mProjectData->getRawFileList().length()) {
//        QFileInfo lRawFile = mProjectData->getRawFileList()[lIndex];

//        try {
////            Method lMeth = ImageProcessorIM4J.class.getMethod("developRawImage",
////                                        File.class, ExposureSettings.class, boolean.class);

////            QFuture<Object> lPreviewResult = QtConcurrent.run(this, lMeth, lRawFile, lSettings, true);
////            mPreviewFileWatcher.setFuture(lPreviewResult);
//        } catch(std::exception e) {
//            qWarning("Exception occured: %s", e.what());
//        }
//    }
}

// TODO: Update to use results of libraw exposure
void RawImageExposureDialog::previewReady() {
    mGUI->PreviewButton->setText("Preview");
    mGUI->PreviewButton->setEnabled(true);
    QFileInfo lResult = mPreviewFileWatcher->result();
    //Java instanceof used here to ensure lResult was only a File
    if(lResult.exists() && lResult.fileName() != "") {
        // TODO: Update Multipliers
//        mBlockUpdateFromGUI = true;
//        mGUI->RSpinBox->setValue(ImageProcessor::mMultipliers[0]);
//        mGUI->G1SpinBox->setValue(ImageProcessor::mMultipliers[1]);
//        mGUI->BSpinBox->setValue(ImageProcessor::mMultipliers[2]);
//        mGUI->G2SpinBox->setValue(ImageProcessor::mMultipliers[3]);
//        mBlockUpdateFromGUI = false;

        // Put in label
        mPreviewImage = QPixmap::fromImage(QImage(lResult.filePath()));
        QFile::remove(lResult.filePath());
        updatePreviewImage();
    } else {
        qWarning() << "\tPreview output is null or does not exist";
    }
}

void RawImageExposureDialog::projectDataChanged() {
    mDefaultSettings = new ExposureSettings(mProjectData->readExposureSettings());
    applySettings(mDefaultSettings);

    mGUI->PreviewImageComboBox->clear();

    QFileInfoList lRawFiles = mProjectData->getRawFileList();
    for(const QFileInfo lRawFile : lRawFiles) {
        mGUI->PreviewImageComboBox->addItem(lRawFile.baseName());
    }

    asyncGeneratePreview();
}

void RawImageExposureDialog::updatePreviewImage() {
    if(mPreviewImage.isNull()) {
        qDebug() << "Image is null";
        return;
    }
    // get label dimensions
    int w = mGUI->ImagePreviewLabel->width();
    int h = mGUI->ImagePreviewLabel->height();
    mGUI->ImagePreviewLabel->setPixmap(mPreviewImage.scaled(w, h, Qt::KeepAspectRatio));
}

void RawImageExposureDialog::resizeEvent(QResizeEvent* e) {
    (void)e;
    updatePreviewImage();
}

ExposureSettings* RawImageExposureDialog::getExposureSettings() const {
    double lCustomWB[] = { mGUI->RSpinBox->value(), mGUI->G1SpinBox->value(),
                           mGUI->BSpinBox->value(), mGUI->G2SpinBox->value() };
    ExposureSettings::WhiteBalanceMode lWBMode =
        (ExposureSettings::WhiteBalanceMode)mGUI->WBModeComboBox->currentIndex();
    ExposureSettings::BrightnessMode lBrightMode =
        (ExposureSettings::BrightnessMode)mGUI->BrightModeComboBox->currentIndex();

    ExposureSettings* lSettings = new ExposureSettings(
                lWBMode, lCustomWB, lBrightMode, mGUI->BrightScaleSpinBox->value());
    return lSettings;
}
