#include <QString>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QtTest>

#include "PSProjectFileData.h"
#include "PSSensorData.h"
#include "PSCameraData.h"

Q_DECLARE_METATYPE(PSCameraData*)

class PSHTest_Test : public QObject
{
    Q_OBJECT

public:
    PSHTest_Test();

private slots:
    void sensorD_data();
    void sensorD();

    void cameraD_data();
    void cameraD();

    void fullXMLParsing_data();
    void fullXMLParsing();
};

/******************************/

PSHTest_Test::PSHTest_Test() {}

void PSHTest_Test::sensorD_data()
{
    QTest::addColumn<QFileInfo>("file");
    QTest::newRow("Sensor 0") << QFileInfo(":/PSHTest/Sensor0.xml");
}

void PSHTest_Test::sensorD()
{
    QFETCH(QFileInfo, file);
    QFile* lXMLFile = new QFile(file.absoluteFilePath());
    lXMLFile->open(QIODevice::ReadOnly);
    QXmlStreamReader* lXMLFileStream = new QXmlStreamReader(lXMLFile);

    PSSensorData* data = PSSensorData::makeFromXML(lXMLFileStream);
    Q_ASSERT(data != NULL);
    QCOMPARE(data->ID, 0);
    QCOMPARE(data->getLabel(), "X-M1 (50 mm)");
    QCOMPARE(data->getType(), "frame");
    QCOMPARE(data->getWidth(), 3288);
    QCOMPARE(data->getHeight(), 4952);
    QVERIFY(qFuzzyCompare(data->getFocalLength(), 5.0000000000000000e+01));
    QVERIFY(!data->isFixed());
    QVERIFY(qFuzzyCompare(data->getFx(), 1.0403074608966994e+04));
    QVERIFY(qFuzzyCompare(data->getFy(), 1.0402965711360808e+04));
    QVERIFY(qFuzzyCompare(data->getCx(), 1.5766459960826696e+03));
    QVERIFY(qFuzzyCompare(data->getCy(), 2.5143645976626149e+03));
    QVERIFY(qFuzzyCompare(data->getK1(), 6.5455032109959954e-02));
    QVERIFY(qFuzzyCompare(data->getK2(), -2.1264260096368184e+00));
    QVERIFY(qFuzzyCompare(data->getK3(), 2.7737688902037522e+01));
    QVERIFY(qFuzzyCompare(data->getP1(), 7.0870343682217255e-03));
    QVERIFY(qFuzzyCompare(data->getP2(), -2.2149281014183105e-03));

    delete data;
    delete lXMLFileStream;
    lXMLFile->close();
    delete lXMLFile;
}

void PSHTest_Test::cameraD_data()
{
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<PSCameraData*>("result");

    PSCameraData* c0 = new PSCameraData(0L);
    c0->setLabel("DSCF4912.tiff");
    c0->setSensoID(0);
    c0->setIsEnabled(true);
    double trans[16] = {
        -8.0239344571999230e-02, 7.4885843668942398e-01, -6.5785461112751042e-01,  1.3600633351819457e+01,
        -2.5441794677868296e-01, 6.2273584815027827e-01,  7.3991321909091001e-01, -1.8150333897639527e+01,
         9.6375990575431969e-01, 2.2674017118196288e-01,  1.4055510959335121e-01,  1.2300903360233244e+01,
         0.0000000000000000e+00, 0.0000000000000000e+00,  0.0000000000000000e+00,  1.0000000000000000e+00
    };
    c0->setTransform(trans);

    PSCameraData* c1 = new PSCameraData(1L);
    c1->setLabel("DSCF4913.tiff");
    c1->setSensoID(0);
    c1->setIsEnabled(true);
    double trans1[16] = {
         5.2480715041327675e-01,  6.0802100096347400e-01, -5.9572469922143689e-01,  1.3296461699719668e+01,
        -2.2648874138345818e-01,  7.7436174560542326e-01,  5.9081870059220276e-01, -1.5312462040338817e+01,
         8.2053659571138771e-01, -1.7514094133096642e-01,  5.4410049234316049e-01,  3.5501997344686425e+00,
         0.0000000000000000e+00,  0.0000000000000000e+00,  0.0000000000000000e+00,  1.0000000000000000e+00
    };
    c1->setTransform(trans1);

    PSCameraData* c2 = new PSCameraData(2L);
    c2->setLabel("DSCF4914.tiff");
    c2->setSensoID(0);
    c2->setIsEnabled(true);
    double trans2[16] = {
         8.8766968356074594e-01,  3.1260354177047422e-01, -3.3811471213143107e-01,  7.1769034885782705e+00,
        -1.0247214611005659e-01,  8.4995281526526656e-01,  5.1679751459759060e-01, -1.3521445549041344e+01,
         4.4893428490010023e-01, -4.2409814606433155e-01,  7.8651050237490949e-01, -1.7722972946925779e+00,
         0.0000000000000000e+00,  0.0000000000000000e+00,  0.0000000000000000e+00,  1.0000000000000000e+00
    };
    c2->setTransform(trans2);

    PSCameraData* c3 = new PSCameraData(3L);
    c3->setLabel("DSCF4915.tiff");
    c3->setSensoID(0);
    c3->setIsEnabled(true);
    double trans3[16] = {
         9.8886715822618010e-01,  8.5954809799989484e-02, -1.2146404428441569e-01,  2.4265412744562620e+00,
        -1.3163671369893409e-02,  8.6361741204896936e-01,  5.0397587577373781e-01, -1.3365675019697344e+01,
         1.4821761412782325e-01, -4.9676627932870820e-01,  8.5513437691638117e-01, -2.5676764953696791e+00,
         0.0000000000000000e+00,  0.0000000000000000e+00,  0.0000000000000000e+00,  1.0000000000000000e+00
    };
    c3->setTransform(trans3);

    PSCameraData* c4 = new PSCameraData(4L);
    c4->setLabel("DSCF4916.tiff");
    c4->setSensoID(0);
    c4->setIsEnabled(true);
    double trans4[16] = {
         8.7974084535545127e-01, -3.4445680520335697e-01, 3.2772786631956186e-01, -7.1781851757438080e+00,
         1.7945735349513550e-01,  8.7887583979873740e-01, 4.4200940769918695e-01, -1.2518371580971705e+01,
        -4.4028525218294334e-01, -3.3004055442802893e-01, 8.3499828092220718e-01, -4.2501427642255063e+00,
         0.0000000000000000e+00,  0.0000000000000000e+00, 0.0000000000000000e+00,  1.0000000000000000e+00
    };
    c4->setTransform(trans4);

    PSCameraData* c5 = new PSCameraData(5L);
    c5->setLabel("DSCF4917.tiff");
    c5->setSensoID(0);
    c5->setIsEnabled(true);
    double trans5[16] = {
         3.5093052738581565e-01, -6.8485508138172846e-01, 6.3860886499823932e-01, -1.3136793710989661e+01,
         2.8158788563800075e-01,  7.2759704955698967e-01, 6.2554839631948855e-01, -1.5703878867751349e+01,
        -8.9305992386324995e-01, -3.9699508581211573e-02, 4.4818291066011967e-01,  6.7039522633509270e+00,
         0.0000000000000000e+00,  0.0000000000000000e+00, 0.0000000000000000e+00,  1.0000000000000000e+00
    };
    c5->setTransform(trans5);

    QTest::newRow("Camera 0") << QFileInfo(":/PSHTest/Camera0.xml") << c0;
    QTest::newRow("Camera 1") << QFileInfo(":/PSHTest/Camera1.xml") << c1;
    QTest::newRow("Camera 2") << QFileInfo(":/PSHTest/Camera2.xml") << c2;
    QTest::newRow("Camera 3") << QFileInfo(":/PSHTest/Camera3.xml") << c3;
    QTest::newRow("Camera 4") << QFileInfo(":/PSHTest/Camera4.xml") << c4;
    QTest::newRow("Camera 5") << QFileInfo(":/PSHTest/Camera5.xml") << c5;
}

void PSHTest_Test::cameraD()
{
    // Fetch file info and read it
    QFETCH(QFileInfo, file);
    QFile* lXMLFile = new QFile(file.absoluteFilePath());
    lXMLFile->open(QIODevice::ReadOnly);
    QXmlStreamReader* lXMLFileStream = new QXmlStreamReader(lXMLFile);

    PSCameraData* data = PSCameraData::makeFromXML(lXMLFileStream);

    // Free file resources
    delete lXMLFileStream;
    lXMLFile->close();
    delete lXMLFile;

    // Compare everything
    Q_ASSERT(data != NULL);
    QFETCH(PSCameraData*, result);
    QCOMPARE(data->ID, result->ID);
    QCOMPARE(data->getLabel(), result->getLabel());
    QCOMPARE(data->getSensorID(), result->getSensorID());
    QCOMPARE(data->isEnabled(), result->isEnabled());
    const double *T0 = data->getTransform(), *T1 = result->getTransform();
    for(int i=0; i<16; i++) {
        QVERIFY(qFuzzyCompare(T0[i], T1[i]));
    }

    // Free dynamic memory
    delete result;
    delete data;
}

void PSHTest_Test::fullXMLParsing_data()
{
    QTest::addColumn<QFileInfo>("file");

    QTest::newRow("Basement - Blue Bird") << QFileInfo(":/PSHTest/BlueBird.xml");
    QTest::newRow("Stout - Chair")     << QFileInfo(":/PSHTest/Chair.xml");
    QTest::newRow("Xanscan - Robbery Gun") << QFileInfo(":/PSHTest/Gun.xml");
}

void PSHTest_Test::fullXMLParsing()
{
    QFETCH(QFileInfo, file);
    PSProjectFileData* data = new PSProjectFileData(file, NULL);
    Q_ASSERT(data != NULL);
    delete data;
}

QTEST_APPLESS_MAIN(PSHTest_Test)

#include "tst_pshtest_test.moc"
