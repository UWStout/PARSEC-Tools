#ifndef PLY_MESH_DATA_H
#define PLY_MESH_DATA_H

#include <QString>
#include <QFileInfo>

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
    int getVertexCount() const { return mVertexCount; }
    int getFaceCount() const { return mFaceCount; }

    // Unit size transformation
    const float* getCenter() const { return mVertexCenter; }
    float getUnitScale() const { return mVertexScale; }

    // Get model data qualities
    bool withNormals() const { return hasNormals; }
    bool withColors() const { return hasColors; }
    bool withTexCoords() const { return hasTexCoords; }
    bool withMultiTextures() const { return hasMultiTex; }

    // Check for missing data
    bool isMissingData() const { return (!hasNormals || !hasColors || !hasTexCoords); }

    // Get VBO names
    int getFaceVBO() const { return mFaceVBO; }
    int getPackedVBO() const { return mPackedVBO; }

    // Read data from a PLY file
    bool readPLYFile(QFileInfo pProjectFile, QString pFilename = "model0.ply");

    // Make sure PLY data names are standard
    QString validate() const;

    // Manage VBO construction
//    void buildPackedData();
//    void buildVBOs();
//    void releaseVBOs();

private:
//    // PLY PArsing helper functions
//    bool parsePLYFileStream(InputStream inStream);
//    void readVertexData(ElementReader reader);
//    void readFaceData(ElementReader reader);

//    // Return a PLY data type as a string
//    static QString dataTypeToString(Property prop);

    // Names of our VBOs
    int mFaceVBO, mPackedVBO;

    // Sizes of the PLY data
    int mVertexElementCount, mPackedElementCount;

    // Raw, unpacked data arrays
    float *mVertexData, *mFaceUVData;
    int *mFace, *mFaceTexNum;

    // Packed data
    unsigned char *mPackedData;

    // Mesh element sizes
    int mVertexCount, mFaceCount;

    // Normalization data
    float mVertexMax[3], mVertexMin[3];
    float mVertexBBox[3], mVertexCenter[3];
    float mVertexScale;

    // Flags for presence of various mesh elements
    bool hasNormals, hasColors, hasTexCoords, hasMultiTex;

    // Names of mesh elements inside of the PLY file
    QString xCoordPropName, yCoordPropName, zCoordPropName, vertexIndexPropName;
};

#endif
