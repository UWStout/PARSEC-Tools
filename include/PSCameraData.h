//package edu.uwstout.berriers.PSHelper.Model;

//import javax.xml.stream.XMLStreamException;
//import javax.xml.stream.XMLStreamReader;

#include <QString>
#include <exception>
// Include QXML Reader goes here

#include "PSSensorData.h"
#include "PSImageData.h"

class PSCameraData {

    public:
        const long ID;

        PSCameraData(long pID);

        static PSCameraData makeFromXML(QXMLReader reader); // TODO

        QString getLabel();
        PSImageData getImageData();
        long getSensorID();
        PSSensorData getSensorData();

        bool isEnabled();
        bool isAligned();

        void setLabel(QString pLabel);
        void setImageData(PSImageData pImageData);
        void setIsEnabled(bool pEnabled);
        void setSensoID(long pSensorID);
        void setSensorData(PSSensorData pSensorData);


    private:
        QString mLabel;
        bool mEnabled;
        double mTransform[];

        long mSensorID;
        PSImageData mImageData;
        PSSensorData mSensorData;
};
