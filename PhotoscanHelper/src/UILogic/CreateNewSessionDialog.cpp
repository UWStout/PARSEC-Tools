#include "include\CreateNewSessionDialog.h"
#include "ui_CreateNewSessionDialog.h"

#include "PSSessionData.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>

CreateNewSessionDialog::CreateNewSessionDialog(const QString& pCollectionDir, PSProjectDataModel* pDataModel, QWidget *parent)
    : QDialog(parent), mCollectionDir(pCollectionDir), mDataModel(pDataModel) {
    mGUI = new Ui::CreateNewSessionDialog();
    mGUI->setupUi(this);

    mGUI->IDLineEdit->setText(QString::number(PSSessionData::getNextID()));
    mGUI->nameLineEdit->setText("NewSession");
    mGUI->descriptionLineEdit->setText("A new PhotoScan Session");

    updateImageFolderLineEdits();
}

CreateNewSessionDialog::~CreateNewSessionDialog() {
    delete mGUI;
}

QString CreateNewSessionDialog::getSessionFolderName() {
    QString lFolderName = mGUI->IDLineEdit->text() + " " + mGUI->nameLineEdit->text();
    return lFolderName;
}

QString CreateNewSessionDialog::getRawFolderPath() {
    return mGUI->rawFolderLineEdit->text();
}

QString CreateNewSessionDialog::getProcessedFolderPath() {
    return mGUI->processedFolderLineEdit->text();
}

QString CreateNewSessionDialog::getMasksFolderPath() {
    return mGUI->masksFolderLineEdit->text();
}

void CreateNewSessionDialog::on_rawBrowseButton_clicked() {
    QString newDir = QFileDialog::getExistingDirectory(parentWidget(),
                        "Select a collection directory", "");
    if(!newDir.isNull() && !newDir.isEmpty() && newDir.contains(QRegularExpression(mCollectionDir))) {
        mGUI->rawFolderLineEdit->setText(newDir);
    } else {
        QMessageBox lMsg;
        lMsg.setText("You must select a folder within the collection");
        lMsg.exec();
    }
}

void CreateNewSessionDialog::on_processedBrowseButton_clicked() {
    QString newDir = QFileDialog::getExistingDirectory(parentWidget(),
                        "Select a collection directory", "");
    if(!newDir.isNull() && !newDir.isEmpty() && newDir.contains(QRegularExpression(mCollectionDir))) {
        mGUI->processedFolderLineEdit->setText(newDir);
    } else {
        QMessageBox lMsg;
        lMsg.setText("You must select a folder within the collection");
        lMsg.exec();
    }
}

void CreateNewSessionDialog::on_masksBrowseButton_clicked() {
    QString newDir = QFileDialog::getExistingDirectory(parentWidget(),
                        "Select a collection directory", "");
    if(!newDir.isNull() && !newDir.isEmpty() && newDir.contains(QRegularExpression(mCollectionDir))) {
        mGUI->masksFolderLineEdit->setText(newDir);
    } else {
        QMessageBox lMsg;
        lMsg.setText("You must select a folder within the collection.");
        lMsg.exec();
    }
}

void CreateNewSessionDialog::on_IDLineEdit_textEdited() {
    updateImageFolderLineEdits();
}
void CreateNewSessionDialog::on_nameLineEdit_textEdited() {
    updateImageFolderLineEdits();
}

void CreateNewSessionDialog::updateImageFolderLineEdits() {
    if(!mGUI->imageFolderGroupBox->isChecked()) {
        mGUI->rawFolderLineEdit->setText(mCollectionDir + QDir::separator() + getSessionFolderName() + QDir::separator() + "Raw");
        mGUI->processedFolderLineEdit->setText(mCollectionDir + QDir::separator() + getSessionFolderName() + QDir::separator() + "Processed");
        mGUI->masksFolderLineEdit->setText(mCollectionDir + QDir::separator() + getSessionFolderName() + QDir::separator() + "Masks");
    }
}

void CreateNewSessionDialog::accept() {
    if(!getRawFolderPath().contains(QRegularExpression(mCollectionDir))
            || !getProcessedFolderPath().contains(QRegularExpression(mCollectionDir))
            || !getMasksFolderPath().contains(QRegularExpression(mCollectionDir))) {
        QMessageBox lMsg;
        lMsg.setText("All three image folders must exist within the collection.");
        lMsg.exec();
    } else if(mGUI->IDLineEdit->text().isEmpty()
              || !mGUI->IDLineEdit->text().contains(QRegularExpression("^\\d+$"))
              || !mDataModel->isIDUnique(mGUI->IDLineEdit->text().toLongLong())
              || mGUI->nameLineEdit->text().isEmpty()) {
        QMessageBox lMsg;
        lMsg.setText("Your Session's ID and name must contain a valid string.");
        lMsg.exec();
    } else {
        if(!mGUI->masksFolderLineEdit->text().contains(QRegularExpression(getSessionFolderName()))) {
            //TODO: Figure out how to use QFile::link to create a shortcut/symlink
        }

        QDialog::accept();
    }

}
