//package edu.uwstout.berriers.PSHelper.UILogic;

//import com.trolltech.qt.core.QFuture;
//import com.trolltech.qt.core.QFutureWatcher;
//import com.trolltech.qt.core.Qt.WindowFlags;
//import com.trolltech.qt.core.Qt.WindowType;
//import com.trolltech.qt.gui.QDialog;
//import com.trolltech.qt.gui.QMessageBox;
//import com.trolltech.qt.gui.QWidget;

//import edu.uwstout.berriers.PSHelper.UIForms.Ui_CancelableProgressDialog;

#include <QFuture>
#include <QFutureWatcher>
#include <QDialog>
#include <QMessageBox>
#include <QWidget>

class CancelableModalProgressDialog<T> : public QDialog {
    Q_OBJECT

    public:

};

///**
// * A modal progress dialog for compute intensive background tasks where you do not want the
// * user to interact with the software but you do want to provide progress on the task.  The
// * task will also be cleanly cancelable by using a QFuture object.
// *
// * @author berriers
// *
// * @param <T> The type associated with the QFuture that will be used to cancel the background task.
// */
//public class CancelableModalProgressDialog<T> extends QDialog {

//	private Ui_CancelableProgressDialog mGUI;
//	final private QFutureWatcher<T> mWatcher;
	
//	/**
//	 * Emitted when the process is complete.
//	 */
//	public Signal0 complete = new Signal0();

//	/**
//	 * Emitted when the process is cancelled before completion
//	 */
//	public Signal0 canceled = new Signal0();
	
//	/**
//	 * Create the dialog with an appropriate label text and parent.
//	 *
//	 * @param pLabelText Text to label the progress bar in the dialog.
//	 * @param parent Parent QWidget (passed as the parent to the dialog).
//	 */
//	public CancelableModalProgressDialog(String pLabelText, QWidget parent) {
//		super(parent);
		
//		// Setup the GUI
//		mGUI = new Ui_CancelableProgressDialog();
//		mGUI.setupUi(this);
		
//		mGUI.ProgressBar.reset();
//		mGUI.DescriptionLabel.setText(pLabelText);
//		setWindowFlags(new WindowFlags(WindowType.Window,
//        		WindowType.WindowTitleHint, WindowType.CustomizeWindowHint));

//		// Setup the future watcher
//		mWatcher = new QFutureWatcher<T>(this);
//		mWatcher.finished.connect(this, "processFinished()");
//		mWatcher.progressRangeChanged.connect(mGUI.ProgressBar, "setRange(int, int)");
//		mWatcher.progressValueChanged.connect(mGUI.ProgressBar, "setValue(int)");
//	}

//	/**
//	 * Set the QFuture for this process which will determine when the process is complete
//	 * and will enable canceling of the process.
//	 *
//	 * @param pFuture The future result available once the process ends.
//	 */
//	public void setFuture(QFuture<T> pFuture) {
//		mWatcher.setFuture(pFuture);
//	}
	
//	/**
//	 * Interrogate the QFutureWatcher to see if it has been canceled.
//	 * @return Weather or not the process has been canceled by the user.
//	 */
//	public boolean wasCanceled() {
//		return mWatcher.isCanceled();
//	}
	
//	/**
//	 * This slot is triggered internally when a stage of the process is completed.
//	 * Checks if the user has requested the process be cancelled before proceeding.
//	 */
//	@SuppressWarnings("unused")
//	private void processFinished() {
//		if(!mWatcher.isCanceled()) {
//			complete.emit();
//			this.accept();
//		} else {
//			this.reject();
//		}
//	}

//	// Respond to cancel button
//	/**
//	 * This slot will respond to the cancel button letting the QFutureWatcher know
//	 * that the user wants to cancel the process.  This is confirmed with the user
//	 * before requesting the cancel.
//	 */
//	@SuppressWarnings("unused")
//	private void on_CancelButton_clicked() {
//		int result = QMessageBox.question(this, "Cancel", "Are you sure you want to cancel this operation?",
//						QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No);
//		if(result == QMessageBox.StandardButton.Yes.value())
//		{
//			canceled.emit();
//			mGUI.CancelButton.setEnabled(false);
//			mGUI.CancelButton.setText("Canceling...");
//			mWatcher.cancel();
//		}
//	}
//}
