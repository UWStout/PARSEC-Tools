#include "include\PhotoScanPhase2Dialog.h"
#include "ui_PhotoScanPhase2Dialog.h"

PhotoScanPhase2Dialog::PhotoScanPhase2Dialog(QWidget *parent) :
    QDialog(parent),
    mGUI(new Ui::PhotoScanPhase2Dialog)
{
    mGUI->setupUi(this);

    QStringList surfaceItems = { "Arbitrary (3D)", "Height field (2.5D)" };
    mGUI->surfaceTypeComboBox->addItems(surfaceItems);

    QStringList sourceItems = { "Sparse cloud", "Dense cloud" };
    mGUI->sourceDataComboBox->addItems(sourceItems);
    mGUI->sourceDataComboBox->setCurrentIndex(1);

    // How to get 'Custom' to become editable for the user
    QStringList faceItems = { "High", "Medium", "Low", "Custom" };
    mGUI->faceCountComboBox->addItems(faceItems);

    QStringList interpolationItems = { "Disbaled", "Enabled (default)", "Extrapolated" };
    mGUI->interpolationComboBox->addItems(interpolationItems);
    mGUI->interpolationComboBox->setCurrentIndex(1);

    QStringList blendingItems = { "Mosaic (default)", "Average", "Max intensity", "Min intensity", "Disabled" };
    mGUI->blendingModeComboBox->addItems(blendingItems);

    mGUI->textureSizeLineEdit->setText("4096");
    
    
}

PhotoScanPhase2Dialog::~PhotoScanPhase2Dialog()
{
    delete mGUI;
}

QString PhotoScanPhase2Dialog::getSurfaceType() {
    switch (mGUI->surfaceTypeComboBox->currentIndex()) {
        case 0:
            return "Arbitrary";
            break;
        case 1:
            return "HeightField";
            break;
        default:
            break;
    }
}

QString PhotoScanPhase2Dialog::getSourceData() {
    switch (mGUI->sourceDataComboBox->currentIndex()) {
        case 0:
            return "PointCloudData";
            break;
        case 1:
            return "DenseCloudData";
            break;
        default:
            break;
    }
}

QString PhotoScanPhase2Dialog::getFaceCount() {
    switch (mGUI->faceCountComboBox->currentIndex()) {
        case 0:
            return "HighFaceCount";
            break;
        case 1:
            return "MediumFaceCount";
            break;
        case 2:
            return "LowFaceCount";
            break;
        case 3:
            return mGUI->faceCountComboBox->currentText();
            break;
        default:
            break;
    }
}

QString PhotoScanPhase2Dialog::getInterpolation() {
    switch (mGUI->interpolationComboBox->currentIndex()) {
        case 0:
            return "DisabledInterpolation";
            break;
        case 1:
            return "EnabledInterpolation";
            break;
        case 2:
            return "Extrapolated";
            break;
        default:
            break;
    }
}

QString PhotoScanPhase2Dialog::getVertexColors() {
    return mGUI->vertexColorsCheckBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase2Dialog::getBuildTexture() {
    return mGUI->buildTextureGroupBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase2Dialog::getBlendingMode() {
    switch(mGUI->blendingModeComboBox->currentIndex()) {
        case 0:
            return "MosaicBlending";
            break;
        case 1:
            return "AverageBlending";
            break;
        case 2:
            return "MaxBlending";
            break;
        case 3:
            return "MinBlending";
            break;
        case 4:
            return "DisabledBlending";
            break;
        default:
            return "N/A";
            break;
    }
}

QString PhotoScanPhase2Dialog::getTextureSize() {
    return mGUI->textureSizeLineEdit->text();
}

QString PhotoScanPhase2Dialog::getHoleFilling() {
    return mGUI->holeFillingCheckBox->isChecked() ? "True" : "False";
}

QString PhotoScanPhase2Dialog::getGhostingFilter() {
    return mGUI->ghostingFilterCheckBox->isChecked() ? "True" : "False";
}
