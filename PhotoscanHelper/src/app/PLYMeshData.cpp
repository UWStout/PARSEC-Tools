#include "PLYMeshData.h"

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

    hasNormals = hasColors = hasMultiTex = true;
    hasTexCoords = true;
    mVertexCount = mFaceCount = 0;
    mVertexScale = 1.0f;

    xCoordPropName = "x";
    yCoordPropName = "y";
    zCoordPropName = "z";
    vertexIndexPropName = "vertex_index";
}

PLYMeshData::~PLYMeshData() {}

bool PLYMeshData::readPLYFile(QFileInfo pProjectFile, QString pFilename) { // throws IOException {
    bool success = false;
    if(pProjectFile != NULL) {
        // Declare variables that will be set inside of try-catch block
        ZipFile lPSXFile = NULL;
        InputStream lPLYFileStream = NULL;

        // Try to open the PSZ file and obtain a stream for the doc.xml file
        try {
            lPSXFile = new ZipFile(pProjectFile, ZipFile.OPEN_READ);
            ZipEntry lPLYFileEntry = lPSXFile.getEntry(pFilename);
            if(lPLYFileEntry != NULL) {
                lPLYFileStream = lPSXFile.getInputStream(lPLYFileEntry);

                // Parse the PLY file
                success = parsePLYFileStream(lPLYFileStream);
            } else {
                throw new ZipException("could not find project PLY file.");
            }
        } catch (ZipException e) {
            throw new IOException("PSZ Error: ZIP error while reading PSZ file ("
                                    + e.getMessage() +").", e);
        } catch (IOException e) {
            throw new IOException("PSZ Error: IO error reading ply file ("
                                    + e.getMessage() + ").", e);
        } catch (Exception e) {
            e.printStackTrace();
            throw new IOException("PSZ Error: general error while reading ply file ("
                                    + e.getMessage() +").", e);
        }

        if(lPLYFileStream != NULL) {
            lPLYFileStream.close();
        }

        if(lPSXFile != NULL) {
            lPSXFile.close();
        }
    }

    return success;
}

QString PLYMeshData::validate() const {

    QString output = "";
    if(!xCoordPropName == "x" || !yCoordPropName == "y" || !zCoordPropName == "z") {
        output += QString.format("\tWarning: file uses non-standard name for one or more vertex coordinate properties [%s, %s, %s].\n",
                xCoordPropName, yCoordPropName, zCoordPropName);
    }

    if(!vertexIndexPropName == "vertex_index") {
        output += QString.format("\tWarning: file uses non-standard name for face vertex index [%s].", vertexIndexPropName);
    }

    return output;
}


void PLYMeshData::buildPackedData() {

    mPackedElementCount = mVertexElementCount + (hasTexCoords?3:0);
    mPackedData = BufferUtils.createByteBuffer(mFaceCount * 3 * mPackedElementCount * 4);

    int curTexNum = 0;
    for(int i=0; i<mFaceCount*3; i++) {

        // Add all the vertex elements first (coordinates, normals, colors)
        mVertexData.position(mFace.get()*mVertexElementCount);
        for(int j=0; j<mVertexElementCount; j++) {
            mPackedData.putFloat(mVertexData.get());
        }

        // Add UV coordinates if present
        if(hasTexCoords) {
            mPackedData.putFloat(mFaceUVData.get());
            mPackedData.putFloat(mFaceUVData.get());

            // Add texture index (will be 'z' of tex coord)
            if(hasMultiTex && i%3 == 0) {
                curTexNum = mFaceTexNum.get();
            }
            mPackedData.putFloat(curTexNum);
        }
    }

    mPackedData.flip();

    mFace.rewind();
    mFaceUVData.rewind();
    mFaceTexNum.rewind();
    mVertexData.rewind();
}

void PLYMeshData::buildVBOs() {

    // Make sure we have OpenGL capabilities before we proceed
    GL.createCapabilities(false);

    mPackedVBO = glGenBuffers();
    glBindBuffer(GL_ARRAY_BUFFER, mPackedVBO);
    glBufferData(GL_ARRAY_BUFFER, mPackedData, GL_STATIC_DRAW);
    GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());

    int stride = mPackedElementCount * 4;

    GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
    glVertexAttribPointer(ATTRIB_LOC_VERTEX, 3, GL_FLOAT, false, stride, 0);
    int offset = 3;

    if(hasNormals) {
        glVertexAttribPointer(ATTRIB_LOC_NORMAL, 3, GL_FLOAT, false, stride, offset*4);
        GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
        offset += 3;
    }

    if(hasColors) {
        glVertexAttribPointer(ATTRIB_LOC_COLORS, 3, GL_FLOAT, false, stride, offset*4);
        GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
        offset += 3;
    }

    if(hasTexCoords) {
        glVertexAttribPointer(ATTRIB_LOC_TEXCOR, 3, GL_FLOAT, false, stride, offset*4);
        GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
        offset += 3;
    }

    mFaceVBO = glGenBuffers();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mFaceVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mFace, GL_STATIC_DRAW);
    GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
}

void releaseVBOs() {
    GL.createCapabilities();
    if(glIsBuffer(packedVBO)) { glDeleteBuffers(packedVBO); }
    if(glIsBuffer(faceVBO)) { glDeleteBuffers(faceVBO); }
}

bool PLYMeshData::parsePLYFileStream(InputStream inStream) { // throws IOException {
    if(inStream == NULL) { return false; }

    PlyReader ply = NULL;
    // Open PLY QFileInfo
    ply = new PlyReaderFile(inStream);

    // Wrap with a normalizing filter
    ply = new NormalizingPlyReader(ply, TesselationMode.PASS_THROUGH,
                NormalMode.ADD_NORMALS_CCW, TextureMode.PASS_THROUGH);

    // Debugging output
//			System.out.printf("\tModel has %d PLY elements\n", ply.getElementTypes().size());
//			int X=1;
//			for(const ElementType ET : ply.getElementTypes()) {
//				System.out.printf("\t\t[%2d]: %s\n", X, ET.getName());
//				X++;
//			}
//			System.out.println();
//			System.out.flush();

    // Examine each element in the PLY file
    ElementReader reader = ply.nextElementReader();
    while (reader != NULL) {
        try {
            // Look at type
            if(reader.getElementType().getName().equals("vertex")) {
                readVertexData(reader);
            } else if(reader.getElementType().getName().equals("face")) {
                readFaceData(reader);
            }
        } catch(Exception e) {
            System.out.println("ERROR: PLY reading error");
            e.printStackTrace();
        }

        // Close the reader for the current type before getting the next one.
        reader.close();
        reader = ply.nextElementReader();
    }

    ply.close();


    buildPackedData();
    return true;
}

void PLYMeshData::readVertexData(ElementReader reader) { // throws IOException {

    // Get number of elements
    mVertexCount = reader.getCount();

    // Allocate all the needed storage
    mVertexElementCount = 9;
    mVertexData = BufferUtils.createFloatBuffer(mVertexCount * mVertexElementCount);

    // Reset bounds
    mVertexMin[0] = mVertexMin[1] = mVertexMin[2] =  Float.MAX_VALUE;
    mVertexMax[0] = mVertexMax[1] = mVertexMax[2] = -Float.MAX_VALUE;

    // Examine the properties of the first element
    Element element = reader.readElement();
    ElementType ET = element.getType();

    // Check coordinate property names (assumes the first three are X, Y and Z)
//		System.out.printf("\tThere are %d vertices with %d properties:\n", vertexCount, ET.getProperties().size()); // Debugging output
    int X=1;
    for(const Property P : ET.getProperties()) {
//			System.out.printf("\t\t[%2d]: %s (%s)\n", X, P.getName(), dataTypeToString(P));	// Debugging output
        switch(X) {
            case 1: xCoordPropName = P.getName(); break;
            case 2: yCoordPropName = P.getName(); break;
            case 3: zCoordPropName = P.getName(); break;
        }
        X++;
    }

    // Debugging output
//		System.out.println();
//		System.out.flush();

    // Loop over all the elements
    int count = 0;
    while (element != NULL) {

        // Check count to ensure we don't overflow the buffers
        if(count >= mVertexCount) {
            System.err.println("Error: more vertices than expected in ply file.");
            break;
        }

        // Extract vertex coordinates (these are always there)
        float x = (float)element.getDouble(xCoordPropName);
        float y = (float)element.getDouble(yCoordPropName);
        float z = (float)element.getDouble(zCoordPropName);

        if(x < mVertexMin[0]) { mVertexMin[0] = x; }
        if(y < mVertexMin[1]) { mVertexMin[1] = y; }
        if(z < mVertexMin[2]) { mVertexMin[2] = z; }

        if(x > mVertexMax[0]) { mVertexMax[0] = x; }
        if(y > mVertexMax[1]) { mVertexMax[1] = y; }
        if(z > mVertexMax[2]) { mVertexMax[2] = z; }

        mVertexData.put(x);
        mVertexData.put(y);
        mVertexData.put(z);

        // Try to extract normals (if not there, disable)
        if(hasNormals) {
            try {
                mVertexData.put((float)element.getDouble("nx"));
                mVertexData.put((float)element.getDouble("ny"));
                mVertexData.put((float)element.getDouble("nz"));
            } catch(Exception e) {
                hasNormals = false;
                mVertexElementCount -= 3;
            }
        }

        // Try to extract colors (if not there, disable)
        if(hasColors) {
            try {
                mVertexData.put(element.getInt("red")/255.0f);
                mVertexData.put(element.getInt("green")/255.0f);
                mVertexData.put(element.getInt("blue")/255.0f);
            } catch(Exception e) {
                hasColors = false;
                mVertexElementCount -= 3;
            }
        }

        // Advance to the next element
        element = reader.readElement();
        count++;
    }

    // Process the bounds to make the model unit size
    float maxDim = -Float.MAX_VALUE;
    for(int i=0; i<3; i++) {
        mVertexBBox[i] = mVertexMax[i] - mVertexMin[i];
        mVertexCenter[i] = (mVertexMax[i] + mVertexMin[i])/2.0f;
        if(mVertexBBox[i] > maxDim) { maxDim = mVertexBBox[i]; }
    }

    mVertexScale = 1.0f/maxDim;

    // Reset buffers for use by renderer
    mVertexData.flip();
}

void PLYMeshData::readFaceData(ElementReader reader) { // throws IOException {

    // Get number of elements
    mFaceCount = reader.getCount();

    // Allocate all the needed storage
    mFace = BufferUtils.createIntBuffer(mFaceCount * 3);
    mFaceUVData = BufferUtils.createFloatBuffer(mFaceCount * 6);
    mFaceTexNum = BufferUtils.createIntBuffer(mFaceCount);

    // Examine the properties of the first element
    Element element = reader.readElement();
    ElementType ET = element.getType();
//		System.out.printf("\tThere are %d Faces with %d properties\n", faceCount, ET.getProperties().size()); // Debugging output

    // Check vertex index list property name (assumes the first one is the index list)
    int X=1;
    for(const Property P : ET.getProperties()) {
//			System.out.printf("\t\t[%2d]: %s (%s)\n", X, P.getName(), dataTypeToString(P));	// Debugging output
        if(X == 1) { vertexIndexPropName = P.getName(); }
        X++;
    }

    // Debugging output
//		System.out.println();
//		System.out.flush();

    // Loop over all the elements
    int count = 0;
    while (element != NULL) {

        // Check count to ensure we don't overflow the buffers
        if(count >= mFaceCount) {
            System.err.println("Warning: more faces than expected in ply file.");
            break;
        }

        // Extract all the properties for this element
        int[] indices = element.getIntList(vertexIndexPropName);
        if(indices.length != 3) {
            System.err.println("Warning: non-triangular face encountered.");
            mFace.put(0); mFace.put(0); mFace.put(0);
        } else {
            mFace.put(indices);
        }

        // Try to extract face UV coordinates
        if(hasTexCoords) {
            try {
                double[] tex = element.getDoubleList("texcoord");
                for(int i=0; i<tex.length && i<6; i++) {
                    mFaceUVData.put((float)tex[i]);
                }
            } catch(Exception e) {
                hasTexCoords = false;
                mFaceUVData = NULL; // Will get garbage collected
            }
        }

        // Try to extract texture number
        if(hasMultiTex) {
            try {
                int texNum = element.getInt("texnumber");
                mFaceTexNum.put(texNum);
            } catch(Exception e) {
                hasMultiTex = false;
                mFaceTexNum = NULL; // Will get garbage collected
            }
        }

        // Advance to the next element
        element = reader.readElement();
        count++;
    }

    // Reset buffers to be used for rendering
    mFace.flip();
    mFaceUVData.flip();
    mFaceTexNum.flip();
}

QString PLYMeshData::dataTypeToString(Property prop) {

    QString suffix = "";
    if(prop instanceof ListProperty) {
        suffix = " List";
    }

    switch(prop.getType()) {
        case CHAR: return "CHAR" + suffix;
        case DOUBLE: return "DOUBLE" + suffix;
        case FLOAT: return "FLOAT" + suffix;
        case INT: return "INT" + suffix;
        case SHORT: return "SHORT" + suffix;
        case UCHAR: return "UCHAR" + suffix;
        case UINT: return "UINT" + suffix;
        case USHORT: return "USHORT" + suffix;
        default: return "unknown" + suffix;
    }
}
