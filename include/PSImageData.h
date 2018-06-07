//package edu.uwstout.berriers.PSHelper.Model;

//import java.util.HashMap;
//import java.util.Map;

//import javax.xml.stream.XMLStreamException;
//import javax.xml.stream.XMLStreamReader;

#include <QString>
#include <QMap>
// Include for QXML Reader goes here

#include "PSCameraData.h"

class PSImageData {

    private:
        long mCamID;
        QString mFilePath;
        QMap<QString, QString> mProperties;
        PSCameraData mCameraData;

    public:
        PSImageData(long pCamID);

        void addProperty(QString key, QString value);
        void setCameraData(PSCameraData pCameraData);

        long getCamID();
        QString getFilePath();
        QString getProperty(QString key);
        PSCameraData getCameraData();

        static PSImageData makeFromXML(QXMLStreamReader reader); // TODO
};
