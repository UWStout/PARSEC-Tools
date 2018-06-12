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

    QString getLabel() const;
    long getSensorID() const;
    PSImageData *getImageData();
    PSSensorData *getSensorData();
    bool isEnabled() const;
    bool isAligned() const;

    void setLabel(QString pLabel);
    void setIsEnabled(bool pEnabled);

    void setSensoID(long pSensorID);
    void setImageData(PSImageData *pImageData);
    void setSensorData(PSSensorData *pSensorData);

    void setTransform(const double pTransform[16]);
    const double* getTransform() const;

private:
    QString mLabel;
    bool mEnabled;
    double* mTransform;

    long mSensorID;
    PSImageData *mImageData;
    PSSensorData *mSensorData;
};

#endif
