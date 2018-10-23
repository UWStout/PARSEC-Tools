#include "include\CaptureSessionDialog.h"
#include "ui_CaptureSessionDialog.h"

CaptureSessionDialog::CaptureSessionDialog(PSSessionData* pCurSession, QWidget *parent) : QDialog(parent) {
    mGUI = new Ui::CaptureSessionDialog();
    mGUI->setupUi(this);

    mCurSession = pCurSession;

    mGUI->rawFilesLineEdit->setText(mCurSession->getRawFolder().absolutePath());
}

CaptureSessionDialog::~CaptureSessionDialog()
{
    delete mGUI;
}
