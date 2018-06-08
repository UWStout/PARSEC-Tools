package edu.uwstout.berriers.PSHelper.UILogic;

import com.trolltech.qt.gui.QAbstractButton;
import com.trolltech.qt.gui.QDialog;
import com.trolltech.qt.gui.QWidget;

import edu.uwstout.berriers.PSHelper.Model.PSSessionData;
import edu.uwstout.berriers.PSHelper.UIForms.Ui_GeneralSettingsDialog;

public class GeneralSettingsDialog extends QDialog {

	private Ui_GeneralSettingsDialog mGUI;
	final private PSSessionData mProjectData;
	
	public GeneralSettingsDialog(PSSessionData pData, QWidget parent) {
		super(parent);
		
		mProjectData = pData;
		mGUI = new Ui_GeneralSettingsDialog();
		mGUI.setupUi(this);
		
		restoreSettings();
	}
	
	private void restoreSettings() {
		
		String mainHeader = (mProjectData.getPSProjectFile()==null?
				mProjectData.getPSProjectFolder().getName():mProjectData.getPSProjectFile().getName());
		mGUI.mainHeadingLabel.setText(mainHeader);
		
		int status = mProjectData.getStatus().ordinal() - PSSessionData.Status.TEXTURE_GEN_DONE.ordinal();
		if(status > 0) {
			mGUI.statusComboBox.setCurrentIndex(status);
		}
		
		if(mProjectData.getID().equals("-1")) {
			mGUI.IDLineEdit.setText(String.format("%05d", PSSessionData.getNextID()));
		} else {
			mGUI.IDLineEdit.setText(mProjectData.getID());
		}
		
		mGUI.DescriptionLineEdit.setText(mProjectData.getNameStrict());		
		mGUI.SpecialNotesTextEdit.setPlainText(mProjectData.getSpecialNotes());
	}
	
	public void on_buttonBox_clicked(QAbstractButton pButton) {
		switch(mGUI.buttonBox.standardButton(pButton)) {
			case RestoreDefaults:
				restoreSettings();
			break;
			
			case Ok:
				int statusIndex = mGUI.statusComboBox.currentIndex();
				mProjectData.setCustomStatus(statusIndex);
				
				mProjectData.setID(mGUI.IDLineEdit.text());
				mProjectData.setName(mGUI.DescriptionLineEdit.text());
				mProjectData.setSpecialNotes(mGUI.SpecialNotesTextEdit.toPlainText());
			break;
			
			case Cancel: this.reject();
			break;
				
			default: break;			
		}
	}
}
