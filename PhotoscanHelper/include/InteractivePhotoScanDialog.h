#ifndef INTERACTIVEPHOTOSCANDIALOG_H
#define INTERACTIVEPHOTOSCANDIALOG_H

#include <QDialog>

class QProcess;
class QTextStream;

namespace Ui {
    class InteractivePhotoScanDialog;
}

class InteractivePhotoScanDialog : public QDialog {
    Q_OBJECT

public:
    explicit InteractivePhotoScanDialog(QWidget *parent = nullptr);
    ~InteractivePhotoScanDialog();

private:
    Ui::InteractivePhotoScanDialog *mGUI;
    QProcess* mPSProc;
    QTextStream* mPSIO;

    void startPhotoScan();

protected slots:
    void inputFromPS();
    void errorFromPS();
};

#endif // INTERACTIVEPHOTOSCANDIALOG_H
