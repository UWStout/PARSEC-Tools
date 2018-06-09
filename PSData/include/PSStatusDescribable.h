#ifndef PS_STATUS_DESCRIBABLE_H
#define PS_STATUS_DESCRIBABLE_H

#include "psdata_global.h"

#include <QString>
#include "EnumFactory.h"

// This is an 'interface', e.g. a purely abstract class with overloadable
// pure-virtual functions that the child MUST implement
class PSDATASHARED_EXPORT PSStatusDescribable {
public:
    // Setup an enum that supports string descriptions and string names
    #define STATUS_ENUM(Status) \
        /* Not yet initialized */ \
        Status(PSS_UNKNOWN,"??","Unknown") \
        /* Automatically identified status (sequential) */ \
        Status(PSS_UNPROCESSSED,"virgin","Unprocessed") \
        Status(PSS_RAW_PROCESSING_DONE,"photos","Images Ready") \
        Status(PSS_ALIGNMENT_DONE,"aligned","Images Aligned") \
        Status(PSS_POINT_CLOUD_DONE,"cloud","Dense Cloud Done") \
        Status(PSS_MODEL_GEN_DONE,"model","Model Generated") \
        Status(PSS_TEXTURE_GEN_DONE,"tex","Complete") \
        /* Manually assigned status (rejection or approval) */ \
        Status(PSS_NEEDS_EXPOSURE_REDO,"RedoExp","Need to redo Raw Image Exposure") \
        Status(PSS_NEEDS_ALLIGNMENT_REDO,"RedoAlign","Need to redo Image Alignment") \
        Status(PSS_NEEDS_POINT_CLOUD_REDO,"RedoCloud","Need to redo Dense Point Cloud") \
        Status(PSS_NEEDS_MODEL_GEN_REDO,"RedoModel","Need to redo Model Generation") \
        Status(PSS_NEEDS_TEXTURE_GEN_REDO,"RedoTex","Need to redo Texture Generation") \
        Status(PSS_NEEDS_GEOMETRY_TOUCHUP,"HandFixGeom","Needs Geometry Touchup by Modeler") \
        Status(PSS_NEEDS_TEXTURE_TOUCHUP,"HandFixTex","Needs Texture Touchup by Modeler") \
        Status(PSS_FINAL_APPROVAL,"Final","Approved!") \

    DECLARE_ENUM(Status,STATUS_ENUM)

    // Functions for describing stages in the processing of an image set
    // These are 'abstract' pure-virtual functions that must be overridden
    virtual QString describeImageAlignPhase() const = 0;
    virtual char getAlignPhaseStatus() const = 0;

    virtual QString describeDenseCloudPhase() const = 0;
    virtual char getDenseCloudPhaseStatus() const = 0;
    virtual int getDenseCloudDepthImages() const = 0;

    virtual QString describeModelGenPhase() const = 0;
    virtual char getModelGenPhaseStatus() const = 0;
    virtual long getModelFaceCount() const = 0;
    virtual long getModelVertexCount() const = 0;

    virtual QString describeTextureGenPhase() const = 0;
    virtual char getTextureGenPhaseStatus() const = 0;
};

#endif
