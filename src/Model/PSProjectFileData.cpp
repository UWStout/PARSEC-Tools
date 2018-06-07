#include "PSProjectFileData.h"

#include <climits>
using namespace std;

#include <QFile>
#include <QSettings>
#include <QXmlStreamReader>

//#include "PSChunkData.h"
//#include "PSModelData.h"

PSProjectFileData::PSProjectFileData(QFile*& pPSProjectFile) { //}, QSettings*& settings) {
    // Clear out any old chunks by re-initializing the array
    mActiveChunk = UINT_MAX;

    if(pPSProjectFile != NULL) {
        mPSProjectFile = pPSProjectFile;
//        mPSProjectFile = mPSProjectFile->getCanonicalFile();
        parseProjectFile();
        mActiveChunk = 0;
    }
}

PSProjectFileData::~PSProjectFileData() {
    // Delete all chunk data
    for(unsigned int i=0; i < (unsigned int)mChunks.size(); i++) {
        delete mChunks[i];
    }

    // Clean up the file stack
    while(!mPathStack.empty()) {
        delete mPathStack.pop();
    }
}

QFile* PSProjectFileData::getPSProjectFile() { return mPSProjectFile; }
QString PSProjectFileData::getPSVersion() { return mPSVersion; }

bool PSProjectFileData::parseProjectFile() {
    // Sanity Checks
    if(mPSProjectFile == NULL || !mPSProjectFile->exists()) {
        return false;
    }

    // Initialize file stack
    QXmlStreamReader* reader = NULL;
    mPathStack.push(mPSProjectFile);

    try {
        // Get the starting XML stream
        reader = getXMLStreamFromFile(mPSProjectFile);

        // Loop over all element tags
        while (!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement())
            {
                // The document tag contains the PS XML version and possibly a path
                if (reader->name() == "document") {
                    // Extract PhotoScan file version
                    mPSVersion = reader->attributes().value(NULL, "version").toString();

                    // Attempt to descend
                    QXmlStreamReader* oldReader = reader;
                    reader = explodeTag(reader, mPathStack);
                    if(oldReader != reader) {
                        delete oldReader;
                    }
                }

                // The chunks tag is an array of chunk tags
                else if (reader->name() == "chunks") {
                    readElementArray(reader, "chunks", "chunk");
                }
            }
        }
    } catch (...) {
        // Log error
        qWarning("XML Parsing encountered an error\n");

        // Finish and return failure
        delete reader;
        return false;
    }

    // Finish and return success
    delete reader;
    return true;
}

void PSProjectFileData::processArrayElement(QXmlStreamReader* reader, QString elementName) {
    if (elementName == "chunk") {
        // Process this chunk tag
        PSChunkData* lNewChunk = new PSChunkData(mPSProjectFile, reader, mPathStack);

        // Save the results
        if(lNewChunk != NULL) {
            mChunks.push_back(lNewChunk);
        }
    } else {
        PSXMLReader::processArrayElement(reader, elementName);
    }
}

unsigned int PSProjectFileData::getChunkCount() {
    return (unsigned int)mChunks.size();
}

unsigned int PSProjectFileData::getActiveChunkIndex() {
    return mActiveChunk;
}

PSChunkData* PSProjectFileData::getActiveChunk() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk];
    }

    return NULL;
}

PSChunkData* PSProjectFileData::getChunk(unsigned int index) {
    if(index < (unsigned int)mChunks.size()) {
        return mChunks[index];
    }

    return NULL;
}

QFile* PSProjectFileData::getModelArchiveFile() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelArchiveFile();
    }

    return NULL;
}

PSModelData* PSProjectFileData::getModelData() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelData();
    }

    return NULL;
}

QString PSProjectFileData::describeImageAlignPhase() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeImageAlignPhase();
    }

    return QString("N/A");
}

char PSProjectFileData::getAlignPhaseStatus() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getAlignPhaseStatus();
    }

    return 0;
}

QString PSProjectFileData::describeDenseCloudPhase() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeDenseCloudPhase();
    }

    return QString("N/A");
}

int PSProjectFileData::getDenseCloudDepthImages() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getDenseCloudDepthImages();
    }

    return 0;
}

char PSProjectFileData::getDenseCloudPhaseStatus() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getDenseCloudPhaseStatus();
    }

    return 0;
}

QString PSProjectFileData::describeModelGenPhase() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeModelGenPhase();
    }

    return QString("N/A");
}

char PSProjectFileData::getModelGenPhaseStatus() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelGenPhaseStatus();
    }

    return 0;
}

long PSProjectFileData::getModelFaceCount() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelFaceCount();
    }

    return 0;
}

long PSProjectFileData::getModelVertexCount() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelVertexCount();
    }

    return 0;
}

QString PSProjectFileData::describeTextureGenPhase() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeTextureGenPhase();
    }

    return QString("N/A");
}

char PSProjectFileData::getTextureGenPhaseStatus() {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getTextureGenPhaseStatus();
    }

    return 0;
}
