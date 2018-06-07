#ifndef PS_CAMERA_DATA_H
#define PS_CAMERA_DATA_H

#include <QString>
#include <QVector>
#include <exception>
// Include QXML Reader goes here

#include "PSSensorData.h"
#include "PSImageData.h"

class PSCameraData {

    public:
        const long ID;

        PSCameraData(const long pID);
        ~PSCameraData();

        static PSCameraData makeFromXML(QXMLReader reader); // TODO

        QString getLabel();
        PSImageData *getImageData();
        long getSensorID();
        PSSensorData *getSensorData();

        bool isEnabled();
        bool isAligned();

        void setLabel(QString pLabel);
        void setImageData(PSImageData *pImageData);
        void setIsEnabled(bool pEnabled);
        void setSensoID(long pSensorID);
        void setSensorData(PSSensorData *pSensorData);


    private:
        QString mLabel;
        bool mEnabled;
        QVector<double> mTransform;

        long mSensorID;
        PSImageData *mImageData;
        PSSensorData *mSensorData;
};

#endif
