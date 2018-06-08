package edu.uwstout.berriers.PSHelper.Model;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import org.smurn.jply.*;
import org.smurn.jply.util.*;

import org.lwjgl.BufferUtils;
import org.lwjgl.opengl.GL;

import static org.lwjgl.opengl.GL11.GL_FLOAT;
import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL20.glVertexAttribPointer;

public class PLYMeshData {

	private int faceVBO, packedVBO, vertexElementCount, packedElementCount;	
	private FloatBuffer vertexData, faceUVData;
	private IntBuffer face, faceTexNum;
	private ByteBuffer packedData;

	private int vertexCount, faceCount;	
	
	private float vertexMax[], vertexMin[];
	private float vertexBBox[], vertexCenter[];
	private float vertexScale;
	
	private boolean hasNormals, hasColors, hasTexCoords, hasMultiTex;

	private String xCoordPropName, yCoordPropName, zCoordPropName, vertexIndexPropName;
	
	public static final int ATTRIB_LOC_VERTEX = 0;
	public static final int ATTRIB_LOC_NORMAL = 1;
	public static final int ATTRIB_LOC_COLORS = 2;
	public static final int ATTRIB_LOC_TEXCOR = 3;

	public PLYMeshData() {

		vertexMax = new float[3];
		vertexMin = new float[3];		
		vertexBBox = new float[3];
		vertexCenter = new float[3];
		
		packedVBO = -1;
		vertexElementCount = packedElementCount = 0;
		vertexData = faceUVData = null;
		face = faceTexNum = null;
		packedData = null;
		
		hasNormals = hasColors = hasMultiTex = true;
		hasTexCoords = true;
		vertexCount = faceCount = 0;
		vertexScale = 1.0f;
		
		xCoordPropName = "x";
		yCoordPropName = "y";
		zCoordPropName = "z";
		vertexIndexPropName = "vertex_index";
	}
	
	// Get model mesh counts
	public int getVertexCount() { return vertexCount; }
	public int getFaceCount() { return faceCount; }

	// Unit size transformation
	public float[] getCenter() { return vertexCenter; }
	public float getUnitScale() { return vertexScale; }

	// Get model data qualities
	public boolean withNormals() { return hasNormals; }
	public boolean withColors() { return hasColors; }
	public boolean withTexCoords() { return hasTexCoords; }
	public boolean withMultiTextures() { return hasMultiTex; }

	// Check for missing data
	public boolean isMissingData() {
		return (!hasNormals || !hasColors || !hasTexCoords);
	}
	
	// Get VBO names
	public int getFaceVBO() { return faceVBO; }
	public int getPackedVBO() { return packedVBO; }

	public boolean readPLYFile(File pProjectFile) throws IOException {
		return readPLYFile(pProjectFile, "model0.ply");
	}
	
	public boolean readPLYFile(File pProjectFile, String pFilename) throws IOException {
		boolean success = false;
		if(pProjectFile != null) {
			// Declare variables that will be set inside of try-catch block
			ZipFile lPSXFile = null;
			InputStream lPLYFileStream = null;
	
			// Try to open the PSZ file and obtain a stream for the doc.xml file
			try {
				lPSXFile = new ZipFile(pProjectFile, ZipFile.OPEN_READ);
				ZipEntry lPLYFileEntry = lPSXFile.getEntry(pFilename);
				if(lPLYFileEntry != null) {
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
			} finally {
				if(lPLYFileStream != null) {
					lPLYFileStream.close();
				}
				
				if(lPSXFile != null) {
					lPSXFile.close();
				}
			}
		}
		
		return success;
	}
	
	private boolean parsePLYFileStream(InputStream inStream) throws IOException {
		if(inStream == null) { return false; }
		
		PlyReader ply = null;
		// Open PLY File
		ply = new PlyReaderFile(inStream);

		// Wrap with a normalizing filter
		ply = new NormalizingPlyReader(ply, TesselationMode.PASS_THROUGH,
            		NormalMode.ADD_NORMALS_CCW, TextureMode.PASS_THROUGH);
					
		// Debugging output
//			System.out.printf("\tModel has %d PLY elements\n", ply.getElementTypes().size());
//			int X=1;
//			for(final ElementType ET : ply.getElementTypes()) {
//				System.out.printf("\t\t[%2d]: %s\n", X, ET.getName());
//				X++;
//			}			
//			System.out.println();
//			System.out.flush();
		
		// Examine each element in the PLY file			
		ElementReader reader = ply.nextElementReader();
		while (reader != null) {
			
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
	
	public String validate() {
		
		String output = "";
		if(!xCoordPropName.equals("x") || !yCoordPropName.equals("y") || !zCoordPropName.equals("z")) {
			output += String.format("\tWarning: file uses non-standard name for one or more vertex coordinate properties [%s, %s, %s].\n",
					xCoordPropName, yCoordPropName, zCoordPropName);
		}
				
		if(!vertexIndexPropName.equals("vertex_index")) {
			output += String.format("\tWarning: file uses non-standard name for face vertex index [%s].", vertexIndexPropName);			
		}		
		
		return output;
	}
	
	private void readVertexData(ElementReader reader) throws IOException {
		
		// Get number of elements
		vertexCount = reader.getCount();
		
		// Allocate all the needed storage
		vertexElementCount = 9;
		vertexData = BufferUtils.createFloatBuffer(vertexCount * vertexElementCount);

		// Reset bounds
		vertexMin[0] = vertexMin[1] = vertexMin[2] =  Float.MAX_VALUE;
		vertexMax[0] = vertexMax[1] = vertexMax[2] = -Float.MAX_VALUE;
		
		// Examine the properties of the first element
		Element element = reader.readElement();
		ElementType ET = element.getType();

		// Check coordinate property names (assumes the first three are X, Y and Z)
//		System.out.printf("\tThere are %d vertices with %d properties:\n", vertexCount, ET.getProperties().size()); // Debugging output
		int X=1;
		for(final Property P : ET.getProperties()) {
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
		while (element != null) {

			// Check count to ensure we don't overflow the buffers
			if(count >= vertexCount) {
				System.err.println("Error: more vertices than expected in ply file.");
				break;
			}
			
			// Extract vertex coordinates (these are always there)
			float x = (float)element.getDouble(xCoordPropName);
			float y = (float)element.getDouble(yCoordPropName);
			float z = (float)element.getDouble(zCoordPropName);
			
			if(x < vertexMin[0]) { vertexMin[0] = x; }
			if(y < vertexMin[1]) { vertexMin[1] = y; }
			if(z < vertexMin[2]) { vertexMin[2] = z; }
			
			if(x > vertexMax[0]) { vertexMax[0] = x; }
			if(y > vertexMax[1]) { vertexMax[1] = y; }
			if(z > vertexMax[2]) { vertexMax[2] = z; }
			
			vertexData.put(x);
			vertexData.put(y);
			vertexData.put(z);

			// Try to extract normals (if not there, disable)
			if(hasNormals) {
				try {
					vertexData.put((float)element.getDouble("nx"));
					vertexData.put((float)element.getDouble("ny"));
					vertexData.put((float)element.getDouble("nz"));
				} catch(Exception e) {
					hasNormals = false;
					vertexElementCount -= 3;
				}
			}
			
			// Try to extract colors (if not there, disable)
			if(hasColors) {
				try {
					vertexData.put(element.getInt("red")/255.0f);
					vertexData.put(element.getInt("green")/255.0f);
					vertexData.put(element.getInt("blue")/255.0f);
				} catch(Exception e) {
					hasColors = false;
					vertexElementCount -= 3;
				}
			}			
			
			// Advance to the next element
			element = reader.readElement();
			count++;
		}
		
		// Process the bounds to make the model unit size
		float maxDim = -Float.MAX_VALUE;
		for(int i=0; i<3; i++) {
			vertexBBox[i] = vertexMax[i] - vertexMin[i];
			vertexCenter[i] = (vertexMax[i] + vertexMin[i])/2.0f;
			if(vertexBBox[i] > maxDim) { maxDim = vertexBBox[i]; }
		}
		
		vertexScale = 1.0f/maxDim;
		
		// Reset buffers for use by renderer
		vertexData.flip();
	}

	private void readFaceData(ElementReader reader) throws IOException {
		
		// Get number of elements
		faceCount = reader.getCount();

		// Allocate all the needed storage
		face = BufferUtils.createIntBuffer(faceCount * 3);
		faceUVData = BufferUtils.createFloatBuffer(faceCount * 6);
		faceTexNum = BufferUtils.createIntBuffer(faceCount);
		
		// Examine the properties of the first element
		Element element = reader.readElement();
		ElementType ET = element.getType();
//		System.out.printf("\tThere are %d Faces with %d properties\n", faceCount, ET.getProperties().size()); // Debugging output

		// Check vertex index list property name (assumes the first one is the index list)
		int X=1;
		for(final Property P : ET.getProperties()) {
//			System.out.printf("\t\t[%2d]: %s (%s)\n", X, P.getName(), dataTypeToString(P));	// Debugging output
			if(X == 1) { vertexIndexPropName = P.getName(); }
			X++;
		}

		// Debugging output
//		System.out.println();
//		System.out.flush();
		
		// Loop over all the elements
		int count = 0;
		while (element != null) {

			// Check count to ensure we don't overflow the buffers
			if(count >= faceCount) {
				System.err.println("Warning: more faces than expected in ply file.");
				break;
			}
			
			// Extract all the properties for this element
			int[] indices = element.getIntList(vertexIndexPropName);
			if(indices.length != 3) {
				System.err.println("Warning: non-triangular face encountered.");
				face.put(0); face.put(0); face.put(0);
			} else {
				face.put(indices);
			}

			// Try to extract face UV coordinates
			if(hasTexCoords) {
				try {
					double[] tex = element.getDoubleList("texcoord");
					for(int i=0; i<tex.length && i<6; i++) {
						faceUVData.put((float)tex[i]);
					}
				} catch(Exception e) {
					hasTexCoords = false;
					faceUVData = null; // Will get garbage collected
				}
			}
			
			// Try to extract texture number
			if(hasMultiTex) {
				try {
					int texNum = element.getInt("texnumber");
					faceTexNum.put(texNum);
				} catch(Exception e) {
					hasMultiTex = false;
					faceTexNum = null; // Will get garbage collected
				}
			}
			
			// Advance to the next element
			element = reader.readElement();
			count++;
		}

		// Reset buffers to be used for rendering
		face.flip();
		faceUVData.flip();
		faceTexNum.flip();		
	}
	
	@SuppressWarnings("unused")
	private static String dataTypeToString(Property prop) {
		
		String suffix = "";
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

	public void buildPackedData() {

		packedElementCount = vertexElementCount + (hasTexCoords?3:0);
		packedData = BufferUtils.createByteBuffer(faceCount * 3 * packedElementCount * 4);
		
		int curTexNum = 0;
		for(int i=0; i<faceCount*3; i++) {
			
			// Add all the vertex elements first (coordinates, normals, colors)
			vertexData.position(face.get()*vertexElementCount);
			for(int j=0; j<vertexElementCount; j++) {
				packedData.putFloat(vertexData.get());
			}

			// Add UV coordinates if present
			if(hasTexCoords) {
				packedData.putFloat(faceUVData.get());
				packedData.putFloat(faceUVData.get());

				// Add texture index (will be 'z' of tex coord)
				if(hasMultiTex && i%3 == 0) {
					curTexNum = faceTexNum.get();
				}
				packedData.putFloat(curTexNum);
			}
		}

		packedData.flip();

		face.rewind();
		faceUVData.rewind();
		faceTexNum.rewind();
		vertexData.rewind();
	}
	
	public void buildVBOs() {

		// Make sure we have OpenGL capabilities before we proceed
		GL.createCapabilities(false);
		
		packedVBO = glGenBuffers();
		glBindBuffer(GL_ARRAY_BUFFER, packedVBO);
		glBufferData(GL_ARRAY_BUFFER, packedData, GL_STATIC_DRAW);
		GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());

		int stride = packedElementCount * 4;
	    
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

		faceVBO = glGenBuffers();
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceVBO);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face, GL_STATIC_DRAW);
		GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
	}
	
	public void releaseVBOs() {
		GL.createCapabilities();
		if(glIsBuffer(packedVBO)) { glDeleteBuffers(packedVBO); }
		if(glIsBuffer(faceVBO)) { glDeleteBuffers(faceVBO); }
	}
}
