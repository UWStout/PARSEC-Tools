#ifndef PS_STATUS_DESCRIBABLE_H
#define PS_STATUS_DESCRIBABLE_H

#include <QString>
#include "EnumFactory.h"

// This is an 'interface', e.g. a purely abstract class with overloadable
// pure-virtual functions that the child MUST implement
class PSStatusDescribable {
public:
    // Setup an enum that supports string descriptions and string names
    #define STATUS_ENUM(Status) \
        /* Not yet initialized */ \
        Status(UNKNOWN,,"Unknown") \
        /* Automatically identified status (sequential) */ \
        Status(UNPROCESSSED,,"Unprocessed") \
        Status(RAW_PROCESSING_DONE,,"Images Ready") \
        Status(ALIGNMENT_DONE,,"Images Aligned") \
        Status(POINT_CLOUD_DONE,,"Dense Cloud Done") \
        Status(MODEL_GEN_DONE,,"Model Generated") \
        Status(TEXTURE_GEN_DONE,,"Complete") \
        /* Manually assigned status (rejection or approval) */ \
        Status(NEEDS_EXPOSURE_REDO,,"Need to redo Raw Image Exposure") \
        Status(NEEDS_ALLIGNMENT_REDO,,"Need to redo Image Alignment") \
        Status(NEEDS_POINT_CLOUD_REDO,,"Need to redo Dense Point Cloud") \
        Status(NEEDS_MODEL_GEN_REDO,,"Need to redo Model Generation") \
        Status(NEEDS_TEXTURE_GEN_REDO,,"Need to redo Texture Generation") \
        Status(NEEDS_GEOMETRY_TOUCHUP,,"Needs Geometry Touchup by Modeler") \
        Status(NEEDS_TEXTURE_TOUCHUP,,"Needs Texture Touchup by Modeler") \
        Status(FINAL_APPROVAL,,"Approved!") \

    DECLARE_ENUM(Status,STATUS_ENUM)

    // Functions for describing stages in the processing of an image set
    // These are 'abstract' pure-virtual functions that must be overridden
    virtual QString describeImageAlignPhase() = 0;
    virtual char getAlignPhaseStatus() = 0;

    virtual QString describeDenseCloudPhase() = 0;
    virtual char getDenseCloudPhaseStatus() = 0;
    virtual int getDenseCloudDepthImages() = 0;

    virtual QString describeModelGenPhase() = 0;
    virtual char getModelGenPhaseStatus() = 0;
    virtual long getModelFaceCount() = 0;
    virtual long getModelVertexCount() = 0;

    virtual QString describeTextureGenPhase() = 0;
    virtual char getTextureGenPhaseStatus() = 0;
};

#endif
