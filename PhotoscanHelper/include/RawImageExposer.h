#include <QFileInfo>
#include <QList>
#include <QString>
#include <QFuture>

#include "QueueableProcess.h"

class ExposureSettings;
class PSSessionData;

class RawImageExposer : public QueueableProcess<QFileInfo> {
public:
    RawImageExposer(PSSessionData &pProject, const ExposureSettings& pSettings, const QFileInfo& pDestination);
    RawImageExposer(PSSessionData &pProject, const ExposureSettings& pSettings);

    ~RawImageExposer();

    static void updateSettings(ExposureSettings* pNewSettings);

    QString describeProcess();
    QFuture<QFileInfo> runProcess();

    static QFileInfo map(QFileInfo pRawFile);

private:    
    const QString mProjectName;
    QList<QFileInfo> mRawFiles;

    static QFileInfo msDestination;
    static ExposureSettings* msSettings;
};
