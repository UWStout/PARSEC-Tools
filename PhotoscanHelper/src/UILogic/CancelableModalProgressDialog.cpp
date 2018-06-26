#include "CancelableModalProgressDialog.h"

InternalModalProgressDialog::InternalModalProgressDialog(QString pLabelText, QWidget* parent) : QDialog(parent) {
    // Setup the GUI
    mGUI = new Ui_CancelableProgressDialog();
    mGUI->setupUi(this);

    mGUI->ProgressBar->reset();
    mGUI->DescriptionLabel->setText(pLabelText);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
}

InternalModalProgressDialog::~InternalModalProgressDialog() {
    delete mGUI;
}

void InternalModalProgressDialog::processFinished() {}
void InternalModalProgressDialog::on_CancelButtonClicked() {}
