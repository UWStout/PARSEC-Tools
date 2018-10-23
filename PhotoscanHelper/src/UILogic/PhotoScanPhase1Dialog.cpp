#include "include\PhotoScanPhase1Dialog.h"
#include "ui_PhotoScanPhase1Dialog.h"

#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>

PhotoScanPhase1Dialog::PhotoScanPhase1Dialog(QWidget *parent) : QDialog(parent)
{
    mGUI = new Ui::PhotScanPhase1Dialog;
    mGUI->setupUi(this);

    QStringList accuracyItems = { "Highest", "High", "Medium", "Low", "Lowest" };
    mGUI->accuracyComboBox->addItems(accuracyItems);

    mGUI->applyMasksCheckBox->setChecked(false);

    QStringList qualityItems = { "Ultra High", "High", "Medium", "Low", "Lowest" };
    mGUI->qualityComboBox->addItems(qualityItems);

    QStringList depthItems = { "Disabled", "Mild", "Moderate", "Aggressive" };
    mGUI->depthFilteringComboBox->addItems(depthItems);
}

PhotoScanPhase1Dialog::~PhotoScanPhase1Dialog() {
    delete mGUI;
}

QString PhotoScanPhase1Dialog::getProjectName() {
    return mGUI->projectNameLineEdit->text();
}

QString PhotoScanPhase1Dialog::getAccuracy() {
    switch (mGUI->accuracyComboBox->currentIndex()) {
        case 0:
            return "Highest";
            break;
        case 1:
            return "High";
            break;
        case 2:
            return "Medium";
            break;
        case 3:
            return "Low";
            break;
        case 4:
            return "Lowest";
            break;
        default:
        return "";
            break;
    }
}

QString PhotoScanPhase1Dialog::getGenericPreselection() {
    return mGUI->genericPreselectionCheckBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase1Dialog::getReferencePreselection() {
    return mGUI->referencePreselectionCheckBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase1Dialog::getKeyPointLimit() {
    return mGUI->keyPointLineEdit->text();
}

QString PhotoScanPhase1Dialog::getTiePointLimit() {
    return mGUI->tiePointLineEdit->text();
}

QString PhotoScanPhase1Dialog::getApplyMasks() {
    return mGUI->applyMasksCheckBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase1Dialog::getAdaptiveCameraModel() {
    return mGUI->adaptiveCameraCheckBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase1Dialog::getBuildDenseCloud() {
    return mGUI->denseCloudGroupBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase1Dialog::getQuality() {
    switch (mGUI->qualityComboBox->currentIndex()) {
        case 0:
            return "Ultra High";
            break;
        case 1:
            return "High";
            break;
        case 2:
            return "Medium";
            break;
        case 3:
            return "Low";
            break;
        case 4:
            return "Lowest";
            break;
        default:
            return "";
            break;
    }
}

QString PhotoScanPhase1Dialog::getDepthFiltering() {
    switch (mGUI->depthFilteringComboBox->currentIndex()) {
        case 0:
            return "Disabled";
            break;
        case 1:
            return "Mild";
            break;
        case 2:
            return "Moderate";
            break;
        case 3:
            return "Aggressive";
            break;
        default:
            return "";
            break;
    }
}

QString PhotoScanPhase1Dialog::getPointColors() {
    return mGUI->pointColorsCheckBox->isChecked() ? "True" : "False";
}

void PhotoScanPhase1Dialog::accept() {
    QMessageBox lMsg;
    if(mGUI->projectNameLineEdit->text().isEmpty()) {
        lMsg.setText("Your project name must contain a valid string.");
        lMsg.exec();
    } else if(!mGUI->keyPointLineEdit->text().contains(QRegularExpression("^\\d+$")) ||
              !mGUI->tiePointLineEdit->text().contains(QRegularExpression("^\\d+$"))) {
        lMsg.setText("The number of Key Points and Tie Points must be an integer.");
        lMsg.exec();
    } else {
        QDialog::accept();
    }
}
