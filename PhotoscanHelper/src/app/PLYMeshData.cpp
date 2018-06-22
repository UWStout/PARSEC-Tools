#include "PLYMeshData.h"

#include <quazip/quazipfile.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

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

#pragma clang diagnostic pop

const int PLYMeshData::ATTRIB_LOC_VERTEX = 0;
const int PLYMeshData::ATTRIB_LOC_NORMAL = 1;
const int PLYMeshData::ATTRIB_LOC_COLORS = 2;
const int PLYMeshData::ATTRIB_LOC_TEXCOR = 3;

PLYMeshData::PLYMeshData() {
    mPackedVBO = mFaceVBO = -1;
    mVertexElementCount = mPackedElementCount = 0;

    mVertexData = mFaceUVData = NULL;
    mFace = mFaceTexNum = NULL;
    mPackedData = NULL;

    mHasNormals = mHasColors = mHasMultiTex = true;
    mHasTexCoords = true;
    mVertexCount = mFaceCount = 0;
    mVertexScale = 1.0f;
}

PLYMeshData::~PLYMeshData() {}

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

//void PLYMeshData::buildPackedData() {

//    mPackedElementCount = mVertexElementCount + (hasTexCoords?3:0);
//    mPackedData = BufferUtils.createByteBuffer(mFaceCount * 3 * mPackedElementCount * 4);

//    int curTexNum = 0;
//    for(int i=0; i<mFaceCount*3; i++) {

//        // Add all the vertex elements first (coordinates, normals, colors)
//        mVertexData.position(mFace.get()*mVertexElementCount);
//        for(int j=0; j<mVertexElementCount; j++) {
//            mPackedData.putFloat(mVertexData.get());
//        }

//        // Add UV coordinates if present
//        if(hasTexCoords) {
//            mPackedData.putFloat(mFaceUVData.get());
//            mPackedData.putFloat(mFaceUVData.get());

//            // Add texture index (will be 'z' of tex coord)
//            if(hasMultiTex && i%3 == 0) {
//                curTexNum = mFaceTexNum.get();
//            }
//            mPackedData.putFloat(curTexNum);
//        }
//    }

//    mPackedData.flip();

//    mFace.rewind();
//    mFaceUVData.rewind();
//    mFaceTexNum.rewind();
//    mVertexData.rewind();
//}

//void PLYMeshData::buildVBOs() {

//    // Make sure we have OpenGL capabilities before we proceed
//    GL.createCapabilities(false);

//    mPackedVBO = glGenBuffers();
//    glBindBuffer(GL_ARRAY_BUFFER, mPackedVBO);
//    glBufferData(GL_ARRAY_BUFFER, mPackedData, GL_STATIC_DRAW);
//    GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());

//    int stride = mPackedElementCount * 4;

//    GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
//    glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, false, stride, 0);
//    int offset = 3;

//    if(hasNormals) {
//        glVertexAttribPointer(ATTRIB_LOC_NORMAL, 3, GL_FLOAT, false, stride, offset*4);
//        GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
//        offset += 3;
//    }

//    if(hasColors) {
//        glVertexAttribPointer(ATTRIB_LOC_COLORS, 3, GL_FLOAT, false, stride, offset*4);
//        GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
//        offset += 3;
//    }

//    if(hasTexCoords) {
//        glVertexAttribPointer(ATTRIB_LOC_TEXCOR, 3, GL_FLOAT, false, stride, offset*4);
//        GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
//        offset += 3;
//    }

//    mFaceVBO = glGenBuffers();
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mFaceVBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mFace, GL_STATIC_DRAW);
//    GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
//}

//void releaseVBOs() {
//    GL.createCapabilities();
//    if(glIsBuffer(packedVBO)) { glDeleteBuffers(packedVBO); }
//    if(glIsBuffer(faceVBO)) { glDeleteBuffers(faceVBO); }
//}

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

    qInfo("Read %lu vertices and %lu faces.", vertColl.size(), faceColl.size());

    return true;
}
