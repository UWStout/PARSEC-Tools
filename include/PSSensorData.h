#ifndef PS_SENSOR_DATA_H
#define PS_SENSOR_DATA_H

#include <QString>
#include <QList>
#include <QVector>

class PSSensorData {
    public:
        const long ID;
        PSSensorData(long pID, QString pLabel);
        ~PSSensorData();
        static PSSensorData makeFromXML(QXMLReader reader);

        int getWidth();
        int getHeight();
        double getPixelWidth();
        double getPixelHeight();
        double getFocalLength();
        bool isFixed();
        double getFx();
        double getFy();
        double getCx();
        double getCy();
        double getB1();
        double getB2();
        double getSkew();
        double getK1();
        double getK2();
        double getK3();
        double getK4();
        double getP1();
        double getP2();
        double getP3();
        double getP4();
        QString getLabel();
        QString getType();
        QString getCovarianceParams();
        QList<QString> getBands();
        double* getCovarianceCoeffs();

    private:
        int mWidth, mHeight;
        double mPixelWidth, mPixelHeight, mFocalLength;
        bool mFixed;
        double mFx, mFy, mCx, mCy, mB1, mB2, mSkew;
        double mK1, mK2, mK3, mK4;
        double mP1, mP2, mP3, mP4;
        QString mLabel, mType;
        QString mCovarianceParams;
        QList<QString> mBands;
        QVector<double> mCovarianceCoeffs;
};

#endif
