#include <QString>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QtTest>

#include <fstream>
#include <exception>

#include <quazip/quazipfile.h>

#ifdef _WIN32
#pragma warning(push)
#pragma warning( disable : 4100 )
#elif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include <ply_impl.h>

namespace PLY {
    const char* Vertex::name = "vertex";
    const Property Vertex::prop_x = Property("x", SCALAR, Float32);
    const Property Vertex::prop_y = Property("y", SCALAR, Float32);
    const Property Vertex::prop_z = Property("z", SCALAR, Float32);
    const Property VertexColor::prop_r = Property("red", SCALAR, Float32);
    const Property VertexColor::prop_g = Property("green", SCALAR, Float32);
    const Property VertexColor::prop_b = Property("blue", SCALAR, Float32);

    const char* Face::name = "face";
    const Property Face::prop_ind = Property("vertex_indices", LIST, Uint32, Uint8);
    const Property FaceTex::prop_tex = Property("texcoord", LIST, Float32, Uint8);
} // namespace PLY

#include <io.h>

#ifdef _WIN32
#pragma warning(pop)
#elif
#pragma clang diagnostic pop
#endif

#include <PSProjectFileData.h>

#include <PSSensorData.h>
#include <PSCameraData.h>
#include <PSImageData.h>
#include <PSModelData.h>
#include <PSChunkData.h>

// So we can put these in as data rows
Q_DECLARE_METATYPE(PSSensorData*)
Q_DECLARE_METATYPE(PSCameraData*)
Q_DECLARE_METATYPE(PSImageData*)
Q_DECLARE_METATYPE(PSChunkData*)
Q_DECLARE_METATYPE(PSModelData*)

using namespace std;

class PSHTest_Test : public QObject
{
    Q_OBJECT

public:
    PSHTest_Test();

private slots:
    void initTestCase();

    void plyZipTest_data();
    void plyZipTest();

    void sensorDataParsing_data();
    void sensorDataParsing();

    void imageDataParsing_data();
    void imageDataParsing();

    void cameraDataParsing_data();
    void cameraDataParsing();

    void modelDataParsing_data();
    void modelDataParsing();

    void chunkParsing_data();
    void chunkParsing();

    void fullXMLParsing_data();
    void fullXMLParsing();

    void cleanupTestCase();

private:
    PSSensorData *s0, *s1, *s2;
    PSImageData *i0, *i1, *i2, *i3, *i4, *i5;
    PSCameraData *c0, *c1, *c2, *c3, *c4, *c5;
    PSModelData *m0, *m1, *m2;
    PSChunkData *k0;
};

/******************************/
PSHTest_Test::PSHTest_Test() {}

void PSHTest_Test::initTestCase() {
    // Build the test sensors
    s0 = new PSSensorData(0L, "X-M1 (50 mm)");
    s0->setType("frame");
    s0->setWidth(3288);
    s0->setHeight(4952);
    s0->setFocalLength(50);
    s0->setFixed(false);
    s0->setFx(1.0403074608966994e+04);
    s0->setFy(1.0402965711360808e+04);
    s0->setCx(1.5766459960826696e+03);
    s0->setCy(2.5143645976626149e+03);
    s0->setK1(6.5455032109959954e-02);
    s0->setK2(-2.1264260096368184e+00);
    s0->setK3(2.7737688902037522e+01);
    s0->setP1(7.0870343682217255e-03);
    s0->setP2(-2.2149281014183105e-03);

    s1 = new PSSensorData(1L, "NEX-7 (35 mm)");
    s1->setType("frame");
    s1->setWidth(6000);
    s1->setHeight(4000);
    s1->setPixelWidth(4.0384615384615376e-03);
    s1->setPixelHeight(4.0384615384615376e-03);
    s1->setFocalLength(35);
    s1->setFixed(true);
    s1->setFx(9.0197125608661372e+03);
    s1->setFy(9.0286015344273928e+03);
    s1->setCx(2.9123942196099242e+03);
    s1->setCy(1.9306826870411276e+03);
    s1->setSkew(7.1131437938187536e-01);
    s1->setK1(1.2453126903681204e-01);
    s1->setK2(6.4129126635342037e-01);
    s1->setK3(-1.1363499220427324e+00);
    s1->setP1(-7.7480326210287486e-03);
    s1->setP2(-6.7937787244356914e-03);

    s2 = new PSSensorData(2L, "Canon EOS REBEL T3i (30 mm)");
    s2->setType("frame");
    s2->setWidth(5202);
    s2->setHeight(3465);
    s2->setPixelWidth(4.4035976380886795e-03);
    s2->setPixelHeight(4.4035976380886795e-03);
    s2->setFocalLength(30);
    s2->setFixed(false);
    s2->setFx(6.9602386774158076e+03);
    s2->setFy(6.9660209910539470e+03);
    s2->setCx(2.6108641630594730e+03);
    s2->setCy(1.8422692572272131e+03);
    s2->setK1(-8.7675575545409945e-02);
    s2->setK2(2.4327078427185639e-01);
    s2->setK3(-2.7655927308161071e-01);
    s2->setP1(2.7942111913686661e-03);
    s2->setP2(1.2707599251386176e-03);

    // Build the test images
    i0 = new PSImageData(0L, "DSCF4912.tiff");
    i0->addProperty("Exif/Artist", "");
    i0->addProperty("Exif/DateTime", "2015:02:28 23:23:56");
    i0->addProperty("Exif/ExposureTime", "6.49802");
    i0->addProperty("Exif/FNumber", "21.8566");
    i0->addProperty("Exif/FocalLength", "50");
    i0->addProperty("Exif/ISOSpeedRatings", "400");
    i0->addProperty("Exif/Make", "Fujifilm");
    i0->addProperty("Exif/Model", "X-M1");
    i0->addProperty("Exif/Software", "dcraw v9.23");
    i0->addProperty("File/ImageHeight", "4952");
    i0->addProperty("File/ImageWidth", "3288");
    i0->addProperty("System/FileModifyDate", "2015:03:02 16:14:13");
    i0->addProperty("System/FileSize", "48848380");

    i1 = new PSImageData(1L, "DSCF4913.tiff");
    i1->addProperty("Exif/Artist", "");
    i1->addProperty("Exif/DateTime", "2015:02:28 23:24:17");
    i1->addProperty("Exif/ExposureTime", "6.49802");
    i1->addProperty("Exif/FNumber", "21.8566");
    i1->addProperty("Exif/FocalLength", "50");
    i1->addProperty("Exif/ISOSpeedRatings", "400");
    i1->addProperty("Exif/Make", "Fujifilm");
    i1->addProperty("Exif/Model", "X-M1");
    i1->addProperty("Exif/Software", "dcraw v9.23");
    i1->addProperty("File/ImageHeight", "4952");
    i1->addProperty("File/ImageWidth", "3288");
    i1->addProperty("System/FileModifyDate", "2015:03:02 16:14:27");
    i1->addProperty("System/FileSize", "48848380");

    i2 = new PSImageData(2L, "DSCF4914.tiff");
    i2->addProperty("Exif/Artist", "");
    i2->addProperty("Exif/DateTime", "2015:02:28 23:24:36");
    i2->addProperty("Exif/ExposureTime", "6.49802");
    i2->addProperty("Exif/FNumber", "21.8566");
    i2->addProperty("Exif/FocalLength", "50");
    i2->addProperty("Exif/ISOSpeedRatings", "400");
    i2->addProperty("Exif/Make", "Fujifilm");
    i2->addProperty("Exif/Model", "X-M1");
    i2->addProperty("Exif/Software", "dcraw v9.23");
    i2->addProperty("File/ImageHeight", "4952");
    i2->addProperty("File/ImageWidth", "3288");
    i2->addProperty("System/FileModifyDate", "2015:03:02 16:14:40");
    i2->addProperty("System/FileSize", "48848380");

    i3 = new PSImageData(3L, "DSCF4915.tiff");
    i3->addProperty("Exif/Artist", "");
    i3->addProperty("Exif/DateTime", "2015:02:28 23:25:37");
    i3->addProperty("Exif/ExposureTime", "6.49802");
    i3->addProperty("Exif/FNumber", "21.8566");
    i3->addProperty("Exif/FocalLength", "50");
    i3->addProperty("Exif/ISOSpeedRatings", "400");
    i3->addProperty("Exif/Make", "Fujifilm");
    i3->addProperty("Exif/Model", "X-M1");
    i3->addProperty("Exif/Software", "dcraw v9.23");
    i3->addProperty("File/ImageHeight", "4952");
    i3->addProperty("File/ImageWidth", "3288");
    i3->addProperty("System/FileModifyDate", "2015:03:02 16:14:54");
    i3->addProperty("System/FileSize", "48848380");

    i4 = new PSImageData(4L, "DSCF4916.tiff");
    i4->addProperty("Exif/Artist", "");
    i4->addProperty("Exif/DateTime", "2015:02:28 23:26:06");
    i4->addProperty("Exif/ExposureTime", "6.49802");
    i4->addProperty("Exif/FNumber", "21.8566");
    i4->addProperty("Exif/FocalLength", "50");
    i4->addProperty("Exif/ISOSpeedRatings", "400");
    i4->addProperty("Exif/Make", "Fujifilm");
    i4->addProperty("Exif/Model", "X-M1");
    i4->addProperty("Exif/Software", "dcraw v9.23");
    i4->addProperty("File/ImageHeight", "4952");
    i4->addProperty("File/ImageWidth", "3288");
    i4->addProperty("System/FileModifyDate", "2015:03:02 16:15:07");
    i4->addProperty("System/FileSize", "48848380");

    i5 = new PSImageData(5L, "DSCF4917.tiff");
    i5->addProperty("Exif/Artist", "");
    i5->addProperty("Exif/DateTime", "2015:02:28 23:26:45");
    i5->addProperty("Exif/ExposureTime", "6.49802");
    i5->addProperty("Exif/FNumber", "21.8566");
    i5->addProperty("Exif/FocalLength", "50");
    i5->addProperty("Exif/ISOSpeedRatings", "400");
    i5->addProperty("Exif/Make", "Fujifilm");
    i5->addProperty("Exif/Model", "X-M1");
    i5->addProperty("Exif/Software", "dcraw v9.23");
    i5->addProperty("File/ImageHeight", "4952");
    i5->addProperty("File/ImageWidth", "3288");
    i5->addProperty("System/FileModifyDate", "2015:03:02 16:15:20");
    i5->addProperty("System/FileSize", "48848380");

    // Build test camera data
    c0 = new PSCameraData(0L);
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

    c1 = new PSCameraData(1L);
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

    c2 = new PSCameraData(2L);
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

    c3 = new PSCameraData(3L);
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

    c4 = new PSCameraData(4L);
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

    c5 = new PSCameraData(5L);
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

    // Build test models
    m0 = new PSModelData(QFileInfo(":/PSHTest/Model0.xml"));
    m0->setFaceCount(4194268);
    m0->setMeshFilename("model0.ply");
    m0->addTextureFile(0, "model0.png");

    m1 = new PSModelData(QFileInfo(":/PSHTest/Model1.xml"));
    m1->setMeshFilename("model0.ply");
    m1->addTextureFile(0, "model0.png");

    m2 = new PSModelData(QFileInfo(":/PSHTest/Model2.xml"));
    m2->setFaceCount(538153);
    m2->setMeshFilename("model0.ply");
    m2->addTextureFile(0, "model0.png");

    // Build test chunk
    k0 = new PSChunkData(QFileInfo(":/PSHTest/Chunk0.xml"));
    k0->setLabel("Chunk 1");
    k0->setEnabled(true);

    k0->addSensor(s0);
    k0->addSensor(s1);
    k0->addSensor(s2);

    k0->addCamera(c0);
    k0->addCamera(c1);
    k0->addCamera(c2);

    k0->addImage(i0);
    k0->addImage(i1);
    k0->addImage(i2);

    k0->setModelData(m0);

    k0->setImageAlignment_matchDurationSeconds(1806.955144);
    k0->setImageAlignment_alignDurationSeconds(31.753054);
    k0->setImageAlignment_featureLimit(40000);
    k0->setImageAlignment_Level((PSChunkData::ImageAlignmentDetail)1);
    k0->setImageAlignment_Masked(false);
    k0->setImageAlignment_tiePointLimit(40000);

    k0->setOptimize_durationSeconds(1.840383);
    k0->setOptimize_aspect(true);
    k0->setOptimize_f(true);
    k0->setOptimize_cx(true);
    k0->setOptimize_cy(true);
    k0->setOptimize_k1(true);
    k0->setOptimize_k2(true);
    k0->setOptimize_k3(true);
    k0->setOptimize_p1(true);
    k0->setOptimize_p2(true);

    k0->setDenseCloud_depthDurationSeconds(48029.322018);
    k0->setDenseCloud_cloudDurationSeconds(22383.277803);
    k0->setDenseCloud_level((PSChunkData::DenseCloudDetail)1);
    k0->setDenseCloud_filterLevel((PSChunkData::DenseCloudFilter)1);
    k0->setDenseCloud_imagesUsed(3);

    k0->setModelGeneration_resolution(0.00143795560157094);
    k0->setModelGeneration_faceCount(4194268);
    k0->setModelGeneration_denseSource(true);
    k0->setModelGeneration_durationSeconds(2247.986924);
    k0->setModelGeneration_interpolationEnabled(true);

    k0->setTextureGeneration_blendDuration(241.538444);
    k0->setTextureGeneration_blendMode(3);
    k0->setTextureGeneration_count(1);
    k0->setTextureGeneration_height(4096);
    k0->setTextureGeneration_mappingMode(1);
    k0->setTextureGeneration_uvGenDuration(45.409368);
    k0->setTextureGeneration_width(4096);
}

void PSHTest_Test::plyZipTest_data()
{
    QTest::addColumn<QFileInfo>("archive");
    QTest::addColumn<QString>("modelFile");

    QTest::newRow("Local Compressed PLY Model 0") << QFileInfo("BlueBird.psz") << QString("model0.ply");
    QTest::newRow("Local Compressed PLY Model 1") << QFileInfo("Chair.psz") << QString("model0.ply");
    QTest::newRow("Local Compressed PLY Model 2") << QFileInfo("Gun.psz") << QString("model0.ply");
    QTest::newRow("Local Compressed PLY Model 3") << QFileInfo("Pumpkin-Moldy.psz") << QString("model0.ply");

    QTest::newRow("Local Uncompressed PLY Model 0") << QFileInfo() << QString("BlueBird.ply");
    QTest::newRow("Local Uncompressed PLY Model 1") << QFileInfo() << QString("Chair.ply");
    QTest::newRow("Local Uncompressed PLY Model 2") << QFileInfo() << QString("Gun.ply");
    QTest::newRow("Local Uncompressed PLY Model 3") << QFileInfo() << QString("Pumpkin-Moldy.ply");

#ifdef Q_OS_WIN32
    QTest::newRow("Network PLY Model 0") << QFileInfo("E:/OtherData/basement-studio/BlueBird/BlueBird.psz") << QString("model0.ply");
    QTest::newRow("Network PLY Model 1") << QFileInfo("E:/OtherData/basement-studio/PumpkinMoldy/Pumpkin-Moldy.psz") << QString("model0.ply");
    QTest::newRow("Network PLY Model 2") << QFileInfo("E:/OtherData/basement-studio/GnomeMesh/Gnome2-HiRes.psz") << QString("model0.ply");
#elif defined(Q_OS_MAC)
    QTest::newRow("Network PLY Model 0") << QFileInfo("/Volumes/PARSEC-Data/OtherData/basement-studio/BlueBird/BlueBird.psz") << QString("model0.ply");
    QTest::newRow("Network PLY Model 1") << QFileInfo("/Volumes/PARSEC-Data/OtherData/basement-studio/PumpkinMoldy/Pumpkin-Moldy.psz") << QString("model0.ply");
    QTest::newRow("Network PLY Model 2") << QFileInfo("/Volumes/PARSEC-Data/OtherData/basement-studio/GnomeMesh/Gnome2-HiRes.psz") << QString("model0.ply");
#endif
}

bool saveAsOBJ(QString pName, std::vector<PLY::VertexColor>& pVerts, std::vector<PLY::FaceTex>& pFaces) {
    QFile lOutput(pName);
    lOutput.open(QFile::WriteOnly);
    if (!lOutput.isOpen() || !lOutput.isWritable()) {
        qWarning("Couldn't open obj file for writing");
        lOutput.close();
        return false;
    }

    // Open and configure our output stream
    QTextStream lOut(&lOutput);
    lOut.setRealNumberNotation(QTextStream::FixedNotation);
    lOut.setRealNumberPrecision(8);

    // Output material details
    lOut << "# Fake materials to check text coords" << endl;
    lOut << "mtllib chair.mtl" << endl;
    lOut << "usemtl Mapped" << endl << endl;

    // Output vertices
    lOut << "# " << QLocale::system().toString((long long)pVerts.size()) << " vertices" << endl;
    for (auto lV : pVerts) {
        lOut << "v " << lV.x() << " " << lV.y() << " " << lV.z() << endl;
    }
    lOut << endl;

    // Output texture coords
    lOut << "# " << QLocale::system().toString((long long)(pFaces.size() * 3)) << " texture coords" << endl;
    for (auto lF : pFaces) {
        for (size_t t = 0; t < lF.size(); t++) {
            float u = lF.texcoord(2*t), v = lF.texcoord((2*t)+1);
            lOut << "vt " << u << " " << v << endl;
        }
    }
    lOut << endl;

    // Output faces
    lOut << "# " << QLocale::system().toString((long long)pFaces.size()) << " faces" << endl;
    size_t vt = 1;
    for (auto lF : pFaces) {
        size_t A = lF.vertex(0), B = lF.vertex(1), C = lF.vertex(2);
        lOut << "f " << (A+1) << "/" << (vt+0) << " "
                     << (B+1) << "/" << (vt+1) << " "
                     << (C+1) << "/" << (vt+2) << endl;
        vt += 3;
    }

    lOutput.close();
    return true;
}

void PSHTest_Test::plyZipTest()
{
    QFETCH(QFileInfo, archive);
    QFETCH(QString, modelFile);

    // Process an archive if there is one
    QuaZipFile* lInsideFile = NULL;
    if (archive.filePath() != "") {
        lInsideFile = new QuaZipFile(archive.filePath(), modelFile);
        if(!lInsideFile->open(QIODevice::ReadOnly)) {
            qWarning("Failed to open zip file: %d.", lInsideFile->getZipError());
            delete lInsideFile;
            return;
        }
    }

    // Open file and read header info
    PLY::Header header;
    PLY::Reader reader(header);
    if (lInsideFile != NULL) {
        if (!reader.use_io_device(lInsideFile)) {
            qWarning("Failed to use archive stream");
            lInsideFile->close();
            delete lInsideFile;
            return;
        }
    } else {
        if (!reader.open_file(modelFile)) {
            qWarning("Failed to open '%s'", modelFile.toLocal8Bit().data());
            return;
        }
    }

    // Prepare storage
    PLY::Storage store(header);
    PLY::Element& vertex = *header.find_element(PLY::VertexColor::name);
    PLY::Element& face = *header.find_element(PLY::FaceTex::name);

    std::vector<PLY::VertexColor> vertColl;
    PLY::VCExternal vertices(vertColl);
    store.set_collection(header, vertex, vertices);

    std::vector<PLY::FaceTex> faceColl;
    PLY::FTExternal faces(faceColl);
    store.set_collection(header, face, faces);

    // Read the data in the file into the storage.
    bool ok = reader.read_data(&store);
    reader.close_file();
    delete lInsideFile;

    if (!ok) {
        qWarning("Error reading data from PLY file");
        return;
    }

    // NOTE: Enable the saveAsOBJ line below to write the data to a file for visual verification
    qInfo("Read %lu vertices and %lu faces.", vertColl.size(), faceColl.size());
//    if (!saveAsOBJ(archive.fileName().append(modelFile.append(".obj")), vertColl, faceColl)) {
//        qWarning("Failed to save to obj file");
//    }
}

void PSHTest_Test::sensorDataParsing_data()
{
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<PSSensorData*>("result");

    QTest::newRow("Sensor 0") << QFileInfo(":/PSHTest/Sensor0.xml") << s0;
    QTest::newRow("Sensor 1") << QFileInfo(":/PSHTest/Sensor1.xml") << s1;
    QTest::newRow("Sensor 2") << QFileInfo(":/PSHTest/Sensor2.xml") << s2;
}

void PSHTest_Test::sensorDataParsing()
{
    QFETCH(QFileInfo, file);
    QFile* lXMLFile = new QFile(file.absoluteFilePath());
    lXMLFile->open(QIODevice::ReadOnly);
    QXmlStreamReader* lXMLFileStream = new QXmlStreamReader(lXMLFile);

    PSSensorData* data = PSSensorData::makeFromXML(lXMLFileStream);

    // Free file resources
    delete lXMLFileStream;
    lXMLFile->close();
    delete lXMLFile;

    Q_ASSERT(data != NULL);
    QFETCH(PSSensorData*, result);

    QCOMPARE(data->ID, result->ID);
    QCOMPARE(data->getLabel(), result->getLabel());
    QCOMPARE(data->getType(), result->getType());
    QCOMPARE(data->getWidth(), result->getWidth());
    QCOMPARE(data->getHeight(), result->getHeight());
    QVERIFY(qFuzzyCompare(data->getPixelWidth(), result->getPixelWidth()));
    QVERIFY(qFuzzyCompare(data->getPixelHeight(), result->getPixelHeight()));
    QVERIFY(qFuzzyCompare(data->getFocalLength(), result->getFocalLength()));
    QCOMPARE(data->isFixed(), result->isFixed());

    QVERIFY(qFuzzyCompare(data->getFx(), result->getFx()));
    QVERIFY(qFuzzyCompare(data->getFy(), result->getFy()));
    QVERIFY(qFuzzyCompare(data->getCx(), result->getCx()));
    QVERIFY(qFuzzyCompare(data->getCy(), result->getCy()));
    QVERIFY(qFuzzyCompare(data->getSkew(), result->getSkew()));
    QVERIFY(qFuzzyCompare(data->getB1(), result->getB1()));
    QVERIFY(qFuzzyCompare(data->getB2(), result->getB2()));
    QVERIFY(qFuzzyCompare(data->getK1(), result->getK1()));
    QVERIFY(qFuzzyCompare(data->getK2(), result->getK2()));
    QVERIFY(qFuzzyCompare(data->getK3(), result->getK3()));
    QVERIFY(qFuzzyCompare(data->getK4(), result->getK4()));
    QVERIFY(qFuzzyCompare(data->getP1(), result->getP1()));
    QVERIFY(qFuzzyCompare(data->getP2(), result->getP2()));
    QVERIFY(qFuzzyCompare(data->getP3(), result->getP3()));
    QVERIFY(qFuzzyCompare(data->getP4(), result->getP4()));

    QCOMPARE(data->getCovarianceParams(), result->getCovarianceParams());

    // Free dynamic memory
    delete data;
}

void PSHTest_Test::imageDataParsing_data()
{
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<PSImageData*>("result");

    QTest::newRow("Image 0") << QFileInfo(":/PSHTest/Image0.xml") << i0;
    QTest::newRow("Image 1") << QFileInfo(":/PSHTest/Image1.xml") << i1;
    QTest::newRow("Image 2") << QFileInfo(":/PSHTest/Image2.xml") << i2;
    QTest::newRow("Image 3") << QFileInfo(":/PSHTest/Image3.xml") << i3;
    QTest::newRow("Image 4") << QFileInfo(":/PSHTest/Image4.xml") << i4;
    QTest::newRow("Image 5") << QFileInfo(":/PSHTest/Image5.xml") << i5;
}

void PSHTest_Test::imageDataParsing()
{
    // Fetch file info and read it
    QFETCH(QFileInfo, file);
    QFile* lXMLFile = new QFile(file.absoluteFilePath());
    lXMLFile->open(QIODevice::ReadOnly);
    QXmlStreamReader* lXMLFileStream = new QXmlStreamReader(lXMLFile);

    PSImageData* data = PSImageData::makeFromXML(lXMLFileStream);

    // Free file resources
    delete lXMLFileStream;
    lXMLFile->close();
    delete lXMLFile;

    // Compare everything
    Q_ASSERT(data != NULL);
    QFETCH(PSImageData*, result);
    QCOMPARE(data->getCamID(), result->getCamID());
    QCOMPARE(data->getPropertyCount(), result->getPropertyCount());
    QStringList keys = data->getPropertyKeys();
    for(QString key : keys) {
        QCOMPARE(data->getProperty(key), result->getProperty(key));
    }

    // Free dynamic memory
    delete data;
}

void PSHTest_Test::cameraDataParsing_data()
{
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<PSCameraData*>("result");

    QTest::newRow("Camera 0") << QFileInfo(":/PSHTest/Camera0.xml") << c0;
    QTest::newRow("Camera 1") << QFileInfo(":/PSHTest/Camera1.xml") << c1;
    QTest::newRow("Camera 2") << QFileInfo(":/PSHTest/Camera2.xml") << c2;
    QTest::newRow("Camera 3") << QFileInfo(":/PSHTest/Camera3.xml") << c3;
    QTest::newRow("Camera 4") << QFileInfo(":/PSHTest/Camera4.xml") << c4;
    QTest::newRow("Camera 5") << QFileInfo(":/PSHTest/Camera5.xml") << c5;
}

void PSHTest_Test::cameraDataParsing()
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
    delete data;
}

void PSHTest_Test::modelDataParsing_data()
{
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<PSModelData*>("result");

    QTest::newRow("Model 0") << QFileInfo(":/PSHTest/Model0.xml") << m0;
    QTest::newRow("Model 1") << QFileInfo(":/PSHTest/Model1.xml") << m1;
    QTest::newRow("Model 2") << QFileInfo(":/PSHTest/Model2.xml") << m2;
}

void PSHTest_Test::modelDataParsing()
{
    // Fetch file info and read it
    QFETCH(QFileInfo, file);
    QFile* lXMLFile = new QFile(file.absoluteFilePath());
    lXMLFile->open(QIODevice::ReadOnly);
    QXmlStreamReader* lXMLFileStream = new QXmlStreamReader(lXMLFile);

    PSModelData* data = PSModelData::makeFromXML(lXMLFileStream, file);

    // Free file resources
    delete lXMLFileStream;
    lXMLFile->close();
    delete lXMLFile;

    // Compare everything
    Q_ASSERT(data != NULL);
    QFETCH(PSModelData*, result);
    QCOMPARE(data->getArchiveFile().filePath(), result->getArchiveFile().filePath());
    QCOMPARE(data->getFaceCount(), result->getFaceCount());
    QCOMPARE(data->getVertexCount(), result->getVertexCount());
    QCOMPARE(data->getMeshFilename(), result->getMeshFilename());
    QCOMPARE(data->hasVtxColors(), result->hasVtxColors());
    QCOMPARE(data->hasUV(), result->hasUV());

    QList<int> texKeys = data->getTextureFiles().keys();
    QCOMPARE(texKeys.length(), result->getTextureFiles().keys().length());
    for (int key : texKeys) {
        QCOMPARE(data->getTextureFile(key), result->getTextureFile(key));
    }

    // Free dynamic memory
    delete data;
}

void PSHTest_Test::chunkParsing_data()
{
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<PSChunkData*>("result");

    m0->setArchiveFile(QFileInfo(":/PSHTest/Chunk0.xml"));
    QTest::newRow("Chunk 0") << QFileInfo(":/PSHTest/Chunk0.xml") << k0;
}

void PSHTest_Test::chunkParsing()
{
    // Fetch file info and read it
    QFETCH(QFileInfo, file);
    QFile* lXMLFile = new QFile(file.absoluteFilePath());
    lXMLFile->open(QIODevice::ReadOnly);
    QXmlStreamReader* lXMLFileStream = new QXmlStreamReader(lXMLFile);

    PSChunkData* data = new PSChunkData(file, lXMLFileStream);

    // Free file resources
    delete lXMLFileStream;
    lXMLFile->close();
    delete lXMLFile;

    // Compare everything
    Q_ASSERT(data != NULL);
    QFETCH(PSChunkData*, result);

    QCOMPARE(data->getLabel(), result->getLabel());
    QCOMPARE(data->isEnabled(), result->isEnabled());
    QCOMPARE(data->getMarkerCount(), result->getMarkerCount());
    QCOMPARE(data->getScalebarCount(), result->getScalebarCount());
    QCOMPARE(data->getImageCount(), result->getImageCount());
    QCOMPARE(data->getCameraCount(), result->getCameraCount());
    QCOMPARE(data->getSensorCount(), result->getSensorCount());
    QCOMPARE(data->getModelArchiveFile().filePath(), result->getModelArchiveFile().filePath());
    QCOMPARE(data->getOptimizeString(), result->getOptimizeString());
    QCOMPARE(data->getImageAlignment_LevelString(), result->getImageAlignment_LevelString());
    QCOMPARE(data->getDenseCloud_levelString(), result->getDenseCloud_levelString());
    QCOMPARE(data->getDenseCloud_filterLevelString(), result->getDenseCloud_filterLevelString());
    QCOMPARE(data->describeImageAlignPhase(), result->describeImageAlignPhase());
    QCOMPARE(data->describeDenseCloudPhase(), result->describeDenseCloudPhase());
    QCOMPARE(data->describeModelGenPhase(), result->describeModelGenPhase());
    QCOMPARE(data->describeTextureGenPhase(), result->describeTextureGenPhase());

    QVERIFY(qFuzzyCompare(data->getImageAlignment_matchDurationSeconds(), result->getImageAlignment_matchDurationSeconds()));
    QVERIFY(qFuzzyCompare(data->getImageAlignment_alignDurationSeconds(), result->getImageAlignment_alignDurationSeconds()));
    QCOMPARE(data->getImageAlignment_Level(), result->getImageAlignment_Level());
    QCOMPARE(data->getImageAlignment_Masked(), result->getImageAlignment_Masked());
    QCOMPARE(data->getImageAlignment_featureLimit(), result->getImageAlignment_featureLimit());
    QCOMPARE(data->getImageAlignment_tiePointLimit(), result->getImageAlignment_tiePointLimit());
    QVERIFY(qFuzzyCompare(data->getOptimize_durationSeconds(), result->getOptimize_durationSeconds()));
    QCOMPARE(data->getOptimize_aspect(), result->getOptimize_aspect());
    QCOMPARE(data->getOptimize_f(), result->getOptimize_f());
    QCOMPARE(data->getOptimize_cx(), result->getOptimize_cx());
    QCOMPARE(data->getOptimize_cy(), result->getOptimize_cy());
    QCOMPARE(data->getOptimize_p1(), result->getOptimize_p1());
    QCOMPARE(data->getOptimize_p2(), result->getOptimize_p2());
    QCOMPARE(data->getOptimize_p3(), result->getOptimize_p3());
    QCOMPARE(data->getOptimize_p4(), result->getOptimize_p4());
    QCOMPARE(data->getOptimize_b1(), result->getOptimize_b1());
    QCOMPARE(data->getOptimize_b2(), result->getOptimize_b2());
    QCOMPARE(data->getOptimize_k1(), result->getOptimize_k1());
    QCOMPARE(data->getOptimize_k2(), result->getOptimize_k2());
    QCOMPARE(data->getOptimize_k3(), result->getOptimize_k3());
    QCOMPARE(data->getOptimize_k4(), result->getOptimize_k4());
    QCOMPARE(data->getOptimize_skew(), result->getOptimize_skew());

    QVERIFY(qFuzzyCompare(data->getDenseCloud_durationSeconds(), result->getDenseCloud_durationSeconds()));
    QVERIFY(qFuzzyCompare(data->getDenseCloud_depthDurationSeconds(), result->getDenseCloud_depthDurationSeconds()));
    QVERIFY(qFuzzyCompare(data->getDenseCloud_cloudDurationSeconds(), result->getDenseCloud_cloudDurationSeconds()));
    QCOMPARE(data->getDenseCloud_level(), result->getDenseCloud_level());
    QCOMPARE(data->getDenseCloud_imagesUsed(), result->getDenseCloud_imagesUsed());
    QCOMPARE(data->getDenseCloud_filterLevel(), result->getDenseCloud_filterLevel());
    QVERIFY(qFuzzyCompare(data->getModelGeneration_resolution(), result->getModelGeneration_resolution()));
    QVERIFY(qFuzzyCompare(data->getModelGeneration_durationSeconds(), result->getModelGeneration_durationSeconds()));
    QCOMPARE(data->getModelGeneration_faceCount(), result->getModelGeneration_faceCount());
    QCOMPARE(data->getModelGeneration_denseSource(), result->getModelGeneration_denseSource());
    QCOMPARE(data->getModelGeneration_interpolationEnabled(), result->getModelGeneration_interpolationEnabled());
    QVERIFY(qFuzzyCompare(data->getTextureGeneration_blendDuration(), result->getTextureGeneration_blendDuration()));
    QVERIFY(qFuzzyCompare(data->getTextureGeneration_uvGenDuration(), result->getTextureGeneration_uvGenDuration()));
    QCOMPARE(data->getTextureGeneration_mappingMode(), result->getTextureGeneration_mappingMode());
    QCOMPARE(data->getTextureGeneration_blendMode(), result->getTextureGeneration_blendMode());
    QCOMPARE(data->getTextureGeneration_count(), result->getTextureGeneration_count());
    QCOMPARE(data->getTextureGeneration_width(), result->getTextureGeneration_width());
    QCOMPARE(data->getTextureGeneration_height(), result->getTextureGeneration_height());
    QCOMPARE(data->getAlignPhaseStatus(), result->getAlignPhaseStatus());
    QCOMPARE(data->getDenseCloudDepthImages(), result->getDenseCloudDepthImages());
    QCOMPARE(data->getDenseCloudPhaseStatus(), result->getDenseCloudPhaseStatus());
    QCOMPARE(data->getModelFaceCount(), result->getModelFaceCount());
    QCOMPARE(data->getModelVertexCount(), result->getModelVertexCount());
    QCOMPARE(data->getModelGenPhaseStatus(), result->getModelGenPhaseStatus());
    QCOMPARE(data->getTextureGenPhaseStatus(), result->getTextureGenPhaseStatus());

    // Free dynamic memory
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

void PSHTest_Test::cleanupTestCase() {
    delete s0;
    delete s1;
    delete s2;

    delete i0;
    delete i1;
    delete i2;
    delete i3;
    delete i4;
    delete i5;

    delete c0;
    delete c1;
    delete c2;
    delete c3;
    delete c4;
    delete c5;

    delete k0;
}

QTEST_APPLESS_MAIN(PSHTest_Test)

#include "tst_pshtest_test.moc"
