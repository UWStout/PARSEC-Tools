#ifndef PS_HELPER_MAIN_WINDOW_H
#define PS_HELPER_MAIN_WINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QString>

#include "ui_PSHelperMain.h"

// Forward declarations
class PSandPhotoScanner;
class PSProjectDataModel;
class PSSessionData;
class QSettings;
class RawImageExposer;
class GLModelWidget;

template <class T>
class CancelableModalProgressDialog;

template <class T>
class QueueableProcess;

namespace Ui {
    class PSHelperMainWindow;
}

class PSHelperMainWindow : public QMainWindow {
    Q_OBJECT

public:
    PSHelperMainWindow(QWidget* parent);
    virtual ~PSHelperMainWindow();

    void setModelData(PSandPhotoScanner* pScanner);
    void addModelData(PSandPhotoScanner* pScanner);
    void showContextMenu(const QPoint &pos);
    bool validateSettings();

private:
    QMenu* mContextMenu;
    PSSessionData* mLastData;
    int mLastDataRow;

    Ui_PSHelperMainWindow* mGUI;
    QString mCollectionDir;
    PSProjectDataModel* mDataModel;

    QList<QueueableProcess<QObject>*> mProcessQueue;

    RawImageExposer* mRawExposer;
    CancelableModalProgressDialog<QFileInfo>* mRawExposureProgressDialog;

    GLModelWidget *mModelViewer;

    // Default search paths for im4Java
    static const QString WINDOWS_PATH, MAC_UNIX_PATH;

    void writeSettings();
    void readSettings();

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void on_PSDataTableView_doubleClicked(const QModelIndex& pIndex);
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
    void runQuickPreviewAction();
    void runPhotoScanPhase1Action() {}
    void runPhotoScanPhase2Action() {}

    void dequeue();

    void queueExposeImagesAction();
    void queuePhotoScanPhase1Action() {}
    void queuePhotoScanPhase2Action() {}

    void scanForNewSessions();
};

#endif
