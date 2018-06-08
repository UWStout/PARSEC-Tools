#include <QString>
#include <QStack>
#include <QVector>

// Qt forward declarations
class QFile;
class QSettings;
class QXmlStreamReader;

// Custom forward declarations
class PSChunkData;
class PSModelData;

#include "PSXMLReader.h"
#include "PSStatusDescribable.h"

class PSProjectFileData : public PSXMLReader, public PSStatusDescribable {
public:
    PSProjectFileData(QFile*& pPSProjectFile); //, QSettings*& settings);
    ~PSProjectFileData();

    QFile* getPSProjectFile();
    QString getPSVersion();

    bool parseProjectFile();
    void processArrayElement(QXmlStreamReader* reader, QString elementName);
    unsigned int getChunkCount();
    unsigned int getActiveChunkIndex();
    PSChunkData* getActiveChunk();
    PSChunkData* getChunk(unsigned int index);
    QFile* getModelArchiveFile();
    PSModelData* getModelData();

    QString describeImageAlignPhase();
    char getAlignPhaseStatus();
    QString describeDenseCloudPhase();
    int getDenseCloudDepthImages();
    char getDenseCloudPhaseStatus();
    QString describeModelGenPhase();
    char getModelGenPhaseStatus();
    long getModelFaceCount();
    long getModelVertexCount();
    QString describeTextureGenPhase();
    char getTextureGenPhaseStatus();

private:
    QFile* mPSProjectFile;
    QStack<QFile*> mPathStack;
    QString mPSVersion;

    // Project Chunks
    QVector<PSChunkData*> mChunks;
    unsigned int mActiveChunk;
};
