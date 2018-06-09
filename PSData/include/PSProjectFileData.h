#ifndef PS_PROJECT_FILE_DATA_H
#define PS_PROJECT_FILE_DATA_H

#include "psdata_global.h"

#include <QString>
#include <QStack>
#include <QVector>
#include <QFileInfo>

// Qt forward declarations
class QSettings;
class QXmlStreamReader;

// Custom forward declarations
class PSChunkData;
class PSModelData;

#include "PSXMLReader.h"
#include "PSStatusDescribable.h"

class PSDATASHARED_EXPORT PSProjectFileData : public PSXMLReader, public PSStatusDescribable {
public:
    PSProjectFileData(QFileInfo pPSProjectFile, QSettings* settings);
    ~PSProjectFileData();

    QFileInfo getPSProjectFile();
    QString getPSVersion();

    bool parseProjectFile();
    void processArrayElement(QXmlStreamReader* reader, QString elementName);

    unsigned int getChunkCount() const;
    unsigned int getActiveChunkIndex() const;
    PSChunkData* getActiveChunk() const;
    PSChunkData* getChunk(unsigned int index) const;
    QFileInfo getModelArchiveFile() const;
    PSModelData* getModelData() const;

    QString describeImageAlignPhase() const;
    char getAlignPhaseStatus() const;
    QString describeDenseCloudPhase() const;
    int getDenseCloudDepthImages() const;
    char getDenseCloudPhaseStatus() const;
    QString describeModelGenPhase() const;
    char getModelGenPhaseStatus() const;
    long getModelFaceCount() const;
    long getModelVertexCount() const;
    QString describeTextureGenPhase() const;
    char getTextureGenPhaseStatus() const;

private:
    QFileInfo mPSProjectFile;
    QStack<QFileInfo> mPathStack;
    QString mPSVersion;

    // Project Chunks
    QVector<PSChunkData*> mChunks;
    unsigned int mActiveChunk;
};

#endif
