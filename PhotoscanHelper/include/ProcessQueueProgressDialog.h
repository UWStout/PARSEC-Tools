#ifndef PROCESS_QUEUE_PROGRESS_DIALOG_H
#define PROCESS_QUEUE_PROGRESS_DIALOG_H

#include <QList>
#include <QQueue>
#include <QFuture>
#include <QFutureWatcher>
#include <QAbstractButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QWidget>
#include <QString>

#include "ui_StageProgressDialog.h"
#include "QueueableProcess.h"

class ProcessQueueProgressDialog : public QDialog {
    Q_OBJECT

public:
    ProcessQueueProgressDialog(const QQueue<QueueableProcess<QObject*>*>& pProcessQueue, QWidget* parent);
    ~ProcessQueueProgressDialog();
    void startProcessQueue();

private:
    Ui_StageProgressDialog* mGUI;
    QQueue<QueueableProcess<QObject*>*> mProcessQueue;
    QFutureWatcher<QObject*>* mCurrentWatcher;

    void startNextProcess();
    void stageFinished();

private slots:
    void on_buttonBox_clicked(QAbstractButton* pButton);

protected:
    void showEvent(QShowEvent pEvent);

signals:
    void stageComplete();
};

#endif
