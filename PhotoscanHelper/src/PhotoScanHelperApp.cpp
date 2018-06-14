//#include "PSHelperMainWindow.h"
#include "CollectionSelectionDialog.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFutureWatcher>

#include <string>
#include <iostream>
using namespace std;

#include "PSandPhotoScanner.h"

PSandPhotoScanner* mScanner;

int main(int argc, char *argv[]) {
    // Prepare the Qt GUI system
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("UW Stout");
    QCoreApplication::setOrganizationDomain("uwstout.edu");
    QCoreApplication::setApplicationName("PhotoScan Helper");

//    // As far as I can tell, the OS X native menu bar doesn't work in Qt Jambi
//    // The Java process owns the native menu bar and won't relinquish it to Qt
//    QApplication.setAttribute(ApplicationAttribute.AA_DontUseNativeMenuBar);

    // Determine the path to the collection
    QString collectionPath = "";
    if(argc != 2) {
        // Show the collection selection dialog
        CollectionSelectionDialog collSelDiag(NULL);
        collSelDiag.exec();

        // Was it rejected and what was the selected path
        if(collSelDiag.result() == QDialog::Rejected) {
            exit(0);
        }
        collectionPath = collSelDiag.getSelectedCollectionPath();
    } else {
        // Use the command line argument
        collectionPath = QString::fromLocal8Bit(argv[1]);
    }

//    // Construct the main window and make sure we have valid settings
//    PSHelperMainWindow myWindow;
//    if(!myWindow.validateSettings()) {
//        exit(-1);
//    }

//    // Prepare a progress dialog
//    QProgressDialog myProgressDiag("Scanning Files/Folders", "Cancel", 0, 0);
//    myProgressDiag.setWindowModality(WindowModality.WindowModal);
//    myProgressDiag.setWindowFlags(new WindowFlags(WindowType.Window, WindowType.WindowTitleHint, WindowType.CustomizeWindowHint));

//    // Scan the given directory for PSZ files and images
//    try {
//        // Use a future watcher to signal the progress dialog to close
//        QFutureWatcher<void> lWatcher;
//        lWatcher.progressRangeChanged.connect(myProgressDiag, "setRange(int, int)");
//        lWatcher.progressValueChanged.connect(myProgressDiag, "setValue(int)");
//        myProgressDiag.canceled.connect(lWatcher, "cancel()");
//        lWatcher.finished.connect(myProgressDiag, "reset()");

//        // Do the scanning in a separate thread with a future signal
//        mScanner = new PSandPhotoScanner(collectionPath, 1);
//        lWatcher.setFuture(mScanner.startScanParallel());

//        // Run dialog in a locally blocking event loop
//        myProgressDiag.exec();
//        if(lWatcher.isCanceled()) {
//            exit(1);
//        }

//        lWatcher.waitForFinished();
//        mScanner->finishDataParallel();
//    } catch(exception& e) {

//        QMessageBox.critical(null, "Error Scanning Files", "Error: failed to scan '" + collectionPath + "'.");

//        // Something went wrong scanning the given file/folder
//        cerr << "Error: failed to scan '" << collectionPath << "'." << endl;
//        cerr << "Exception: " << e.what() << endl;
//        exit(1);
//    }

//    // Setup the main GUI window
//    myWindow.setModelData(mScanner);
//    myWindow.show();

    // Start the main Qt event loop
    return app.exec();
}
