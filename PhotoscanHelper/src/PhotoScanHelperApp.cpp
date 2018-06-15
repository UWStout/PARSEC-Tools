//#include "PSHelperMainWindow.h"
#include "CollectionSelectionDialog.h"
#include "PSHelperMainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QProgressDialog>

#include <string>
#include <iostream>
using namespace std;

#include "PSandPhotoScanner.h"

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

    // Construct the main window and make sure we have valid settings
    PSHelperMainWindow* myWindow = new PSHelperMainWindow(NULL);
    if(!myWindow->validateSettings()) {
        exit(-1);
    }

    // Prepare a progress dialog
    QProgressDialog* myProgressDiag = new QProgressDialog("Scanning Files/Folders", "Cancel", 0, 0);
    myProgressDiag->setWindowModality(Qt::WindowModal);
    myProgressDiag->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    // Scan the given directory for PSZ files and images
    PSandPhotoScanner* lScanner = NULL;
    try {
        // Use a future watcher to signal the progress dialog to close
        QFutureWatcher<PSSessionData*>* lWatcher = new QFutureWatcher<PSSessionData*>();
        QObject::connect(lWatcher, &QFutureWatcher<PSSessionData*>::progressRangeChanged,
                myProgressDiag, &QProgressDialog::setRange);
        QObject::connect(lWatcher, &QFutureWatcher<PSSessionData*>::progressValueChanged,
                myProgressDiag, &QProgressDialog::setValue);
        QObject::connect(lWatcher, &QFutureWatcher<PSSessionData*>::finished,
                myProgressDiag, &QProgressDialog::reset);
        QObject::connect(myProgressDiag, &QProgressDialog::canceled,
                lWatcher, &QFutureWatcher<PSSessionData*>::cancel);

        // Do the scanning in a separate thread with a future signal
        lScanner = new PSandPhotoScanner(collectionPath, 1);
        lWatcher->setFuture(lScanner->startScanParallel());

        // Run dialog in a locally blocking event loop
        myProgressDiag->exec();
        if(lWatcher->isCanceled()) {
            exit(1);
        }

        lWatcher->waitForFinished();
        lScanner->finishDataParallel();

        delete lWatcher;
        delete myProgressDiag;
    } catch(const exception& e) {

        QMessageBox::critical(NULL, "Error Scanning Files", "Error: failed to scan '" + collectionPath + "'.");

        // Something went wrong scanning the given file/folder
        qWarning("Error: failed to scan '%s'.", collectionPath.toLocal8Bit().data());
        qWarning("Exception: %s", e.what());
        exit(1);
    }

    // Setup the main GUI window
    myWindow->setModelData(lScanner);
    myWindow->show();

    // Start the main Qt event loop
    return app.exec();
    delete myWindow;
}
