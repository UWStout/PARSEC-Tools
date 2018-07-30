#include "PSProjectFileData.h"

#include <climits>
using namespace std;

#include <QFile>
#include <QSettings>
#include <QXmlStreamReader>

#include "PSChunkData.h"
//#include "PSModelData.h"

PSProjectFileData::PSProjectFileData(QFileInfo pPSProjectFile) {
    // Clear out any old chunks by re-initializing the array
    mPSProjectFile = pPSProjectFile;
    parseProjectFile();
    mActiveChunk = 0;
}

PSProjectFileData::~PSProjectFileData() {
    // Delete all chunk data
    for(unsigned int i=0; i < (unsigned int)mChunks.size(); i++) {
        delete mChunks[i];
    }
}

QFileInfo PSProjectFileData::getPSProjectFile() { return mPSProjectFile; }
QString PSProjectFileData::getPSVersion() { return mPSVersion; }

bool PSProjectFileData::parseProjectFile() {
    // Sanity Checks
    if(!mPSProjectFile.isFile() || !mPSProjectFile.isReadable()) {
        return false;
    }

    // Initialize file stack
    QXmlStreamReader* reader = NULL;
    mPathStack.push(mPSProjectFile);

    try {
        // Get the starting XML stream
        reader = getXMLStreamFromFile(mPSProjectFile);
        if (reader == NULL) {
            qWarning("NULL QXmlStreamReader, cannot proceed.\n");
            return false;
        }

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

unsigned int PSProjectFileData::getChunkCount() const {
    return (unsigned int)mChunks.size();
}

unsigned int PSProjectFileData::getActiveChunkIndex() const {
    return mActiveChunk;
}

PSChunkData* PSProjectFileData::getActiveChunk() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk];
    }

    return NULL;
}

PSChunkData* PSProjectFileData::getChunk(unsigned int index) const {
    if(index < (unsigned int)mChunks.size()) {
        return mChunks[index];
    }

    return NULL;
}

QFileInfo PSProjectFileData::getModelArchiveFile() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelArchiveFile();
    }

    return QFileInfo();
}

PSModelData* PSProjectFileData::getModelData() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelData();
    }

    return NULL;
}

QString PSProjectFileData::describeImageAlignPhase() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeImageAlignPhase();
    }

    return QString("N/A");
}

char PSProjectFileData::getAlignPhaseStatus() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getAlignPhaseStatus();
    }

    return 0;
}

QString PSProjectFileData::describeDenseCloudPhase() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeDenseCloudPhase();
    }

    return QString("N/A");
}

int PSProjectFileData::getDenseCloudDepthImages() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getDenseCloudDepthImages();
    }

    return 0;
}

char PSProjectFileData::getDenseCloudPhaseStatus() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getDenseCloudPhaseStatus();
    }

    return 0;
}

QString PSProjectFileData::describeModelGenPhase() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeModelGenPhase();
    }

    return QString("N/A");
}

char PSProjectFileData::getModelGenPhaseStatus() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelGenPhaseStatus();
    }

    return 0;
}

long PSProjectFileData::getModelFaceCount() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelFaceCount();
    }

    return 0;
}

long PSProjectFileData::getModelVertexCount() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getModelVertexCount();
    }

    return 0;
}

QString PSProjectFileData::describeTextureGenPhase() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->describeTextureGenPhase();
    }

    return QString("N/A");
}

char PSProjectFileData::getTextureGenPhaseStatus() const {
    if(mActiveChunk < (unsigned int)mChunks.size()) {
        return mChunks[mActiveChunk]->getTextureGenPhaseStatus();
    }

    return 0;
}
