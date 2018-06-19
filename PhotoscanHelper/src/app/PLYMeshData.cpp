#include "PLYMeshData.h"

using namespace tinyply;

#include <ZipLib/ZipFile.h>
#include <ZipLib/ZipArchive.h>
#include <ZipLib/ZipArchiveEntry.h>

#include <fstream>
using namespace std;

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

    mXCoordPropName = "x";
    mYCoordPropName = "y";
    mZCoordPropName = "z";
    mVertexIndexPropName = "vertex_index";
}

PLYMeshData::~PLYMeshData() {}

bool PLYMeshData::readPLYFile(QFileInfo pProjectFile, QString pFilename) {
    // Setup an input stream for reading
    istream* lInput = NULL;
    bool lIsDynamic = false;

    // Declare shared pointers at this scope so they aren't garbage collected
    // too soon and cause CRAZY errors.
    ZipArchive::Ptr lZipFile = nullptr;
    ZipArchiveEntry::Ptr lEntry = nullptr;

    // If needed, access through the psz project file
    if (pProjectFile.filePath() != "" && pProjectFile.exists()) {
        lZipFile = ZipFile::Open(pProjectFile.filePath().toStdString());
        if (lZipFile == nullptr) {
            qWarning("Could not open zip file %s", pProjectFile.filePath().toLocal8Bit().data());
            return false;
        }

        lEntry = lZipFile->GetEntry(pFilename.toStdString());
        if (lEntry == nullptr) {
            qWarning("Could not retrieve zip entry %s", pFilename.toLocal8Bit().data());
            return false;
        }

        lInput = lEntry->GetDecompressionStream();
        if (lInput == NULL) {
            qWarning("Error getting iostream for zip entry %s", pFilename.toLocal8Bit().data());
            return false;
        }
    }

    // No psz file so just open directly
    else {
        lInput = new ifstream(pFilename.toStdString(), ios::binary);
        lIsDynamic = true;
    }

    // Did we get a valid input stream?
    if (lInput == NULL || !lInput->good()) {
        if (lInput != NULL) {
            qWarning("PLY input stream is in a bad state.");
            if (lIsDynamic) {
                delete lInput;
            }
        } else {
            qWarning("Did not get an input stream for the PLY file.");
        }
        return false;
    }

    parsePLYFileStream(lInput);

    if (lEntry != nullptr) { lEntry->CloseDecompressionStream(); }
    if (lIsDynamic) { delete lInput; }

    return true;
}

QString PLYMeshData::validate() const {

    QString output = "";
    if(mXCoordPropName != "x" || mYCoordPropName != "y" || mZCoordPropName != "z") {
        output += QString::asprintf("\tWarning: file uses non-standard name for one or more vertex coordinate properties [%s, %s, %s].\n",
                mXCoordPropName.toLocal8Bit().data(), mYCoordPropName.toLocal8Bit().data(), mZCoordPropName.toLocal8Bit().data());
    }

    if(mVertexIndexPropName != "vertex_index") {
        output += QString::asprintf("\tWarning: file uses non-standard name for face vertex index [%s].",
                                 mVertexIndexPropName.toLocal8Bit().data());
    }

    return output;
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

bool PLYMeshData::parsePLYFileStream(std::istream* pInput) { // throws IOException {
    if(pInput == NULL) { return false; }

    // Parse the header info
    PlyFile lPLY;
    if(!lPLY.parse_header(*pInput)) {
        qWarning("Failed to parse ply header");
        return false;
    } else {
        // Examine elements and header information
        for (auto e : lPLY.get_elements()) {
            // Parse vertex element properties
            if (e.name.compare("vertex") == 0) {
                int i = 0;
                for (auto p : e.properties) {
                    switch(i) {
                        case 0: mXCoordPropName = QString::fromStdString(p.name); break;
                        case 1: mYCoordPropName = QString::fromStdString(p.name); break;
                        case 2: mZCoordPropName = QString::fromStdString(p.name); break;
                        default:
                            mHasColors = (p.name.compare("red") == 0 ||
                                          p.name.compare("green") == 0 ||
                                          p.name.compare("blue") == 0);
                        break;
                    }

                    i++;
                }
            }

            // Parse face element properties
            else if (e.name.compare("face") == 0) {
                int i = 0;
                for (auto p : e.properties) {
                    switch(i) {
                        case 0: mVertexIndexPropName = QString::fromStdString(p.name); break;
                        default:
                            mHasTexCoords = (p.name.compare("texcoord") == 0);
                        break;
                    }
                    i++;
                }
            }
        }
    }

    // Prepare to read the data
    std::shared_ptr<PlyData> lVerts, lFaces;

    // - Vertex data
    try {
        lVerts = lPLY.request_properties_from_element("vertex", {
            "x", "y", "z"
        });
    } catch (const std::exception & e) {
        qWarning("PLY vertex property request exception: : %s", e.what());
    }

    // - Face data
    try {
        if (false) {
            lFaces = lPLY.request_properties_from_element("face", {
                "vertex_indices",
                "texcoord"
            });
        } else {
            lFaces = lPLY.request_properties_from_element("face", {
                "vertex_indices"
            });
        }
    } catch (const std::exception & e) {
        qWarning("PLY face property request exception: %s", e.what());
    }

    // Read the data
    try {
        lPLY.read(*pInput);
        if (lVerts) { qInfo("\tRead %lu total vertices", lVerts->count); }
        else { qWarning("\tError reading vertices"); }

        if (lFaces) { qInfo("\tRead %lu total faces", lFaces->count); }
        else { qWarning("\tError reading faces"); }
    } catch (const std::exception & e) {
        qWarning("PLY reading exception: %s", e.what());
    }

    //    buildPackedData();
    return true;
}

//void PLYMeshData::readVertexData(ElementReader reader) { // throws IOException {

//    // Get number of elements
//    mVertexCount = reader.getCount();

//    // Allocate all the needed storage
//    mVertexElementCount = 9;
//    mVertexData = BufferUtils.createFloatBuffer(mVertexCount * mVertexElementCount);

//    // Reset bounds
//    mVertexMin[0] = mVertexMin[1] = mVertexMin[2] =  Float.MAX_VALUE;
//    mVertexMax[0] = mVertexMax[1] = mVertexMax[2] = -Float.MAX_VALUE;

//    // Examine the properties of the first element
//    Element element = reader.readElement();
//    ElementType ET = element.getType();

//    // Check coordinate property names (assumes the first three are X, Y and Z)
////		System.out.printf("\tThere are %d vertices with %d properties:\n", vertexCount, ET.getProperties().size()); // Debugging output
//    int X=1;
//    for(const Property P : ET.getProperties()) {
////			System.out.printf("\t\t[%2d]: %s (%s)\n", X, P.getName(), dataTypeToString(P));	// Debugging output
//        switch(X) {
//            case 1: xCoordPropName = P.getName(); break;
//            case 2: yCoordPropName = P.getName(); break;
//            case 3: zCoordPropName = P.getName(); break;
//        }
//        X++;
//    }

//    // Debugging output
////		System.out.println();
////		System.out.flush();

//    // Loop over all the elements
//    int count = 0;
//    while (element != NULL) {

//        // Check count to ensure we don't overflow the buffers
//        if(count >= mVertexCount) {
//            System.err.println("Error: more vertices than expected in ply file.");
//            break;
//        }

//        // Extract vertex coordinates (these are always there)
//        float x = (float)element.getDouble(xCoordPropName);
//        float y = (float)element.getDouble(yCoordPropName);
//        float z = (float)element.getDouble(zCoordPropName);

//        if(x < mVertexMin[0]) { mVertexMin[0] = x; }
//        if(y < mVertexMin[1]) { mVertexMin[1] = y; }
//        if(z < mVertexMin[2]) { mVertexMin[2] = z; }

//        if(x > mVertexMax[0]) { mVertexMax[0] = x; }
//        if(y > mVertexMax[1]) { mVertexMax[1] = y; }
//        if(z > mVertexMax[2]) { mVertexMax[2] = z; }

//        mVertexData.put(x);
//        mVertexData.put(y);
//        mVertexData.put(z);

//        // Try to extract normals (if not there, disable)
//        if(hasNormals) {
//            try {
//                mVertexData.put((float)element.getDouble("nx"));
//                mVertexData.put((float)element.getDouble("ny"));
//                mVertexData.put((float)element.getDouble("nz"));
//            } catch(Exception e) {
//                hasNormals = false;
//                mVertexElementCount -= 3;
//            }
//        }

//        // Try to extract colors (if not there, disable)
//        if(hasColors) {
//            try {
//                mVertexData.put(element.getInt("red")/255.0f);
//                mVertexData.put(element.getInt("green")/255.0f);
//                mVertexData.put(element.getInt("blue")/255.0f);
//            } catch(Exception e) {
//                hasColors = false;
//                mVertexElementCount -= 3;
//            }
//        }

//        // Advance to the next element
//        element = reader.readElement();
//        count++;
//    }

//    // Process the bounds to make the model unit size
//    float maxDim = -Float.MAX_VALUE;
//    for(int i=0; i<3; i++) {
//        mVertexBBox[i] = mVertexMax[i] - mVertexMin[i];
//        mVertexCenter[i] = (mVertexMax[i] + mVertexMin[i])/2.0f;
//        if(mVertexBBox[i] > maxDim) { maxDim = mVertexBBox[i]; }
//    }

//    mVertexScale = 1.0f/maxDim;

//    // Reset buffers for use by renderer
//    mVertexData.flip();
//}

//void PLYMeshData::readFaceData(ElementReader reader) { // throws IOException {

//    // Get number of elements
//    mFaceCount = reader.getCount();

//    // Allocate all the needed storage
//    mFace = BufferUtils.createIntBuffer(mFaceCount * 3);
//    mFaceUVData = BufferUtils.createFloatBuffer(mFaceCount * 6);
//    mFaceTexNum = BufferUtils.createIntBuffer(mFaceCount);

//    // Examine the properties of the first element
//    Element element = reader.readElement();
//    ElementType ET = element.getType();
////		System.out.printf("\tThere are %d Faces with %d properties\n", faceCount, ET.getProperties().size()); // Debugging output

//    // Check vertex index list property name (assumes the first one is the index list)
//    int X=1;
//    for(const Property P : ET.getProperties()) {
////			System.out.printf("\t\t[%2d]: %s (%s)\n", X, P.getName(), dataTypeToString(P));	// Debugging output
//        if(X == 1) { vertexIndexPropName = P.getName(); }
//        X++;
//    }

//    // Debugging output
////		System.out.println();
////		System.out.flush();

//    // Loop over all the elements
//    int count = 0;
//    while (element != NULL) {

//        // Check count to ensure we don't overflow the buffers
//        if(count >= mFaceCount) {
//            System.err.println("Warning: more faces than expected in ply file.");
//            break;
//        }

//        // Extract all the properties for this element
//        int[] indices = element.getIntList(vertexIndexPropName);
//        if(indices.length != 3) {
//            System.err.println("Warning: non-triangular face encountered.");
//            mFace.put(0); mFace.put(0); mFace.put(0);
//        } else {
//            mFace.put(indices);
//        }

//        // Try to extract face UV coordinates
//        if(hasTexCoords) {
//            try {
//                double[] tex = element.getDoubleList("texcoord");
//                for(int i=0; i<tex.length && i<6; i++) {
//                    mFaceUVData.put((float)tex[i]);
//                }
//            } catch(Exception e) {
//                hasTexCoords = false;
//                mFaceUVData = NULL; // Will get garbage collected
//            }
//        }

//        // Try to extract texture number
//        if(hasMultiTex) {
//            try {
//                int texNum = element.getInt("texnumber");
//                mFaceTexNum.put(texNum);
//            } catch(Exception e) {
//                hasMultiTex = false;
//                mFaceTexNum = NULL; // Will get garbage collected
//            }
//        }

//        // Advance to the next element
//        element = reader.readElement();
//        count++;
//    }

//    // Reset buffers to be used for rendering
//    mFace.flip();
//    mFaceUVData.flip();
//    mFaceTexNum.flip();
//}

//QString PLYMeshData::dataTypeToString(Property prop) {

//    QString suffix = "";
//    if(prop instanceof ListProperty) {
//        suffix = " List";
//    }

//    switch(prop.getType()) {
//        case CHAR: return "CHAR" + suffix;
//        case DOUBLE: return "DOUBLE" + suffix;
//        case FLOAT: return "FLOAT" + suffix;
//        case INT: return "INT" + suffix;
//        case SHORT: return "SHORT" + suffix;
//        case UCHAR: return "UCHAR" + suffix;
//        case UINT: return "UINT" + suffix;
//        case USHORT: return "USHORT" + suffix;
//        default: return "unknown" + suffix;
//    }
//}
