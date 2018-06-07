#include "PSSensorData.h"

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

    mBands = new QList<QString>();
}

PSSensorData::~PSSensorData(){

}

PSSensorData PSSensorData::makeFromXML(QXMLReader &reader) { // TODO
    // Sanity check
    if(reader == NULL || reader.getLocalName() != "sensor") {
        return NULL;
    }

    // Build the basic sensor
    PSSensorData newSensor = new PSSensorData(
            Long.parseLong(reader.getAttributeValue(null, "id")),
            reader.getAttributeValue(null, "label"));
    newSensor.mType = reader.getAttributeValue(null, "type");

    // Track XML state
    boolean lInsideCalib = false, lInsideBands = false, lInsideCovar = false;

    // Parse out internal sensor tag data
    try {
        while(reader.hasNext()) {
            reader.next();
            if(reader.isStartElement()) {
                switch(reader.getLocalName()) {
                    case "bands": lInsideBands = true; break;
                    case "calibration": lInsideCalib = true; break;
                    case "covariance": lInsideCovar = true; break;

                    case "resolution":
                        if(!lInsideCalib) {
                            newSensor.mWidth = Integer.parseInt(reader.getAttributeValue(null, "width"));
                            newSensor.mHeight = Integer.parseInt(reader.getAttributeValue(null, "height"));
                        }
                    break;

                    case "property":
                        double lValue = 0;
                        try { Double.parseDouble(reader.getAttributeValue(null, "value")); }
                        catch(Exception e) { }

                        switch(reader.getAttributeValue(null, "name")) {
                            case "pixel_width": newSensor.mPixelWidth = lValue; break;
                            case "pixel_height": newSensor.mPixelHeight = lValue; break;
                            case "focal_length": newSensor.mFocalLength = lValue; break;
                            case "fixed":
                                newSensor.mFixed = (
                                        reader.getAttributeValue(null, "value").equalsIgnoreCase("true"));
                            break;
                        }
                    break;

                    case "band":
                        if(lInsideBands) {
                            newSensor.mBands.add(reader.getAttributeValue(null, "label"));
                        }
                    break;

                    case "fx": newSensor.mFx = Double.parseDouble(reader.getElementText()); break;
                    case "fy": newSensor.mFy = Double.parseDouble(reader.getElementText()); break;

                    case "cx": newSensor.mCx = Double.parseDouble(reader.getElementText()); break;
                    case "cy": newSensor.mCy = Double.parseDouble(reader.getElementText()); break;

                    case "b1": newSensor.mB1 = Double.parseDouble(reader.getElementText()); break;
                    case "b2": newSensor.mB2 = Double.parseDouble(reader.getElementText()); break;

                    case "skew": newSensor.mSkew = Double.parseDouble(reader.getElementText()); break;

                    case "p1": newSensor.mP1 = Double.parseDouble(reader.getElementText()); break;
                    case "p2": newSensor.mP2 = Double.parseDouble(reader.getElementText()); break;
                    case "p3": newSensor.mP3 = Double.parseDouble(reader.getElementText()); break;
                    case "p4": newSensor.mP4 = Double.parseDouble(reader.getElementText()); break;

                    case "k1": newSensor.mK1 = Double.parseDouble(reader.getElementText()); break;
                    case "k2": newSensor.mK2 = Double.parseDouble(reader.getElementText()); break;
                    case "k3": newSensor.mK3 = Double.parseDouble(reader.getElementText()); break;
                    case "k4": newSensor.mK4 = Double.parseDouble(reader.getElementText()); break;

                    case "params":
                        if(lInsideCovar) {
                            newSensor.mCovarianceParams = reader.getElementText();
                        }
                    break;

                    case "coeffs":
                        if(lInsideCovar) {
                            QVector<QString> coeffs = reader.getElementText().split("\\s");
                            newSensor.mCovarianceCoeffs = new double[coeffs.length];
                            for(int i=0; i<coeffs.length; i++) {
                                newSensor.mCovarianceCoeffs[i] = Double.parseDouble(coeffs[i]);
                            }
                        }
                    break;
                }
            }

            // Calling getElementText advances to the end element so this should not be mutually exclusive
            if(reader.isEndElement()) {
                switch(reader.getLocalName()) {
                    case "calibration": lInsideCalib = false; break;
                    case "bands": lInsideBands = false; break;
                    case "covariance": lInsideCovar = false; break;
                    case "sensor": return newSensor;
                }
            }
        }
    } catch (XMLStreamException e) {
        throw e;
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
QList<QString> PSSensorData::getBands() { return mbands; }
double* PSSensorData::getCovarianceCoeffs() { return mCovarianceCoeffs; }
