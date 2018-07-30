#include "GeneralSettingsDialog.h"

#include <QDialogButtonBox>

GeneralSettingsDialog::GeneralSettingsDialog(PSSessionData* pData, QWidget* parent) : QDialog(parent), mProjectData(pData) {
    mGUI = new Ui_GeneralSettingsDialog();
    mGUI->setupUi(this);

    restoreSettings();
}

GeneralSettingsDialog::~GeneralSettingsDialog() {

}

void GeneralSettingsDialog::restoreSettings() {
    QString mainHeader = (mProjectData->getPSProjectFile()== QFileInfo() ? mProjectData->getSessionFolder().dirName() :
                                                                          mProjectData->getPSProjectFile().fileName());
    mGUI->mainHeadingLabel->setText(mainHeader);

    int status = (int)mProjectData->getStatus() - (int)PSSessionData::PSS_TEXTURE_GEN_DONE;
    if(status > 0) {
        mGUI->statusComboBox->setCurrentIndex(status);
    }

    if(mProjectData->getID() == "-1") {
        QString str;
        mGUI->IDLineEdit->setText(str.sprintf("%05d", PSSessionData::getNextID()));
    } else {
        mGUI->IDLineEdit->setText(mProjectData->getID());
    }

    mGUI->DescriptionLineEdit->setText(mProjectData->getName());
    mGUI->SpecialNotesTextEdit->setPlainText(mProjectData->getNotes().join("; "));
}

void GeneralSettingsDialog::on_buttonBox_clicked(QAbstractButton* pButton) {
    int statusIndex = mGUI->statusComboBox->currentIndex();
    switch(mGUI->buttonBox->standardButton(pButton)) {
        case QDialogButtonBox::RestoreDefaults:
            restoreSettings();
        break;

        case QDialogButtonBox::Ok:
            mProjectData->setCustomStatus(statusIndex);

            mProjectData->setID(mGUI->IDLineEdit->text());
            mProjectData->setName(mGUI->DescriptionLineEdit->text());
            mProjectData->addNotes(mGUI->SpecialNotesTextEdit->toPlainText());
        break;

        case QDialogButtonBox::Cancel:
            this->reject();
        break;

        default: break;
    }
}
