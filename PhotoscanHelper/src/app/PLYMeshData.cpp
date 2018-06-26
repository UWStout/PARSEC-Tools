#include <cfloat>

#include "PLYMeshData.h"

#include <quazip/quazipfile.h>

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

#include <QApplication>
#include <QScreen>
#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
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

    // TODO: Disabled until I'm sure we need this
//    if (mFaceBuffer == NULL) {
//        mFaceBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
//    }

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
    // Initialize all members
    initMembers();

    // Setup mesh metrics
    mVertexCount = pVerts.size();
    mFaceCount = pFaces.size();

    // Is there anything to process?
    if (mVertexCount || mFaceCount == 0) {
       return;
    }

    // Re-alloc the packed data array
    mPackedElementCount = 3 + (mHasTexCoords?3:0) + (mHasColors?1:0);
    mDataStrideBytes = mPackedElementCount * 4;  // Elements are 4 bytes each, always
    mPackedData = malloc(mFaceCount * 3 * mDataStrideBytes);

    // Compute packed data stats in bytes
    mColorOffsetBytes = (mHasColors?12:0);
    mTexcoordOffsetBytes = (mHasColors?16:0);

    // Flatten the data into raw triangle buffer
    float* lCur = (float*)mPackedData;
    for(PLY::FaceTex& lF : pFaces) {
        for(int i=0; i<3; i++) {
            // Vertex position
            PLY::VertexColor& lV = pVerts[lF.vertex(i)];
            lCur[0] = lV.x();
            lCur[1] = lV.y();
            lCur[2] = lV.z();

            // Update max/min
            if(lCur[0] < mVertexMin[0]) mVertexMin[0] = lCur[0];
            if(lCur[1] < mVertexMin[1]) mVertexMin[1] = lCur[1];
            if(lCur[2] < mVertexMin[2]) mVertexMin[2] = lCur[2];

            if(lCur[0] > mVertexMax[0]) mVertexMax[0] = lCur[0];
            if(lCur[1] > mVertexMax[1]) mVertexMax[1] = lCur[1];
            if(lCur[2] > mVertexMax[2]) mVertexMax[2] = lCur[2];

            // Color values
            if (mHasColors) {
                unsigned char* lColor = (unsigned char*)lCur + 3;
                lColor[0] = lV.r();
                lColor[1] = lV.g();
                lColor[2] = lV.b();
            }

            // Texture coordinates
            if (mHasTexCoords) {
                lCur[4] = lF.texcoord(i*2);
                lCur[5] = lF.texcoord(i*2+1);
                lCur[6] = 0.0f;
            }

            // Advance to next vertex
            lCur += mPackedElementCount;
        }
    }

    // Compute the bounding box, center, and scale
    for(unsigned char i = 0; i<3; i++) {
        mVertexBBox[i] = mVertexMax[i] - mVertexMin[i];
        mVertexCenter[i] = (mVertexMax[i] + mVertexMin[i])/2.0f;
    }

    mVertexScale = 2.0/std::max(mVertexBBox[0], std::max(mVertexBBox[1], mVertexBBox[2]));
}

void PLYMeshData::buildBuffers() {
    // Make sure we have OpenGL capabilities before we proceed
    if (msGLSurface == NULL) {
        msGLSurface = new QOffscreenSurface(QApplication::screens()[0]);
        msGLSurface->create();
    }

    if (!msGLSurface->isValid()) {
        msGLSurface->setScreen(QApplication::screens()[0]);
        msGLSurface->create();
    }

    if (!msGLSurface->isValid()) {
        qWarning("Offscreen surface is not valid");
    }

    if (!msGLSurface->supportsOpenGL()) {
        qWarning("Offscreen surface does not support OpenGL");
    }

    if (!msGLContext->create() || !msGLContext->makeCurrent(msGLSurface)) {
        qWarning("Could not get a current OpenGL Context");
        return;
    }

    // This will only work if we have a current opengl context
    mVertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if(!mVertexBuffer->create()) {
        qWarning("Could not create vertex buffer");
        return;
    }

    // Copy data to video memory
    mVertexBuffer->allocate(mPackedData, mDataStrideBytes * 3 * mFaceCount);

    // Build the Vertex Array Object
    if(!mVAO->create()) {
        qWarning("Could not create vertex array object");
        return;
    }

    // Setup VAO data layout
    glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, GL_FALSE,
                          mDataStrideBytes, (void*)0);
    size_t lOffset4Byte = 3;
    if(mHasColors) {
        glVertexAttribPointer(ATTRIB_LOC_COLORS, 4, GL_UNSIGNED_BYTE, GL_FALSE,
                              mDataStrideBytes, (void*)(lOffset4Byte*4));
        lOffset4Byte += 1;
    }

    if(mHasTexCoords) {
        glVertexAttribPointer(ATTRIB_LOC_TEXCOR, 3, GL_FLOAT, GL_FALSE,
                              mDataStrideBytes, (void*)(lOffset4Byte*4));
        lOffset4Byte += 3;
    }
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
    buildBuffers();
    return true;
}
