#include "CancelableModalProgressDialog.h"

template<class T>
CancelableModalProgressDialog<T>::CancelableModalProgressDialog(QString pLabelText, QWidget parent) : QWidget(parent) {
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
        mWatcher, &QFutureWatcher<T>::finished,
        this, &CancelableModalProgressDialog::processFinished
    );

    connect(
        mWatcher, &QFutureWatcher<T>::progressRangeChanged,
        mGui->ProgressBar, &QProgressBar::setRange
    );

    connect(
        mWatcher, &QFutureWatcher<T>::progressValueChanged,
        mGui->ProgressBar, &QProgressBar::setValue
    );
}

template<class T>
CancelableModalProgressDialog<T>::~CancelableModalProgressDialog() {

}

/**
 * Set the QFuture for this process which will determine when the process is complete
 * and will enable canceling of the process.
 *
 * @param pFuture The future result available once the process ends.
 */
template<class T>
void CancelableModalProgressDialog<T>::setFuture(QFuture<T> pFuture) {
    mWatcher.setFuture(pFuture);
}

///**
// * Interrogate the QFutureWatcher to see if it has been canceled.
// * @return Weather or not the process has been canceled by the user.
// */
//template<class T>
//bool CancelableModalProgressDialog<T>::wasCanceled() {
//    return mWatcher.isCanceled();
//}

///**
// * This slot is triggered internally when a stage of the process is completed.
// * Checks if the user has requested the process be cancelled before proceeding.
// */
//template<class T>
//void CancelableModalProgressDialog<T>::processFinished() {
//    if(!mWatcher.isCanceled()) {
//        emit complete();
//        this->accept();
//    } else {
//        this->reject();
//    }
//}

// Respond to cancel button
/**
 * This slot will respond to the cancel button letting the QFutureWatcher know
 * that the user wants to cancel the process.  This is confirmed with the user
 * before requesting the cancel.
 */
template<class T>
void CancelableModalProgressDialog<T>::on_CancelButtonClicked() {
    int result = QMessageBox.question(this,
                                      "Cancel",
                                      "Are you sure you want to cancel this operation?",
                                      QMessageBox::Yes, QMessageBox::No);

//    if(result == QMessageBox::Yes) {
//        emit canceled();
//        mGUI->CancelButton->setEnabled(false);
//        mGUI->CancelButton->setText("Cancelling...");
//        mWatcher.cancel();
//    }
//}
