#ifndef PS_IMAGE_DATA_H
#define PS_IMAGE_DATA_H

#include "psdata_global.h"

#include <QString>
#include <QMap>

class QXmlStreamReader;
class PSCameraData;

class PSDATASHARED_EXPORT PSImageData {
public:
    PSImageData(long pCamID);
    ~PSImageData();

    static PSImageData* makeFromXML(QXmlStreamReader* reader);

    void addProperty(QString key, QString value);
    void setCameraData(PSCameraData* pCameraData);

    long getCamID();
    QString getFilePath();
    QString getProperty(QString key);
    PSCameraData* getCameraData();

private:
    long mCamID;
    QString mFilePath;
    QMap<QString, QString> mProperties;
    PSCameraData* mCameraData;
};

#endif
