#ifndef QUICKPREVIEWDIALOG_H
#define QUICKPREVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class QuickPreviewDialog;
}

class QuickPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickPreviewDialog(QWidget *parent = 0);
    ~QuickPreviewDialog();

private:
    Ui::QuickPreviewDialog *ui;
};

#endif // QUICKPREVIEWDIALOG_H
