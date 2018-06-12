//package edu.uwstout.berriers.PSHelper.UILogic;

//import java.util.LinkedList;
//import java.util.Queue;

//import com.trolltech.qt.core.QFuture;
//import com.trolltech.qt.core.QFutureWatcher;
//import com.trolltech.qt.gui.QAbstractButton;
//import com.trolltech.qt.gui.QDialog;
//import com.trolltech.qt.gui.QDialogButtonBox;
//import com.trolltech.qt.gui.QMessageBox;
//import com.trolltech.qt.gui.QWidget;

//import edu.uwstout.berriers.PSHelper.UIForms.Ui_StageProgressDialog;
//import edu.uwstout.berriers.PSHelper.app.QueueableProcess;

#include "ProcessQueueProgressDialog.h"

//ProcessQueueProgressDialog::ProcessQueueProgressDialog(QQueue<QueueableProcess<QObject*>> &pProcessQueue, QWidget parent) : QDialog(parent)
//{
//    // Copy provided queue or make an empty one
//    if(pProcessQueue == NULL) {
//        mProcessQueue = new QQueue<QueueableProcess<QObject*>>();
//    } else {
//        mProcessQueue = pProcessQueue;
//    }

//    // Setup the GUI
//    mGUI = new Ui_StageProgressDialog();
//    mGUI->setupUi(this);

//    mGUI->TotalProgressBar->reset();
//    mGUI->StageProgressBar->reset();
//    mGUI->StageDescriptionLabel->setText("");
//    mGUI->TotalDescriptionLabel->setText("");

//    // Setup the future watcher
//    mCurrentWatcher = new QFutureWatcher<QObject>(this);
//    connect(
//        mCurrentWatcher, &QFutureWatcher::finished,
//        this, &ProcessQueueProgressDialog::stageFinished
//    );

//    connect(
//        mCurrentWatcher, &QFutureWatcher::progressRangeChanged,
//        mGUI->StageProgressBar, &ProgressBar::setRange
//    );

//    connect(
//        mCurrentWatcher, &QFutureWatcher::progressValueChanged,
//        this, &ProgressBar::setValue
//    );
//}

//public class ProcessQueueProgressDialog extends QDialog {

//	private Ui_StageProgressDialog mGUI;
//	private Queue<QueueableProcess<? extends Object>> mProcessQueue;
//	private QFutureWatcher<Object> mCurrentWatcher;
	
//	public Signal0 stageComplete = new Signal0();
	
//	public ProcessQueueProgressDialog(Queue<QueueableProcess<? extends Object>> pProcessQueue, QWidget parent) {
//		super(parent);
		
//		// Copy provided queue or make an empty one
//		if(pProcessQueue == null) {
//			mProcessQueue = new LinkedList<QueueableProcess<? extends Object>>();
//		} else {
//			mProcessQueue = pProcessQueue;
//		}

//		// Setup the GUI
//		mGUI = new Ui_StageProgressDialog();
//		mGUI.setupUi(this);
		
//		mGUI.TotalProgressBar.reset();
//		mGUI.StageProgressBar.reset();
//		mGUI.StageDescriptionLabel.setText("");
//		mGUI.TotalDescriptionLabel.setText("");

//		// Setup the future watcher
//		mCurrentWatcher = new QFutureWatcher<Object>(this);
//		mCurrentWatcher.finished.connect(this, "stageFinished()");
//		mCurrentWatcher.progressRangeChanged.connect(mGUI.StageProgressBar, "setRange(int, int)");
//		mCurrentWatcher.progressValueChanged.connect(mGUI.StageProgressBar, "setValue(int)");
//	}

//	// Begin processing the queue
//	public void startProcessQueue() {
//		mGUI.TotalProgressBar.reset();
//		mGUI.TotalProgressBar.setRange(0, mProcessQueue.size());
//		mGUI.TotalDescriptionLabel.setText(String.format("Stage %02d/%02d", 0, mProcessQueue.size()));
		
//		startNextProcess();
//	}

//	// Advance to the next process
//	@SuppressWarnings("unchecked")
//	private void startNextProcess() {
//		if(!mProcessQueue.isEmpty()) {
//			QueueableProcess<? extends Object> lProcess = mProcessQueue.peek();
//			if(lProcess != null) {
//				mGUI.StageDescriptionLabel.setText(lProcess.describeProcess());
//				mCurrentWatcher.setFuture((QFuture<Object>)lProcess.runProcess());
//			}
//		} else {
//			this.accept();
//		}
//	}
	
//	@Override
//	protected void showEvent(com.trolltech.qt.gui.QShowEvent pEvent) {
//		if(pEvent.spontaneous()) {
//			startProcessQueue();
//		}
//	}
	
//	// Slot triggered when a stage is complete
//	@SuppressWarnings("unused")
//	private void stageFinished() {
//		if(!mCurrentWatcher.isCanceled()) {
//			mProcessQueue.poll();
//			int currentStage = mGUI.TotalProgressBar.maximum()-mProcessQueue.size();
//			mGUI.TotalProgressBar.setValue(currentStage);
//			mGUI.TotalDescriptionLabel.setText(String.format("Stage %02d/%02d", currentStage, mGUI.TotalProgressBar.maximum()));
//			stageComplete.emit();
//			startNextProcess();
//		} else {
//			this.reject();
//		}
//	}

//	// Respond to cancel button
//	@SuppressWarnings("unused")
//	private void on_buttonBox_clicked(QAbstractButton pButton) {
//		switch(mGUI.buttonBox.buttonRole(pButton)) {
//			case RejectRole: {
//				int result = QMessageBox.question(this, "Cancel Queue", "Are you sure you want to cancel the queue?",
//								QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No);
//				if(result == QMessageBox.StandardButton.Yes.value())
//				{
//					QAbstractButton cancelButton = mGUI.buttonBox.button(QDialogButtonBox.StandardButton.Cancel);
//					if(cancelButton != null) {
//						cancelButton.setEnabled(false);
//						cancelButton.setText("Canceling...");
//					}
//					mCurrentWatcher.cancel();
//				}
//			}  break;
//			default: break;
//		}
//	}
	
//}
