#include <QMessageBox>
#include <QSettings>

#include "PSHelperMainWindow.h"

#include "PSandPhotoScanner.h"
#include "PSProjectDataModel.h"
#include "PSSessionData.h"
#include "RawImageExposer.h"
#include "GLModelViewer.h"
#include "QueueableProcess.h"
#include "CancelableModalProgressDialog.h"

#include "ui_PSHelperMainWindow.h"

string PSHelperMainWindow::WINDOWS_PATH = "C:\\Windows;C:\\Program Files\\ImageMagick;C:\\Program Files\\GraphicsMagick;C:\\Program Files\\exiftool;C:\\Program Files\\dcraw";
string PSHelperMainWindow::MAC_UNIX_PATH = "/usr/bin:/usr/local/bin:/opt/local/bin";

PSHelperMainWindow::PSHelperMainWindow(QWidget parent) : QWindow(parent) {
    mContextMenu = new QMenu(this);
    mContextMenu.addAction("View Model", this, "viewModel()");
    mContextMenu.addAction("Edit General Settings", this, "editGeneralSettings()");
    mContextMenu.addSeparator();
    mContextMenu.addAction("Expose Raw Images", this, "runExposeImagesAction()");
    mContextMenu.addAction("Run PhotoScan Phase 1", this, "runPhotoScanPhase1Action()");
    mContextMenu.addAction("Run PhotoScan Phase 2", this, "runPhotoScanPhase2Action()");
    mContextMenu.addSeparator();
    mContextMenu.addAction("Queue For Expose Raw Images", this, "queueExposeImagesAction()");
    mContextMenu.addAction("Queue For PhotoScan Phase 1", this, "queuePhotoScanPhase1Action()");
    mContextMenu.addAction("Queue For PhotoScan Phase 2", this, "queuePhotoScanPhase2Action()");

    mGUI = new Ui_PSHelperMainWindow();
    mGUI.setupUi(this);
    setWindowTitle("PhotoScan Helper");

    mModelViewer = null;

    mRawExposer = null;
    mRawExposureProgressDialog = new CancelableModalProgressDialog<File>("Exposing Raw Images", this);
//    mProcessQueue = new LinkedList<QueueableProcess<? extends Object>>();

    readSettings();

    mModelViewer = new GLModelViewer((PSSessionData)null, null);
    mModelViewer.show();
}

PSHelperMainWindow::~PSHelperMainWindow() {
    // TODO: Cleanup memory
}

void PSHelperMainWindow::setModelData(PSandPhotoScanner pScanner) {
    mDataModel = new PSProjectDataModel(pScanner.getPSProjectData(), this);
    mDataInfoStore = pScanner.getInfoStore();

    mGUI.PSDataTableView.setModel(mDataModel);
    mGUI.PSDataTableView.setContextMenuPolicy(ContextMenuPolicy.CustomContextMenu);
    mGUI.PSDataTableView.customContextMenuRequested.connect(this, "showContextMenu(QPoint)");

    QSettings settings = new QSettings();
    settings.beginGroup("ViewOptions");
    int extended = Integer.parseInt(settings.value("ExtendedInfo", "0").toString());
    int colors = Integer.parseInt(settings.value("ColorsForStatus", "0").toString());
    settings.endGroup();

    if(extended == 1) {
        mGUI.action_ShowExtendedInfo.setChecked(true);
        mDataModel.setExtendedColsEnabled(true);
    }

    if(colors == 1) {
        mGUI.action_ShowColorsForStatus.setChecked(true);
        mDataModel.setShowColorForStatus(true);
    }


    mGUI.DataInfoLabel.setText(
            String.format("%d projects (%d unique), %d w/o PSZ fies, " +
                          "%d w/o image align, %d w/o dense cloud, %d w/o model",
            pScanner.getPSProjectData().size(), pScanner.countUniqueDirs(), pScanner.countDirsWithoutProjects(),
            pScanner.countDirsWithoutImageAlign(), pScanner.countDirsWithoutDenseCloud(),
            pScanner.countDirsWithoutModels()));
    mGUI.PSDataTableView.resizeColumnsToContents();
    mGUI.PSDataTableView.verticalHeader().setResizeMode(ResizeMode.ResizeToContents);
}

void PSHelperMainWindow::showContextMenu(QPoint pos) {
    QModelIndex lIndex = mGUI.PSDataTableView.indexAt(pos);
    mLastData = mDataModel.getDataAtIndex(lIndex.row());
    mContextMenu.popup(QCursor.pos());
}

bool PSHelperMainWindow::validateSettings() {

    // Keep looping until we get valid paths (proceeding is useless until this passes)
    while(true) {
        // Test to see if the paths work
        if(ImageProcessorIM4J.locatePrograms()) {
            return true;
        } else {
            // Settings are wrong so warn and prompt to update or cancel
            int button = QMessageBox.warning(this, "IM4J Search Failed", "Could not locate the necessary utilities. "
                                    + "Please update your im4j path settings.", QMessageBox.StandardButton.Ok,
                                    QMessageBox.StandardButton.Cancel);
            if(button == QMessageBox.StandardButton.Cancel.value()) {
                return false;
            }

            // Show the preferences dialog for updating (
            ProgramPreferencesDialog prefsDiag = new ProgramPreferencesDialog(null, this);
            if(prefsDiag.exec() == QDialog.DialogCode.Accepted.value()) {
                prefsDiag.writeResults(null);
                writeSettings();
            } else {
                // They cancelled so return false
                return false;
            }
        }
    }
}

void PSHelperMainWindow::writeSettings() {
    QSettings settings = new QSettings();

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    String searchPath = ImageProcessorIM4J.getSearchPath();
    String useGM = (ImageProcessorIM4J.getUseGraphicsMagick()?"true":"false");
    String exifOverride = (ImageProcessorIM4J.getExiftoolOverrideBin()==null?"":
                            ImageProcessorIM4J.getExiftoolOverrideBin());
    String dcrawOverride = (ImageProcessorIM4J.getDcrawOverrideBin()==null?"":
                            ImageProcessorIM4J.getDcrawOverrideBin());
    String IMOverride = (ImageProcessorIM4J.getImageMagickOverrideBin()==null?"":
                            ImageProcessorIM4J.getImageMagickOverrideBin());

    settings.beginGroup("Paths");
    settings.setValue("im4j Search Path", searchPath);
    settings.setValue("im4j exiftool Override Path", exifOverride);
    settings.setValue("im4j dcraw Override Path", dcrawOverride);
    settings.setValue("im4j ImageMagick Override Path", IMOverride);
    settings.setValue("im4j Use GraphicsMagick", useGM);

    settings.endGroup();
}

void PSHelperMainWindow::readSettings() {
    QSettings settings = new QSettings();

    // Remember window position and such
    settings.beginGroup("MainWindow");
    resize((QSize)settings.value("size", new QSize(800, 600)));
    move((QPoint)settings.value("pos", new QPoint(100, 100)));
    settings.endGroup();

    // Remember the search path for use by im4Java
    String defaultPath = "";
    if(System.getProperty("os.name").contains("Windows")) {
            defaultPath = WINDOWS_PATH;
    } else {
            defaultPath = MAC_UNIX_PATH;
    }

    settings.beginGroup("Paths");
    String searchPath = settings.value("im4j Search Path", defaultPath).toString();
    String exifOverridePath = settings.value("im4j exiftool Override Path", "").toString();
    String dcrawOverridePath = settings.value("im4j dcraw Override Path", "").toString();
    String IMOverridePath = settings.value("im4j ImageMagick Override Path", "").toString();
    String useGM = settings.value("im4j Use GraphicsMagick", "false").toString();
    settings.endGroup();

    useGM = useGM.toLowerCase();
    ImageProcessorIM4J.setSearchPath(searchPath);
    ImageProcessorIM4J.setExiftoolOverrideBin(exifOverridePath);
    ImageProcessorIM4J.setDcrawOverrideBin(dcrawOverridePath);
    ImageProcessorIM4J.setImageMagickOverrideBin(IMOverridePath);
    ImageProcessorIM4J.setUseGraphicsMagick(useGM.equals("true"));
}

void PSHelperMainWindow::closeEvent(QCloseEvent event) {
    writeSettings();
    event.accept();
}

void PSHelperMainWindow::on_PSDataTableView_doubleClicked(QModelIndex pIndex) {
    PSSessionData lProjData = mDataModel.getDataAtIndex(pIndex.row());
    if(lProjData == null) return;

    PSProjectInfoDialog lProjDialog = new PSProjectInfoDialog(lProjData, this);
    lProjDialog.exec();
}

void PSHelperMainWindow::on_action_WriteToCSVFile_triggered(boolean pChecked) {
    String filename = QFileDialog.getSaveFileName(this, "Specify a CSV file to save data.", "");
    if(filename != null && !filename.isEmpty()) {
        if(!mDataModel.outputToCSVFile(filename)) {
            QMessageBox.warning(this, "CSV Saving Failed", "An error occured while saving to a CSV file.");
        }
    }
}

void PSHelperMainWindow::on_action_ShowExtendedInfo_triggered(boolean pChecked) {
    mDataModel.setExtendedColsEnabled(pChecked);
    mGUI.PSDataTableView.resizeColumnsToContents();

    QSettings settings = new QSettings();
    settings.beginGroup("ViewOptions");
    settings.setValue("ExtendedInfo", (pChecked?1:0));
    settings.endGroup();
}

void PSHelperMainWindow::on_action_ShowColorsForStatus_triggered(boolean pChecked) {
    mDataModel.setShowColorForStatus(pChecked);
    mGUI.PSDataTableView.resizeColumnsToContents();

    QSettings settings = new QSettings();
    settings.beginGroup("ViewOptions");
    settings.setValue("ColorsForStatus", (pChecked?1:0));
    settings.endGroup();
}

void PSHelperMainWindow::on_PreferencesAction_triggered(boolean pChecked) {
    ProgramPreferencesDialog prefsDiag = new ProgramPreferencesDialog(mDataInfoStore, this);
    if(prefsDiag.exec() == QDialog.DialogCode.Accepted.value()) {
        prefsDiag.writeResults(mDataInfoStore);
        writeSettings();
    }
}

void PSHelperMainWindow::on_AboutAction_triggered(boolean pChecked) {
    QDialog aboutDialog = new QDialog();
    Ui_AboutDialog aboutUI = new Ui_AboutDialog();

    aboutUI.setupUi(aboutDialog);
    QPixmap iconPix = new QPixmap("classpath:icons/PhotoScanHelper.png");
    if(iconPix != null && !iconPix.isNull()) {
        int w = aboutUI.iconLabel.width();
        int h = aboutUI.iconLabel.height();
        aboutUI.iconLabel.setPixmap(
                iconPix.scaled(w,h,Qt.AspectRatioMode.KeepAspectRatio));
    }

    aboutDialog.exec();
}

void PSHelperMainWindow::on_QuitAction_triggered(boolean pChecked) {
    this.close();
}

void PSHelperMainWindow::on_ClearQueueButton_clicked() {
    QMessageBox::StandardButton lResult = QMessageBox::question(this, "Clear the queue?",
            "Remove all queued processes?", QMessageBox::Cancel, QMessageBox::Yes);
    if(lResult == QMessageBox::Yes) {
        mGUI.QueueListWidget.clear();
        mProcessQueue.clear();
    }
}

void PSHelperMainWindow::on_RunQueueButton_clicked() {
    if(mGUI.QueueListWidget.count() < 1) {
        QMessageBox::warning(this, "Nothing to do", "Please add something to the queue first.");
    } else {
        QMessageBox::StandardButton lResult = QMessageBox.question(this, "Begin processing?",
                "Run all queued actions?\n\nNote: This may take a long time.",
                QMessageBox::Cancel, QMessageBox::Yes);
        if(lResult == QMessageBox::Yes) {
            ProcessQueueProgressDialog lQueueDialog = new ProcessQueueProgressDialog(mProcessQueue, this);
            lQueueDialog.stageComplete.connect(this, "dequeue()");
            lQueueDialog.show();
            lQueueDialog.startProcessQueue();
        }
    }
}

void PSHelperMainWindow::viewModel() {
    if(mModelViewer == null) {
        mModelViewer = new GLModelViewer(mLastData, null);
        mModelViewer.show();
    } else {
        mModelViewer.loadNewModel(mLastData);
        mModelViewer.show();
    }
}

void PSHelperMainWindow::editGeneralSettings() {
    GeneralSettingsDialog lDialog = new GeneralSettingsDialog(mLastData, this);
    if(lDialog.exec() == QDialog.DialogCode.Accepted.value()) {
        mLastData.writeGeneralSettings(mDataInfoStore);
    }
}

void PSHelperMainWindow::runExposeImagesAction() {
    if(mLastData == null) return;

    RawImageExposureDialog lExposureDialog = new RawImageExposureDialog(this);
    lExposureDialog.setProjectData(mLastData, mDataInfoStore);
    int result = lExposureDialog.exec();

    if(result == 1) {
        try {
            mRawExposer = new RawImageExposer(mLastData, lExposureDialog.getExposureSettings(),
                    lExposureDialog.getDestinationPath());
        } catch (Exception e) {}

        mRawExposureProgressDialog.setFuture(mRawExposer.runProcess());
        mRawExposureProgressDialog.exec();

        if(!mRawExposureProgressDialog.wasCanceled()) {
            try {
                mLastData.examineProjects(mDataInfoStore);
            } catch (IOException e) {
                System.err.println("Error: exception while rebuilding PS Project Data.");
                System.err.println(e.getMessage());
            }
        }
    }
}

void PSHelperMainWindow::dequeue() {
    if(mGUI.QueueListWidget.count() > 0) {
        mGUI.QueueListWidget.removeItemWidget(mGUI.QueueListWidget.item(0));
    }
}

void PSHelperMainWindow::queueExposeImagesAction() {
    if(mLastData == null) return;
    RawImageExposureDialog lExposureDialog = new RawImageExposureDialog(this);
    lExposureDialog.setEnqueueMode(true);
    lExposureDialog.setProjectData(mLastData, mDataInfoStore);
    int result = lExposureDialog.exec();

    if(result == 1) {
        try {
            mRawExposer = new RawImageExposer(mLastData, lExposureDialog.getExposureSettings(),
                    lExposureDialog.getDestinationPath());
            mProcessQueue.add(mRawExposer);
            QListWidgetItem lNewItem = new QListWidgetItem(mRawExposer.describeProcess(), mGUI.QueueListWidget);
        } catch (Exception e) {}
    }
}
