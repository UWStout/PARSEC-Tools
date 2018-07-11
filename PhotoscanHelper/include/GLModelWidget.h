#ifndef GLMODELWIDGET_H
#define GLMODELWIDGET_H

#include <QWidget>
#include <QFileInfo>
#include <QString>

template <class T>
class QFutureWatcher;

namespace Ui {
    class GLModelViewer;
}

class PSModelData;
class PSSessionData;
class PLYMeshData;

class GLModelWidget : public QWidget
{
    Q_OBJECT

public:
    GLModelWidget(QWidget* parent = NULL);
    GLModelWidget(const PSSessionData* pSession, QWidget* parent = NULL);
    GLModelWidget(const PSModelData* pModel, QWidget* parent = NULL);
    ~GLModelWidget();

    void loadNewModel(const PSSessionData* pSession);
    void loadNewModel(const PSModelData* pModel);

    bool loadAllData(const PSModelData* pModel);

public slots:
    void on_renderModeComboBox_currentIndexChanged(int index);

private:
    Ui::GLModelViewer* mGUI;

    PSModelData* mModelData;
    PLYMeshData* mPlyMesh;
    QString mName;

    QImage mPngTextures[4];
    QFutureWatcher<bool>* mDataLoading;

    void initMembers();

    QImage readTexture(QString pTextureFilename, QFileInfo pArchiveFile = QFileInfo());
    void dataLoadingFinished();
};

#endif // GLMODELWIDGET_H
