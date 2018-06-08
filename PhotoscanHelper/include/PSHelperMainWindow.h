#include <QMainWindow>

#include <list>
#include <string>
using namespace std;

// Forward declarations
class PSandPhotoScanner;
class PSProjectDataModel;
class PSSessionData;
class QSettings;
class RawImageExposer;
class GLModelViewer;

//template <class T>
//class CancelableModalProgressDialog<T>;

//template <class T>
//class QueueableProcess<T>;

namespace Ui {
    class PSHelperMainWindow;
}

class PSHelperMainWindow : public QMainWindow {
    Q_OBJECT

public:
    PSHelperMainWindow(QWidget* parent);
    virtual ~PSHelperMainWindow();

    void setModelData(PSandPhotoScanner* pScanner);
    void showContextMenu(QPoint pos);
    bool validateSettings();

private:
    QMenu* mContextMenu;
    PSSessionData* mLastData;

    Ui::PSHelperMainWindow* mGUI;
    PSProjectDataModel* mDataModel;
    QSettings* mDataInfoStore;

    list<QueueableProcess<QObject>*> mProcessQueue;

    RawImageExposer* mRawExposer;
    CancelableModalProgressDialog<File>* mRawExposureProgressDialog;

    GLModelViewer* mModelViewer;

    // Default search paths for im4Java
    static const QString WINDOWS_PATH, MAC_UNIX_PATH;

    void writeSettings();
    void readSettings();

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void on_PSDataTableView_doubleClicked(QModelIndex pIndex);
    void on_action_WriteToCSVFile_triggered(bool pChecked);
    void on_action_ShowExtendedInfo_triggered(bool pChecked);
    void on_action_ShowColorsForStatus_triggered(bool pChecked);
    void on_PreferencesAction_triggered(bool pChecked);
    void on_AboutAction_triggered(bool pChecked);
    void on_QuitAction_triggered(bool pChecked);
    void on_ClearQueueButton_clicked();
    void on_RunQueueButton_clicked();

    void viewModel();
    void editGeneralSettings();

    void runExposeImagesAction();
    void runPhotoScanPhase1Action() {}
    void runPhotoScanPhase2Action() {}

    void dequeue();

    void queueExposeImagesAction();
    void queuePhotoScanPhase1Action() {}
    void queuePhotoScanPhase2Action() {}
};
