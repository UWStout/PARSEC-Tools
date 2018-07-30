#include "GLModelWidget.h"

#include <QtConcurrent>

#include <QSurface>
#include <QSurfaceFormat>
#include <QFutureWatcher>
#include <QLocale>

#include <quazip/quazipfile.h>

#include <PSModelData.h>
#include <PSSessionData.h>
#include <PLYMeshData.h>

#include "ui_GLModelWidget.h"
#include "QtModelViewerWidget.h"

GLModelWidget::GLModelWidget(QWidget* parent) : QWidget(parent) {
    initMembers();
}

GLModelWidget::GLModelWidget(const PSSessionData* pSession, QWidget* parent) : QWidget(parent) {
    initMembers();
    mName = pSession->getPSProjectFile().baseName();
    if(pSession->getModelData() != nullptr) {
        loadNewModel(pSession->getModelData());
    }
}

GLModelWidget::GLModelWidget(const PSModelData* pModel, QWidget* parent) : QWidget(parent) {
    initMembers();
    loadNewModel(pModel);
}

GLModelWidget::~GLModelWidget() {}

void GLModelWidget::initMembers() {
    mGUI = new Ui::GLModelViewer();
    mGUI->setupUi(this);

    // Rebuild the combobox
    mGUI->renderModeComboBox->clear();
    for(int i=0; i<QtModelViewerWidget::RENDER_COUNT; i++) {
        mGUI->renderModeComboBox->addItem(QtModelViewerWidget::getDescription((QtModelViewerWidget::RenderMode)i));
    }
    mGUI->renderModeComboBox->insertSeparator(3);
    mGUI->renderModeComboBox->insertSeparator(7);
    mGUI->renderModeComboBox->setCurrentIndex(QtModelViewerWidget::RENDER_TEXTURE_COLOR);

    // Set color of label for selecting flat rendering color
    QPalette Pal = mGUI->flatColorLabel->palette();
    Pal.setColor(QPalette::Window, mGUI->modelViewer->getFlatColor());
    mGUI->flatColorLabel->setAutoFillBackground(true);
    mGUI->flatColorLabel->setPalette(Pal);
}

void GLModelWidget::loadNewModel(const PSSessionData* pSession) {
    if(pSession != nullptr) {
        mName = pSession->getPSProjectFile().baseName();
        loadNewModel(pSession->getModelData());
    } else {
        loadNewModel((PSModelData*)nullptr);
    }
}

void GLModelWidget::loadNewModel(const PSModelData* pModel) {
    if(pModel == nullptr) {
        mPlyMesh = nullptr;
        mGUI->modelViewer->setModelData(nullptr, nullptr);
        mGUI->statusLabel->setText("Please load a model.");
    } else {
        if (pModel->getArchiveFile().filePath() == "") {
            mName = QFileInfo(pModel->getMeshFilename()).baseName();
        } else {
            mName = pModel->getArchiveFile().baseName();
        }
        mGUI->statusLabel->setText(QString::asprintf("Loading mesh for '%s' ...", mName.toLocal8Bit().data()));
        mDataLoading = new QFutureWatcher<bool>();
        connect(mDataLoading, &QFutureWatcher<bool>::canceled, this, &GLModelWidget::dataLoadingFinished);
        connect(mDataLoading, &QFutureWatcher<bool>::finished, this, &GLModelWidget::dataLoadingFinished);

        // Read the mesh and texture data in a separate thread
        QFuture<bool> loadingFuture = QtConcurrent::run(this, &GLModelWidget::loadAllData, pModel);
        mDataLoading->setFuture(loadingFuture);
    }
}

bool GLModelWidget::loadAllData(const PSModelData* pModel) { //throws IOException {

    // Read all the texture files
    mPngTextures[0] = mPngTextures[1] = mPngTextures[2] = mPngTextures[3] = QImage();
    for(int texID : pModel->getTextureFiles().keys()) {
        qInfo("Reading texture %s\n", pModel->getTextureFile(texID).toLocal8Bit().data());
        mPngTextures[texID] = readTexture(pModel->getTextureFile(texID), pModel->getArchiveFile());
    }

    // Read the model
    qInfo("Reading model %s\n", pModel->getMeshFilename().toLocal8Bit().data());
    mPlyMesh = new PLYMeshData();
    return mPlyMesh->readPLYFile(pModel->getArchiveFile(), pModel->getMeshFilename());
}

void GLModelWidget::on_renderModeComboBox_currentIndexChanged(int index) {
    // Account for separators which do affect the index
    if(index < 3) { mGUI->modelViewer->setRenderMode(index); }
    else if(index < 7) { mGUI->modelViewer->setRenderMode(index-1); }
    else { mGUI->modelViewer->setRenderMode(index-2); }
}

QImage GLModelWidget::readTexture(QString pTextureFilename, QFileInfo pArchiveFile) {
    QIODevice* lFileDev = nullptr;
    if(pArchiveFile.filePath() != "") {
        lFileDev = new QuaZipFile(pArchiveFile.filePath(), pTextureFilename);
        if(!lFileDev->open(QIODevice::ReadOnly)) {
            qWarning("Failed to open zipped texture file '%s': %d.",
                     pArchiveFile.filePath().toLocal8Bit().data(),
                     static_cast<QuaZipFile*>(lFileDev)->getZipError());
            delete lFileDev;
            return QImage();
        }
    } else {
        lFileDev = new QFile(pTextureFilename);
        if(!lFileDev->open(QIODevice::ReadOnly)) {
            qWarning("Failed to open texture file: %s", pTextureFilename.toLocal8Bit().data());
            delete lFileDev;
            return QImage();
        }
    }

    // Make the image from the cached data
    QImage lTexture;
    bool result = lTexture.load(lFileDev, "png");
    delete lFileDev;

    if (result) return lTexture;
    return QImage();
}

void GLModelWidget::dataLoadingFinished() {
    if(!mDataLoading->future().result() || mPlyMesh == nullptr) {
        mGUI->statusLabel->setText("There was an error loading the model.");
    } else {

        QString modelInfo = QString::asprintf("'%s' (%s vertices, %s faces)", mName.toLocal8Bit().data(),
                QLocale::system().toString((long long)mPlyMesh->getVertexCount()).toLocal8Bit().data(),
                QLocale::system().toString((long long)mPlyMesh->getFaceCount()).toLocal8Bit().data());

        if(mPlyMesh->isMissingData()) {
            modelInfo += QString::asprintf(" - Missing: %s%s%s",
                        (mPlyMesh->withNormals()?"":"surface normals, "),
                        (mPlyMesh->withColors()?"":"vertex colors, "),
                        (mPlyMesh->withTexCoords()?"":"texture coords, "));
            modelInfo = modelInfo.left(modelInfo.length() - 3);
        }

        mGUI->statusLabel->setText(modelInfo);
//        if(mPngTextures[0] == nullptr) {
            mGUI->modelViewer->setModelData(nullptr, mPlyMesh);
//        } else {
//            mGUI->modelViewer->setModelData(mPngTextures, mPlyMesh);
//        }

    }
}
