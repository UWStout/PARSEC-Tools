#ifndef CREATENEWSESSIONDIALOG_H
#define CREATENEWSESSIONDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class CreateNewSessionDialog;
}

class CreateNewSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateNewSessionDialog(QWidget *parent = 0);
    ~CreateNewSessionDialog();

    QString getFolderName();

private:
    Ui::CreateNewSessionDialog* mGUI;
};

#endif // CREATENEWSESSIONDIALOG_H
