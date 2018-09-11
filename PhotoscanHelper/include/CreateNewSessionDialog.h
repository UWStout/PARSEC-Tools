#ifndef CREATENEWSESSIONDIALOG_H
#define CREATENEWSESSIONDIALOG_H

#include "PSProjectDataModel.h"

#include <QDialog>
#include <QString>

namespace Ui {
class CreateNewSessionDialog;
}

class CreateNewSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateNewSessionDialog(const QString& pCollectionDir, PSProjectDataModel* pDataModel, QWidget *parent = 0);
    ~CreateNewSessionDialog();

    QString getSessionFolderName();

    QString getRawFolderPath();
    QString getProcessedFolderPath();
    QString getMasksFolderPath();

private:
    Ui::CreateNewSessionDialog* mGUI;
    const QString& mCollectionDir;
    const PSProjectDataModel* mDataModel;

    void updateImageFolderLineEdits();

private slots:
    void on_rawBrowseButton_clicked();
    void on_processedBrowseButton_clicked();
    void on_masksBrowseButton_clicked();

    void on_IDLineEdit_textEdited();
    void on_nameLineEdit_textEdited();

    void accept();
};

#endif // CREATENEWSESSIONDIALOG_H
