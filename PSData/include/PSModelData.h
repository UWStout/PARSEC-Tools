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
    PSModelData(QFileInfo pFilename);
    ~PSModelData();
	
    static PSModelData* makeFromXML(QXmlStreamReader* reader, QFileInfo pZipFile,
                                    PSChunkData* pParent = nullptr);

    void setFaceCount(long pFaceCount);
    void setVertexCount(long pVertexCount);
    void setHasVertexColors(bool pHasVtxColors);
    void setHasUV(bool pHasUV);
    void setMeshFilename(QString pMeshFilepath);
    void setArchiveFile(QFileInfo pArchiveFile);
	
    void addTextureFile(int pId, QString pFilepath);

    QFileInfo getArchiveFile() const;

    long getFaceCount() const;
    long getVertexCount() const;
    QString getMeshFilename() const;
    bool hasVtxColors() const;
    bool hasUV() const;

    QMap<int, QString> getTextureFiles() const;
    QString getTextureFile(int id) const;
	
private:
    long mFaceCount, mVertexCount;
    QFileInfo mZipFile;
    QString mMeshFilepath;
    bool mHasVtxColors, mHasUV;

    QMap<int, QString> textureFiles;
};

#endif
