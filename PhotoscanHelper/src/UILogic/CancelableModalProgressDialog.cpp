//package edu.uwstout.berriers.PSHelper.UILogic;

//import com.trolltech.qt.core.QFuture;
//import com.trolltech.qt.core.QFutureWatcher;
//import com.trolltech.qt.core.Qt.WindowFlags;
//import com.trolltech.qt.core.Qt.WindowType;
//import com.trolltech.qt.gui.QDialog;
//import com.trolltech.qt.gui.QMessageBox;
//import com.trolltech.qt.gui.QWidget;

//import edu.uwstout.berriers.PSHelper.UIForms.Ui_CancelableProgressDialog;

#include "CancelableModalProgressDialog.h"

template<typename T>
CancelableModalProgressDialog::CancelableModalProgressDialog(QString pLabelText, QWidget parent) : QWidget(parent) {
    // Setup the GUI
    mGUI = new Ui_CancelableProgressDialog();
    mGUI->setupUi(this);

    mGUI->ProgressBar->reset();
    mGUI->DescriptionLabel->setText(pLabelText);
    setWindowFlags(new windowFlags(windowType().Window,
                                   windowType().WindowTitleHint,
                                   windowType().CustomizeWindowHint));

    // Setup the future watcher
    mWatcher = new QFutureWatcher<T>(this);
    connect(
        mWatcher, &QFutureWatcher::finished,
        this, &CancelableModalProgressDialog::processFinished
    );

    connect(
        mWatcher, &QFutureWatcher::progressRangeChanged,
        mGui->ProgressBar, &QProgressBar::setRange
    );

    connect(
        mWatcher, &QFutureWatcher::progressValueChanged,
        mGui->ProgressBar, &QProgressBar::setValue
    );
}

/**
 * Set the QFuture for this process which will determine when the process is complete
 * and will enable canceling of the process.
 *
 * @param pFuture The future result available once the process ends.
 */
template<typename T>
void CancelableModalProgressDialog::setFuture(QFuture<T> pFuture) {
    mWatcher.setFuture(pFuture);
}

/**
 * Interrogate the QFutureWatcher to see if it has been canceled.
 * @return Weather or not the process has been canceled by the user.
 */
bool CancelableModalProgressDialog::wasCanceled() {
    return mWatcher.isCanceled();
}

/**
 * This slot is triggered internally when a stage of the process is completed.
 * Checks if the user has requested the process be cancelled before proceeding.
 */
void CancelableModalProgressDialog::processFinished() {
    if(!mWatcher.isCanceled()) {
        emit complete();
        this->accept();
    } else {
        this->reject();
    }
}

// Respond to cancel button
/**
 * This slot will respond to the cancel button letting the QFutureWatcher know
 * that the user wants to cancel the process.  This is confirmed with the user
 * before requesting the cancel.
 */
void CancelableModalProgressDialog::on_CancelButtonClicked() {
    int result = QMessageBox.question(this,
                                      "Cancel",
                                      "Are you sure you want to cancel this operation?",
                                      QMessageBox.standardButton(QMessageBox::Yes),
                                      QMessageBox.standardButton(QMessageBox::No));

    if(result == QMessageBox::Yes) {
        emit canceled();
        mGUI->CancelButton->setEnabled(false);
        mGUI->CancelButton->setText("Cancelling...");
        mWatcher.cancel();
    }
}
