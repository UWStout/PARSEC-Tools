#ifndef QUICKPREVIEWDIALOG_H
#define QUICKPREVIEWDIALOG_H

#include <QDialog>
#include <QFileInfo>

namespace Ui {
class QuickPreviewDialog;
}

class QuickPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickPreviewDialog(QWidget* parent = nullptr);
    ~QuickPreviewDialog();

    QFileInfo getMaskDir() const;
    int getTolerance() const;
    int getTextureSize() const;

private:
    Ui::QuickPreviewDialog *mGUI;

private slots:
    void on_browseButton_clicked();
};

#endif // QUICKPREVIEWDIALOG_H
