#ifndef PS_SENSOR_DATA_H
#define PS_SENSOR_DATA_H

#include "psdata_global.h"

#include <QString>
#include <QList>

class QXmlStreamReader;

class PSDATASHARED_EXPORT PSSensorData {
public:
    PSSensorData(long pID, QString pLabel);
    ~PSSensorData();

    static PSSensorData* makeFromXML(QXmlStreamReader* reader);

    const long ID;
    int getWidth() const;
    int getHeight() const;
    double getPixelWidth() const;
    double getPixelHeight() const;
    double getFocalLength() const;
    bool isFixed() const;
    double getFx() const;
    double getFy() const;
    double getCx() const;
    double getCy() const;
    double getB1() const;
    double getB2() const;
    double getSkew() const;
    double getK1() const;
    double getK2() const;
    double getK3() const;
    double getK4() const;
    double getP1() const;
    double getP2() const;
    double getP3() const;
    double getP4() const;
    QString getLabel() const;
    QString getType() const;
    QString getCovarianceParams() const;
    QList<QString> getBands() const;
    const double* getCovarianceCoeffs() const;

    void setWidth(int pWidth);
    void setHeight(int pHeight);
    void setPixelWidth(double pPixelWidth);
    void setPixelHeight(double pPixelHeight);
    void setFocalLength(double pFocalLength);
    void setFixed(bool fixed);
    void setFx(double pFx);
    void setFy(double pFy);
    void setCx(double pCx);
    void setCy(double pCy);
    void setB1(double pB1);
    void setB2(double pB2);
    void setSkew(double pSkew);
    void setK1(double pK1);
    void setK2(double pK2);
    void setK3(double pK3);
    void setK4(double pK4);
    void setP1(double pP1);
    void setP2(double pP2);
    void setP3(double pP3);
    void setP4(double pP4);
    void setLabel(QString pLabel);
    void setType(QString pType);
    void setCovarianceParams(QString pParams);
    void setCovarianceCoeffs(const double* pCoeffs, int length);

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
    double* mCovarianceCoeffs;
};

#endif
