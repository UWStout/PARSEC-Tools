#ifndef PROGRAM_PREFERENCES_DIALOG_H
#define PROGRAM_PREFERENCES_DIALOG_H

#include <QSettings>
#include <QAbstractButton>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <QString>

#include "ui_ProgramPreferencesDialog.h"
#include "ImageProcessorIM4J.h"

class ProgramPreferencesDialog : public QDialog {
    Q_OBJECT

public:
    ProgramPreferencesDialog(QSettings* infoStore, QWidget* parent);
    ~ProgramPreferencesDialog();
    void writeResults(QSettings* infoStore);
    bool validateSearchPath();


private:
    Ui_ProgramPreferencesDialog* prefsUI;

    void updateIM4JPathSettings();
    QString pickExecutableBinary(QString caption);

public slots:
    void on_buttonBox_clicked(QAbstractButton* pButton);

private slots:
    void on_exiftoolBrowseButton_clicked();
};

#endif
