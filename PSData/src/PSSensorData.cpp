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
    mCovarianceCoeffs = nullptr;
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
    if(reader == nullptr || reader->name() != "sensor") {
        return nullptr;
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
                    QString lText = reader->readElementText();
                    QVector<QStringRef> coeffs = lText.splitRef("\\s");
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
    return nullptr;
}

QString PSSensorData::getLabel() const { return mLabel; }
QString PSSensorData::getType() const { return mType; }

int PSSensorData::getWidth() const { return mWidth; }
int PSSensorData::getHeight() const { return mHeight; }

double PSSensorData::getPixelWidth() const { return mPixelWidth; }
double PSSensorData::getPixelHeight() const { return mPixelHeight; }

double PSSensorData::getFocalLength() const { return mFocalLength; }
bool PSSensorData::isFixed() const { return mFixed; }

double PSSensorData::getFx() const { return mFx; }
double PSSensorData::getFy() const { return mFy; }
double PSSensorData::getCx() const { return mCx; }
double PSSensorData::getCy() const { return mCy; }
double PSSensorData::getB1() const { return mB1; }
double PSSensorData::getB2() const { return mB2; }

double PSSensorData::getSkew() const { return mSkew; }

double PSSensorData::getK1() const { return mK1; }
double PSSensorData::getK2() const { return mK2; }
double PSSensorData::getK3() const { return mK3; }
double PSSensorData::getK4() const { return mK4; }

double PSSensorData::getP1() const { return mP1; }
double PSSensorData::getP2() const { return mP2; }
double PSSensorData::getP3() const { return mP3; }
double PSSensorData::getP4() const { return mP4; }

QString PSSensorData::getCovarianceParams() const { return mCovarianceParams; }
QList<QString> PSSensorData::getBands() const { return mBands; }
const double* PSSensorData::getCovarianceCoeffs() const { return mCovarianceCoeffs; }

void PSSensorData::setLabel(QString pLabel) { mLabel = pLabel; }
void PSSensorData::setType(QString pType) { mType = pType; }

void PSSensorData::setWidth(int pWidth) { mWidth = pWidth; }
void PSSensorData::setHeight(int pHeight) { mHeight = pHeight; }

void PSSensorData::setPixelWidth(double pPixelWidth) { mPixelWidth = pPixelWidth; }
void PSSensorData::setPixelHeight(double pPixelHeight) { mPixelHeight = pPixelHeight; }

void PSSensorData::setFocalLength(double pFocalLength) { mFocalLength = pFocalLength; }
void PSSensorData::setFixed(bool pFixed) { mFixed = pFixed; }

void PSSensorData::setFx(double pFx) { mFx = pFx; }
void PSSensorData::setFy(double pFy) { mFy = pFy; }

void PSSensorData::setCx(double pCx) { mCx = pCx; }
void PSSensorData::setCy(double pCy) { mCy = pCy; }

void PSSensorData::setB1(double pB1) { mB1 = pB1; }
void PSSensorData::setB2(double pB2) { mB2 = pB2; }

void PSSensorData::setSkew(double pSkew) { mSkew = pSkew; }

void PSSensorData::setK1(double pK1) { mK1 = pK1; }
void PSSensorData::setK2(double pK2) { mK2 = pK2; }
void PSSensorData::setK3(double pK3) { mK3 = pK3; }
void PSSensorData::setK4(double pK4) { mK4 = pK4; }

void PSSensorData::setP1(double pP1) { mP1 = pP1; }
void PSSensorData::setP2(double pP2) { mP2 = pP2; }
void PSSensorData::setP3(double pP3) { mP3 = pP3; }
void PSSensorData::setP4(double pP4) { mP4 = pP4; }

void PSSensorData::setCovarianceParams(QString pParams) { mCovarianceParams = pParams; }
void PSSensorData::setCovarianceCoeffs(const double* pCoeffs, int length) {
    delete mCovarianceCoeffs;
    mCovarianceCoeffs = new double[length];
    for(int i=0; i<length; i++) {
        mCovarianceCoeffs[i] = pCoeffs[i];
    }
}
