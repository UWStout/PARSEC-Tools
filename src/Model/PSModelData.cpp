#include "PSModelData.h"

#include "PSChunkData.h"

#include <QXmlStreamReader>
#include <QFile>

PSModelData::PSModelData(QFile* pZipFile) {
    mZipFile = pZipFile;

    mFaceCount = mVertexCount = -1;
    mHasVtxColors = mHasUV = false;
    mMeshFilepath = "";
}

PSModelData::~PSModelData() { }

void PSModelData::setFaceCount(long pFaceCount) { mFaceCount = pFaceCount; }
void PSModelData::setVertexCount(long pVertexCount) { mVertexCount = pVertexCount; }
void PSModelData::setHasVertexColors(bool pHasVtxColors) { mHasVtxColors = pHasVtxColors; }
void PSModelData::setHasUV(bool pHasUV) { mHasUV = pHasUV; }
void PSModelData::setMeshFilename(QString pMeshFilepath) { mMeshFilepath = pMeshFilepath; }

void PSModelData::addTextureFile(int pId, QString pFilepath) {
    if(textureFiles.contains(pId)) {
        qWarning("Warning: possible texture ID collision (%d already exists)\n", pId);
    }

    textureFiles.insert(pId, pFilepath);
}

QFile* PSModelData::getArchiveFile() { return mZipFile; }

long PSModelData::getFaceCount() { return mFaceCount; }
long PSModelData::getVertexCount() { return mVertexCount; }
QString PSModelData::getMeshFilename() { return mMeshFilepath; }
bool PSModelData::hasVtxColors() { return mHasVtxColors; }
bool PSModelData::hasUV() { return mHasUV; }

QMap<int, QString> PSModelData::getTextureFiles() { return textureFiles; }
QString PSModelData::getTextureFile(int id) { return textureFiles.value(id); }

PSModelData* PSModelData::makeFromXML(QXmlStreamReader* reader, QFile* pZipFile, PSChunkData* pParent) {
    // If this is a fresh XML doc, push to first tag.
    if(reader->tokenType() == QXmlStreamReader::StartDocument) {
        reader->readNextStartElement();
    }

    // Sanity check
    if(reader == NULL || reader->name() != "model" || !reader->isStartElement()) {
        return NULL;
    }

    // Make a new object
    PSModelData* newModel = new PSModelData(pZipFile);

    // Parse the remaining XML data
    try {
        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement()) {
                // Data inside the 'mesh' tag
                if (reader->name() == "hasVertexColors") { newModel->mHasVtxColors = (reader->readElementText() == "true"); }
                else if (reader->name() == "hasUV") { newModel->mHasUV = (reader->readElementText() == "true"); }
                else if (reader->name() == "faceCount") { newModel->mFaceCount = reader->readElementText().toLong(); }
                else if (reader->name() == "vertexCount") { newModel->mVertexCount = reader->readElementText().toLong(); }

                // Start of the mesh tag
                else if (reader->name() == "mesh") { newModel->mMeshFilepath = reader->attributes().value("", "path").toString(); }

                // Texture tag
                else if (reader->name() == "texture") {
                    QString texPath = reader->attributes().value("", "path").toString();

                    // Note, sometimes textures don't have an ID (if there's only one texture for instance)
                    if (reader->attributes().hasAttribute("", "id")) {
                        int texID = reader->attributes().value("", "id").toInt();
                        newModel->addTextureFile(texID, texPath);
                    } else {
                        newModel->addTextureFile(0, texPath);
                    }
                }

                // From inside the meta tag
                else if (reader->name() == "property") {
                    QString lPropertyName = reader->attributes().value("", "name").toString();
                    QString lPropertyValue = reader->attributes().value("", "value").toString();

                    // Sometimes, this is the only way to get the face count (on older file versions)
                    if(lPropertyName.contains("face_count") && newModel->mFaceCount <= 0) {
                        newModel->mFaceCount = lPropertyValue.toLong();
                    }

                    pParent->parseChunkProperty(lPropertyName, lPropertyValue);
                }
            }

            // Tag close cases
            if(reader->isEndElement()) {
                if(reader->name() == "model") {
                    return newModel;
                }
            }
        }
    } catch (...) {
        throw new std::logic_error("Error parsing model XML tag to PSModelData");
    }

    // Should never reach this except when XML is malformed
    return NULL;
}
