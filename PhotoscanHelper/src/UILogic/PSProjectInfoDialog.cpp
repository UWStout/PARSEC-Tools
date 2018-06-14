#include "PSChunkData.h"
#include "PSSessionData.h"
#include "PSProjectInfoDialog.h"

PSProjectInfoDialog::PSProjectInfoDialog(PSSessionData pProjData, QWidget *parent) : QDialog(parent) {
    mGUI = new Ui_PSProjectInfoDialog();
    mGUI->setupUi(this);

    setProjectData(pProjData);
}

PSProjectInfoDialog::~PSProjectInfoDialog() {

}

void PSProjectInfoDialog::setProjectData(PSSessionData pProjData) {
    QString mainHeader = (pProjData.getPSProjectFile() == QFileInfo() ? pProjData.getPSProjectFolder().dirName() :
                                                                 pProjData.getPSProjectFile().fileName());
    mGUI->MainHeaderLabel->setText(mainHeader);

    mGUI->IDLabel->setText(pProjData.getID());
    mGUI->PSFileLabel->setText(pProjData.getPSProjectFile() == QFileInfo() ? "N/A" : pProjData.getPSProjectFile().fileName());
    mGUI->PSFolderLabel->setText(pProjData.getPSProjectFolder().path());

    mGUI->DescriptionLabel->setText(pProjData.getNameStrict());
    QString str;
    mGUI->ImageInfoLabel->setText(str.sprintf("%d raw, %d normal, %d depth map",
                                              pProjData.getRawImageCount(), pProjData.getProcessedImageCount(),
                                              pProjData.getDenseCloudDepthImages()));

    if(pProjData.isImageExposureKnown()) {
        const double* lWB = pProjData.getWhiteBalanceMultipliers();
        QString str;
        mGUI->ExposureInfoLabel->setText(str.sprintf("White Balance [%.4f, %.4f, %.4f, %.4f], Brightness %.4f",
                                                     lWB[0], lWB[1], lWB[2], lWB[3], pProjData.getBrightnessMultiplier()));
    } else {
        mGUI->ExposureInfoLabel->setText("N/A");
    }
    mGUI->SpecialNotesLabel->setText(pProjData.getSpecialNotes());

    for(int chunk = 0; chunk < pProjData.getChunkCount(); chunk++)
    {
        PSChunkData* lChunk = pProjData.getChunk(chunk);
        if(lChunk == NULL) continue;

        QWidget* lChunkInfo = new QWidget(this);
        Ui_PSChunkInfo* lChunkGUI = new Ui_PSChunkInfo();
        lChunkGUI->setupUi(lChunkInfo);

        QString label = lChunk->getLabel();
        lChunkGUI->ChunkLabelLabel->setText((label.isNull() || label.isEmpty()) ? "[none]" : label);
        QString str;
        lChunkGUI->ImageInfoLabel->setText(str.sprintf("%d images, %d sensors, %d depth maps",
                                                       lChunk->getImageCount(), lChunk->getSensorCount(),
                                                       lChunk->getDenseCloudDepthImages()));

        lChunkGUI->IADetailLabel->setText(lChunk->getImageAlignment_LevelString());
        lChunkGUI->IAMaskedCheckBox->setChecked(lChunk->getImageAlignment_Masked());
        lChunkGUI->IALimitsLabel->setText(str.sprintf("%dk/%dk",
                                                       lChunk->getImageAlignment_featureLimit()/1000,
                                                       lChunk->getImageAlignment_tiePointLimit()/1000));
        //lChunkGUI->IADurationLabel->setText(lChunk.getImageAlignment_durationString()); // TODO

        lChunkGUI->OptInfoLabel->setText(lChunk->getOptimizeString());

        lChunkGUI->DCDetailLabel->setText(lChunk->getDenseCloud_levelString());
        lChunkGUI->DCFilterLabel->setText(lChunk->getDenseCloud_filterLevelString());
        //lChunkGUI->ModDurationLabel->setText(lChunk.getModelGeneration_durationString()); // TODO

        mGUI->tabWidget->addTab(lChunkInfo, str.sprintf("Chunk %d", chunk+1));
    }
}
