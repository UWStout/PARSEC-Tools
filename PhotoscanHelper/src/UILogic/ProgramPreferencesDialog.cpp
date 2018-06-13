#include <QFileInfo>

#include "ProgramPreferencesDialog.h"

ProgramPreferencesDialog::ProgramPreferencesDialog(QSettings* infoStore, QWidget* parent) : QDialog(parent) {
    prefsUI = new Ui_ProgramPreferencesDialog();
    prefsUI->setupUi(this);

    QString searchPath = ImageProcessorIM4J::getSearchPath();
    QString exifPath = ImageProcessorIM4J::getExiftoolOverrideBin();
    QString dcrawPath = ImageProcessorIM4J::getDcrawOverrideBin();
    QString IMPath = ImageProcessorIM4J::getImageMagickOverrideBin();
    bool useGM = ImageProcessorIM4J::getUseGraphicsMagick();

    prefsUI->im4jLineEdit->setText(searchPath);

    if(!exifPath.isNull()) {
        prefsUI->exiftoolPathEdit->setText(exifPath);
        prefsUI->exiftoolOverrideCheckBox->setChecked(true);
    } else {
        prefsUI->exiftoolOverrideCheckBox->setChecked(false);
    }

    if(!dcrawPath.isNull()) {
        prefsUI->dcrawPathEdit->setText(dcrawPath);
        prefsUI->dcrawOverrideCheckBox->setChecked(true);
    } else {
        prefsUI->dcrawOverrideCheckBox->setChecked(false);
    }

    if(!IMPath.isNull()) {
        prefsUI->imageMagickPathEdit->setText(IMPath);
        prefsUI->imageMagickOverrideCheckBox->setChecked(true);
    } else {
        prefsUI->imageMagickOverrideCheckBox->setChecked(false);
    }

    prefsUI->useGMCheckBox->setChecked(useGM);

    if(infoStore != NULL) {
        infoStore->beginGroup("Collection");
        QString colName = infoStore->value("Name", "none").toString();
        QString ownerName = infoStore->value("Owner", "none").toString();
        infoStore->endGroup();

        prefsUI->ownerNameLineEdit->setText(ownerName);
        prefsUI->collectionNameLineEdit->setText(colName);
    } else {
        prefsUI->tabWidget->removeTab(1);
    }
}

ProgramPreferencesDialog::~ProgramPreferencesDialog() {

}

void ProgramPreferencesDialog::on_buttonBox_clicked(QAbstractButton* pButton) {
    switch(prefsUI->buttonBox->standardButton(pButton)) {

        case QDialogButtonBox::Save: {
            QMessageBox::StandardButton lResult = QMessageBox::Yes;
            if(!validateSearchPath()) {
                QMessageBox::StandardButtons lButtons = QMessageBox::Yes | QMessageBox::Cancel;
                lResult = QMessageBox::question(this,
                                               "Invalid Path",
                                               "The search path appears to be invalid. Are you sure you want to save?",
                                               lButtons);
            }

            if(lResult == QMessageBox::Yes) {
                this->accept();
            }
        } break;

        case QDialogButtonBox::Cancel:
            this->reject();
        break;

        default: break;
    }
}

void ProgramPreferencesDialog::writeResults(QSettings* infoStore) {
    if(infoStore != NULL) {
        infoStore->beginGroup("Collection");
        infoStore->setValue("Name", prefsUI->collectionNameLineEdit->text());
        infoStore->setValue("Owner", prefsUI->ownerNameLineEdit->text());
        infoStore->endGroup();
    }

    updateIM4JPathSettings();
}

bool ProgramPreferencesDialog::validateSearchPath() {
    // Save current settings
    QString oldSearchPath = ImageProcessorIM4J::getSearchPath();
    QString oldExifPath = ImageProcessorIM4J::getExiftoolOverrideBin();
    QString oldDcrawPath = ImageProcessorIM4J::getDcrawOverrideBin();
    QString oldIMPath = ImageProcessorIM4J::getImageMagickOverrideBin();
    bool oldUseGM = ImageProcessorIM4J::getUseGraphicsMagick();

    // Load new settings for testing purposes
    updateIM4JPathSettings();

    // Test the new settings
    bool results = ImageProcessorIM4J::locatePrograms();

    // Restore old settings and return the results of the test
    ImageProcessorIM4J::setSearchPath(oldSearchPath);
    ImageProcessorIM4J::setExiftoolOverrideBin(oldExifPath);
    ImageProcessorIM4J::setDcrawOverrideBin(oldDcrawPath);
    ImageProcessorIM4J::setImageMagickOverrideBin(oldIMPath);
    ImageProcessorIM4J::setUseGraphicsMagick(oldUseGM);

    return results;
}

void ProgramPreferencesDialog::updateIM4JPathSettings() {
    ImageProcessorIM4J::setSearchPath(prefsUI->im4jLineEdit->text());
    if(prefsUI->exiftoolOverrideCheckBox->isChecked()) {
        ImageProcessorIM4J::setExiftoolOverrideBin(prefsUI->exiftoolPathEdit->text());
    } else {
        ImageProcessorIM4J::setExiftoolOverrideBin(NULL);
    }

    if(prefsUI->dcrawOverrideCheckBox->isChecked()) {
        ImageProcessorIM4J::setDcrawOverrideBin(prefsUI->dcrawPathEdit->text());
    } else {
        ImageProcessorIM4J::setDcrawOverrideBin(NULL);
    }

    if(prefsUI->imageMagickOverrideCheckBox->isChecked()) {
        ImageProcessorIM4J::setImageMagickOverrideBin(prefsUI->imageMagickPathEdit->text());
    } else {
        ImageProcessorIM4J::setImageMagickOverrideBin(NULL);
    }

    ImageProcessorIM4J::setUseGraphicsMagick(prefsUI->useGMCheckBox->isChecked());
}

void ProgramPreferencesDialog::on_exiftoolBrowseButton_clicked() {
    QString newBin = pickExecutableBinary("Select the executable for exiftool");
    if(!newBin.isNull() && !newBin.isEmpty()) { prefsUI->exiftoolPathEdit->setText(newBin); }
}

QString ProgramPreferencesDialog::pickExecutableBinary(QString caption) {
    QString startDir = ImageProcessorIM4J::getSearchPath();
    char splitChar = ':';
    if(startDir.indexOf(";") != -1) { splitChar = ';'; }
    startDir = startDir.left(startDir.indexOf(splitChar));

    QString newBin = QFileDialog::getOpenFileName(this, caption, startDir);
    if(!newBin.isNull() && !newBin.isEmpty()) {
        QFileInfo filePath(newBin);
        QString nameOnly = filePath.fileName();
        return nameOnly;
    }

    return NULL;
}
