#include "CollectionSelectionDialog.h"
#include "ui_CollectionSelectionDialog.h"

#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>
using namespace std;

CollectionSelectionDialog::CollectionSelectionDialog(QWidget* parent) : QDialog(parent) {
    mGUI = new Ui::CollectionSelectionDialog();
    mGUI->setupUi(this);
    mGUI->collectionsComboBox->setInsertPolicy(QComboBox::NoInsert);
    readPaths();

    connect(
        this, &CollectionSelectionDialog::accepted,
        this, &CollectionSelectionDialog::onAccepted
    );
}

CollectionSelectionDialog::~CollectionSelectionDialog() {
    delete mGUI;
}

QString CollectionSelectionDialog::getSelectedCollectionPath() {
    return mGUI->collectionsComboBox->currentText();
}

void CollectionSelectionDialog::on_browseButton_clicked() {
    QString newDir = QFileDialog::getExistingDirectory(parentWidget(),
                        "Select a collection directory", "");
    if(newDir != nullptr && !newDir.isEmpty()) {
        mGUI->collectionsComboBox->setEditText(newDir);
    }
}

void CollectionSelectionDialog::onAccepted() {
    storePaths();
}

void CollectionSelectionDialog::closeEvent(QCloseEvent* event) {
    // A manual close is the same as a reject
    this->reject();
    event->accept();
}

// Extract the paths from the properties and offer them
// as a dropdown list
void CollectionSelectionDialog::readPaths() {

    // Clear out the combobox
    mGUI->collectionsComboBox->clear();

    // Read any previously used folders and add to combobox
    QSettings settings;
    int colCount = settings.beginReadArray("Collection");
    for (int i=0; i<colCount; i++) {
        settings.setArrayIndex(i);
        QString folder = settings.value("Root Folder", "").toString();
        mGUI->collectionsComboBox->addItem(folder);
    }
    settings.endArray();

    // Read index of last selected collection
    int lastColl = settings.value("Last Collection", "0").toInt();
    if(mGUI->collectionsComboBox->count() > lastColl) {
        mGUI->collectionsComboBox->setCurrentIndex(lastColl);
    }
}

void CollectionSelectionDialog::storePaths() {

    // Read the global settings and open the collection array
    QSettings settings;
    settings.beginWriteArray("Collection");

    int curIndex = -1;
    QString displayedText = mGUI->collectionsComboBox->currentText();

    // Add all the options from the combo box back in the settings array
    for (int i=0; i<mGUI->collectionsComboBox->count(); i++) {
        QString folder = mGUI->collectionsComboBox->itemText(i);
        settings.setArrayIndex(i);
        settings.setValue("Root Folder", folder);

        // Is this the currently displayed text?
        if(folder == displayedText) {
            curIndex = i;
        }
    }

    // Is the displayed text a new folder?
    if(curIndex == -1) {
        curIndex = mGUI->collectionsComboBox->count();
        settings.setArrayIndex(curIndex);
        settings.setValue("Root Folder", displayedText);
    }

    // End the collection array
    settings.endArray();

    // Write the selected index
    settings.setValue("Last Collection", QString::number(curIndex));
}

