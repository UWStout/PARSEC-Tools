#include <QDialog>
#include <QPixmap>
#include <QFutureWatcher>
#include <QSettings>
#include <QFileInfo>

#include "ui_RawImageExposureDialog.h"
#include "PSSessionData.h"

class RawImageExposureDialog : public QDialog {
    Q_OBJECT

public:
    RawImageExposureDialog(QWidget* parent);
    ~RawImageExposureDialog();

    void setEnqueueMode(bool pEnabled);
    void setprojectData(PSSessionData pData, QSettings pInfoStore);
    void applySettings(ExposureSettings pSettings);
    void setWBCustomEnabled(bool pEnable);
    QFileInfo getDestinationPath();
    void updateFromGUI();
    void asyncGeneratePreview();
    void projectDataChanged();
    void updatePreviewImage();
    ExposureSettings getExposureSettings();

private:
    Ui_RawImageExposureDialog* mGUI;
    PSSessionData* mProjectData;
    QPixmap* mPreviewImage;
    QFutureWatcher<QObject*>* mPreviewFileWatcher;
    QSettings mProjectInfoStore;

    ExposureSettings mDefaultSettings;
    bool mEnqueueMode;
    bool mBlockUpdateFromGUI;

    void setUpGUI();

public slots:
    void on_PreviewButton_clicked();
    void on_WBModeComboBox_currentIndexChanged(int pNewIndex);
    void on_BrightModeComboBox_currentIndexChanged(int pNewIndex);
    void on_DestinationBrowseButton_clicked();
    void on_buttonBox_clicked(QAbstractButton* pButton);
    void previewReady();
};
