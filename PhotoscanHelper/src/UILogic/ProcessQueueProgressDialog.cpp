#include <QPushButton>

#include "ProcessQueueProgressDialog.h"

ProcessQueueProgressDialog::ProcessQueueProgressDialog(const QQueue<QueueableProcess<QObject*>*>& pProcessQueue, QWidget* parent) : QDialog(parent)
{
    // Copy provided queue or make an empty one
    mProcessQueue = pProcessQueue;

    // Setup the GUI
    mGUI = new Ui_StageProgressDialog();
    mGUI->setupUi(this);

    mGUI->TotalProgressBar->reset();
    mGUI->StageProgressBar->reset();
    mGUI->StageDescriptionLabel->setText("");
    mGUI->TotalDescriptionLabel->setText("");

    // Setup the future watcher
    mCurrentWatcher = new QFutureWatcher<QObject*>(this);
    connect(
        mCurrentWatcher, &QFutureWatcher<QObject*>::finished,
        this, &ProcessQueueProgressDialog::stageFinished
    );

    connect(
        mCurrentWatcher, &QFutureWatcher<QObject*>::progressRangeChanged,
        mGUI->StageProgressBar, &QProgressBar::setRange
    );

    connect(
        mCurrentWatcher, &QFutureWatcher<QObject*>::progressValueChanged,
        mGUI->StageProgressBar, &QProgressBar::setValue
    );
}

ProcessQueueProgressDialog::~ProcessQueueProgressDialog() {

}

// Begin processing the queue
void ProcessQueueProgressDialog::startProcessQueue() {
    mGUI->TotalProgressBar->reset();
    mGUI->TotalProgressBar->setRange(0, mProcessQueue.size());
    QString str;
    mGUI->TotalDescriptionLabel->setText(str.sprintf("Stage %02d/%02d", 0, mProcessQueue.size()));

    startNextProcess();
}

// Advance to the next process
void ProcessQueueProgressDialog::startNextProcess() {
    if(!mProcessQueue.isEmpty()) {
        QueueableProcess<QObject*>* lProcess = mProcessQueue.head();
        if(lProcess != NULL) {
            mGUI->StageDescriptionLabel->setText(lProcess->describeProcess());
            mCurrentWatcher->setFuture((QFuture<QObject*>)lProcess->runProcess());
        }
    } else {
        this->accept();
    }
}

void ProcessQueueProgressDialog::showEvent(QShowEvent pEvent) {
    if(pEvent.spontaneous()) {
        startProcessQueue();
    }
}

// Slot triggered when a stage is complete
void ProcessQueueProgressDialog::stageFinished() {
    if(mCurrentWatcher->isCanceled()) {
        mProcessQueue.dequeue(); // Maybe handle exception here?
        int currentStage = mGUI->TotalProgressBar->maximum()-mProcessQueue.size();
        mGUI->TotalProgressBar->setValue(currentStage);
        QString str;
        mGUI->TotalDescriptionLabel->setText(str.sprintf("Stage %02d/%02d", currentStage, mGUI->TotalProgressBar->maximum()));
        emit stageComplete();
        startNextProcess();
    } else {
        this->reject();
    }
}

// Respond to cancel button
void ProcessQueueProgressDialog::on_buttonBox_clicked(QAbstractButton* pButton) {
    switch (mGUI->buttonBox->buttonRole(pButton)) {
        case mGUI->buttonBox->RejectRole: {
            int result = QMessageBox::question(this,
                                              "Cancel Queue",
                                              "Are you sure you want to cancel the queue?",
                                              QMessageBox::Yes, QMessageBox::No);
            if(result == QMessageBox::Yes) {
                QPushButton* cancelButton = mGUI->buttonBox->button(QDialogButtonBox::Cancel);
                if(cancelButton != NULL) {
                    cancelButton->setEnabled(false);
                    cancelButton->setText("Cancelling...");
                }
                mCurrentWatcher->cancel();
            }
        } break;
        default: break;
    }
}
