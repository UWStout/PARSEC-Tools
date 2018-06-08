#include "EnumFactory.h"

class QFile;
class PSModelData;
class PSSensorData;
class PSCameraData;
class PSImageData;

#include "PSXMLReader.h"
#include "PSStatusDescribable.h"

#include <QStack>
#include <QMap>

class PSChunkData : public PSXMLReader, public PSStatusDescribable {
public:
    /**
     * @author berriers
     * Set of fixed levels of detail for the PhotoScan Image Alignment phase.
     * These match values found in the standard PhotoScan XML file.
     */
    #define IMAGE_ALIGNMENT_DETAIL_ENUM(ImageAlignmentDetail) \
        ImageAlignmentDetail(IAD_UNKNOWN,,"Unknown Detail") \
        ImageAlignmentDetail(IAD_HIGH,,"High Detail") \
        ImageAlignmentDetail(IAD_MEDIUM,,"Medium Detail") \
        ImageAlignmentDetail(IAD_LOW,,"Low Detail") \

    DECLARE_ENUM(ImageAlignmentDetail, IMAGE_ALIGNMENT_DETAIL_ENUM)

    /**
     * @author berriers
     * Set of fixed levels of detail for the PhotoScan Dense Cloud phase.
     * These match values found in the standard PhotoScan XML file.
     */
    #define DENSE_CLOUD_DETAIL_ENUM(DenseCloudDetail) \
        DenseCloudDetail(DCD_UNKNOWN,,"Unknown Detail") \
        DenseCloudDetail(DCD_ULTRA_HIGH,,"Ultra High Detail") \
        DenseCloudDetail(DCD_HIGH,,"High Detail") \
        DenseCloudDetail(DCD_MEDIUM,,"Medium Detail") \
        DenseCloudDetail(DCD_LOW,,"Low Detail") \
        DenseCloudDetail(DCD_LOWEST,,"Lowest Detail") \

    DECLARE_ENUM(DenseCloudDetail, DENSE_CLOUD_DETAIL_ENUM)

    /**
     * @author berriers
     * Set of fixed levels of filtering for the PhotoScan Dense Cloud phase.
     * These match values found in the standard PhotoScan XML file.
     */
    #define DENSE_CLOUD_FILTER_ENUM(DenseCloudFilter) \
        DenseCloudFilter(DCF_UNKNOWN,,"Unknown Filter") \
        DenseCloudFilter(DCF_DISABLED,,"Filter Disabled") \
        DenseCloudFilter(DCF_AGRESSIVE,,"Aggressive Filter") \
        DenseCloudFilter(DCF_MODERATE,,"Moderate Filter") \
        DenseCloudFilter(DCF_MILD,,"Mild Filter") \

    DECLARE_ENUM(DenseCloudFilter, DENSE_CLOUD_FILTER_ENUM)

    /**
     * @author berriers
     * Set of fixed levels of detail for the PhotoScan Mesh Generation phase.
     * These match values found in the standard PhotoScan XML file.
     */
    #define MODEL_GENERATION_DETAIL_ENUM(ModelGenerationDetail) \
        ModelGenerationDetail(MGD_UNKNOWN,,"Unknown Detail") \
        ModelGenerationDetail(MGD_LOW,,"Low Detail") \
        ModelGenerationDetail(MGD_MEDIUM,,"Medium Detail") \
        ModelGenerationDetail(MGD_HIGH,,"High Detail") \
        ModelGenerationDetail(MGD_CUSTOM,,"Custom Detail") \

    DECLARE_ENUM(ModelGenerationDetail, MODEL_GENERATION_DETAIL_ENUM)

    PSChunkData(QFile* pSourceFile, QXmlStreamReader* reader);
    PSChunkData(QFile* pSourceFile, QXmlStreamReader* reader, const QStack<QFile*>* pFileStack);
    virtual ~PSChunkData();

    void parseXMLChunk(QXmlStreamReader* reader);
    void processArrayElement(QXmlStreamReader* reader, QString elementName);
    void parseXMLFrame(QXmlStreamReader* reader);
    void parseChunkProperty(QString pPropN, QString pPropV);
    QString toString();

    QString addOptimizeElement(QString pName, int pCount);

    QString getLabel() { return mLabel; }
    void addMarker() { mMarkerCount++; }
    void addScalebar() { mScalebarCount++; }
    void addDepthImage() { mDenseCloud_imagesUsed++; }
    long getMarkerCount() { return mMarkerCount; }
    long getScalebarCount() { return mScalebarCount; }

    void addImage(PSImageData* pImage) { mImages.push_back(pImage); }
    unsigned int getImageCount() { return (unsigned int)mImages.size(); }
    unsigned int getCameraCount() { return (unsigned int)mCameras.size(); }

    void addSensor() { mSensorCount_inChunk++; }
    long getSensorCount() { return mSensorCount_inChunk; }

    bool hasMesh();
    void setModelData(PSModelData* pModelData) { mModelData = pModelData; }
    PSModelData* getModelData() { return mModelData; }
    QFile* getModelArchiveFile();

    // Convert the optimization values to string
    QString getOptimizeString();

    // Enum values as descriptive strings
    QString getImageAlignment_LevelString() { return getDescription(mImageAlignment_Level); }
    QString getDenseCloud_levelString() { return getDescription(mDenseCloud_level); }
    QString getDenseCloud_filterLevelString() { return getDescription(mDenseCloud_filterLevel); }

    // Durations converted to human-readable strings
//    QString getImageAlignment_durationString();
//    QString getOptimize_durationString();
//    QString getDenseCloud_durationString();
//    QString getModelGeneration_durationString();
//    QString getTextureGeneration_durationString();

    // Summary descriptsion of the various phases
    QString describeImageAlignPhase();
    QString describeDenseCloudPhase();
    QString describeModelGenPhase();
    QString describeTextureGenPhase();

    double getImageAlignment_matchDurationSeconds() { return mImageAlignment_matchDurationSeconds; }
    void setImageAlignment_matchDurationSeconds(double pImageAlignment_matchDurationSeconds) {
        mImageAlignment_matchDurationSeconds = pImageAlignment_matchDurationSeconds;
    }

    double getImageAlignment_alignDurationSeconds() { return mImageAlignment_alignDurationSeconds; }
    void setImageAlignment_alignDurationSeconds(double pImageAlignment_alignDurationSeconds) {
        mImageAlignment_alignDurationSeconds = pImageAlignment_alignDurationSeconds;
    }

    ImageAlignmentDetail getImageAlignment_Level() { return mImageAlignment_Level; }
    void setImageAlignment_Level(ImageAlignmentDetail pImageAlignment_Level) {
        mImageAlignment_Level = pImageAlignment_Level;
    }

    bool getImageAlignment_Masked() { return mImageAlignment_Masked; }
    void setImageAlignment_Masked(bool pImageAlignment_Masked) {
        mImageAlignment_Masked = pImageAlignment_Masked;
    }

    long getImageAlignment_featureLimit() { return mImageAlignment_featureLimit; }
    void setImageAlignment_featureLimit(long pImageAlignment_featureLimit) {
        mImageAlignment_featureLimit = pImageAlignment_featureLimit;
    }

    long getImageAlignment_tiePointLimit() { return mImageAlignment_tiePointLimit; }
    void setImageAlignment_tiePointLimit(long pImageAlignment_tiePointLimit) {
        mImageAlignment_tiePointLimit = pImageAlignment_tiePointLimit;
    }

    double getOptimize_durationSeconds() { return mOptimize_durationSeconds; }
    void setOptimize_durationSeconds(double pOptimize_durationSeconds) {
        mOptimize_durationSeconds = pOptimize_durationSeconds;
    }

    bool getOptimize_aspect() { return mOptimize_aspect; }
    void setOptimize_aspect(bool pOptimize_aspect) {
        mOptimize_aspect = pOptimize_aspect;
    }

    bool getOptimize_f() { return mOptimize_f; }
    void setOptimize_f(bool pOptimize_f) { mOptimize_f = pOptimize_f; }

    bool getOptimize_cx() { return mOptimize_cx; }
    bool getOptimize_cy() { return mOptimize_cy; }
    void setOptimize_cx(bool pOptimize_cx) { mOptimize_cx = pOptimize_cx; }
    void setOptimize_cy(bool pOptimize_cy) { mOptimize_cy = pOptimize_cy; }

    bool getOptimize_p1() { return mOptimize_p1; }
    bool getOptimize_p2() { return mOptimize_p2; }
    bool getOptimize_p3() { return mOptimize_p3; }
    bool getOptimize_p4() { return mOptimize_p4; }

    void setOptimize_p1(bool pOptimize_p1) { mOptimize_p1 = pOptimize_p1; }
    void setOptimize_p2(bool pOptimize_p2) { mOptimize_p2 = pOptimize_p2; }
    void setOptimize_p3(bool pOptimize_p3) { mOptimize_p3 = pOptimize_p3; }
    void setOptimize_p4(bool pOptimize_p4) { mOptimize_p4 = pOptimize_p4; }

    bool getOptimize_b1() { return mOptimize_b1; }
    bool getOptimize_b2() { return mOptimize_b2; }
    void setOptimize_b1(bool pOptimize_b1) { mOptimize_b1 = pOptimize_b1; }
    void setOptimize_b2(bool pOptimize_b2) { mOptimize_b2 = pOptimize_b2; }

    bool getOptimize_k1() { return mOptimize_k1; }
    bool getOptimize_k2() { return mOptimize_k2; }
    bool getOptimize_k3() { return mOptimize_k3; }
    bool getOptimize_k4() { return mOptimize_k4; }

    void setOptimize_k1(bool pOptimize_k1) { mOptimize_k1 = pOptimize_k1; }
    void setOptimize_k2(bool pOptimize_k2) { mOptimize_k2 = pOptimize_k2; }
    void setOptimize_k3(bool pOptimize_k3) { mOptimize_k3 = pOptimize_k3; }
    void setOptimize_k4(bool pOptimize_k4) { mOptimize_k4 = pOptimize_k4; }

    bool getOptimize_skew() { return mOptimize_skew; }
    void setOptimize_skew(bool pOptimize_skew) { mOptimize_skew = pOptimize_skew; }

    double getDenseCloud_durationSeconds() { return mDenseCloud_durationSeconds; }
    void setDenseCloud_durationSeconds(double pDenseCloud_durationSeconds) {
        mDenseCloud_durationSeconds = pDenseCloud_durationSeconds;
    }

    DenseCloudDetail getDenseCloud_level() { return mDenseCloud_level; }
    void setDenseCloud_level(DenseCloudDetail pDenseCloud_level) {
        mDenseCloud_level = pDenseCloud_level;
    }

    int getDenseCloud_imagesUsed() { return mDenseCloud_imagesUsed; }
    DenseCloudFilter getDenseCloud_filterLevel() { return mDenseCloud_filterLevel; }
    void setDenseCloud_filterLevel(DenseCloudFilter pDenseCloud_filterLevel) {
        mDenseCloud_filterLevel = pDenseCloud_filterLevel;
    }

    double getModelGeneration_resolution() { return mModelGeneration_resolution; }
    void setModelGeneration_resolution(double pModelGeneration_resolution) {
        mModelGeneration_resolution = pModelGeneration_resolution;
    }

    double getModelGeneration_durationSeconds() { return mModelGeneration_durationSeconds; }
    void setModelGeneration_durationSeconds(double pModelGeneration_durationSeconds) {
        mModelGeneration_durationSeconds = pModelGeneration_durationSeconds;
    }

    long getModelGeneration_faceCount() { return mModelGeneration_faceCount; }
    void setModelGeneration_faceCount(long pModelGeneration_faceCount) {
        mModelGeneration_faceCount = pModelGeneration_faceCount;
    }

    bool getModelGeneration_denseSource() { return mModelGeneration_denseSource; }
    void setModelGeneration_denseSource(bool pModelGeneration_denseSource) {
        mModelGeneration_denseSource = pModelGeneration_denseSource;
    }

    bool getModelGeneration_interpolationEnabled() { return mModelGeneration_interpolationEnabled; }
    void setModelGeneration_interpolationEnabled(bool pModelGeneration_interpolationEnabled) {
        mModelGeneration_interpolationEnabled = pModelGeneration_interpolationEnabled;
    }

    double getTextureGeneration_blendDuration() { return mTextureGeneration_blendDuration; }
    void setTextureGeneration_blendDuration(double pTextureGeneration_blendDuration) {
        mTextureGeneration_blendDuration = pTextureGeneration_blendDuration;
    }

    double getTextureGeneration_uvGenDuration() { return mTextureGeneration_uvGenDuration; }
    void setTextureGeneration_uvGenDuration(double pTextureGeneration_uvGenDuration) {
        mTextureGeneration_uvGenDuration = pTextureGeneration_uvGenDuration;
    }

    char getTextureGeneration_mappingMode() { return mTextureGeneration_mappingMode; }
    void setTextureGeneration_mappingMode(char pTextureGeneration_mappingMode) {
        mTextureGeneration_mappingMode = pTextureGeneration_mappingMode;
    }

    char getTextureGeneration_blendMode() { return mTextureGeneration_blendMode; }
    void setTextureGeneration_blendMode(char pTextureGeneration_blendMode) {
        mTextureGeneration_blendMode = pTextureGeneration_blendMode;
    }

    char getTextureGeneration_count() { return mTextureGeneration_count; }
    void setTextureGeneration_count(char pTextureGeneration_count) {
        mTextureGeneration_count = pTextureGeneration_count;
    }

    int getTextureGeneration_width() { return mTextureGeneration_width; }
    void setTextureGeneration_width(int pTextureGeneration_width) {
        mTextureGeneration_width = pTextureGeneration_width;
    }

    int getTextureGeneration_height() { return mTextureGeneration_height; }
    void setTextureGeneration_height(int pTextureGeneration_height) {
        mTextureGeneration_height = pTextureGeneration_height;
    }

    // Compute ratio of total images to aligned images and return status
    char getAlignPhaseStatus();
    int getDenseCloudDepthImages();
    char getDenseCloudPhaseStatus();
    long getModelFaceCount();
    long getModelVertexCount();
    char getModelGenPhaseStatus();
    char getTextureGenPhaseStatus();

private:
    // What files was this chunk data read from
    QFile* mSourceFile;
    QFile* mFrameFile;

    // General Information (from 'chunk' tag attributes)
    long mID;
    QString mLabel;
    bool mEnabled;
    bool mInsideFrame;

    // Sensors in this chunk
    long mSensorCount_inChunk;

    // Markers & Scalebars
    long mMarkerCount, mScalebarCount;

    // Model data
    PSModelData* mModelData;

    // Image Alignment phase details
    double mImageAlignment_matchDurationSeconds;
    double mImageAlignment_alignDurationSeconds;
    ImageAlignmentDetail mImageAlignment_Level;
    bool mImageAlignment_Masked;
    long mImageAlignment_featureLimit;
    long mImageAlignment_tiePointLimit;

    // Camera Optimization Phase
    double mOptimize_durationSeconds;
    bool mOptimize_aspect;
    bool mOptimize_f;
    bool mOptimize_cx;
    bool mOptimize_cy;
    bool mOptimize_b1;
    bool mOptimize_b2;
    bool mOptimize_p1;
    bool mOptimize_p2;
    bool mOptimize_p3;
    bool mOptimize_p4;
    bool mOptimize_k1;
    bool mOptimize_k2;
    bool mOptimize_k3;
    bool mOptimize_k4;
    bool mOptimize_skew;

    // Dense cloud generation phase
    double mDenseCloud_durationSeconds;
    DenseCloudDetail mDenseCloud_level;
    DenseCloudFilter mDenseCloud_filterLevel;
    char mDenseCloud_imagesUsed;

    // Model Generation phase
    double mModelGeneration_resolution;
    double mModelGeneration_durationSeconds;
    long mModelGeneration_faceCount;
    bool mModelGeneration_denseSource;
    bool mModelGeneration_interpolationEnabled;

    // Texture Generation phase
    double mTextureGeneration_blendDuration;
    double mTextureGeneration_uvGenDuration;
    char mTextureGeneration_mappingMode;
    char mTextureGeneration_blendMode;
    char mTextureGeneration_count;
    int mTextureGeneration_width;
    int mTextureGeneration_height;

    // Camera and Image data
    QMap<long, PSSensorData*> mSensors;
    QMap<long, PSCameraData*> mCameras;
    QVector<PSImageData*> mImages;

    // Stack used to track descending file structures
    QStack<QFile*> mTempFileStack;
};