//package edu.uwstout.berriers.PSHelper.UILogic;

//import org.apache.commons.io.FilenameUtils;

//import com.trolltech.qt.core.QSettings;
//import com.trolltech.qt.gui.QAbstractButton;
//import com.trolltech.qt.gui.QDialog;
//import com.trolltech.qt.gui.QFileDialog;
//import com.trolltech.qt.gui.QMessageBox;
//import com.trolltech.qt.gui.QWidget;

//import edu.uwstout.berriers.PSHelper.UIForms.Ui_ProgramPreferencesDialog;
//import edu.uwstout.berriers.PSHelper.app.ImageProcessorIM4J;

#include "ProgramPreferencesDialog.h"

ProgramPreferencesDialog::ProgramPreferencesDialog(QSettings* infoStore, QWidget parent) : QDialog(parent) {
    prefsUI = new Ui_ProgramPreferencesDialog();
    prefsUI.setupUi(this);

    QString searchPath = ImageProcessorIM4J.getSearchPath();
    QString exifPath = ImageProcessorIM4J.getExiftoolOverrideBin();
    QString dcrawPath = ImageProcessorIM4J.getDcrawOverrideBin();
    QString IMPath = ImageProcessorIM4J.getImageMagickOverrideBin();
    bool useGM = ImageProcessorIM4J.getUseGraphicsMagick();

    prefsUI.im4jLineEdit->setText(searchPath);

    if(!exifPath.isNull()) {
        prefsUI.exiftoolPathEdit->setText(exifPath);
        prefsUI.exiftoolOverrideCheckBox->setChecked(true);
    } else {
        prefsUI.exiftoolOverrideCheckBox->setChecked(false);
    }

    if(!dcrawPath.isNull()) {
        prefsUI.dcrawPathEdit->setText(dcrawPath);
        prefsUI.dcrawOverrideCheckBox->setChecked(true);
    } else {
        prefsUI.dcrawOverrideCheckBox->setChecked(false);
    }

    if(!IMPath.isNull()) {
        prefsUI.imageMagickPathEdit->setText(IMPath);
        prefsUI.imageMagickOverrideCheckBox->setChecked(true);
    } else {
        prefsUI.imageMagickOverrideCheckBox->setChecked(false);
    }

    prefsUI.useGMCheckBox->setChecked(useGM);

    if(infoStore != NULL) {
        infoStore->beginGroup("Collection");
        QString colName = infoStore->value("Name", "none").toString();
        QString ownerName = infoStore->value("Owner", "none").toString();
        infoStore->endGroup();

        prefsUI.ownerNameLineEdit->setText(ownerName);
        prefsUI.collectionNameLineEdit->setText(colName);
    } else {
        prefsUI.tabWidget->removeTab(1);
    }
}

void ProgramPreferencesDialog::on_buttonBox_clicked(QAbstractButton pButton) {
    switch(prefsUI.buttonBox->standardButton(pButton)) {

        case QDialogButtonBox::Save: {
            QMessageBox::StandardButton lResult = QMessageBox::Yes;
            if(!validateSearchPath()) {
                QMessageBox::StandardButtons lButtons = QMessageBox::Yes | QMessageBox::Cancel;
                lResult = QMessageBox.question(this,
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
        infoStore->setValue("Name", prefsUI.collectionNameLineEdit->text());
        infoStore->setValue("Owner", prefsUI.ownerNameLineEdit->text());
        infoStore->endGroup();
    }

    updateIM4JPathSettings();
}

//public class ProgramPreferencesDialog extends QDialog {

//	private Ui_ProgramPreferencesDialog prefsUI;
	
//	public ProgramPreferencesDialog(QSettings infoStore, QWidget parent) {
//		super(parent);
//		prefsUI = new Ui_ProgramPreferencesDialog();
//		prefsUI.setupUi(this);
		
//	    String searchPath = ImageProcessorIM4J.getSearchPath();
//	    String exifPath = ImageProcessorIM4J.getExiftoolOverrideBin();
//	    String dcrawPath = ImageProcessorIM4J.getDcrawOverrideBin();
//	    String IMPath = ImageProcessorIM4J.getImageMagickOverrideBin();
//	    boolean useGM = ImageProcessorIM4J.getUseGraphicsMagick();

//	    prefsUI.im4jLineEdit.setText(searchPath);
	    
//	    if(exifPath != null) {
//	    	prefsUI.exiftoolPathEdit.setText(exifPath);
//	    	prefsUI.exiftoolOverrideCheckBox.setChecked(true);
//	    } else {
//	    	prefsUI.exiftoolOverrideCheckBox.setChecked(false);
//	    }

//	    if(dcrawPath != null) {
//	    	prefsUI.dcrawPathEdit.setText(dcrawPath);
//	    	prefsUI.dcrawOverrideCheckBox.setChecked(true);
//	    } else {
//	    	prefsUI.dcrawOverrideCheckBox.setChecked(false);
//	    }

//	    if(IMPath != null) {
//	    	prefsUI.imageMagickPathEdit.setText(IMPath);
//	    	prefsUI.imageMagickOverrideCheckBox.setChecked(true);
//	    } else {
//	    	prefsUI.imageMagickOverrideCheckBox.setChecked(false);
//	    }
	    	
//	    prefsUI.useGMCheckBox.setChecked(useGM);
	    
//	    if(infoStore != null) {
//		    infoStore.beginGroup("Collection");
//		    String colName = infoStore.value("Name", "none").toString();
//		    String ownerName = infoStore.value("Owner", "none").toString();
//		    infoStore.endGroup();
		    
//		    prefsUI.ownerNameLineEdit.setText(ownerName);
//		    prefsUI.collectionNameLineEdit.setText(colName);
//	    } else {
//	    	prefsUI.tabWidget.removeTab(1);
//	    }
//	}

//	public void on_buttonBox_clicked(QAbstractButton pButton) {
//		switch(prefsUI.buttonBox.standardButton(pButton)) {
		
//			case Save: {
//				QMessageBox.StandardButton lResult = QMessageBox.StandardButton.Yes;
//				if(!validateSearchPath()) {
//					QMessageBox.StandardButtons lButtons = new QMessageBox.StandardButtons(
//							QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Yes);
//					lResult = QMessageBox.question(this, "Invalid Path", "The serch path appears to be invalid. "
//														+ "Are you sure you want to save?", lButtons);
//				}
				
//				if(lResult == QMessageBox.StandardButton.Yes) {
//					this.accept();
//				}
//			} break;
				
//			case Cancel:
//				this.reject();
//			break;
				
//			default: break;
//		}
//	}
	
//	public void writeResults(QSettings infoStore) {

//		if(infoStore != null) {
//			infoStore.beginGroup("Collection");
//			infoStore.setValue("Name", prefsUI.collectionNameLineEdit.text());
//			infoStore.setValue("Owner", prefsUI.ownerNameLineEdit.text());
//			infoStore.endGroup();
//		}
		
//		updateIM4JPathSettings();
//	}
	
//	public boolean validateSearchPath() {
//		// Save current settings
//		String oldSearchPath = ImageProcessorIM4J.getSearchPath();
//		String oldExifPath = ImageProcessorIM4J.getExiftoolOverrideBin();
//		String oldDcrawPath = ImageProcessorIM4J.getDcrawOverrideBin();
//		String oldIMPath = ImageProcessorIM4J.getImageMagickOverrideBin();
//		boolean oldUseGM = ImageProcessorIM4J.getUseGraphicsMagick();

//		// Load new settings for testing purposes
//		updateIM4JPathSettings();
		
//		// Test the new settings
//		boolean results = ImageProcessorIM4J.locatePrograms();

//		// Restore old settings and return the results of the test
//		ImageProcessorIM4J.setSearchPath(oldSearchPath);
//		ImageProcessorIM4J.setExiftoolOverrideBin(oldExifPath);
//		ImageProcessorIM4J.setDcrawOverrideBin(oldDcrawPath);
//		ImageProcessorIM4J.setImageMagickOverrideBin(oldIMPath);
//		ImageProcessorIM4J.setUseGraphicsMagick(oldUseGM);
		
//		return results;
//	}

//	private void updateIM4JPathSettings() {
//		ImageProcessorIM4J.setSearchPath(prefsUI.im4jLineEdit.text());
//		if(prefsUI.exiftoolOverrideCheckBox.isChecked()) {
//			ImageProcessorIM4J.setExiftoolOverrideBin(prefsUI.exiftoolPathEdit.text());
//		} else {
//			ImageProcessorIM4J.setExiftoolOverrideBin(null);
//		}
		
//		if(prefsUI.dcrawOverrideCheckBox.isChecked()) {
//			ImageProcessorIM4J.setDcrawOverrideBin(prefsUI.dcrawPathEdit.text());
//		} else {
//			ImageProcessorIM4J.setDcrawOverrideBin(null);
//		}

//		if(prefsUI.imageMagickOverrideCheckBox.isChecked()) {
//			ImageProcessorIM4J.setImageMagickOverrideBin(prefsUI.imageMagickPathEdit.text());
//		} else {
//			ImageProcessorIM4J.setImageMagickOverrideBin(null);
//		}
		
//		ImageProcessorIM4J.setUseGraphicsMagick(prefsUI.useGMCheckBox.isChecked());
//	}
	
//	@SuppressWarnings("unused")
//	private void on_exiftoolBrowseButton_clicked() {
//		String newBin = pickExecutableBinary("Select the executable for exiftool");
//		if(newBin != null && !newBin.isEmpty()) { prefsUI.exiftoolPathEdit.setText(newBin); }
//	}
	
//	private String pickExecutableBinary(String caption) {
		
//		String startDir = ImageProcessorIM4J.getSearchPath();
//		char splitChar = ':';
//		if(startDir.indexOf(";") != -1) { splitChar = ';'; }
//		startDir = startDir.substring(0, startDir.indexOf(splitChar));
		
//		String newBin = QFileDialog.getOpenFileName(this, caption, startDir);
//		if(newBin != null && !newBin.isEmpty()) {
//			String nameOnly = FilenameUtils.getName(newBin);
//			return nameOnly;
//		}
		
//		return null;
//	}
//}
