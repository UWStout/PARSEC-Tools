#ifndef PS_PROJECT_INFO_DIALOG_H
#define PS_PROJECT_INFO_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QString>

#include "PSChunkData.h"
#include "PSSessionData.h"
#include "ui_PSChunkInfo.h"
#include "ui_PSProjectInfo.h"

class PSProjectInfoDialog : public QDialog {
    Q_OBJECT

public:
    PSProjectInfoDialog(PSSessionData pProjData, QWidget* parent);
    ~PSProjectInfoDialog();

    void setProjectData(PSSessionData pProjData);

private:
    Ui_PSProjectInfoDialog* mGUI;
};

#endif