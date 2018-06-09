#include "PSChunkData.h"

#include <QXmlStreamReader>

#include "PSModelData.h"
#include "PSSensorData.h"
#include "PSCameraData.h"
#include "PSImageData.h"

// Make the final enum definitions
DEFINE_ENUM(ImageAlignmentDetail, IMAGE_ALIGNMENT_DETAIL_ENUM, PSChunkData)
DEFINE_ENUM(DenseCloudDetail, DENSE_CLOUD_DETAIL_ENUM, PSChunkData)
DEFINE_ENUM(DenseCloudFilter, DENSE_CLOUD_FILTER_ENUM, PSChunkData)
DEFINE_ENUM(ModelGenerationDetail, MODEL_GENERATION_DETAIL_ENUM, PSChunkData)

PSChunkData::PSChunkData(QFileInfo pSourceFile, QXmlStreamReader* reader) {
    mSourceFile = pSourceFile;
    init(reader);
}

PSChunkData::PSChunkData(QFileInfo pSourceFile, QXmlStreamReader* reader, QStack<QFileInfo> pFileStack) {
    mSourceFile = pSourceFile;
    mTempFileStack = pFileStack;
    init(reader);
}

PSChunkData::~PSChunkData() {}

void PSChunkData::init(QXmlStreamReader* reader) {
    mID = 0;
    mLabel = "";
    mEnabled = false;
    mInsideFrame = false;

    mModelData = NULL;
    mMarkerCount = mScalebarCount = 0;

    mOptimize_aspect = mOptimize_f = mOptimize_cx = mOptimize_cy = mOptimize_b1 = mOptimize_b2 = false;
    mOptimize_p1 = mOptimize_p2 = mOptimize_k1 = mOptimize_k2 = mOptimize_k3 = false;
    mOptimize_p3 = mOptimize_p4 = mOptimize_k4 = mOptimize_skew = false;

    mImageAlignment_Level = IAD_UNKNOWN; mImageAlignment_Masked = false;
    mImageAlignment_featureLimit = mImageAlignment_tiePointLimit = 0;
    mDenseCloud_level = DCD_UNKNOWN;
    mDenseCloud_filterLevel = DCF_UNKNOWN;
    mDenseCloud_imagesUsed = 0;

    mModelGeneration_resolution = 0.0;
    mModelGeneration_faceCount = 0;
    mModelGeneration_denseSource = false;
    mModelGeneration_interpolationEnabled = false;

    mTextureGeneration_uvGenDuration = 0.0;
    mTextureGeneration_blendDuration = 0.0;
    mTextureGeneration_count = 0;
    mTextureGeneration_width = mTextureGeneration_height = 0;

    mImageAlignment_matchDurationSeconds = mImageAlignment_alignDurationSeconds = 0;
    mOptimize_durationSeconds = mDenseCloud_durationSeconds = 0;
    mModelGeneration_durationSeconds = 0;

    parseXMLChunk(reader);
}

void PSChunkData::parseXMLChunk(QXmlStreamReader* reader) {

    // Sanity checks
    if(reader == NULL || reader->name() != "chunk") {
        return;
    }

    if(mTempFileStack.empty()) {
        mTempFileStack.push(mSourceFile);
    }

    try { reader = explodeTag(reader, mTempFileStack); }
    catch (...) {
        qWarning("Error exploding chunk tag");
    }

    try {
        while(!reader->atEnd()) {
            reader->readNext();
            if (reader->isStartElement()) {
                QStringRef elem = reader->name();
                if (elem == "sensors") { readElementArray(reader, "sensors", "sensor"); }
                else if (elem == "cameras") { readElementArray(reader, "cameras", "camera"); }
                else if (elem == "Markers") { readElementArray(reader, "markers", "marker"); }
                else if (elem == "scalebars") { readElementArray(reader, "scalebars", "scalebar"); }
                else if (elem == "frames") { readElementArray(reader, "frames", "frame"); }
                else if (elem == "property") {
                    QString lPropertyName = reader->attributes().value(NULL, "name").toString();
                    QString lPropertyValue = reader->attributes().value(NULL, "value").toString();
                    parseChunkProperty(lPropertyName, lPropertyValue);
                }
            }
        }
    } catch (...) {
        qWarning("XML parsing error in chunk tag.\n");
    }
}

void PSChunkData::processArrayElement(QXmlStreamReader* reader, QString elem) {
    if (elem == "sensor") {
        try {
            PSSensorData* lNewSensor = PSSensorData::makeFromXML(reader);
            mSensors.insert(lNewSensor->ID, lNewSensor);
        } catch (...) {
            qWarning("Error while parsing XML to make PSSensorData.");
        }
     } else if (elem == "camera") {
        if(mInsideFrame) {
            try {
                PSImageData* lNewImage = PSImageData::makeFromXML(reader);
                if(mCameras.contains(lNewImage->getCamID())) {
                    lNewImage->setCameraData(mCameras.value(lNewImage->getCamID()));
                }
                mImages.push_back(lNewImage);
            } catch (...) {
                qWarning("Error while parsing XML to make PSImageData.");
            }
        } else {
            try {
                PSCameraData* lNewCamera = PSCameraData::makeFromXML(reader);
                if(mSensors.contains(lNewCamera->getSensorID())) {
                    lNewCamera->setSensorData(mSensors.value(lNewCamera->getSensorID()));
                }
                mCameras.insert(lNewCamera->ID, lNewCamera);
            } catch (...) {
                qWarning("Error while parsing XML to make PSCameraData.");
            }
        }
    } else if (elem == "depth_map") { addDepthImage(); }
    else if (elem == "marker") { mMarkerCount++; }
    else if (elem == "scalebar") { mScalebarCount++; }
    else if (elem == "frame") {
        try { parseXMLFrame(reader); }
        catch(...) {
            qWarning("Error while parsing XML frame tag from chunk.");
        }
    } else {
        PSXMLReader::processArrayElement(reader, elem);
    }
}

void PSChunkData::parseXMLFrame(QXmlStreamReader* reader) {
    // Sanity checks
    if(reader == NULL || reader->name() != "frame") { return; }

    // Follow any path attrib if needed
    QXmlStreamReader* preFrameReader = reader;
    try { reader = explodeTag(reader, mTempFileStack); }
    catch (...) { qWarning("Error exploding a frame tag"); }

    mFrameFile = mTempFileStack.top();
    mInsideFrame = true;

    // Parse the elements within the frame tag
    try {
        while(!reader->atEnd()) {
            reader->readNext();
            if (reader->isStartElement()) {
                QStringRef elem = reader->name();
                if (elem == "cameras") {
                    readElementArray(reader, "cameras", "camera");
                } else if (elem == "Markers") {
                    // Ignore all the markers inside the frame tag
                    while(!reader->isEndElement() || !(reader->name() == "markers")) {
                        reader->readNext();
                    }
                } else if (elem == "depth_maps") {
                    readElementArray(reader, "depth_maps", "depth_map");
                } else if (elem == "thumbnails") {
                } else if (elem == "point_cloud") {
                } else if (elem == "dense_cloud") {
                } else if (elem == "model") {
                    // Dive into the model tag
                    QXmlStreamReader* preModelReader = reader;
                    try { reader = explodeTag(reader, mTempFileStack); }
                    catch (...) {
                        qWarning("Error exploding a model tag");
                    }

                    // Build the model object
                    mModelData = PSModelData::makeFromXML(reader, mTempFileStack.top(), this);

                    // Return to old stream
                    if(reader != preModelReader) {
                        mTempFileStack.pop();
                        delete reader->device();
                        delete reader;
                        reader = preFrameReader;
                    }
                } else if (elem == "property") {
                    QString lPropertyName = reader->attributes().value(NULL, "name").toString();
                    QString lPropertyValue = reader->attributes().value(NULL, "value").toString();
                    parseChunkProperty(lPropertyName, lPropertyValue);
                }
            }
        }

        // Return to old stream
        if(preFrameReader != reader) {
            delete reader->device();
            delete reader;
            reader = preFrameReader;
        }
    } catch (...) {
        qWarning("XML parsing error while inside frame tag");
    }

    mInsideFrame = false;
}

// Read a property tag that's inside a chunk (1 or MORE levels below)
void PSChunkData::parseChunkProperty(QString pPropN, QString pPropV) {
    // Pre-convert values for use in if-else below
    double lPropVD = pPropV.toDouble();
    long long lPropVL = pPropV.toLongLong();

    // Texture Generation Properties
    if(pPropN == "atlas/atlas_blend_mode") { this->setTextureGeneration_blendMode((char)lPropVL); }
    else if(pPropN == "atlas/atlas_count") { this->setTextureGeneration_count((char)lPropVL); }
    else if(pPropN == "atlas/atlas_height") { this->setTextureGeneration_height((int)lPropVL); }
    else if(pPropN == "atlas/atlas_mapping_mode") { this->setTextureGeneration_mappingMode((char)lPropVL); }
    else if(pPropN == "atlas/atlas_width") { this->setTextureGeneration_width((int)lPropVL); }

    // Model Generation Properties
    else if(pPropN == "model/depth_downscale") { }
    else if(pPropN == "model/depth_filter_mode") { } //value="2"/>
    else if(pPropN == "model/mesh_face_count") { this->setModelGeneration_faceCount((long)lPropVL); }
    else if(pPropN == "model/mesh_interpolation") { this->setModelGeneration_interpolationEnabled((lPropVL)==1LL); }
    else if(pPropN == "model/mesh_object_type") { } //value="0"/>
    else if(pPropN == "model/mesh_source_data") { this->setModelGeneration_denseSource((lPropVL)==1LL); }
    else if(pPropN == "model/resolution") { this->setModelGeneration_resolution(lPropVD); }

    // Dense cloud properties
    else if(pPropN == "dense_cloud/depth_downscale") { this->setDenseCloud_level((DenseCloudDetail)lPropVL); }
    else if(pPropN == "dense_cloud/depth_filter_mode") { this->setDenseCloud_filterLevel((DenseCloudFilter)lPropVL); }
    else if(pPropN == "dense_cloud/density") { }
    else if(pPropN == "dense_cloud/resolution") { }

    // Image Alignment properties
    else if(pPropN == "match/match_downscale") { this->setImageAlignment_Level((ImageAlignmentDetail)lPropVL); }
    else if(pPropN == "match/match_filter_mask") { this->setImageAlignment_Masked((lPropVL)==0LL?false:true); }
    else if(pPropN == "match/match_point_limit") { this->setImageAlignment_featureLimit((long)lPropVL); }
    else if(pPropN == "match/match_tiepoint_limit") { this->setImageAlignment_tiePointLimit((long)lPropVL); }
    else if(pPropN == "match/match_select_pairs") { }

    // Duration properties
    else if(pPropN == "match/duration") { this->setImageAlignment_matchDurationSeconds(lPropVD); }
    else if(pPropN == "optimize/duration") { this->setOptimize_durationSeconds(lPropVD); }
    else if(pPropN == "dense_cloud/duration") { this->setDenseCloud_durationSeconds(lPropVD); }
    else if(pPropN == "align/duration") { this->setImageAlignment_alignDurationSeconds(lPropVD); }
    else if(pPropN == "model/duration") { this->setModelGeneration_durationSeconds(lPropVD); }
    else if(pPropN == "atlas/duration_blend") { this->setTextureGeneration_blendDuration(lPropVD); }
    else if(pPropN == "atlas/duration_uv") { this->setTextureGeneration_uvGenDuration(lPropVD); }

    // Fitting properties
    else if(pPropN == "optimize/fit_flags") {
        QVector<QStringRef> tokens = pPropV.splitRef(" ");
        for(int i=0; i<tokens.size(); i++) {
            if (tokens[i] == "f") { this->setOptimize_f(true); }
            else if (tokens[i] == "cx") { this->setOptimize_cx(true); }
            else if (tokens[i] == "cy") { this->setOptimize_cy(true); }
            else if (tokens[i] == "b1") { this->setOptimize_b1(true); }
            else if (tokens[i] == "b2") { this->setOptimize_b2(true); }
            else if (tokens[i] == "k1") { this->setOptimize_k1(true); }
            else if (tokens[i] == "k2") { this->setOptimize_k2(true); }
            else if (tokens[i] == "k3") { this->setOptimize_k3(true); }
            else if (tokens[i] == "k4") { this->setOptimize_k4(true); }
            else if (tokens[i] == "p1") { this->setOptimize_p1(true); }
            else if (tokens[i] == "p2") { this->setOptimize_p2(true); }
            else if (tokens[i] == "p3") { this->setOptimize_p3(true); }
            else if (tokens[i] == "p4") { this->setOptimize_p4(true); }
        }
    }

    else if(pPropN == "optimize/fit_aspect") { this->setOptimize_aspect((lPropVL==0LL?false:true)); }
    else if(pPropN == "optimize/fit_cxcy") {
        this->setOptimize_cx((lPropVL==0LL?false:true));
        this->setOptimize_cy((lPropVL==0LL?false:true));
    }
    else if(pPropN == "optimize/fit_f") { this->setOptimize_f((lPropVL==0LL?false:true)); }
    else if(pPropN == "optimize/fit_k1k2k3") {
        this->setOptimize_k1((lPropVL==0LL?false:true));
        this->setOptimize_k2((lPropVL==0LL?false:true));
        this->setOptimize_k3((lPropVL==0LL?false:true));
    }
    else if(pPropN == "optimize/fit_p1p2") {
        this->setOptimize_p1((lPropVL==0LL?false:true));
        this->setOptimize_p2((lPropVL==0LL?false:true));
    }
    else if(pPropN == "optimize/fit_skew") { this->setOptimize_skew((lPropVL==0LL?false:true)); }
    else if(pPropN == "optimize/fit_k4") { this->setOptimize_k4((lPropVL==0LL?false:true)); }

    // Accuracy properties
    else if(pPropN == "accuracy_tiepoints") { }
    else if(pPropN == "accuracy_cameras") { }
    else if(pPropN == "accuracy_cameras_ypr") { }
    else if(pPropN == "accuracy_markers") { }
    else if(pPropN == "accuracy_scalebars") { }
    else if(pPropN == "accuracy_projections") { }
}

QString PSChunkData::toString() const {
    // General details
    QString lDetails = "\tChunk ID: " + QString::number(mID) + ", Label: ";
    if(mLabel != NULL && mLabel != "") {
        lDetails += mLabel + "\n";
    } else {
        lDetails += "[none]\n";
    }

    lDetails += "\t" + QString::number(getImageCount()) + " image(s), "
             + QString::number(mSensorCount_inChunk) + " sensor(s), "
             + QString::number(getDenseCloud_imagesUsed()) + " depth map(s)\n";

    if(!mEnabled) { lDetails += "\tDISABLED\n"; }

    // Image alignment details
    if(mImageAlignment_Level == 0) {
        lDetails += "\n\tImage Align - no data\n";
    } else {
//        long lMillisecs = (long)((mImageAlignment_matchDurationSeconds + mImageAlignment_alignDurationSeconds)*1000);
//        Duration lAlignTime = Duration.ofMillis(lMillisecs);

        lDetails += "\n\tImage Align";
//        if(lAlignTime != Duration.ZERO) {	lDetails += " - " + formatDuration(lAlignTime); }
        lDetails += "\n";

        lDetails.append("\t              ");
        lDetails.append(getDescription(mImageAlignment_Level));
        if(mImageAlignment_Masked) { lDetails.append(", MASKED"); }
        lDetails += "\n";

        lDetails += "\t              " + QString::number(mImageAlignment_featureLimit) + " key point limit\n";
        lDetails += "\t              " + QString::number(mImageAlignment_tiePointLimit) + " tie point limit\n";
    }

    // Optimization details
//    Duration lOptimizeTime = Duration.ofMillis((long)(mOptimize_durationSeconds*1000));
//    if(lOptimizeTime != Duration.ZERO) {
        lDetails += "\n\tOptimization - " ; //+ formatDuration(lOptimizeTime);

        int lCount = 0;
        if(mOptimize_aspect) { lDetails += addOptimizeElement("aspect", lCount++); }
        if(mOptimize_f) { lDetails += addOptimizeElement("f", lCount++); }
        if(mOptimize_cx) { lDetails += addOptimizeElement("Cx", lCount++); }
        if(mOptimize_cy) { lDetails += addOptimizeElement("Cy", lCount++); }
        if(mOptimize_b1) { lDetails += addOptimizeElement("B1", lCount++); }
        if(mOptimize_b2) { lDetails += addOptimizeElement("B2", lCount++); }
        if(mOptimize_p1) { lDetails += addOptimizeElement("P1", lCount++); }
        if(mOptimize_p2) { lDetails += addOptimizeElement("P2", lCount++); }
        if(mOptimize_p3) { lDetails += addOptimizeElement("P3", lCount++); }
        if(mOptimize_p4) { lDetails += addOptimizeElement("P4", lCount++); }
        if(mOptimize_k1) { lDetails += addOptimizeElement("k1", lCount++); }
        if(mOptimize_k2) { lDetails += addOptimizeElement("k2", lCount++); }
        if(mOptimize_k3) { lDetails += addOptimizeElement("k3", lCount++); }
        if(mOptimize_k4) { lDetails += addOptimizeElement("k4", lCount++); }
        if(mOptimize_skew) { lDetails += addOptimizeElement("skew", lCount++); }
        if(lCount > 0) { lDetails = lDetails.left(lDetails.length()-2); }
        lDetails += "\n";
//    }

    // Dense cloud details
    if(mDenseCloud_filterLevel == 0) {
        lDetails += "\n\tDense Cloud - no data\n";
    } else {
//        Duration lDenseCloudTime = Duration.ofMillis((long)(mDenseCloud_durationSeconds*1000));
        lDetails += "\n\tDense Cloud";
//        if(lDenseCloudTime != Duration.ZERO) {	lDetails += " - " + formatDuration(lDenseCloudTime); }
        lDetails += "\n";

        lDetails.append("\t              ");
        lDetails.append(getDescription(mDenseCloud_level));
        lDetails.append(", ");
        lDetails.append(getDescription(mDenseCloud_filterLevel));
        lDetails.append("\n");
    }

    // Model generation details
    if(mModelData == NULL) {
        lDetails += "\n\tModel Generation - no data\n";
    } else {
//        Duration lModelGenTime = Duration.ofMillis((long)(mModelGeneration_durationSeconds*1000));
        lDetails += "\n\tModel Generation";
//        if(lModelGenTime != Duration.ZERO) {	lDetails += " - " + formatDuration(lModelGenTime); }
        lDetails += "\n";

        lDetails += "\t                   " + QString::number(mModelData->getFaceCount()) + " faces\n";
    }

    return lDetails;
}

/**
 * A helper for adding the various 'optimization' options to the main string
 * returned by toString().
 *
 * @param pName The name of the optimization (will be added to the string)
 * @param pCount How many optimizations have been added already (will break at 4)
 * @return A string to be appended to the toString() results
 */
QString PSChunkData::addOptimizeElement(QString pName, int pCount) const
{
    QString result = "";
    if(pCount%4 == 0) { result += "\n\t               "; }
    result += pName + ", ";
    return result;
}

/**
 * Format a java.time.Duration as a more human readable string than
 * Duration.toString() gives.
 *
 * @param pTime The duration object you want to format as a string.
 * @return The duration object expressed as a human readable string.
 */
//QString PSChunkData::formatDuration(Duration pTime) {
//    QString result = pTime.toDays() + "d ";
//    pTime = pTime.minusDays(pTime.toDays());

//    result += pTime.toHours() + "h ";
//    pTime = pTime.minusHours(pTime.toHours());

//    result += pTime.toMinutes() + "m ";
//    pTime = pTime.minusMinutes(pTime.toMinutes());

//    result += String.format("%.2f", pTime.toNanos()*1.0e-9) + "s";

//    return result;
//}

QString PSChunkData::getOptimizeString() const {
    QString lDesc = "";

    if(mOptimize_aspect) { lDesc += "aspect, "; }
    if(mOptimize_f) { lDesc += "fv, "; }
    if(mOptimize_cx) { lDesc += "Cx, "; }
    if(mOptimize_cy) { lDesc += "Cy, "; }
    if(mOptimize_b1) { lDesc += "B1, "; }
    if(mOptimize_b2) { lDesc += "B2, "; }
    if(mOptimize_p1) { lDesc += "P1, "; }
    if(mOptimize_p2) { lDesc += "P2, "; }
    if(mOptimize_p3) { lDesc += "P3, "; }
    if(mOptimize_p4) { lDesc += "P4, "; }
    if(mOptimize_k1) { lDesc += "k1, ";  }
    if(mOptimize_k2) { lDesc += "k2, ";  }
    if(mOptimize_k3) { lDesc += "k3, ";  }
    if(mOptimize_k4) { lDesc += "k4, "; }
    if(mOptimize_skew) { lDesc += "skew, "; }

    if(!lDesc.isEmpty()) { lDesc = lDesc.left(lDesc.length()-2); }
    return lDesc;
}

//QString PSChunkData::getOptimize_durationString() const {
//    long lMillisecs = (long)(mOptimize_durationSeconds*1000);
//    Duration lOptimizeTime = Duration.ofMillis(lMillisecs);
//    return formatDuration(lOptimizeTime);
//}

//QString PSChunkData::getDenseCloud_durationString() const {
//    long lMillisecs = (long)(mDenseCloud_durationSeconds*1000);
//    Duration lDenseCloudTime = Duration.ofMillis(lMillisecs);
//    return formatDuration(lDenseCloudTime);
//}

//QString PSChunkData::getModelGeneration_durationString() const {
//    long lMillisecs = (long)(mModelGeneration_durationSeconds*1000);
//    Duration lModelGenTime = Duration.ofMillis(lMillisecs);
//    return formatDuration(lModelGenTime);
//}

//QString PSChunkData::getTextureGeneration_durationString() const {
//    long lMillisecs = (long)((mTextureGeneration_blendDuration+mTextureGeneration_uvGenDuration)*1000);
//    Duration lTexGenTime = Duration.ofMillis(lMillisecs);
//    return formatDuration(lTexGenTime);
//}

// Output string of the form ([num aligned images] - [feature limit]/[tie point limit])
QString PSChunkData::describeImageAlignPhase() const {
    QString lData = getImageAlignment_LevelString();
    long featLimit = getImageAlignment_featureLimit()/1000;
    long tieLimit = getImageAlignment_tiePointLimit()/1000;
    lData += " (" + QString::number(mImages.size()) + " - " +
            QString::number(featLimit) + "k/" +
            QString::number(tieLimit) + "k)";
    return lData;
}

// Compute ratio of total images to aligned images and return status
char PSChunkData::getAlignPhaseStatus() const {
    long allImages = mCameras.size();
    long alignedImages = mImages.size();
    double ratio = alignedImages/(double)allImages;

    if(alignedImages == 0 || describeImageAlignPhase() == "N/A") {
        return 5;
    } else if(ratio < .100) {
        return 4;
    } else if(ratio < .3333) {
        return 3;
    } else if(ratio < .6667) {
        return 2;
    } else if(ratio < .950) {
        return 1;
    } else {
        return 0;
    }
}

QString PSChunkData::describeDenseCloudPhase() const {
    QString lData = getDenseCloud_levelString();
    lData += " (" + QString::number(getDenseCloud_imagesUsed()) + ")";
    return lData;
}

int PSChunkData::getDenseCloudDepthImages() const {
    return getDenseCloud_imagesUsed();
}

char PSChunkData::getDenseCloudPhaseStatus() const {
    long projectImages = mCameras.size();
    long depthImages = mDenseCloud_imagesUsed;
    double ratio = depthImages/(double)projectImages;
    if(depthImages == 0) {
        if(describeDenseCloudPhase() == "N/A") {
            return 5;
        } else {
            return 3;
        }
    } else if(ratio < .100) {
        return 4;
    } else if(ratio < .3333) {
        return 3;
    } else if(ratio < .6667) {
        return 2;
    } else if(ratio < .950) {
        return 1;
    } else {
        return 0;
    }
}

QString PSChunkData::describeModelGenPhase() const {
    double faces = getModelFaceCount();
    if(faces < 0) {
        if(hasMesh()) { return "?"; }
        else { return "N/A"; }
    }

    faces /= 1000;
    if(faces >= 1000) {
        return QString::asprintf("%.1fM faces", faces/1000);
    } else {
        return QString::asprintf("%.1fK faces", faces);
    }
}

bool PSChunkData::hasMesh() const { return mModelData != NULL; }

QFileInfo PSChunkData::getModelArchiveFile() const {
    if(mModelData != NULL) {
        return mModelData->getArchiveFile();
    }

    return QFileInfo();
}

long PSChunkData::getModelFaceCount() const {
    if(!hasMesh()) return -1L;
    return mModelData->getFaceCount();
}

long PSChunkData::getModelVertexCount() const {
    if(!hasMesh()) return -1L;
    return mModelData->getVertexCount();
}

char PSChunkData::getModelGenPhaseStatus() const {
    long faceCount = getModelFaceCount();

    // Examine the model resolution
    if(faceCount < 0) {
        return 5;
    } else if(faceCount < 5000) {
        return 4;
    } else if(faceCount < 10000) {
        return 3;
    } else if(faceCount < 50000) {
        return 2;
    } else if(faceCount < 1000000) {
        return 1;
    } else {
        return 0;
    }
}

QString PSChunkData::describeTextureGenPhase() const {
    if(getTextureGeneration_count() != 0) {
        return QString::asprintf("%d @ (%d x %d)", getTextureGeneration_count(),
                getTextureGeneration_width(), getTextureGeneration_height());
    }

    return "N/A";
}

char PSChunkData::getTextureGenPhaseStatus() const {
    // Examine the texture resolution
    if(getTextureGeneration_width() == 0 || getTextureGeneration_height() == 0) {
        return 5;
    } else if(getTextureGeneration_width() < 1024 || getTextureGeneration_height() < 1024) {
        return 4;
    } else if(getTextureGeneration_width() < 2048 || getTextureGeneration_height() < 2048) {
        return 3;
    } else if(getTextureGeneration_width() < 3072 || getTextureGeneration_height() < 3072) {
        return 2;
    } else if(getTextureGeneration_width() < 4096 || getTextureGeneration_height() < 4069) {
        return 1;
    } else {
        return 0;
    }
}
