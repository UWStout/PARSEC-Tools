#ifndef COLLECTION_SELECTION_DIALOG_H
#define COLLECTION_SELECTION_DIALOG_H

//package edu.uwstout.berriers.PSHelper.UILogic;

//import com.trolltech.qt.core.QSettings;
//import com.trolltech.qt.gui.QCloseEvent;
//import com.trolltech.qt.gui.QComboBox;
//import com.trolltech.qt.gui.QDialog;
//import com.trolltech.qt.gui.QFileDialog;
//import com.trolltech.qt.gui.QWidget;

//import edu.uwstout.berriers.PSHelper.UIForms.Ui_CollectionSelectionDialog;

#include <QDialog>

// Forward Declarations
class QCloseEvent;
namespace Ui {
    class CollectionSelectionDialog;
}

class CollectionSelectionDialog : public QDialog {
    Q_OBJECT

public:
    CollectionSelectionDialog(QWidget* parent);
    ~CollectionSelectionDialog();

    QString getSelectedCollectionPath();

private slots:
    void on_browseButton_clicked();
	
protected slots:
    void onAccepted();

protected:
    void closeEvent(QCloseEvent* event);

private:
    Ui::CollectionSelectionDialog* mGUI;

    void readPaths();
    void storePaths();
};

#endif
