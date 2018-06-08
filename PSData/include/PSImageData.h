#ifndef PS_IMAGE_DATA_H
#define PS_IMAGE_DATA_H

#include "psdata_global.h"

#include <QString>
#include <QMap>

class QXmlStreamReader;
class PSCameraData;

class PSDATASHARED_EXPORT PSImageData {

    private:
        long mCamID;
        QString mFilePath;
        QMap<QString, QString> mProperties;
        PSCameraData* mCameraData;

    public:
        PSImageData(long pCamID);
        ~PSImageData();

        void addProperty(QString key, QString value);
        void setCameraData(PSCameraData* pCameraData);

        long getCamID();
        QString getFilePath();
        QString getProperty(QString key);
        PSCameraData* getCameraData();

        static PSImageData* makeFromXML(QXmlStreamReader* reader);
};

#endif
