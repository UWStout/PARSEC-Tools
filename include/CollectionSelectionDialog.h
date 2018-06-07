#ifndef COLLECTION_SELECTION_DIALOG_H
#define COLLECTION_SELECTION_DIALOG_H

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
