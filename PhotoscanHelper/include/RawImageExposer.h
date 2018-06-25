#include <QtConcurrent>
#include <QFile>
#include <QFileInfo>
#include <QVector>
#include <QString>
#include <QFuture>

#include "QueueableProcess.h"
#include "ExposureSettings.h"
#include "PSSessionData.h"

class RawImageExposer : QueueableProcess<QFile*> {
public:
    RawImageExposer(PSSessionData pProject, ExposureSettings pSettings, QFileInfo pDestination);
    ~RawImageExposer();

    QString describeProcess();
    QFuture<QFileInfo> runProcess();
    QFileInfo map(QFileInfo pRawFile);

private:
    const QFileInfo mDestination;
    const ExposureSettings mSettings;
    const QString mProjectName;
    QVector<QFileInfo> mRawFiles;
};
