#include "PSSensorData.h"

#include <QXmlStreamReader>

PSSensorData::PSSensorData(long pID, QString pLabel) : ID(pID) {
    mLabel = pLabel;
    mType = "";

    mWidth = mHeight = 0;
    mPixelWidth = mPixelHeight = mFocalLength = 0.0;
    mFixed = false;

    mFx = mFy = mCx = mCy = mB1 = mB2 = mSkew = 1.0;
    mK1 = mK2 = mK3 = mK4 = mP1 = mP2 = mP3 = mP4 = 1.0;

    mCovarianceParams = "";
    mCovarianceCoeffs = NULL;
}

PSSensorData::~PSSensorData() {}

PSSensorData* PSSensorData::makeFromXML(QXmlStreamReader* reader) {
    // If this is a fresh XML doc, push to first non-document tag.
    while(reader->tokenType() == QXmlStreamReader::NoToken ||
          reader->tokenType() == QXmlStreamReader::StartDocument ||
          reader->name() == "document") {
        reader->readNextStartElement();
    }

    // Sanity check
    if(reader == NULL || reader->name() != "sensor") {
        return NULL;
    }

    // Build the basic sensor
    PSSensorData* newSensor = new PSSensorData(
            reader->attributes().value("", "id").toLong(),
            reader->attributes().value("", "label").toString());
    newSensor->mType = reader->attributes().value("", "type").toString();

    // Track XML state
    bool lInsideCalib = false, lInsideBands = false, lInsideCovar = false;

    // Parse out internal sensor tag data
    try {
        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement()) {
                if(reader->name() == "bands") { lInsideBands = true; }
                else if(reader->name() == "calibration") { lInsideCalib = true; }
                else if(reader->name() == "covariance") { lInsideCovar = true; }

                else if(reader->name() == "resolution" && !lInsideCalib) {
                    newSensor->mWidth = reader->attributes().value("", "width").toInt();
                    newSensor->mHeight = reader->attributes().value("", "height").toInt();
                }

                else if(reader->name() == "property") {
                    QStringRef lPropName = reader->attributes().value("", "name");
                    if (lPropName == "fixed") {
                        newSensor->mFixed = (reader->attributes().value("", "value") == "true");
                    } else {
                        double lValue = reader->attributes().value("", "value").toDouble();
                        if (lPropName == "pixel_width") { newSensor->mPixelWidth = lValue; }
                        else if (lPropName == "pixel_height") { newSensor->mPixelHeight = lValue; }
                        else if (lPropName == "focal_length") { newSensor->mFocalLength = lValue; }
                    }
                }

                else if(reader->name() == "band" && lInsideBands) {
                    newSensor->mBands.append(reader->attributes().value("", "label").toString());
                }

                else if(reader->name() == "fx") { newSensor->mFx = reader->readElementText().toDouble(); }
                else if(reader->name() == "fy") { newSensor->mFy = reader->readElementText().toDouble(); }

                else if(reader->name() == "cx") { newSensor->mCx = reader->readElementText().toDouble(); }
                else if(reader->name() == "cy") { newSensor->mCy = reader->readElementText().toDouble(); }

                else if(reader->name() == "b1") { newSensor->mB1 = reader->readElementText().toDouble(); }
                else if(reader->name() == "b2") { newSensor->mB2 = reader->readElementText().toDouble(); }

                else if(reader->name() == "skew") { newSensor->mSkew = reader->readElementText().toDouble(); }

                else if(reader->name() == "p1") { newSensor->mP1 = reader->readElementText().toDouble(); }
                else if(reader->name() == "p2") { newSensor->mP2 = reader->readElementText().toDouble(); }
                else if(reader->name() == "p3") { newSensor->mP3 = reader->readElementText().toDouble(); }
                else if(reader->name() == "p4") { newSensor->mP4 = reader->readElementText().toDouble(); }

                else if(reader->name() == "k1") { newSensor->mK1 = reader->readElementText().toDouble(); }
                else if(reader->name() == "k2") { newSensor->mK2 = reader->readElementText().toDouble(); }
                else if(reader->name() == "k3") { newSensor->mK3 = reader->readElementText().toDouble(); }
                else if(reader->name() == "k4") { newSensor->mK4 = reader->readElementText().toDouble(); }

                else if(reader->name() == "params" && lInsideCovar) {
                    newSensor->mCovarianceParams = reader->readElementText().toDouble();
                }

                else if(reader->name() == "coeffs" && lInsideCovar) {
                    QVector<QStringRef> coeffs = reader->readElementText().splitRef("\\s");
                    newSensor->mCovarianceCoeffs = new double[coeffs.length()];
                    for(int i=0; i<coeffs.length(); i++) {
                        newSensor->mCovarianceCoeffs[i] = coeffs[i].toDouble();
                    }
                }
            }

            // Calling readElementText() might advance to the end element so this should not be mutually exclusive
            if(reader->isEndElement()) {
                if (reader->name() == "calibration") { lInsideCalib = false; }
                else if (reader->name() == "bands") { lInsideBands = false; }
                else if (reader->name() == "covariance") { lInsideCovar = false; }
                else if (reader->name() == "sensor") { return newSensor; }
            }
        }
    } catch (...) {
        throw new std::logic_error("Error parsing XML sensor tag to PSSensorData");
    }

    // Should never reach this except when XML is malformed
    return NULL;
}

int PSSensorData::getWidth() { return mWidth; }
int PSSensorData::getHeight() { return mHeight; }

double PSSensorData::getPixelWidth() { return mPixelWidth; }
double PSSensorData::getPixelHeight() { return mPixelHeight; }
double PSSensorData::getFocalLength() { return mFocalLength; }

bool PSSensorData::isFixed() { return mFixed; }

double PSSensorData::getFx() { return mFx; }
double PSSensorData::getFy() { return mFy; }
double PSSensorData::getCx() { return mCx; }
double PSSensorData::getCy() { return mCy; }
double PSSensorData::getB1() { return mB1; }
double PSSensorData::getB2() { return mB2; }

double PSSensorData::getSkew() { return mSkew; }

double PSSensorData::getK1() { return mK1; }
double PSSensorData::getK2() { return mK2; }
double PSSensorData::getK3() { return mK3; }
double PSSensorData::getK4() { return mK4; }

double PSSensorData::getP1() { return mP1; }
double PSSensorData::getP2() { return mP2; }
double PSSensorData::getP3() { return mP3; }
double PSSensorData::getP4() { return mP4; }

QString PSSensorData::getLabel() { return mLabel; }
QString PSSensorData::getType() { return mType; }

QString PSSensorData::getCovarianceParams() { return mCovarianceParams; }
QList<QString> PSSensorData::getBands() { return mBands; }
double* PSSensorData::getCovarianceCoeffs() { return mCovarianceCoeffs; }
