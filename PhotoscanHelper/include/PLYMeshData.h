#ifndef PLY_MESH_DATA_H
#define PLY_MESH_DATA_H

#include <QString>
#include <QFileInfo>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <ply_impl.h>
#pragma clang diagnostic pop

class QuaZipFile;
class QOpenGLContext;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOffscreenSurface;
class QScreen;

class PLYMeshData {
public:
    // OpenGL vertex attribute names
    static const int ATTRIB_LOC_VERTEX;
    static const int ATTRIB_LOC_NORMAL;
    static const int ATTRIB_LOC_COLORS;
    static const int ATTRIB_LOC_TEXCOR;

    // Constructor/Destructor
    PLYMeshData();
    ~PLYMeshData();

    // Get model mesh counts
    size_t getVertexCount() const { return mVertexCount; }
    size_t getFaceCount() const { return mFaceCount; }

    // Unit size transformation
    const float* getCenter() const { return mVertexCenter; }
    float getUnitScale() const { return mVertexScale; }

    // Get model data qualities
    bool withNormals() const { return mHasNormals; }
    bool withColors() const { return mHasColors; }
    bool withTexCoords() const { return mHasTexCoords; }
    bool withMultiTextures() const { return mHasMultiTex; }

    // Check for missing data
    bool isMissingData() const { return (!mHasNormals || !mHasColors || !mHasTexCoords); }

    // Get VBO names
    const QOpenGLBuffer* getFaceBuffer() const { return mFaceBuffer; }
    const QOpenGLBuffer* getVertexBuffer() const { return mVertexBuffer; }

    // Read data from a PLY file
    bool readPLYFile(QFileInfo pProjectFile, QString pFilename = "model0.ply");

    // Manage vertex buffer construction
    void buildBuffers();
    void destroyBuffers();
    void releaseBuffers();

private:
    // Initialize all members back to a starting state
    void initMembers();

    // PLY Parsing helper functions
    bool parsePLYFileStream(QString pFilename = "", QuaZipFile* pInsideFile = NULL);
    void processRawData(std::vector<PLY::VertexColor>& pVerts, std::vector<PLY::FaceTex>& pFaces);

    // Buffers for the vertex and face data
    QOpenGLBuffer *mVertexBuffer, *mFaceBuffer;
    QOpenGLVertexArrayObject *mVAO;

    // Packed data and metrics
    void *mPackedData;
    unsigned int mPackedElementCount, mDataStrideBytes;
    unsigned int mColorOffsetBytes, mTexcoordOffsetBytes;

    // Mesh element sizes
    size_t mVertexCount, mFaceCount;

    // Normalization data
    float mVertexMax[3], mVertexMin[3];
    float mVertexBBox[3], mVertexCenter[3];
    float mVertexScale;

    // Flags for presence of various mesh elements
    bool mHasNormals, mHasColors, mHasTexCoords, mHasMultiTex;

    // An offscreen surface and OpenGL context for general use
    static QOpenGLContext* msGLContext;
    static QOffscreenSurface* msGLSurface;
    static QScreen* mGLScreen;
};

#endif
