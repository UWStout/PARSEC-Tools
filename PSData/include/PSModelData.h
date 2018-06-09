#ifndef PS_MODEL_DATA_H
#define PS_MODEL_DATA_H

#include "psdata_global.h"

#include <QFileInfo>
#include <QString>
#include <QMap>

class QXmlStreamReader;
class QFile;
class PSChunkData;

class PSDATASHARED_EXPORT PSModelData {
public:
    PSModelData(QFileInfo pZipFile);
    ~PSModelData();
	
    static PSModelData* makeFromXML(QXmlStreamReader* reader, QFileInfo pZipFile, PSChunkData* pParent);

    void setFaceCount(long pFaceCount);
    void setVertexCount(long pVertexCount);
    void setHasVertexColors(bool pHasVtxColors);
    void setHasUV(bool pHasUV);
    void setMeshFilename(QString pMeshFilepath);
	
    void addTextureFile(int pId, QString pFilepath);

    QFileInfo getArchiveFile();

    long getFaceCount();
    long getVertexCount();
    QString getMeshFilename();
    bool hasVtxColors();
    bool hasUV();

    QMap<int, QString> getTextureFiles();
    QString getTextureFile(int id);
	
private:
    long mFaceCount, mVertexCount;
    QFileInfo mZipFile;
    QString mMeshFilepath;
    bool mHasVtxColors, mHasUV;

    QMap<int, QString> textureFiles;
};

#endif
