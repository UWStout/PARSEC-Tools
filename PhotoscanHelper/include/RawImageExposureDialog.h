#include <QDialog>
#include <QFileInfo>
#include <QPixmap>
#include <QTemporaryFile>

// Forward declarations
namespace Ui {
    class RawImageExposureDialog;
}

template <typename T>
class QFutureWatcher;
class QSettings;
class QResizeEvent;
class PSSessionData;
class QAbstractButton;
class ExposureSettings;

class RawImageExposureDialog : public QDialog {
    Q_OBJECT

public:
    RawImageExposureDialog(QWidget* parent);
    virtual ~RawImageExposureDialog();

    QFileInfo getDestinationPath() const;
    ExposureSettings* getExposureSettings() const;

    void setEnqueueMode(bool pEnabled);
    void setProjectData(PSSessionData *pData, QSettings *pInfoStore);
    void applySettings(const ExposureSettings* pSettings);
    void setWBCustomEnabled(bool pEnable);

    void asyncGeneratePreview();
    void projectDataChanged();
    void updatePreviewImage();

protected:
    void resizeEvent(QResizeEvent* e);

private:
    Ui::RawImageExposureDialog* mGUI;
    PSSessionData* mProjectData;
    QPixmap mPreviewImage;
    QFutureWatcher<QFileInfo>* mPreviewFileWatcher;
    QSettings* mProjectInfoStore;
    QTemporaryFile mTempFile;

    ExposureSettings* mDefaultSettings;
    bool mEnqueueMode;
    bool mBlockUpdateFromGUI;

    void setUpGUI();

public slots:
    void updateFromGUI();
    void on_PreviewButton_clicked();
    void on_WBModeComboBox_currentIndexChanged(int pNewIndex);
    void on_BrightModeComboBox_currentIndexChanged(int pNewIndex);
    void on_DestinationBrowseButton_clicked();
    void on_buttonBox_clicked(QAbstractButton* pButton);
    void previewReady();
};
