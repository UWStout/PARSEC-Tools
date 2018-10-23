#ifndef PHOTOSCANPHASE2DIALOG_H
#define PHOTOSCANPHASE2DIALOG_H

#include <QDialog>

namespace Ui {
class PhotoScanPhase2Dialog;
}

class PhotoScanPhase2Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit PhotoScanPhase2Dialog(QWidget *parent = 0);
    ~PhotoScanPhase2Dialog();

    QString getSurfaceType();
    QString getSourceData();
    QString getFaceCount();
    QString getInterpolation();
    QString getVertexColors();
    QString getBuildTexture();
    QString getBlendingMode();
    QString getTextureSize();
    QString getHoleFilling();
    QString getGhostingFilter();

private:
    Ui::PhotoScanPhase2Dialog *mGUI;
};

#endif // PHOTOSCANPHASE2DIALOG_H
