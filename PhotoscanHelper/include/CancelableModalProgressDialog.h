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
 *
 * @param <T> The type associated with the QFuture that will be used to cancel the background task.
 */
template<class T>
class CancelableModalProgressDialog : public QDialog {
    Q_OBJECT

public:
    CancelableModalProgressDialog(QString pLabelText, QWidget parent);
    ~CancelableModalProgressDialog();
    void setFuture(QFuture<T> pFuture);
    bool wasCanceled();

private:
    Ui_CancelableProgressDialog* mGUI;
    const QFutureWatcher<T> mWatcher;
    void processFinished();
    void on_CancelButtonClicked();

signals:
    void complete();
    void canceled();

};

#endif
