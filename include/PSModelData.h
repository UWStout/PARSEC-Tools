//package edu.uwstout.berriers.PSHelper.Model;

//import java.io.QFile;
//import java.util.HashMap;
//import java.util.Map;

//import javax.xml.stream.XMLStreamException;
//import javax.xml.stream.XMLStreamReader;

#include <QString>
#include <QMap>

class QXmlStreamReader;
class QFile;
class PSChunkData;

class PSModelData {
public:
    PSModelData(QFile* pZipFile);
    ~PSModelData();
	
    static PSModelData* makeFromXML(QXmlStreamReader* reader, QFile* pZipFile, PSChunkData* pParent);

    void setFaceCount(long pFaceCount);
    void setVertexCount(long pVertexCount);
    void setHasVertexColors(bool pHasVtxColors);
    void setHasUV(bool pHasUV);
    void setMeshFilename(QString pMeshFilepath);
	
    void addTextureFile(int pId, QString pFilepath);

    QFile* getArchiveFile();

    long getFaceCount();
    long getVertexCount();
    QString getMeshFilename();
    bool hasVtxColors();
    bool hasUV();

    QMap<int, QString> getTextureFiles();
    QString getTextureFile(int id);
	
private:
    long mFaceCount, mVertexCount;
    QFile* mZipFile;
    QString mMeshFilepath;
    bool mHasVtxColors, mHasUV;

    QMap<int, QString> textureFiles;
};
