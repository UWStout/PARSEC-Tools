#ifndef PHOTOSCANPHASE1DIALOG_H
#define PHOTOSCANPHASE1DIALOG_H

#include <QDialog>

namespace Ui {
class PhotScanPhase1Dialog;
}

class PhotoScanPhase1Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit PhotoScanPhase1Dialog(QWidget *parent = 0);
    ~PhotoScanPhase1Dialog();

    QString getProjectName();
    QString getAccuracy();
    QString getGenericPreselection();
    QString getReferencePreselection();
    QString getKeyPointLimit();
    QString getTiePointLimit();
    QString getApplyMasks();
    QString getAdaptiveCameraModel();
    QString getBuildDenseCloud();
    QString getQuality();
    QString getDepthFiltering();
    QString getPointColors();


private:
    Ui::PhotScanPhase1Dialog* mGUI;

private slots:
    void accept();
};

#endif // PHOTOSCANPHASE1DIALOG_H
