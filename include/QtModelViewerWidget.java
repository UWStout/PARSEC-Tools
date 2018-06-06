package edu.uwstout.berriers.PSHelper.UILogic;

import org.lwjgl.opengl.*;

// We only support up to OpenGL 2.1 (can't go higher on mac due to bug in Qt 4.8)
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL13.*;
import static org.lwjgl.opengl.GL15.*;
import static org.lwjgl.opengl.GL20.*;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;

import org.lwjgl.BufferUtils;

import com.trolltech.qt.core.QPointF;
import com.trolltech.qt.core.QTimer;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QColor;
import com.trolltech.qt.gui.QImage;
import com.trolltech.qt.gui.QMatrix4x4;
import com.trolltech.qt.gui.QMouseEvent;
import com.trolltech.qt.gui.QVector3D;
import com.trolltech.qt.gui.QWheelEvent;
import com.trolltech.qt.gui.QWidget;

import com.trolltech.qt.opengl.QGLWidget;
import com.trolltech.qt.opengl.QGLFormat;
import com.trolltech.qt.opengl.QGLShader;
import com.trolltech.qt.opengl.QGLShaderProgram;

import edu.uwstout.berriers.PSHelper.Model.*;

public class QtModelViewerWidget extends QGLWidget {

	// Different rendering modes
	public enum RenderMode {
		RENDER_SINGLE_COLOR("Flat Color: single solid color over entire model."),
		RENDER_VERTEX_COLOR("Flat Color: per-vertex color values (no shading)."),
		RENDER_TEXTURE_COLOR("Flat Color: colors from the texture image (no shading)."),

		RENDER_SHADED("Lit and Shaded: fixed light source using surface normals."),
		RENDER_VERTEX_SHADED("Lit and shaded: per-vertex color values."),
		RENDER_TEXTURE_SHADED("Lit and shaded: colors from the texture image."),
		
		RENDER_VERTEX_DATA("Analysis: vertex coordinates as colors."),
		RENDER_NORMS_DATA("Analysis: surface normals as colors."),
		RENDER_UV_DATA("Analysis: texture coordinates as colors."),
		RENDER_TEXNUM_DATA("Analysis: texture index as color.");
		
		public final String description;
		
		private RenderMode(String description) {
			this.description = description;
		}
		
		@Override
		public String toString() { return description; }
	}
	
	private QMatrix4x4 persp, model, view;
	private QGLShaderProgram texturedShader;
	private float camZPos, translateScale;
	
	private int perspLoc, modelLoc, viewLoc, normalMatLoc, colorUniformLoc;
	private int renderModeLoc, lightPositionLoc;

	private int colorTexLoc[] = new int[4]; 
	private int colorTextureID[] = new int[4];

	private PLYMeshData meshData;
	private QImage colorTexture[] = new QImage[4];
	private QColor uniformColor;
	private RenderMode renderMode;
	
	// Cube example object
	private int cubePackedVBO, cubeFaceElements;
	
	// Trackball and translate related
    QtTrackball trackball;
    boolean trackballEnabled, rotating;
    QTimer idleTimer;
    QPointF translate, lastMousePos;
	
	public QtModelViewerWidget(QWidget parent)
	{
		super(QGLFormat.defaultFormat(), parent);
	    
		persp = new QMatrix4x4();
		model = new QMatrix4x4();
		view = new QMatrix4x4();
		camZPos = 5.0f;		
		adjustCameraPosition(0.0f);
		
		meshData = null;
		uniformColor = new QColor(255, 255, 255);
		renderMode = RenderMode.RENDER_TEXTURE_COLOR;
		cubePackedVBO = cubeFaceElements = -1;

		colorTexLoc[0] = colorTexLoc[1] = colorTexLoc[2] = colorTexLoc[3] = -1;
		colorTextureID[0] = colorTextureID[1] = colorTextureID[2] = colorTextureID[3] = -1;
		
	    // Update similar to an idle function
	    idleTimer = new QTimer(this);
	    idleTimer.setInterval(20);
	    idleTimer.timeout.connect(this, "updateGLFromTimer()");
	    idleTimer.start();

	    trackball = new QtTrackball(0.02f, true, new QVector3D(0, 1, 0), QtTrackball.TrackMode.TRACKMODE_SPHERE);
	    trackballEnabled = true; rotating = false;
	    lastMousePos = new QPointF();
	    translate = new QPointF(0, 0);
	}

	public void setModelData(QImage[] colorTexture, PLYMeshData meshData)
	{
		// Make the context current in this thread so we can do OpenGL stuff!
		context().makeCurrent();
		
		// Clean up any previous texture data
		for(int i=0; i<4; i++) {
			if(glIsTexture(colorTextureID[i])) {
				deleteTexture(colorTextureID[i]);
			}
			
			colorTextureID[i] = -1;
			this.colorTexture[i] = null;
		}

		// Clear out the VBO data from the GPU
		if(this.meshData != null) {
			this.meshData.releaseVBOs();
		}

		// Set new textures
		if(colorTexture != null) {
			for(int i=0; i<4; i++) {
				this.colorTexture[i] = colorTexture[i];
		
				// Pass texture(s) into the context
				if(this.colorTexture[i] != null) {
		    		glActiveTexture(GL_TEXTURE0 + i);
					colorTextureID[i] = bindTexture(this.colorTexture[i]);
				}
			}
		}

		// Set new mesh dat
		this.meshData = meshData;
		if(this.meshData != null) {
			this.meshData.buildVBOs();
		}
	}
	
	public void setRenderMode(int index) {
		if(index < 0 || index >= RenderMode.values().length) { return; }
		renderMode = RenderMode.values()[index];
		
		System.out.println("Render mode changed to '" + renderMode.description + "'");
		
		updateGL();
	}

	public void setFlatColor(QColor newColor) {
		uniformColor = newColor;
		updateGL();
	}

	public QColor getFlatColor() { return uniformColor; }

	private QPointF normalizePixelPos(QPointF p)
	{
	    return new QPointF(p.x()/width(), 1.0-p.y()/height());
	}

	private QPointF normalizeAndCenterPixelPos(QPointF p)
	{
	    return new QPointF(2.0 * p.x() / width() - 1.0,
	                       1.0 - 2.0 * p.y() / height());
	}
	
    // Overriding mouse event methods
	@Override
	protected void mousePressEvent(QMouseEvent event)
	{
	    if(trackballEnabled) {	
		    if (event.button().equals(Qt.MouseButton.LeftButton)) {
		        trackball.push(normalizeAndCenterPixelPos(event.posF()), trackball.rotation().conjugate());
		        event.accept(); rotating = true;
		    }
	    }
	    
	    lastMousePos = normalizePixelPos(event.posF());
	    updateGL();
	}

	@Override
	protected void mouseReleaseEvent(QMouseEvent event)
	{
	    if(trackballEnabled) {	
		    if (event.button().equals(Qt.MouseButton.LeftButton)) {
		        trackball.release(normalizeAndCenterPixelPos(event.posF()), trackball.rotation().conjugate());
		        event.accept();
		    }
	    }
	
	    updateGL();
	}

	@Override
	protected void mouseMoveEvent(QMouseEvent event)
	{
	    if (trackballEnabled) {
	    	if(event.buttons().isSet(Qt.MouseButton.LeftButton)) {
		        trackball.move(normalizeAndCenterPixelPos(event.posF()), trackball.rotation().conjugate());
		        event.accept();
		    } else if(rotating) {
		        trackball.release(normalizeAndCenterPixelPos(event.posF()), trackball.rotation().conjugate());
		        rotating = false;
		    }
	    }
	    
	    QPointF normPosition = normalizePixelPos(event.posF());
	    if (event.buttons().isSet(Qt.MouseButton.RightButton)) {
		    QPointF delta = normPosition.clone();
	    	delta.subtract(lastMousePos);
	    	translate = translate.add(delta);
	    }
	    
	    lastMousePos = normPosition;
	    updateGL();
	}
	
	@Override
	protected void wheelEvent(QWheelEvent event) {
		adjustCameraPosition(event.delta()/1200.0f);
	}
	
	protected String readFileResourceToString(String resourceName)
	{
		try
		{
			InputStream in = Object.class.getResourceAsStream(resourceName);
			BufferedReader reader = new BufferedReader(new InputStreamReader(in));
			
			StringBuilder builder = new StringBuilder();
			String aux = "";
			while ((aux = reader.readLine()) != null) {
			    builder.append(aux);
			    builder.append("\n");
			}
			reader.close();
			
			return builder.toString();
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return null;
		}
	}

	@Override
	public void updateGL()
	{
		if(this.isVisible()) {
			if(idleTimer != null && idleTimer.isActive()) return;
			else super.updateGL();
		}
	}

	// A special update slot for the timer only (will always honor update request)
	protected void updateGLFromTimer()
	{
		if(this.isVisible()) {
			super.updateGL();
		}
	}
	
	@Override
    protected void initializeGL()
    {
    	// Ensures LWJGL is ready to roll
    	GL.createCapabilities();
    	
    	// Query and print info about OpenGL (for debugging)
    	GLException.reportOpenGLInfo(context());
    	
        // Set up the OpenGL context basics
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        
        // Read the shaders
        String vertexShader = readFileResourceToString("/Shaders/QtModelViewer.vert");
        String fragmentShader = readFileResourceToString("/Shaders/QtModelViewer.frag");
        
        // Make shader program
        texturedShader = new QGLShaderProgram();
        texturedShader.addShaderFromSourceCode(
        		new QGLShader.ShaderType(QGLShader.ShaderTypeBit.Vertex),
        		vertexShader);        
        
        texturedShader.addShaderFromSourceCode(
        		new QGLShader.ShaderType(QGLShader.ShaderTypeBit.Fragment),
				fragmentShader);

        // Bind our vertex attributes to their pre-determined locations
        texturedShader.bindAttributeLocation("vertexAttrib", PLYMeshData.ATTRIB_LOC_VERTEX);
        texturedShader.bindAttributeLocation("normalAttrib", PLYMeshData.ATTRIB_LOC_NORMAL);
        texturedShader.bindAttributeLocation("colorAttrib", PLYMeshData.ATTRIB_LOC_COLORS);
        texturedShader.bindAttributeLocation("texCoordAttrib", PLYMeshData.ATTRIB_LOC_TEXCOR);
        
        // Try to link the shader
        if(!texturedShader.link()) {
        	System.err.println("Shader link failed:\n" + texturedShader.log());
        	perspLoc = modelLoc = viewLoc = -1;
    		colorTexLoc[0] = colorTexLoc[1] = colorTexLoc[2] = colorTexLoc[3] = -1;
        } else {
        	texturedShader.bind();

        	// Grab the uniforms
        	perspLoc = texturedShader.uniformLocation("perspectiveMatrix");
        	modelLoc = texturedShader.uniformLocation("modelMatrix");
        	viewLoc = texturedShader.uniformLocation("viewMatrix");
        	normalMatLoc = texturedShader.uniformLocation("normalMatrix");
        	
        	lightPositionLoc = texturedShader.uniformLocation("lightPosition");
        	colorUniformLoc = texturedShader.uniformLocation("colorUniform");
        	colorTexLoc[0] = texturedShader.uniformLocation("colorTex");
        	colorTexLoc[1] = texturedShader.uniformLocation("colorTex1");
        	colorTexLoc[2] = texturedShader.uniformLocation("colorTex2");
        	colorTexLoc[3] = texturedShader.uniformLocation("colorTex3");
        	renderModeLoc = texturedShader.uniformLocation("renderMode");
        	
        	// Setup default values
        	texturedShader.setUniformValue(colorTexLoc[0], 0);
        	texturedShader.setUniformValue(colorTexLoc[1], 1);
        	texturedShader.setUniformValue(colorTexLoc[2], 2);
        	texturedShader.setUniformValue(colorTexLoc[3], 3);        	
        	texturedShader.setUniformValue(lightPositionLoc, new QVector3D(3.0, 5.0, 2.0));
        }
        
        // Build the example cube geometry
        makeExampleCubeVBOs();
    }
    
    protected void resizeGL(int w, int h)
    {
    	glViewport(0, 0, w-1, h-1);

    	persp.setToIdentity();
    	persp.perspective(42.0, w/(double)h, 0.01, 100.0);    	
    }

    protected void adjustCameraPosition(float delta)
    {
    	// Adjust and clamp the camera z position
    	camZPos += delta;
    	if(camZPos > 10.0f) { camZPos = 10.0f; }
    	if(camZPos < 1.0f) { camZPos = 1.0f; }
    	
    	translateScale = camZPos * 0.375f + 0.125f;
    	
    	// Setup the proper view matrix
    	view.setToIdentity();
    	view.lookAt(new QVector3D(0.0, 0.0, camZPos),
    				new QVector3D(0.0, 0.0, 0.0),
    				new QVector3D(0.0, 1.0, 0.0));    	
    }
    
    protected void makeExampleCubeVBOs() {
    	
    	// Make the native buffers
    	FloatBuffer packedCubeBuffer = BufferUtils.createFloatBuffer(EXAMPLE_CUBE_PACKED_DATA.length);
    	packedCubeBuffer.put(EXAMPLE_CUBE_PACKED_DATA).flip();
    	
    	// Generate the VBO storage and pass in the data
		cubePackedVBO = glGenBuffers();
		glBindBuffer(GL_ARRAY_BUFFER, cubePackedVBO);
	    glBufferData(GL_ARRAY_BUFFER, packedCubeBuffer, GL_STATIC_DRAW);

	    // Compute stride (you could just say 44 but I want to show how I get that number)
	    int numElements = (3+3+3+3);	// verts, norms, colors, texcoords
	    int sizeElement = 4;			// Bytes in a float
	    int stride = numElements*sizeElement;

	    // Set the offset into the VBO for each attribute
	    glVertexAttribPointer(PLYMeshData.ATTRIB_LOC_VERTEX, 3, GL_FLOAT, false, stride, 0);
		glVertexAttribPointer(PLYMeshData.ATTRIB_LOC_NORMAL, 3, GL_FLOAT, false, stride, 3*sizeElement);
		glVertexAttribPointer(PLYMeshData.ATTRIB_LOC_COLORS, 3, GL_FLOAT, false, stride, 6*sizeElement);
		glVertexAttribPointer(PLYMeshData.ATTRIB_LOC_TEXCOR, 3, GL_FLOAT, false, stride, 9*sizeElement);
		
    	IntBuffer cubeFaceBuffer = BufferUtils.createIntBuffer(EXAMPLE_CUBE_TRI_FACES.length);
    	cubeFaceBuffer.put(EXAMPLE_CUBE_TRI_FACES).flip();

    	// Generate the index buffer storage and pass in the data
    	cubeFaceElements = glGenBuffers();
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeFaceElements);
    	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeFaceBuffer, GL_STATIC_DRAW);
    }
    
    protected void paintGL()
    {
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	
    	if(!texturedShader.bind()) {
    		System.err.println("Shader failed to bind.");
    	} else {
        	if(meshData == null) {
        		drawExampleCube();
        	} else {
        		drawMesh();
	    	}
    	}
    }
    
    private void drawExampleCube() {
    	
    	// Apply the trackball rotation
    	model.setToIdentity();
    	model.rotate(trackball.rotation());    	
    	
		// Set the matrix uniform values
		texturedShader.setUniformValue(perspLoc, persp);
		texturedShader.setUniformValue(modelLoc, model);
		texturedShader.setUniformValue(normalMatLoc, model.normalMatrix());
		texturedShader.setUniformValue(viewLoc, view);
		
		texturedShader.setUniformValue(colorUniformLoc, uniformColor);
		texturedShader.setUniformValue(renderModeLoc, renderMode.ordinal());
    			
		// Enable the vertex array VBOs
		glEnableVertexAttribArray(PLYMeshData.ATTRIB_LOC_VERTEX);
		glEnableVertexAttribArray(PLYMeshData.ATTRIB_LOC_NORMAL);
		glEnableVertexAttribArray(PLYMeshData.ATTRIB_LOC_COLORS);
		glEnableVertexAttribArray(PLYMeshData.ATTRIB_LOC_TEXCOR);

		// Draw it indirectly (as triangles)
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeFaceElements);
    	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// Disable the attribute arrays
		glDisableVertexAttribArray(PLYMeshData.ATTRIB_LOC_TEXCOR);
		glDisableVertexAttribArray(PLYMeshData.ATTRIB_LOC_COLORS);
		glDisableVertexAttribArray(PLYMeshData.ATTRIB_LOC_NORMAL);
		glDisableVertexAttribArray(PLYMeshData.ATTRIB_LOC_VERTEX);
	}
    
    private void drawMesh() {
    	
		// Apply the model and trackball transformations
		float[] center = meshData.getCenter();
		float scale = meshData.getUnitScale();
		
		model.setToIdentity();
		model.scale(2.0*scale);
		model.translate(translateScale*translate.x(), translateScale*translate.y(), 0.0);
    	model.rotate(trackball.rotation());
		model.translate(-center[0], -center[1], -center[2]);
		
		// Set the matrix uniform values
		texturedShader.setUniformValue(perspLoc, persp);
		texturedShader.setUniformValue(modelLoc, model);
		texturedShader.setUniformValue(viewLoc, view);
		texturedShader.setUniformValue(normalMatLoc, model.normalMatrix());
		
		// Set other uniform values
		texturedShader.setUniformValue(colorUniformLoc, uniformColor);
		texturedShader.setUniformValue(renderModeLoc, renderMode.ordinal());

		// Bind the color textures
		for(int i=0; i<4; i++) {
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, colorTextureID[i]);
		}
		
		GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
		
		// Enable the attribute arrays
		texturedShader.enableAttributeArray(PLYMeshData.ATTRIB_LOC_VERTEX);
		if(meshData.withNormals()) { texturedShader.enableAttributeArray(PLYMeshData.ATTRIB_LOC_NORMAL); }
		if(meshData.withColors()) { texturedShader.enableAttributeArray(PLYMeshData.ATTRIB_LOC_COLORS); }
		if(meshData.withTexCoords()) { texturedShader.enableAttributeArray(PLYMeshData.ATTRIB_LOC_TEXCOR); }
		
		// Draw the raw face data
		GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
		glDrawArrays(GL_TRIANGLES, 0, meshData.getFaceCount()*3);
		GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());

		// Disable the attribute arrays
		texturedShader.disableAttributeArray(PLYMeshData.ATTRIB_LOC_VERTEX);
		if(meshData.withNormals()) { texturedShader.disableAttributeArray(PLYMeshData.ATTRIB_LOC_NORMAL); }
		if(meshData.withColors()) { texturedShader.disableAttributeArray(PLYMeshData.ATTRIB_LOC_COLORS); }
		if(meshData.withTexCoords()) { texturedShader.disableAttributeArray(PLYMeshData.ATTRIB_LOC_TEXCOR); }
		GLException.openGLErrorCheckAndOutput(Thread.currentThread().getStackTrace());
    }

    // Example Cube Full VBO data packed in one array
	private static final float EXAMPLE_CUBE_PACKED_DATA[] = {
	/*	   Vertex Location		 |	   Surface Normal	 	 |	    Vertex Color	 |   Tex Coords   w/  index  */			
		// Front Face
		-1.00f, -1.00f,  1.00f,		 0.00f,  0.00f,  1.00f,		0.25f, 0.00f, 0.00f,	 0.00f, 0.00f,    0.0f,
		 1.00f, -1.00f,  1.00f,		 0.00f,  0.00f,  1.00f,		0.00f, 0.25f, 0.00f,	 1.00f, 0.00f,    0.0f,		
		 1.00f,  1.00f,  1.00f,		 0.00f,  0.00f,  1.00f,		0.00f, 0.00f, 0.25f,	 1.00f, 1.00f,    0.0f,
		-1.00f,  1.00f,  1.00f,		 0.00f,  0.00f,  1.00f,		0.25f, 0.00f, 0.25f,	 0.00f, 1.00f,    0.0f,
		
		// Right Face
		 1.00f, -1.00f,  1.00f,		 1.00f,  0.00f,  0.00f,		0.50f, 0.00f, 0.00f,	 0.00f, 0.00f,    1.0f,
		 1.00f, -1.00f, -1.00f,		 1.00f,  0.00f,  0.00f,		0.00f, 0.50f, 0.00f,	 1.00f, 0.00f,    1.0f,		
		 1.00f,  1.00f, -1.00f,		 1.00f,  0.00f,  0.00f,		0.00f, 0.00f, 0.50f,	 1.00f, 1.00f,    1.0f,
		 1.00f,  1.00f,  1.00f,		 1.00f,  0.00f,  0.00f,		0.50f, 0.00f, 0.50f,	 0.00f, 1.00f,    1.0f,

		// Back Face
		 1.00f, -1.00f, -1.00f,		 0.00f,  0.00f, -1.00f,		0.75f, 0.00f, 0.00f,	 0.00f, 0.00f,    2.0f,
		-1.00f, -1.00f, -1.00f,		 0.00f,  0.00f, -1.00f,		0.00f, 0.75f, 0.00f,	 1.00f, 0.00f,    2.0f,		
		-1.00f,  1.00f, -1.00f,		 0.00f,  0.00f, -1.00f,		0.00f, 0.00f, 0.75f,	 1.00f, 1.00f,    2.0f,
		 1.00f,  1.00f, -1.00f,		 0.00f,  0.00f, -1.00f,		0.75f, 0.00f, 0.75f,	 0.00f, 1.00f,    2.0f,
		 
		// Left Face
		-1.00f, -1.00f, -1.00f,		-1.00f,  0.00f,  0.00f,		1.00f, 0.00f, 0.00f,	 0.00f, 0.00f,    3.0f,
		-1.00f, -1.00f,  1.00f,		-1.00f,  0.00f,  0.00f,		0.00f, 1.00f, 0.00f,	 1.00f, 0.00f,    3.0f,		
		-1.00f,  1.00f,  1.00f,		-1.00f,  0.00f,  0.00f,		0.00f, 0.00f, 1.00f,	 1.00f, 1.00f,    3.0f,
		-1.00f,  1.00f, -1.00f,		-1.00f,  0.00f,  0.00f,		1.00f, 0.00f, 1.00f,	 0.00f, 1.00f,    3.0f,

		// Top Face
		-1.00f,  1.00f,  1.00f,		 0.00f,  1.00f,  0.00f,		1.00f, 0.25f, 0.25f,	 0.00f, 0.00f,    4.0f,
		 1.00f,  1.00f,  1.00f,		 0.00f,  1.00f,  0.00f,		0.25f, 1.00f, 0.25f,	 1.00f, 0.00f,    4.0f,		
		 1.00f,  1.00f, -1.00f,		 0.00f,  1.00f,  0.00f,		0.25f, 0.25f, 1.00f,	 1.00f, 1.00f,    4.0f,
		-1.00f,  1.00f, -1.00f,		 0.00f,  1.00f,  0.00f,		1.00f, 0.25f, 1.00f,	 0.00f, 1.00f,    4.0f,

		// Bottom Face
		 1.00f, -1.00f,  1.00f,		 0.00f, -1.00f,  0.00f,		1.00f, 0.75f, 0.75f,	 0.00f, 0.00f,    5.0f,
		-1.00f, -1.00f,  1.00f,		 0.00f, -1.00f,  0.00f,		0.75f, 1.00f, 0.75f,	 1.00f, 0.00f,    5.0f,		
		-1.00f, -1.00f, -1.00f,		 0.00f, -1.00f,  0.00f,		0.75f, 0.75f, 1.00f,	 1.00f, 1.00f,    5.0f,
		 1.00f, -1.00f, -1.00f,		 0.00f, -1.00f,  0.00f,		1.00f, 0.75f, 1.00f,	 0.00f, 1.00f,    5.0f
	}; // End example cube vertices
	
	private static final int EXAMPLE_CUBE_TRI_FACES[] = {
			 0,  1,  2,    0,  2,  3,
			 4,  5,  6,    4,  6,  7,
			 8,  9, 10,    8, 10, 11,
			12, 13, 14,   12, 14, 15,
			16, 17, 18,   16, 18, 19,
			20, 21, 22,   20, 22, 23
	};

}
