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

struct PackedVertex {
    float x, y, z;
    float nx, ny, nz;
    float r, g, b, a;
    float tu, tv, tn;
};

PLYMeshData::PLYMeshData() {
    mVertexBuffer = NULL;
    mPackedData = NULL;
    mVAO = NULL;
    initMembers();
}

PLYMeshData::~PLYMeshData() {
    destroyBuffers();
    free(mPackedData);
    delete mVertexBuffer;
    delete mVAO;
}

void PLYMeshData::initMembers() {
    // Free any dynamic memory
    destroyBuffers();
    free(mPackedData);
    mPackedData = NULL;

    mPLYVertCollection.clear();
    mPLYFaceCollection.clear();

    // Allocate buffer structures
    if (mVertexBuffer == NULL) {
        mVertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    }

    if (mVAO == NULL) {
        mVAO = new QOpenGLVertexArrayObject();
    }

    // Set mesh properties and metrics back to initial values
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

    // Prepare to compute smoothed normals
    unsigned int i = 0;
    for(PLY::FaceTex& lF : mPLYFaceCollection) {
        // Extract the face indices
        unsigned int A = lF.vertex(0);
        unsigned int B = lF.vertex(1);
        unsigned int C = lF.vertex(2);

        // Update vertex reverse lookup table
        lFaceLookup[A].push_back(i);
        lFaceLookup[B].push_back(i);
        lFaceLookup[C].push_back(i);
        i++;

        // Compute a flat face normal
        QVector3D Av(mPLYVertCollection[A].value_x.val, mPLYVertCollection[A].value_y.val, mPLYVertCollection[A].value_z.val);
        QVector3D Bv(mPLYVertCollection[B].value_x.val, mPLYVertCollection[B].value_y.val, mPLYVertCollection[B].value_z.val);
        QVector3D Cv(mPLYVertCollection[C].value_x.val, mPLYVertCollection[C].value_y.val, mPLYVertCollection[C].value_z.val);
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

    // Allocate space for a direct-indexed vertex buffer
    if (mPackedData != NULL) { free(mPackedData); }
    mPackedData = malloc(mFaceCount * 3 * sizeof(PackedVertex));

    // Create a final direct-indexed list of triangles
    PackedVertex* lCur = static_cast<PackedVertex*>(mPackedData);
    for(PLY::FaceTex& lF : mPLYFaceCollection) {
        for(int i=0; i<3; i++) {
            // Get current vertex index
            unsigned int idx = lF.vertex(i);

            // Copy the raw vertex information
            lCur[i].x = mPLYVertCollection[idx].value_x.val;
            lCur[i].y = mPLYVertCollection[idx].value_y.val;
            lCur[i].z = mPLYVertCollection[idx].value_z.val;
            lCur[i].nx = mPLYVertCollection[idx].value_nx.val;
            lCur[i].ny = mPLYVertCollection[idx].value_ny.val;
            lCur[i].nz = mPLYVertCollection[idx].value_nz.val;
            lCur[i].r = mPLYVertCollection[idx].value_r.val/255.0;
            lCur[i].g = mPLYVertCollection[idx].value_g.val/255.0;
            lCur[i].b = mPLYVertCollection[idx].value_b.val/255.0;
            lCur[i].a = mPLYVertCollection[idx].value_a.val/255.0;

            // Assign proper texture coordinates
            lCur[i].tu = lF.texcoord(i*2 + 0);
            lCur[i].tv = lF.texcoord(i*2 + 1);
            lCur[i].tn = 1.0f;
        }

        // Move forward three vertices (one triangle)
        lCur += 3;
    }
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
    mVertexBuffer->allocate(mPackedData, mFaceCount*3*sizeof(PackedVertex));

    // Setup VAO data layout
    mVertexBuffer->bind();
    GL->glEnableVertexAttribArray(ATTRIB_LOC_VERTEX);
    GL->glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), (void*)0);
    size_t lOffset4Byte = 3;
    if(mHasNormals) {
        GL->glEnableVertexAttribArray(ATTRIB_LOC_NORMAL);
        GL->glVertexAttribPointer(ATTRIB_LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), (void*)(lOffset4Byte*4));
        lOffset4Byte += 3;
    }
    if(mHasColors) {
        GL->glEnableVertexAttribArray(ATTRIB_LOC_COLORS);
        GL->glVertexAttribPointer(ATTRIB_LOC_COLORS, 4, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), (void*)(lOffset4Byte*4));
        lOffset4Byte += 4;
    }
    if(mHasTexCoords) {
        GL->glEnableVertexAttribArray(ATTRIB_LOC_TEXCOR);
        GL->glVertexAttribPointer(ATTRIB_LOC_TEXCOR, 3, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), (void*)(lOffset4Byte*4));
        lOffset4Byte += 3;
    }
    mVertexBuffer->release();
}

void PLYMeshData::releaseBuffers() {
    if (mVAO != NULL && mVAO->isCreated()) {
        mVAO->release();
    }

    if (mVertexBuffer != NULL && mVertexBuffer->isCreated()) {
        mVertexBuffer->release();
    }
}

void PLYMeshData::destroyBuffers() {
    if (mVAO != NULL && mVAO->isCreated()) {
        mVAO->destroy();
    }

    if (mVertexBuffer != NULL && mVertexBuffer->isCreated()) {
        mVertexBuffer->destroy();
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
    mHasTexCoords = (face.props.size() > 1);

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

    // Process the raw data for display with OpenGL
    mPLYVertCollection.shrink_to_fit();
    mPLYFaceCollection.shrink_to_fit();
    processRawData();

    // Free up the raw PLY data
    mPLYVertCollection.clear();
    mPLYFaceCollection.clear();

    // Inidicate success
    return true;
}
