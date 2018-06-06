package edu.uwstout.berriers.PSHelper.UILogic;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;

import com.trolltech.qt.core.QFuture;
import com.trolltech.qt.core.QFutureWatcher;
import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.core.QtConcurrent;
import com.trolltech.qt.gui.QAbstractButton;
import com.trolltech.qt.gui.QDialog;
import com.trolltech.qt.gui.QDialogButtonBox;
import com.trolltech.qt.gui.QFileDialog;
import com.trolltech.qt.gui.QImage;
import com.trolltech.qt.gui.QMessageBox;
import com.trolltech.qt.gui.QPixmap;
import com.trolltech.qt.gui.QResizeEvent;
import com.trolltech.qt.gui.QWidget;

import edu.uwstout.berriers.PSHelper.Model.PSSessionData;
import edu.uwstout.berriers.PSHelper.UIForms.Ui_RawImageExposureDialog;
import edu.uwstout.berriers.PSHelper.app.ExposureSettings;
import edu.uwstout.berriers.PSHelper.app.ImageProcessorIM4J;

public class RawImageExposureDialog extends QDialog {

	private Ui_RawImageExposureDialog mGUI;
	private PSSessionData mProjectData;
	private QPixmap mPreviewImage;
	private QFutureWatcher<Object> mPreviewFileWatcher;
	private QSettings mProjectInfoStore;
	
	private ExposureSettings mDefaultSettings;
	private boolean mEnqueueMode;
	private boolean mBlockUpdateFromGUI;
	
	public RawImageExposureDialog(QWidget parent) {
		super(parent);
		mDefaultSettings = ExposureSettings.DEFAULT_EXPOSURE;
		setupGUI();
	}

	private void setupGUI() {
		mPreviewImage = null;
		mProjectData = null;
		mBlockUpdateFromGUI = false;
		mEnqueueMode = false;

		mGUI = new Ui_RawImageExposureDialog();
		mGUI.setupUi(this);
		mGUI.BrightScaleSpinBox.setEnabled(false);
		setWBCustomEnabled(false);
		setEnqueueMode(false);
		
		mPreviewFileWatcher = new QFutureWatcher<Object>(this);
		mPreviewFileWatcher.finished.connect(this, "previewReady()");
		
		applySettings(mDefaultSettings);
	}
	
	public void setEnqueueMode(boolean pEnabled) {
		mEnqueueMode = pEnabled;
		if(mEnqueueMode) {
			mGUI.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setText("Enqueue");
		} else {
			mGUI.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setText("Expose");			
		}
	}
	
	public void setProjectData(PSSessionData pData, QSettings pInfoStore) {
		mProjectData = pData;
		mProjectInfoStore = pInfoStore;
		try {
			mGUI.DestinationDirLineEdit.setText(mProjectData.getPSProjectFolder().getCanonicalPath());
		} catch (IOException e) {}
		projectDataChanged();
	}

	public void applySettings(ExposureSettings pSettings) {
		mBlockUpdateFromGUI = true;
		mGUI.WBModeComboBox.setCurrentIndex(pSettings.getWBMode().ordinal());
		mGUI.RSpinBox.setValue(pSettings.getWBCustom()[0]);
		mGUI.G1SpinBox.setValue(pSettings.getWBCustom()[1]);
		mGUI.BSpinBox.setValue(pSettings.getWBCustom()[2]);
		mGUI.G2SpinBox.setValue(pSettings.getWBCustom()[3]);

		mGUI.BrightModeComboBox.setCurrentIndex(pSettings.getBrightMode().ordinal());
		mGUI.BrightScaleSpinBox.setValue(pSettings.getBrightScale());
		mBlockUpdateFromGUI = false;
		
		asyncGeneratePreview();
	}
	
	public void on_PreviewButton_clicked() {
		asyncGeneratePreview();
	}

	public void on_WBModeComboBox_currentIndexChanged(int pNewIndex) {
		setWBCustomEnabled(pNewIndex == 3);
	}

	public void on_BrightModeComboBox_currentIndexChanged(int pNewIndex) {
		mGUI.BrightScaleSpinBox.setEnabled(pNewIndex == 2);
	}
	
	public void on_DestinationBrowseButton_clicked() {
		String lDestination = QFileDialog.getExistingDirectory(this, "Select Destination",
				mProjectData.getPSProjectFolder().getPath());
		if(lDestination != null && !lDestination.isEmpty()) {
			mGUI.DestinationDirLineEdit.setText(lDestination);
		}
	}

	public void on_buttonBox_clicked(QAbstractButton pButton) {
		switch(mGUI.buttonBox.standardButton(pButton)) {
			case RestoreDefaults:
				applySettings(mDefaultSettings);
				break;
			
			case Ok: {
				QMessageBox.StandardButton lResult = null;
				QMessageBox.StandardButtons lButtons = new QMessageBox.StandardButtons(
						QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes);
				if(mEnqueueMode) {
					lResult = QMessageBox.question(this, "Add to queue?", "Add to processing queue?", lButtons);					
				} else {
					lResult = QMessageBox.question(this, "Start Processing?", "Begin processing raw images?\n\n" +
									"(Note: Processing may take a long time and use significant resources)", lButtons);
				}
				
				if(lResult == QMessageBox.StandardButton.Yes) {
					mProjectData.writeExposureSettings(getExposureSettings(), mProjectInfoStore);
					this.accept();
				}
			} break;
				
			case Cancel: this.reject();
			break;
				
			default: break;			
		}
	}
	
	public void setWBCustomEnabled(boolean pEnable) {
		mGUI.RSpinBox.setEnabled(pEnable);
		mGUI.G1SpinBox.setEnabled(pEnable);
		mGUI.BSpinBox.setEnabled(pEnable);
		mGUI.G2SpinBox.setEnabled(pEnable);		
	}
	
	public File getDestinationPath() {
		return new File(mGUI.DestinationDirLineEdit.text());
	}

	public void updateFromGUI() {
		if(isVisible() && !mBlockUpdateFromGUI) {
			asyncGeneratePreview();
		}
	}
		
	public void asyncGeneratePreview() {
		if(mProjectData == null) return;

		System.out.println("Generating preview.");
		mGUI.PreviewButton.setEnabled(false);
		mGUI.PreviewButton.setText("Wait ...");
		ExposureSettings lSettings = getExposureSettings();
		if(lSettings != null) {
			int lIndex = mGUI.PreviewImageComboBox.currentIndex();
			if(lIndex >= 0 && lIndex < mProjectData.getRawFileList().length) {
				File lRawFile = mProjectData.getRawFileList()[lIndex];
				
				try {
					Method lMeth = ImageProcessorIM4J.class.getMethod("developRawImage",
							File.class, ExposureSettings.class, boolean.class);
	
					QFuture<Object> lPreviewResult = QtConcurrent.run(this, lMeth, lRawFile, lSettings, true);
					mPreviewFileWatcher.setFuture(lPreviewResult);
				} catch (Exception e) {
					System.err.println(e.getMessage());
					e.printStackTrace();
				}
			}
		}
	}
	
	public void previewReady() {
		mGUI.PreviewButton.setText("Preview");
		mGUI.PreviewButton.setEnabled(true);
		Object lResult = mPreviewFileWatcher.result();
		if(!(lResult instanceof File)) {
			System.out.println("\tResult is not a file.");
			return;
		}
		File lExposedFile = (File)lResult;
		if(lExposedFile != null && lExposedFile.exists()) {
			// Update multipliers
			mBlockUpdateFromGUI = true;
			mGUI.RSpinBox.setValue(ImageProcessorIM4J.mMultipliers[0]);
			mGUI.G1SpinBox.setValue(ImageProcessorIM4J.mMultipliers[1]);
			mGUI.BSpinBox.setValue(ImageProcessorIM4J.mMultipliers[2]);
			mGUI.G2SpinBox.setValue(ImageProcessorIM4J.mMultipliers[3]);
			mBlockUpdateFromGUI = false;

			// Put in label
			mPreviewImage = QPixmap.fromImage(new QImage(lExposedFile.getPath()));
			lExposedFile.delete();
			updatePreviewImage();
		} else {
			System.out.println("\tPreview output is null or does not exist.");			
		}
	}
	
	public void projectDataChanged() {

		mDefaultSettings = mProjectData.readExposureSettings(mProjectInfoStore);
		applySettings(mDefaultSettings);
		
		mGUI.PreviewImageComboBox.clear();
		
		File[] lRawFiles = mProjectData.getRawFileList();
		for(final File lRawFile : lRawFiles) {
			mGUI.PreviewImageComboBox.addItem(lRawFile.getName());
		}
		
		asyncGeneratePreview();
	}
	
	public void updatePreviewImage() {
		if(mPreviewImage == null) return;
		
		// get label dimensions
		int w = mGUI.ImagePreviewLabel.width();
		int h = mGUI.ImagePreviewLabel.height();
		mGUI.ImagePreviewLabel.setPixmap(mPreviewImage.scaled(w,h,Qt.AspectRatioMode.KeepAspectRatio));		
	}

	@Override
	public void resizeEvent(QResizeEvent e) {
		updatePreviewImage();
	}
	
	public ExposureSettings getExposureSettings() {
		double[] lCustomWB = {mGUI.RSpinBox.value(), mGUI.G1SpinBox.value(),
				 mGUI.BSpinBox.value(), mGUI.G2SpinBox.value()};
		ImageProcessorIM4J.WhiteBalanceMode lWBMode = 
				ImageProcessorIM4J.WhiteBalanceMode.values()[mGUI.WBModeComboBox.currentIndex()];
		ImageProcessorIM4J.BrightnessMode lBrightMode = 
				ImageProcessorIM4J.BrightnessMode.values()[mGUI.BrightModeComboBox.currentIndex()];
		
		ExposureSettings lSettings = null;
		try {
			lSettings = new ExposureSettings(lWBMode, lCustomWB, lBrightMode, mGUI.BrightScaleSpinBox.value());
		} catch (Exception e) {
			System.err.println("Error: could not build exposure settings.\n");
			System.err.println(e.getMessage());
			return null;
		}
		
		return lSettings;
	}
}
