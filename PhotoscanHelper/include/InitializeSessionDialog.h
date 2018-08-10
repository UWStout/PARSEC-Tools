#ifndef INITIALIZESESSIONDIALOG_H
#define INITIALIZESESSIONDIALOG_H

#include <QDialog>

namespace Ui {
class InitializeSessionDialog;
}

class InitializeSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitializeSessionDialog(QWidget *parent = 0);
    ~InitializeSessionDialog();

    bool getApplyToAll();
    void setTitle(QString pText);

public slots:
    int exec();

private:
    Ui::InitializeSessionDialog* mGUI;
};

#endif // INITIALIZESESSIONDIALOG_H
