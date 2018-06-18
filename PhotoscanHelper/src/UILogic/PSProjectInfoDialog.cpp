#include "PSChunkData.h"
#include "PSSessionData.h"

#include "PSProjectInfoDialog.h"
#include <QString>

#include "ui_PSChunkInfo.h"
#include "ui_PSProjectInfo.h"

PSProjectInfoDialog::PSProjectInfoDialog(const PSSessionData* pProjData, QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::PSProjectInfoDialog();
    mGUI->setupUi(this);

    setProjectData(pProjData);
}

PSProjectInfoDialog::~PSProjectInfoDialog() {

}

void PSProjectInfoDialog::setProjectData(const PSSessionData* pProjData) {
    QString mainHeader;
    if (pProjData->getPSProjectFile().filePath() == "" ) {
        mainHeader = pProjData->getPSProjectFolder().dirName();
    } else {
        mainHeader = pProjData->getPSProjectFile().fileName();
    }

    mGUI->MainHeaderLabel->setText(mainHeader);

    mGUI->IDLabel->setText(pProjData->getID());
    mGUI->PSFileLabel->setText(pProjData->getPSProjectFile().filePath() == "" ? "N/A" : pProjData->getPSProjectFile().fileName());
    mGUI->PSFolderLabel->setText(pProjData->getPSProjectFolder().path());

    mGUI->DescriptionLabel->setText(pProjData->getNameStrict());
    mGUI->ImageInfoLabel->setText(QString::asprintf("%ld raw, %ld normal, %d depth map",
                                  pProjData->getRawImageCount(), pProjData->getProcessedImageCount(),
                                  pProjData->getDenseCloudDepthImages()));

    if(pProjData->isImageExposureKnown()) {
        const double* lWB = pProjData->getWhiteBalanceMultipliers();
        mGUI->ExposureInfoLabel->setText(QString::asprintf("White Balance [%.4f, %.4f, %.4f, %.4f], Brightness %.4f",
                                         lWB[0], lWB[1], lWB[2], lWB[3], pProjData->getBrightnessMultiplier()));
    } else {
        mGUI->ExposureInfoLabel->setText("N/A");
    }
    mGUI->SpecialNotesLabel->setText(pProjData->getSpecialNotes());

    for(int chunk = 0; chunk < pProjData->getChunkCount(); chunk++)
    {
        PSChunkData* lChunk = pProjData->getChunk(chunk);
        if(lChunk == NULL) continue;

        QWidget* lChunkInfo = new QWidget(this);
        Ui::PSChunkInfo* lChunkGUI = new Ui::PSChunkInfo();
        lChunkGUI->setupUi(lChunkInfo);

        QString label = lChunk->getLabel();
        lChunkGUI->ChunkLabelLabel->setText((label.isNull() || label.isEmpty()) ? "[none]" : label);
        lChunkGUI->ImageInfoLabel->setText(QString::asprintf("%d images, %ld sensors, %d depth maps",
                                                       lChunk->getImageCount(), lChunk->getSensorCount(),
                                                       lChunk->getDenseCloudDepthImages()));

        // Fill in image alignment phase details
        lChunkGUI->IADetailLabel->setText(lChunk->getImageAlignment_LevelString());
        lChunkGUI->IAMaskedCheckBox->setChecked(lChunk->getImageAlignment_Masked());
        lChunkGUI->IALimitsLabel->setText(QString::asprintf("%ldk/%ldk",
                                   lChunk->getImageAlignment_featureLimit()/1000,
                                   lChunk->getImageAlignment_tiePointLimit()/1000));
        //lChunkGUI->IADurationLabel->setText(lChunk.getImageAlignment_durationString()); // TODO

        // Fill in the optimization details
        lChunkGUI->OptInfoLabel->setText(lChunk->getOptimizeString());

        // Fill in the dense cloud phase details
        lChunkGUI->DCDetailLabel->setText(lChunk->getDenseCloud_levelString());
        lChunkGUI->DCFilterLabel->setText(lChunk->getDenseCloud_filterLevelString());
        //lChunkGUI->DCDurationLabel->setText(lChunk->getDenseCloud_durationString()); // TODO

        // Fill in the model generation phase details
        lChunkGUI->ModDetailLabel->setText(lChunk->getModelGeneration_levelString());
        lChunkGUI->ModFacesLabel->setText(QLocale::system().toString((int)lChunk->getModelFaceCount()));
        //lChunkGUI->ModDurationLabel->setText(lChunk.getModelGeneration_durationString()); // TODO

        mGUI->tabWidget->addTab(lChunkInfo, QString::asprintf("Chunk %d", chunk+1));
    }
}
