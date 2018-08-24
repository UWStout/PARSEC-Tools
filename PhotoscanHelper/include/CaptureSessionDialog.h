#ifndef CAPTURESESSIONDIALOG_H
#define CAPTURESESSIONDIALOG_H

#include <QDialog>

#include <PSSessionData.h>

namespace Ui {
class CaptureSessionDialog;
}

class CaptureSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaptureSessionDialog(PSSessionData* pCurSession, QWidget *parent = 0);
    ~CaptureSessionDialog();

private:
    Ui::CaptureSessionDialog* mGUI;
    PSSessionData* mCurSession;
};

#endif // CAPTURESESSIONDIALOG_H
