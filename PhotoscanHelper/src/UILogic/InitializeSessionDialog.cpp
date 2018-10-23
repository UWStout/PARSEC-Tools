#include "include\InitializeSessionDialog.h"
#include "ui_InitializeSessionDialog.h"

InitializeSessionDialog::InitializeSessionDialog(QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::InitializeSessionDialog();
    mGUI->setupUi(this);

    mGUI->buttonBox->buttons()[0]->setText("Convert");
    mGUI->buttonBox->addButton("Ignore", QDialogButtonBox::RejectRole);

    mGUI->questionLabel->setText("Would you like to convert this folder into a session?");
}

InitializeSessionDialog::~InitializeSessionDialog() {
    delete mGUI;
}

bool InitializeSessionDialog::getApplyToAll() {
    return mGUI->applyToAllCheckBox->isChecked();
}

void InitializeSessionDialog::setTitle(QString pText) {
    mGUI->titleLabel->setText(pText);
}

int InitializeSessionDialog::exec() {
    int lResult = QDialog::exec();
    if (lResult == QDialog::Accepted) {
        return static_cast<int>(QDialogButtonBox::Yes);
    } else {
        return static_cast<int>(QDialogButtonBox::Ignore);
    }
}
