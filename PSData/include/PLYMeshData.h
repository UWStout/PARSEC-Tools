#ifndef PLY_MESH_DATA_H
#define PLY_MESH_DATA_H

#include "psdata_global.h"

#include <QString>
#include <QFileInfo>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4100)
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include <ply_impl.h>

#ifdef _WIN32
#pragma warning(pop)
#else
#pragma clang diagnostic pop
#endif

class QuaZipFile;

class QOpenGLTexture;
class QOpenGLBuffer;
class QOpenGLContext;
class QOpenGLVertexArrayObject;
class QOpenGLFunctions;

class PSDATASHARED_EXPORT PLYMeshData {
public:
    // OpenGL vertex attribute names
    static const int ATTRIB_LOC_VERTEX;
    static const int ATTRIB_LOC_NORMAL;
    static const int ATTRIB_LOC_COLORS;
    static const int ATTRIB_LOC_TEXCOR;

    // Constructor/Destructor
    PLYMeshData();
    ~PLYMeshData();

    // Access texture file name and texture object
    QFileInfo getTextureFile(int pIdx = 0) const { return mTextureFile[pIdx]; }
    void setTextureFile(QFileInfo pTextureFile, int pIdx = 0) { mTextureFile[pIdx] = pTextureFile; }
    void bindTextures(QOpenGLFunctions* GL);

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

    // Get VBO/VAO objects
    QOpenGLBuffer* getVertexBuffer() { return mVertexBuffer; }
    QOpenGLVertexArrayObject* getVAO() { return mVAO; }

    // Read data from a PLY file
    bool readPLYFile(QFileInfo pProjectFile, QString pFilename = "model0.ply", QFileInfo pTextureFile = QFileInfo());

    // Manage OpenGL Texture Construction
    void buildTextures();

    // Manage vertex buffer construction
    void buildBuffers(QOpenGLContext *pGLContext);
    void destroyBuffers();
    void releaseBuffers();

private:
    // Initialize all members back to a starting state
    void initMembers();

    // PLY Parsing helper functions
    bool parsePLYFileStream(QString pFilename = "", QuaZipFile* pInsideFile = nullptr);
    void processRawData();

    // Buffers for the vertex and face data
    QOpenGLBuffer *mVertexBuffer;
    QOpenGLVertexArrayObject *mVAO;

    // Texture
    QFileInfo mTextureFile[4];
    QOpenGLTexture *mGLTexture[4];

    // Packed data and metrics
    void *mPackedData;

    // PLY Data Storage
    std::vector<PLY::VertexNCT> mPLYVertCollection;
    std::vector<PLY::FaceTex> mPLYFaceCollection;

    // Mesh element sizes
    size_t mVertexCount, mFaceCount;

    // Normalization data
    float mVertexMax[3], mVertexMin[3];
    float mVertexBBox[3], mVertexCenter[3];
    float mVertexScale;

    // Flags for presence of various mesh elements
    bool mHasNormals, mHasColors, mHasTexCoords, mHasMultiTex;
};

#endif
