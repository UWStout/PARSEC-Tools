#ifndef PS_CAMERA_DATA_H
#define PS_CAMERA_DATA_H

#include "psdata_global.h"

#include <QString>

// Forward declarations
class QXmlStreamReader;
class PSSensorData;
class PSImageData;

class PSDATASHARED_EXPORT PSCameraData {
public:
    const long ID;

    PSCameraData(const long pID);
    ~PSCameraData();

    static PSCameraData* makeFromXML(QXmlStreamReader* reader);

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
    double* mTransform;

    long mSensorID;
    PSImageData *mImageData;
    PSSensorData *mSensorData;
};

#endif
