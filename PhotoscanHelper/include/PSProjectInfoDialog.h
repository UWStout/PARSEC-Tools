#ifndef PS_PROJECT_INFO_DIALOG_H
#define PS_PROJECT_INFO_DIALOG_H

#include <QDialog>

class PSSessionData;

namespace Ui {
    class PSProjectInfoDialog;
}
class PSSessionData;

class PSProjectInfoDialog : public QDialog {
    Q_OBJECT

public:
    PSProjectInfoDialog(const PSSessionData* pProjData, QWidget* parent);
    ~PSProjectInfoDialog();

    void setProjectData(const PSSessionData* pProjData);

private:
    Ui::PSProjectInfoDialog* mGUI;
};

#endif
