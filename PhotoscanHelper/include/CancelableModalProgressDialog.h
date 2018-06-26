#ifndef CANCELABLE_MODAL_PROGRESS_DIALOG_H
#define CANCELABLE_MODAL_PROGRESS_DIALOG_H

#include <QFuture>
#include <QFutureWatcher>
#include <QDialog>
#include <QMessageBox>
#include <QWidget>
#include <QString>

#include "ui_CancelableProgressDialog.h"

/**
 * A modal progress dialog for compute intensive background tasks where you do not want the
 * user to interact with the software but you do want to provide progress on the task.  The
 * task will also be cleanly cancelable by using a QFuture object.
 *
 * @author berriers
 */
class InternalModalProgressDialog : public QDialog {
    Q_OBJECT

public:
    InternalModalProgressDialog(QString pLabelText, QWidget* parent);
    virtual ~InternalModalProgressDialog();

protected:
    Ui_CancelableProgressDialog* mGUI;

protected slots:
    virtual void processFinished();
    virtual void on_CancelButtonClicked();

signals:
    void complete();
    void canceled();
};

/**
 * A modal progress dialog for compute intensive background tasks where you do not want the
 * user to interact with the software but you do want to provide progress on the task.  The
 * task will also be cleanly cancelable by using a QFuture object.
 *
 * @author berriers
 *
 * @param <T> The type associated with the QFuture that will be used to cancel the background task.
 */
template<class T>
class CancelableModalProgressDialog : public InternalModalProgressDialog {
public:
    CancelableModalProgressDialog(QString pLabelText, QWidget* parent);
    virtual ~CancelableModalProgressDialog();

    void setFuture(QFuture<T> pFuture);
    bool wasCanceled();

private:
    void processFinished();
    void on_CancelButtonClicked();

private:
    QFutureWatcher<T>* mWatcher;
};

template <class T>
CancelableModalProgressDialog<T>::CancelableModalProgressDialog(QString pLabelText, QWidget* parent)
    : InternalModalProgressDialog(pLabelText, parent) {
    // Setup the future watcher
    mWatcher = new QFutureWatcher<T>(this);
    connect(
        mWatcher, &QFutureWatcher<T>::finished,
        this, &CancelableModalProgressDialog::processFinished
    );

    connect(
        mWatcher, &QFutureWatcher<T>::progressRangeChanged,
        mGUI->ProgressBar, &QProgressBar::setRange
    );

    connect(
        mWatcher, &QFutureWatcher<T>::progressValueChanged,
        mGUI->ProgressBar, &QProgressBar::setValue
    );
}

template<class T>
CancelableModalProgressDialog<T>::~CancelableModalProgressDialog() {
    delete mWatcher;
}

/**
 * Set the QFuture for this process which will determine when the process is complete
 * and will enable canceling of the process.
 *
 * @param pFuture The future result available once the process ends.
 */
template<class T>
void CancelableModalProgressDialog<T>::setFuture(QFuture<T> pFuture) {
    mWatcher->setFuture(pFuture);
}

///**
// * Interrogate the QFutureWatcher to see if it has been canceled.
// * @return Weather or not the process has been canceled by the user.
// */
template<class T>
bool CancelableModalProgressDialog<T>::wasCanceled() {
    return mWatcher->isCanceled();
}

///**
// * This slot is triggered internally when a stage of the process is completed.
// * Checks if the user has requested the process be cancelled before proceeding.
// */
template<class T>
void CancelableModalProgressDialog<T>::processFinished() {
    if(!mWatcher->isCanceled()) {
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
template<class T>
void CancelableModalProgressDialog<T>::on_CancelButtonClicked() {
    int result = QMessageBox::question(this, "Cancel",
                                      "Are you sure you want to cancel this operation?",
                                      QMessageBox::Yes, QMessageBox::No);

    if(result == QMessageBox::Yes) {
        emit canceled();
        mGUI->CancelButton->setEnabled(false);
        mGUI->CancelButton->setText("Cancelling...");
        mWatcher->cancel();
    }
}

#endif
