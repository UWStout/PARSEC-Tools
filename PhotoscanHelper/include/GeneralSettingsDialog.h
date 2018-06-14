#ifndef GENERAL_SETTINGS_DIALOG_H
#define GENERAL_SETTINGS_DIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QWidget>
#include <QString>

#include "PSSessionData.h"
#include "ui_GeneralSettingsDialog.h"

class GeneralSettingsDialog : public QDialog {
    Q_OBJECT

public:
    GeneralSettingsDialog(PSSessionData pData, QWidget* parent);
    ~GeneralSettingsDialog();

private:
    Ui_GeneralSettingsDialog* mGUI;
    PSSessionData mProjectData;

    void restoreSettings();

public slots:
    void on_buttonBox_clicked(QAbstractButton* pButton);
};

#endif
