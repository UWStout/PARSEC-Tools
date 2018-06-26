#include <QFileInfo>
#include <QList>
#include <QString>
#include <QFuture>

#include "QueueableProcess.h"

class ExposureSettings;
class PSSessionData;

class RawImageExposer : public QueueableProcess<QFileInfo> {
public:
    RawImageExposer(const PSSessionData &pProject, const ExposureSettings& pSettings, const QFileInfo& pDestination);
    RawImageExposer(const PSSessionData &pProject, const ExposureSettings& pSettings);

    ~RawImageExposer();

    QString describeProcess();
    QFuture<QFileInfo> runProcess();
    QFileInfo map(QFileInfo pRawFile);

private:
    const QFileInfo mDestination;
    const ExposureSettings* mSettings;
    const QString mProjectName;
    QList<QFileInfo> mRawFiles;
};
