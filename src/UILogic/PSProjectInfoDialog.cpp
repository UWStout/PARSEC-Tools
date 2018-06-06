package edu.uwstout.berriers.PSHelper.UILogic;

import com.trolltech.qt.gui.QDialog;
import com.trolltech.qt.gui.QWidget;

import edu.uwstout.berriers.PSHelper.Model.PSChunkData;
import edu.uwstout.berriers.PSHelper.Model.PSSessionData;
import edu.uwstout.berriers.PSHelper.UIForms.Ui_PSChunkInfo;
import edu.uwstout.berriers.PSHelper.UIForms.Ui_PSProjectInfoDialog;

public class PSProjectInfoDialog extends QDialog {

	Ui_PSProjectInfoDialog mGUI;
	
	public PSProjectInfoDialog(PSSessionData pProjData, QWidget parent) {
		super(parent);
		mGUI = new Ui_PSProjectInfoDialog();
		mGUI.setupUi(this);
		
		setProjectData(pProjData);
	}

	public void setProjectData(PSSessionData pProjData) {
		String mainHeader = (pProjData.getPSProjectFile()==null?
				pProjData.getPSProjectFolder().getName():pProjData.getPSProjectFile().getName());
		mGUI.MainHeaderLabel.setText(mainHeader);

		mGUI.IDLabel.setText(pProjData.getID());
		mGUI.PSFileLabel.setText(pProjData.getPSProjectFile()==null?"N/A":pProjData.getPSProjectFile().getName());
		mGUI.PSFolderLabel.setText(pProjData.getPSProjectFolder().getPath());
		
		mGUI.DescriptionLabel.setText(pProjData.getNameStrict());
		mGUI.ImageInfoLabel.setText(String.format("%d raw, %d normal, %d depth map",
				pProjData.getRawImageCount(), pProjData.getProcessedImageCount(),
				pProjData.getDenseCloudDepthImages()));

		if(pProjData.isImageExposureKnown()) {
			double[] lWB = pProjData.getWhiteBalanceMultipliers();
			mGUI.ExposureInfoLabel.setText(String.format("White Balance [%.4f, %.4f, %.4f, %.4f], Brightness %.4f",
					lWB[0], lWB[1], lWB[2], lWB[3], pProjData.getBrightnessMultiplier()));
		} else {
			mGUI.ExposureInfoLabel.setText("N/A");
		}
		mGUI.SpecialNotesLabel.setText(pProjData.getSpecialNotes());
		
		for(int chunk = 0; chunk < pProjData.getChunkCount(); chunk++)
		{
			PSChunkData lChunk = pProjData.getChunk(chunk);
			if(lChunk == null) continue;
			
			QWidget lChunkInfo = new QWidget(this);
			Ui_PSChunkInfo lChunkGUI = new Ui_PSChunkInfo();
			lChunkGUI.setupUi(lChunkInfo);
			
			String label = lChunk.getLabel();
			lChunkGUI.ChunkLabelLabel.setText((label==null || label.isEmpty())?"[none]":label);
			lChunkGUI.ImageInfoLabel.setText(String.format("%d images, %d sensors, %d depth maps",
					lChunk.getImageCount(), lChunk.getSensorCount(), lChunk.getDenseCloudDepthImages()));
			
			lChunkGUI.IADetailLabel.setText(lChunk.getImageAlignment_LevelString());
			lChunkGUI.IAMaskedCheckBox.setChecked(lChunk.getImageAlignment_Masked());
			lChunkGUI.IALimitsLabel.setText(String.format("%dk/%dk",
					lChunk.getImageAlignment_featureLimit()/1000,
					lChunk.getImageAlignment_tiePointLimit()/1000));
			lChunkGUI.IADurationLabel.setText(lChunk.getImageAlignment_durationString());

			lChunkGUI.OptInfoLabel.setText(lChunk.getOptimizeString());
			
			lChunkGUI.DCDetailLabel.setText(lChunk.getDenseCloud_levelString());
			lChunkGUI.DCFilterLabel.setText(lChunk.getDenseCloud_filterLevelString());
			lChunkGUI.DCDurationLabel.setText(lChunk.getDenseCloud_durationString());

			lChunkGUI.ModDetailLabel.setText(String.format("%.4f", lChunk.getModelGeneration_resolution()));
			lChunkGUI.ModFacesLabel.setText(String.format("%d", lChunk.getModelGeneration_faceCount()));
			lChunkGUI.ModDurationLabel.setText(lChunk.getModelGeneration_durationString());
			
			mGUI.tabWidget.addTab(lChunkInfo, String.format("Chunk %d", chunk+1));
		}
	}

}
