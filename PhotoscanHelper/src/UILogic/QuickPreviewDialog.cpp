#include "QuickPreviewDialog.h"
#include "ui_QuickPreviewDialog.h"

QuickPreviewDialog::QuickPreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuickPreviewDialog)
{
    ui->setupUi(this);
}

QuickPreviewDialog::~QuickPreviewDialog()
{
    delete ui;
}
