#include <cfloat>

#include "PLYMeshData.h"

#include <quazip/quazipfile.h>
#include <QVector3D>

// Defining the property names used by our PLY files
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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4100)
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include <io.h>

#ifdef _WIN32
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif

#include <QScreen>
#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>

const int PLYMeshData::ATTRIB_LOC_VERTEX = 0;
const int PLYMeshData::ATTRIB_LOC_NORMAL = 1;
const int PLYMeshData::ATTRIB_LOC_COLORS = 2;
const int PLYMeshData::ATTRIB_LOC_TEXCOR = 3;

// Static OpenGL offscreen context stuff
QOffscreenSurface* PLYMeshData::msGLSurface = NULL;
QOpenGLContext* PLYMeshData::msGLContext = new QOpenGLContext();

PLYMeshData::PLYMeshData() {
    mVertexBuffer = mFaceBuffer = NULL;
    mPackedData = NULL;
    mVAO = NULL;
    initMembers();
}

PLYMeshData::~PLYMeshData() {
    destroyBuffers();
    free(mPackedData);
    delete mVertexBuffer;
    delete mFaceBuffer;
    delete mVAO;
}

void PLYMeshData::initMembers() {
    destroyBuffers();
    free(mPackedData);

    if (mVertexBuffer == NULL) {
        mVertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    }

    if (mFaceBuffer == NULL) {
        mFaceBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    }

    if (mVAO == NULL) {
        mVAO = new QOpenGLVertexArrayObject();
    }

    mPackedData = NULL;

    mPackedElementCount = mDataStrideBytes = 0;
    mColorOffsetBytes = mTexcoordOffsetBytes = 0;

    mVertexCount = mFaceCount = 0;
    mHasNormals = mHasColors = mHasMultiTex = mHasTexCoords = false;

    mVertexMax[0] = mVertexMax[1] = mVertexMax[2] = -FLT_MAX;
    mVertexMin[0] = mVertexMin[1] = mVertexMin[2] = FLT_MAX;
    mVertexBBox[0] = mVertexBBox[1] = mVertexBBox[2] = 0.0f;
    mVertexCenter[0] = mVertexCenter[1] = mVertexCenter[2] = 0.0f;
    mVertexScale = 1.0f;
}

bool PLYMeshData::readPLYFile(QFileInfo pProjectFile, QString pFilename) {

    // Process an archive if there is one
    QuaZipFile* lInsideFile = NULL;
    if (pProjectFile.filePath() != "") {
        lInsideFile = new QuaZipFile(pProjectFile.filePath(), pFilename);
        if(!lInsideFile->open(QIODevice::ReadOnly)) {
            qWarning("Failed to open zip file '%s': %d.",
                     pProjectFile.filePath().toLocal8Bit().data(),
                     lInsideFile->getZipError());
            delete lInsideFile;
            return false;
        }
    }

    if (!parsePLYFileStream(pFilename, lInsideFile)) {
        qWarning("Could not parse PLY file");
        return false;
    }

    return true;
}

void PLYMeshData::processRawData(std::vector<PLY::VertexColor>& pVerts, std::vector<PLY::FaceTex>& pFaces) {
    // Setup mesh metrics
    mVertexCount = pVerts.size();
    mFaceCount = pFaces.size();

    // Is there anything to process?
    if (mVertexCount  == 0 || mFaceCount == 0) {
       return;
    }

    // Re-alloc the packed data array (coords, norms, possibly a color)
    mPackedElementCount = 3 + 3 + (mHasColors?1:0); // + (mHasTexCoords?3:0)
    mDataStrideBytes = mPackedElementCount * 4;  // Elements are 4 bytes each, always

    size_t lVTXBytes = mVertexCount * mDataStrideBytes;
    size_t lFaceBytes = mFaceCount * 3 * 4; // each face is 3 unsigned ints which are 4 bytes each

    // Also allocate room for the faces after the packed vertex data
    mPackedData = malloc(lVTXBytes + lFaceBytes);

    // Compute packed data stats in bytes
    mColorOffsetBytes = (mHasColors?24:0);

    // Easy way to lookup vertices later
    std::vector<float*> lPVertNorm;
    lPVertNorm.reserve(pVerts.size());

    // Build vertex data buffer
    float* lCur = (float*)mPackedData;
    for(PLY::VertexColor& lVC : pVerts) {
        // Extract the coordinates
        lCur[0] = lVC.x();
        lCur[1] = lVC.y();
        lCur[2] = lVC.z();

        // Update max/min
        if(lCur[0] < mVertexMin[0]) mVertexMin[0] = lCur[0];
        if(lCur[1] < mVertexMin[1]) mVertexMin[1] = lCur[1];
        if(lCur[2] < mVertexMin[2]) mVertexMin[2] = lCur[2];

        if(lCur[0] > mVertexMax[0]) mVertexMax[0] = lCur[0];
        if(lCur[1] > mVertexMax[1]) mVertexMax[1] = lCur[1];
        if(lCur[2] > mVertexMax[2]) mVertexMax[2] = lCur[2];

        // Add a fake normal (will compute real one later)
        lPVertNorm.push_back(lCur + 3);
        lCur[3] = 0.0f;
        lCur[4] = 1.0f;
        lCur[5] = 0.0f;

        // Color values
        if (mHasColors) {
            unsigned char* lColor = (unsigned char*)lCur + 6;
            lColor[0] = lVC.r();
            lColor[1] = lVC.g();
            lColor[2] = lVC.b();
            lColor[3] = 255;
        }

        // Advance the cursor into the packed data array
        lCur += mPackedElementCount;
    }

    // Compute the bounding box, center, and scale
    for(unsigned char i = 0; i<3; i++) {
        mVertexBBox[i] = mVertexMax[i] - mVertexMin[i];
        mVertexCenter[i] = (mVertexMax[i] + mVertexMin[i])/2.0f;
    }

    mVertexScale = 2.0/std::max(mVertexBBox[0], std::max(mVertexBBox[1], mVertexBBox[2]));

    // Vertex and face computed info
    std::vector<unsigned int>* lFaceLookup = new std::vector<unsigned int>[pVerts.size()];
    std::vector<QVector3D> lFaceNorms;
    lFaceNorms.reserve(pFaces.size());

    // Add face data after packed vertex data
    unsigned int i = 0;
    unsigned int* lFCur = (unsigned int*)lCur;
    for(PLY::FaceTex& lF : pFaces) {
        unsigned int A = lF.vertex(0);
        unsigned int B = lF.vertex(1);
        unsigned int C = lF.vertex(2);

        lFCur[0] = A;
        lFCur[1] = B;
        lFCur[2] = C;

        // Update vertex reverse lookup table
        lFaceLookup[A].push_back(i);
        lFaceLookup[B].push_back(i);
        lFaceLookup[C].push_back(i);

        // Compute a per-face normal
        QVector3D Av(pVerts[A].x(), pVerts[A].y(), pVerts[A].z());
        QVector3D Bv(pVerts[B].x(), pVerts[B].y(), pVerts[B].z());
        QVector3D Cv(pVerts[C].x(), pVerts[C].y(), pVerts[C].z());
        lFaceNorms.push_back(QVector3D::crossProduct(Bv-Av, Cv-Av));

        lFCur += 3;
        i++;
    }

    // Create smooth, per-vertex normals
    for(size_t i=0; i<pVerts.size(); i++) {
//        QString output = QString::asprintf("%9lu: ", i);
        // Sum of adjacent face cross products
        QVector3D lSum(0, 0, 0);
        for(auto lFIndex : lFaceLookup[i]) {
            lSum += lFaceNorms[lFIndex];
//            output.append(QString::asprintf("%d, ", lFIndex));
        }

//        qInfo("%s", output.toLocal8Bit().data());

        // Normalize and assign to this vertex
        lSum.normalize();
        (lPVertNorm[i])[0] = lSum.x();
        (lPVertNorm[i])[1] = lSum.y();
        (lPVertNorm[i])[2] = lSum.z();
    }

    // Clean up dynamic memory
    delete [] lFaceLookup;
}

void PLYMeshData::buildBuffers(QOpenGLContext* pGLContext) {
    // Make sure we have OpenGL capabilities before we proceed
//    if (msGLSurface == NULL) {
//        msGLSurface = new QOffscreenSurface();
//        msGLSurface->create();
//    }

//    if (!msGLSurface->isValid()) {
////        msGLSurface->setScreen();
//        msGLSurface->create();
//    }

//    if (!msGLSurface->isValid()) {
//        qWarning("Offscreen surface is not valid");
//    }

//    if (!msGLSurface->supportsOpenGL()) {
//        qWarning("Offscreen surface does not support OpenGL");
//    }

//    if (!msGLContext->create() || !msGLContext->makeCurrent(msGLSurface)) {
//        qWarning("Could not get a current OpenGL Context");
//        return;
//    }

    if (pGLContext == NULL) {
        qWarning("No current OpenGL Context for building VBs");
        return;
    }

    QOpenGLFunctions* GL = pGLContext->functions();
    if (GL == NULL) {
        qWarning("Could not get GL Functions object");
        return;
    }

    // Build the Vertex Array Object
    if(!mVAO->create()) {
        qWarning("Could not create cube vertex array object");
        return;
    }
    QOpenGLVertexArrayObject::Binder vaoBinder(mVAO);

    // This will only work if we have a current opengl context
    mVertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if(!mVertexBuffer->create()) {
        qWarning("Could not create vertex buffer");
        return;
    }
    mVertexBuffer->bind();

    // Copy data to video memory
    mVertexBuffer->allocate(mPackedData, mDataStrideBytes * mVertexCount);

    // This will only work if we have a current opengl context
    mFaceBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    if(!mFaceBuffer->create()) {
        qWarning("Could not create cube index buffer");
        return;
    }
    mFaceBuffer->bind();

    // Copy face data to video memory
    void* lFaces = ((unsigned char*)mPackedData) + mVertexCount * mDataStrideBytes;
    mFaceBuffer->allocate(lFaces, mFaceCount * 3 * 4);

    // Setup VAO data layout
    mVertexBuffer->bind();
    GL->glEnableVertexAttribArray(ATTRIB_LOC_VERTEX);
    GL->glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, mDataStrideBytes, (void*)0);
    size_t lOffset4Byte = 3;
    if(mHasNormals) {
        GL->glEnableVertexAttribArray(ATTRIB_LOC_NORMAL);
        GL->glVertexAttribPointer(ATTRIB_LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, mDataStrideBytes, (void*)(lOffset4Byte*4));
        lOffset4Byte += 3;
    }
    if(mHasColors) {
        GL->glEnableVertexAttribArray(ATTRIB_LOC_COLORS);
        GL->glVertexAttribPointer(ATTRIB_LOC_COLORS, 4, GL_UNSIGNED_BYTE, GL_FALSE, mDataStrideBytes, (void*)(lOffset4Byte*4));
        lOffset4Byte += 1;
    }
//    if(mHasTexCoords) {
//        GL->glEnableVertexAttribArray(ATTRIB_LOC_TEXCOR);
//        GL->glVertexAttribPointer(ATTRIB_LOC_TEXCOR, 3, GL_FLOAT, GL_FALSE, mDataStrideBytes, (void*)(lOffset4Byte*4));
//        lOffset4Byte += 3;
//    }
    mVertexBuffer->release();
}

void PLYMeshData::releaseBuffers() {
    if (mVAO != NULL && mVAO->isCreated()) {
        mVAO->release();
    }

    if (mVertexBuffer != NULL && mVertexBuffer->isCreated()) {
        mVertexBuffer->release();
    }

    if (mFaceBuffer != NULL && mFaceBuffer->isCreated()) {
        mFaceBuffer->release();
    }
}

void PLYMeshData::destroyBuffers() {
    if (mVAO != NULL && mVAO->isCreated()) {
        mVAO->destroy();
    }

    if (mVertexBuffer != NULL && mVertexBuffer->isCreated()) {
        mVertexBuffer->destroy();
    }

    if (mFaceBuffer != NULL && mFaceBuffer->isCreated()) {
        mFaceBuffer->destroy();
    }
}

bool PLYMeshData::parsePLYFileStream(QString pFilename, QuaZipFile* pInsideFile) { // throws IOException {
    // Open file and read header info
    PLY::Header header;
    PLY::Reader reader(header);
    if (pInsideFile != NULL) {
        if (!reader.use_io_device(pInsideFile)) {
            qWarning("Failed to use archive stream");
            return false;
        }
    } else {
        if (!reader.open_file(pFilename)) {
            qWarning("Failed to open '%s'", pFilename.toLocal8Bit().data());
            return false;
        }
    }

    // Prepare storage
    PLY::Storage store(header);
    PLY::Element& vertex = *header.find_element(PLY::Vertex::name);
    PLY::Element& face = *header.find_element(PLY::Face::name);

    // Make some assumptions about properties
    mHasColors = false; // (vertex.props.size() > 3);
    mHasNormals = true;
    mHasTexCoords = false; //(face.props.size() > 1);

    // Prepare external arrays
    std::vector<PLY::VertexColor> vertColl;
    PLY::VCExternal vertices(vertColl);
    store.set_collection(header, vertex, vertices);

    std::vector<PLY::FaceTex> faceColl;
    PLY::FTExternal faces(faceColl);
    store.set_collection(header, face, faces);

    // Read the data in the file into the storage.
    bool ok = reader.read_data(&store);
    reader.close_file();
    if (!ok) {
        qWarning("Error reading data from PLY file");
        return false;
    }

    // Process the data and return success
    processRawData(vertColl, faceColl);
    return true;
}
