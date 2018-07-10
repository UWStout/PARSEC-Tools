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

    const Property VertexN::prop_nx = Property("nx", SCALAR, Float32);
    const Property VertexN::prop_ny = Property("ny", SCALAR, Float32);
    const Property VertexN::prop_nz = Property("nz", SCALAR, Float32);

    const Property VertexNC::prop_r = Property("red", SCALAR, Float32);
    const Property VertexNC::prop_g = Property("green", SCALAR, Float32);
    const Property VertexNC::prop_b = Property("blue", SCALAR, Float32);
    const Property VertexNC::prop_a = Property("alpha", SCALAR, Float32);

    const Property VertexNCT::prop_tu = Property("tu", SCALAR, Float32);
    const Property VertexNCT::prop_tv = Property("tv", SCALAR, Float32);
    const Property VertexNCT::prop_tn = Property("tn", SCALAR, Float32);

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

void PLYMeshData::processRawData() {
    // Setup mesh metrics
    mVertexCount = mPLYVertCollection.size();
    mFaceCount = mPLYFaceCollection.size();

    // Is there anything to process?
    if (mVertexCount  == 0 || mFaceCount == 0) {
        mPLYVertCollection.clear();
        mPLYFaceCollection.clear();
        return;
    }

    // Allocate space for a direct vertex buffer
    if (mPackedData != NULL) { free(mPackedData); }
    mPackedData = malloc(mFaceCount * 3 * sizeof(PLY::VertexNCT));

    // Build the min/max bounding box
    for(PLY::VertexNCT& lVC : mPLYVertCollection) {
        if(lVC.value_x.val < mVertexMin[0]) mVertexMin[0] = lVC.value_x.val;
        if(lVC.value_y.val < mVertexMin[1]) mVertexMin[1] = lVC.value_y.val;
        if(lVC.value_z.val < mVertexMin[2]) mVertexMin[2] = lVC.value_z.val;

        if(lVC.value_x.val > mVertexMax[0]) mVertexMax[0] = lVC.value_x.val;
        if(lVC.value_y.val > mVertexMax[1]) mVertexMax[1] = lVC.value_y.val;
        if(lVC.value_z.val > mVertexMax[2]) mVertexMax[2] = lVC.value_z.val;
    }

    // Compute the bounding box, center, and scale
    for(unsigned char i = 0; i<3; i++) {
        mVertexBBox[i] = mVertexMax[i] - mVertexMin[i];
        mVertexCenter[i] = (mVertexMax[i] + mVertexMin[i])/2.0f;
    }

    mVertexScale = 2.0/std::max(mVertexBBox[0], std::max(mVertexBBox[1], mVertexBBox[2]));

    // Vertex and face computed info
    std::vector<unsigned int>* lFaceLookup = new std::vector<unsigned int>[mPLYVertCollection.size()];
    std::vector<QVector3D> lFaceNorms;
    lFaceNorms.reserve(mPLYFaceCollection.size());

    // Create direct list of vertices (three for each fact)
    PLY::VertexNCT* lCur = (PLY::VertexNCT)mPackedData;
    unsigned int i = 0;
    for(PLY::FaceTex& lF : mPLYFaceCollection) {
        // Extract the face indices
        unsigned int A = lF.vertex(0);
        unsigned int B = lF.vertex(1);
        unsigned int C = lF.vertex(2);

        // Copy the raw vertex information
        memcpy(lCur + 0, &(mPLYVertCollection[A]), sizeof(PLY::VertexNCT));
        memcpy(lCur + 1, &(mPLYVertCollection[B]), sizeof(PLY::VertexNCT));
        memcpy(lCur + 2, &(mPLYVertCollection[C]), sizeof(PLY::VertexNCT));

        // Update the texture coordinates
        lCur[0].tu(lF.texcoord(0));
        lCur[0].tv(lF.texcoord(1));
        lCur[1].tu(lF.texcoord(2));
        lCur[1].tv(lF.texcoord(3));
        lCur[2].tu(lF.texcoord(4));
        lCur[2].tv(lF.texcoord(5));

        // Update vertex reverse lookup table
        lFaceLookup[A].push_back(i);
        lFaceLookup[B].push_back(i);
        lFaceLookup[C].push_back(i);
        i++;

        // Compute a face normal
        QVector3D Av(mPLYVertCollection[A].x(), mPLYVertCollection[A].y(), mPLYVertCollection[A].z());
        QVector3D Bv(mPLYVertCollection[B].x(), mPLYVertCollection[B].y(), mPLYVertCollection[B].z());
        QVector3D Cv(mPLYVertCollection[C].x(), mPLYVertCollection[C].y(), mPLYVertCollection[C].z());
        lFaceNorms.push_back(QVector3D::crossProduct(Bv-Av, Cv-Av));
    }

    // Create smooth, per-vertex normals
    for(size_t i=0; i<mPLYVertCollection.size(); i++) {
        // Sum of adjacent face cross products
        QVector3D lSum(0, 0, 0);
        for(auto lFIndex : lFaceLookup[i]) {
            lSum += lFaceNorms[lFIndex];
        }

        // Normalize and assign to this vertex
        lSum.normalize();
        mPLYVertCollection[i].value_nx.val = lSum.x();
        mPLYVertCollection[i].value_ny.val = lSum.y();
        mPLYVertCollection[i].value_nz.val = lSum.z();
    }

    // Clean up dynamic memory
    delete [] lFaceLookup;
}

void PLYMeshData::buildBuffers(QOpenGLContext* pGLContext) {
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
        GL->glVertexAttribPointer(ATTRIB_LOC_COLORS, 3, GL_FLOAT, GL_FALSE, mDataStrideBytes, (void*)(lOffset4Byte*4));
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
    mHasColors = (vertex.props.size() > 3);
    mHasNormals = true;
    mHasTexCoords = false; // (face.props.size() > 1);

    // Prepare external arrays
    mPLYVertCollection.clear();
    PLY::VNCTExternal vertices(mPLYVertCollection);
    store.set_collection(header, vertex, vertices);

    mPLYFaceCollection.clear();
    PLY::FTExternal faces(mPLYFaceCollection);
    store.set_collection(header, face, faces);

    // Read the data in the file into the storage.
    bool ok = reader.read_data(&store);
    reader.close_file();
    if (!ok) {
        qWarning("Error reading data from PLY file");
        return false;
    }

    // Process the data and return success
    mPLYVertCollection.shrink_to_fit();
    mPLYFaceCollection.shrink_to_fit();
    processRawData();
    return true;
}
