#ifndef QT_MODEL_VIEWER_H
#define QT_MODEL_VIEWER_H

#include <EnumFactory.h>

#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPointF>

class PLYMeshData;
class QtTrackball;
class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QTimer;

class QtModelViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
Q_OBJECT

public:
    #define RENDER_MODE_ENUM(RenderMode) \
        RenderMode(RENDER_SINGLE_COLOR, "Single Color", "Flat Color: single solid color over entire model.") \
        RenderMode(RENDER_VERTEX_COLOR, "Vertex Color", "Flat Color: per-vertex color values (no shading).") \
        RenderMode(RENDER_TEXTURE_COLOR, "Texture Color", "Flat Color: colors from the texture image (no shading).") \
        RenderMode(RENDER_SHADED, "Shaded Single Color", "Lit and Shaded: fixed light source using surface normals.") \
        RenderMode(RENDER_VERTEX_SHADED, "Shaded Vertex Color", "Lit and shaded: per-vertex color values.") \
        RenderMode(RENDER_TEXTURE_SHADED, "Shaded Texture Color", "Lit and shaded: colors from the texture image.") \
        RenderMode(RENDER_VERTEX_DATA, "Analysis Coordinates", "Analysis: vertex coordinates as colors.") \
        RenderMode(RENDER_NORMS_DATA, "Analysis Normals", "Analysis: surface normals as colors.") \
        RenderMode(RENDER_UV_DATA, "Analysis UVs", "Analysis: texture coordinates as colors.") \
        RenderMode(RENDER_TEXNUM_DATA, "Analysis Tex Index", "Analysis: texture index as color.") \
        RenderMode(RENDER_COUNT, "Number of Modes", "INTERNAL USE ONLY") \

    DECLARE_ENUM(RenderMode, RENDER_MODE_ENUM)

    QtModelViewerWidget(QWidget* parent = nullptr);
    virtual ~QtModelViewerWidget();

    void setModelData(QImage mColorTexture[], PLYMeshData* pMeshData);
    void setRenderMode(int index);

    void setFlatColor(QColor newColor);
    QColor getFlatColor() const;

protected:
    // Overriding mouse event methods
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);

    QString readFileResourceToString(QString resourceName);
    void adjustCameraPosition(float delta);
    void makeExampleCubeVBOs();

protected slots:
    // A special update slot for the timer only
    void updateGLFromTimer();

    // Core QOpenGLWidget Actions
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    QMatrix4x4 mPersp, mModel, mView;
    QOpenGLShaderProgram* mTexturedShader;
    float mCamZPos, mTranslateScale;

    int mPerspLoc, mModelLoc, mViewLoc, mNormalMatLoc, mColorUniformLoc;
    int mRenderModeLoc, mLightPositionLoc;

    int mColorTexLoc[4];
    int mColorTextureID[4];

    PLYMeshData* mMeshData;
    QImage mColorTexture[4];
    QColor mUniformColor;
    QTimer* mIdleTimer;
    RenderMode mRenderMode;

    // Cube example object
    QOpenGLBuffer *mCubeVBuffer, *mCubeElemBuffer;
    QOpenGLVertexArrayObject *mCubeVAO;

    // Trackball and translate related
    QtTrackball* mTrackball;
    bool mTrackballEnabled, mRotating;
    QPointF mTranslate, mLastMousePos;

    QPointF normalizePixelPos(QPointF p);
    QPointF normalizeAndCenterPixelPos(QPointF p);

    void drawExampleCube();
    void drawMesh();

    static const float EXAMPLE_CUBE_PACKED_DATA[];
    static const int EXAMPLE_CUBE_TRI_FACES[];
};

#endif
