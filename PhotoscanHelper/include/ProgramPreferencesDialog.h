#include <QSettings>
#include <QString>

#include "ui_ProgramPreferencesDialog.h"

class ProgramPreferencesDialog : public QDialog {
    Q_OBJECT
public:
    ProgramPreferencesDialog(QSettings* infoStore, QWidget* parent);
    ~ProgramPreferencesDialog();

    void writeResults(QSettings* inforStore);
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
