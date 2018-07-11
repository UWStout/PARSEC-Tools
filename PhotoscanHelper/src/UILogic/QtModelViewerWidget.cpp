#include "QtModelViewerWidget.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

#include <PLYMeshData.h>

#include "QtTrackball.h"

DEFINE_ENUM(RenderMode, RENDER_MODE_ENUM, QtModelViewerWidget)

QtModelViewerWidget::QtModelViewerWidget(QWidget* parent) : QOpenGLWidget(parent), mUniformColor(255, 255, 255) {
    setFormat(QSurfaceFormat::defaultFormat());
    mCamZPos = 5.0f;
    adjustCameraPosition(0.0f);

    mMeshData = NULL;
    mRenderMode = RENDER_SHADED;
    mTexturedShader = NULL;
    mCubeVBuffer = mCubeElemBuffer = NULL;
    mCubeVAO = new QOpenGLVertexArrayObject(this);

    mColorTexLoc[0] = mColorTexLoc[1] = mColorTexLoc[2] = mColorTexLoc[3] = -1;
    mColorTextureID[0] = mColorTextureID[1] = mColorTextureID[2] = mColorTextureID[3] = -1;

    // Update similar to an idle function
    mIdleTimer = new QTimer(this);
    mIdleTimer->setInterval(20);
    connect(mIdleTimer, &QTimer::timeout, this, &QtModelViewerWidget::updateGLFromTimer);
    mIdleTimer->start();

    mTrackball = new QtTrackball(QtTrackball::TRACKMODE_SPHERE, 0.02f);
    mTrackballEnabled = true; mRotating = false;
}

QtModelViewerWidget::~QtModelViewerWidget() {
    delete mTrackball;
}

void QtModelViewerWidget::setModelData(QImage pColorTexture[], PLYMeshData *pMeshData) {
    // Make the context current in this thread so we can do OpenGL stuff!
    makeCurrent();

    // Clean up any previous texture data
//    for(int i=0; i<4; i++) {
//        if(glIsTexture(mColorTextureID[i])) {
//            deleteTexture(mColorTextureID[i]);
//        }

//        mColorTextureID[i] = -1;
//    }

    // Clear out the VBO data from the GPU
    if(mMeshData != NULL) {
        mMeshData->releaseBuffers();
    }

    // Set new textures
    if (pColorTexture != NULL) {
//        for(int i=0; i<4; i++) {
//            mColorTexture[i] = pColorTexture[i];

//        // Pass texture(s) into the context
//        if(mColorTexture[i] != NULL) {
//            glActiveTexture(GL_TEXTURE0 + i);
//            mColorTextureID[i] = bindTexture(mColorTexture[i]);
//        }
//        }
    }

    // Set new mesh dat
    mMeshData = pMeshData;
    if (mMeshData != NULL) {
        makeCurrent();
        mMeshData->buildBuffers(QOpenGLContext::currentContext());
        mMeshData->buildTextures();
    }
}

void QtModelViewerWidget::setRenderMode(int index) {
    if(index < 0 || index >= RENDER_COUNT) { return; }
    mRenderMode = (RenderMode)(RENDER_SINGLE_COLOR + index);
    qInfo("Render mode changed to %s", getShortName(mRenderMode));
    update();
}

void QtModelViewerWidget::setFlatColor(QColor newColor) {
    mUniformColor = newColor;
    update();
}

QColor QtModelViewerWidget::getFlatColor() const { return mUniformColor; }

void QtModelViewerWidget::mousePressEvent(QMouseEvent* event) {
    if(mTrackballEnabled) {
        if (event->button() == Qt::LeftButton) {
            mTrackball->push(normalizeAndCenterPixelPos(event->pos()));
            event->accept(); mRotating = true;
        }
    }

    mLastMousePos = normalizePixelPos(event->pos());
    update();
}

void QtModelViewerWidget::mouseReleaseEvent(QMouseEvent* event) {
    if(mTrackballEnabled) {
        if (event->button() == Qt::LeftButton) {
            mTrackball->release(normalizeAndCenterPixelPos(event->pos()), mTrackball->rotation().conjugated());
            event->accept();
        }
    }

    update();
}

void QtModelViewerWidget::mouseMoveEvent(QMouseEvent* event) {
    if (mTrackballEnabled) {
        if((event->buttons() & Qt::LeftButton) != 0) {
            mTrackball->move(normalizeAndCenterPixelPos(event->pos()), mTrackball->rotation().conjugated());
            event->accept();
        } else if(mRotating) {
            mTrackball->release(normalizeAndCenterPixelPos(event->pos()), mTrackball->rotation().conjugated());
            mRotating = false;
        }
    }

    QPointF normPosition = normalizePixelPos(event->pos());
    if ((event->buttons() & Qt::RightButton) != 0) {
        mTranslate += normPosition - mLastMousePos;
    }

    mLastMousePos = normPosition;
    update();
}

void QtModelViewerWidget::wheelEvent(QWheelEvent *event) {
    adjustCameraPosition(event->delta()/1200.0f);
}

void QtModelViewerWidget::updateGLFromTimer() {
    update();
}

QString QtModelViewerWidget::readFileResourceToString(QString pResourceName) {
    QString lData;
    QFile lRsrc(pResourceName);
    if(!lRsrc.open(QIODevice::ReadOnly)) {
        qDebug("Failed to open resource %s:", pResourceName.toLocal8Bit().data());
    } else {
        lData = lRsrc.readAll();
    }
    lRsrc.close();

    return lData;
}

void QtModelViewerWidget::adjustCameraPosition(float delta) {
    // Adjust and clamp the camera z position
    mCamZPos += delta;
    if(mCamZPos > 10.0f) { mCamZPos = 10.0f; }
    if(mCamZPos < 1.0f) { mCamZPos = 1.0f; }

    mTranslateScale = mCamZPos * 0.375f + 0.125f;

    // Setup the proper view matrix
    mView.setToIdentity();
    QVector3D lEye(0.0, 0.0, mCamZPos);
    QVector3D lCenter(0.0, 0.0, 0.0);
    QVector3D lUp(0.0, 1.0, 0.0);
    mView.lookAt(lEye, lCenter, lUp);
}

void QtModelViewerWidget::makeExampleCubeVBOs() {
    // Make sure we have a current openGL context
    makeCurrent();

    // Get pointer to cross-platform friendly OpenGL functions
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();

    // Build the Vertex Array Object
    if(!mCubeVAO->create()) {
        qWarning("Could not create cube vertex array object");
        return;
    }
    QOpenGLVertexArrayObject::Binder vaoBinder(mCubeVAO);

    // This will only work if we have a current opengl context
    mCubeVBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    if(!mCubeVBuffer->create()) {
        qWarning("Could not create cube vertex buffer");
        return;
    }
    mCubeVBuffer->bind();

    // Compute stride (you could just say 44 but I want to show how I get that number)
    int numElements = (3+3+3+3);	// verts, norms, colors, texcoords
    long long sizeElement = 4;		// Bytes in a float
    long long strideBytes = numElements*sizeElement;

    // Copy data to video memory
    mCubeVBuffer->allocate(EXAMPLE_CUBE_PACKED_DATA, 24 * strideBytes);

    // This will only work if we have a current opengl context
    mCubeElemBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    if(!mCubeElemBuffer->create()) {
        qWarning("Could not create cube index buffer");
        return;
    }
    mCubeElemBuffer->bind();

    // Copy face data to video memory
    mCubeElemBuffer->allocate(EXAMPLE_CUBE_TRI_FACES, 36*sizeElement);

    // Set the offset into the VBO for each attribute
    mCubeVBuffer->bind();
    gl->glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_VERTEX);
    gl->glVertexAttribPointer(PLYMeshData::ATTRIB_LOC_VERTEX, 3, GL_FLOAT, false, strideBytes, (void*)(0LL));
    gl->glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_NORMAL);
    gl->glVertexAttribPointer(PLYMeshData::ATTRIB_LOC_NORMAL, 3, GL_FLOAT, false, strideBytes, (void*)(3LL*sizeElement));
    gl->glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_COLORS);
    gl->glVertexAttribPointer(PLYMeshData::ATTRIB_LOC_COLORS, 3, GL_FLOAT, false, strideBytes, (void*)(6LL*sizeElement));
    gl->glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_TEXCOR);
    gl->glVertexAttribPointer(PLYMeshData::ATTRIB_LOC_TEXCOR, 3, GL_FLOAT, false, strideBytes, (void*)(9LL*sizeElement));
    mCubeVBuffer->release();
}

void QtModelViewerWidget::initializeGL() {
    makeCurrent();
    initializeOpenGLFunctions();

    // Get pointer to cross-platform friendly OpenGL functions
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();

    // Set up the OpenGL context basics
    gl->glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
    gl->glEnable(GL_DEPTH_TEST);
    gl->glEnable(GL_MULTISAMPLE);

    // Read the shaders
    QString vertexShader = readFileResourceToString(":/GLModelViewer/QtModelViewer.vert");
    QString fragmentShader = readFileResourceToString(":/GLModelViewer/QtModelViewer.frag");

    // Make shader program
    mTexturedShader = new QOpenGLShaderProgram();
    mTexturedShader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    mTexturedShader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);

    // Bind our vertex attributes to their pre-determined locations
    mTexturedShader->bindAttributeLocation("vertexAttrib", PLYMeshData::ATTRIB_LOC_VERTEX);
    mTexturedShader->bindAttributeLocation("normalAttrib", PLYMeshData::ATTRIB_LOC_NORMAL);
    mTexturedShader->bindAttributeLocation("colorAttrib", PLYMeshData::ATTRIB_LOC_COLORS);
    mTexturedShader->bindAttributeLocation("texCoordAttrib", PLYMeshData::ATTRIB_LOC_TEXCOR);

    // Try to link the shader
    if(!mTexturedShader->link()) {
        qWarning("Shader link failed: %s", mTexturedShader->log().toLocal8Bit().data());
        mPerspLoc = mModelLoc = mViewLoc = -1;
        mColorTexLoc[0] = mColorTexLoc[1] = mColorTexLoc[2] = mColorTexLoc[3] = -1;
    } else {
        mTexturedShader->bind();

        // Grab the uniforms
        mPerspLoc = mTexturedShader->uniformLocation("perspectiveMatrix");
        mModelLoc = mTexturedShader->uniformLocation("modelMatrix");
        mViewLoc = mTexturedShader->uniformLocation("viewMatrix");
        mNormalMatLoc = mTexturedShader->uniformLocation("normalMatrix");

        mLightPositionLoc = mTexturedShader->uniformLocation("lightPosition");
        mColorUniformLoc = mTexturedShader->uniformLocation("colorUniform");
        mColorTexLoc[0] = mTexturedShader->uniformLocation("colorTex");
        mColorTexLoc[1] = mTexturedShader->uniformLocation("colorTex1");
        mColorTexLoc[2] = mTexturedShader->uniformLocation("colorTex2");
        mColorTexLoc[3] = mTexturedShader->uniformLocation("colorTex3");
        mRenderModeLoc = mTexturedShader->uniformLocation("renderMode");

        // Setup default values
        mTexturedShader->setUniformValue(mColorTexLoc[0], 0);
        mTexturedShader->setUniformValue(mColorTexLoc[1], 1);
        mTexturedShader->setUniformValue(mColorTexLoc[2], 2);
        mTexturedShader->setUniformValue(mColorTexLoc[3], 3);

        QVector3D lLight(3.0, 5.0, 2.0);
        mTexturedShader->setUniformValue(mLightPositionLoc, lLight);
    }

    // Build the example cube geometry
    makeExampleCubeVBOs();
}

void QtModelViewerWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w-1, h-1);

    mPersp.setToIdentity();
    mPersp.perspective(42.0, w/(float)h, 0.01, 100.0);
}

void QtModelViewerWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(!mTexturedShader->bind()) {
        qWarning("QtModelViewerWidget: Shader failed to bind.");
    } else {
        if(mMeshData == nullptr) {
            drawExampleCube();
        } else {
            drawMesh();
        }
    }
}

QPointF QtModelViewerWidget::normalizePixelPos(QPointF p) {
    return QPointF(p.x() / width(), 1.0 - p.y() / height());
}

QPointF QtModelViewerWidget::normalizeAndCenterPixelPos(QPointF p) {
    return QPointF(2.0 * p.x() / width() - 1.0, 1.0 - 2.0 * p.y() / height());
}

void QtModelViewerWidget::drawExampleCube() {
    QOpenGLVertexArrayObject::Binder vaoBinder(mCubeVAO);

    // Apply the trackball rotation
    mModel.setToIdentity();
    mModel.rotate(mTrackball->rotation());

    // Set the matrix uniform values
    mTexturedShader->setUniformValue(mPerspLoc, mPersp);
    mTexturedShader->setUniformValue(mModelLoc, mModel);
    mTexturedShader->setUniformValue(mNormalMatLoc, mModel.normalMatrix());
    mTexturedShader->setUniformValue(mViewLoc, mView);

    mTexturedShader->setUniformValue(mColorUniformLoc, mUniformColor);
    mTexturedShader->setUniformValue(mRenderModeLoc, (int)mRenderMode);

    // Enable the vertex array VBOs
    glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_VERTEX);
    glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_NORMAL);
    glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_COLORS);
    glEnableVertexAttribArray(PLYMeshData::ATTRIB_LOC_TEXCOR);

    // Draw it indirectly (as triangles)
    mCubeElemBuffer->bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Disable the attribute arrays
    glDisableVertexAttribArray(PLYMeshData::ATTRIB_LOC_TEXCOR);
    glDisableVertexAttribArray(PLYMeshData::ATTRIB_LOC_COLORS);
    glDisableVertexAttribArray(PLYMeshData::ATTRIB_LOC_NORMAL);
    glDisableVertexAttribArray(PLYMeshData::ATTRIB_LOC_VERTEX);
}

void QtModelViewerWidget::drawMesh() {
    QOpenGLVertexArrayObject::Binder vaoBinder(mMeshData->getVAO());

    // Apply the model and trackball transformations
    const float* center = mMeshData->getCenter();
    float scale = mMeshData->getUnitScale();

    mModel.setToIdentity();
    mModel.scale(scale);
//    mModel.translate(mTranslateScale*mTranslate.x(), mTranslateScale*mTranslate.y(), 0.0);
    mModel.rotate(mTrackball->rotation());
    mModel.translate(-center[0], -center[1], -center[2]);

    // Set the matrix uniform values
    mTexturedShader->setUniformValue(mPerspLoc, mPersp);
    mTexturedShader->setUniformValue(mModelLoc, mModel);
    mTexturedShader->setUniformValue(mViewLoc, mView);
    mTexturedShader->setUniformValue(mNormalMatLoc, mModel.normalMatrix());

    // Set other uniform values
    mTexturedShader->setUniformValue(mColorUniformLoc, mUniformColor);
    mTexturedShader->setUniformValue(mRenderModeLoc, (int)mRenderMode);

    // Bind the color textures
//    for(int i=0; i<4; i++) {
//        glActiveTexture(GL_TEXTURE0+i);
//        glBindTexture(GL_TEXTURE_2D, mColorTextureID[i]);
//    }

    // Enable the attribute arrays
    mTexturedShader->enableAttributeArray(PLYMeshData::ATTRIB_LOC_VERTEX);
    if(mMeshData->withNormals()) { mTexturedShader->enableAttributeArray(PLYMeshData::ATTRIB_LOC_NORMAL); }
    if(mMeshData->withColors()) { mTexturedShader->enableAttributeArray(PLYMeshData::ATTRIB_LOC_COLORS); }
    if(mMeshData->withTexCoords()) { mTexturedShader->enableAttributeArray(PLYMeshData::ATTRIB_LOC_TEXCOR); }

    // Draw the face index elements
    mMeshData->bindTextures();
    glDrawArrays(GL_TRIANGLES, 0, mMeshData->getFaceCount()*3);

    // Disable the attribute arrays
    mTexturedShader->disableAttributeArray(PLYMeshData::ATTRIB_LOC_VERTEX);
    if(mMeshData->withNormals()) { mTexturedShader->disableAttributeArray(PLYMeshData::ATTRIB_LOC_NORMAL); }
    if(mMeshData->withColors()) { mTexturedShader->disableAttributeArray(PLYMeshData::ATTRIB_LOC_COLORS); }
    if(mMeshData->withTexCoords()) { mTexturedShader->disableAttributeArray(PLYMeshData::ATTRIB_LOC_TEXCOR); }
}

// Example Cube Full VBO data packed in one array
const float QtModelViewerWidget::EXAMPLE_CUBE_PACKED_DATA[] = {
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

const int QtModelViewerWidget::EXAMPLE_CUBE_TRI_FACES[] = {
     0,  1,  2,    0,  2,  3,
     4,  5,  6,    4,  6,  7,
     8,  9, 10,    8, 10, 11,
    12, 13, 14,   12, 14, 15,
    16, 17, 18,   16, 18, 19,
    20, 21, 22,   20, 22, 23
};
