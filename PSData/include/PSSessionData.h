#ifndef PS_SESSION_DATA_H
#define PS_SESSION_DATA_H

#include "PSStatusDescribable.h"
#include "ExposureSettings.h"

#include <QString>
#include <QDateTime>
#include <QDir>
#include <QFileInfoList>

class QSettings;

class PSProjectFileData;
class PSChunkData;
class PSModelData;

class PSSessionData : public PSStatusDescribable { // Comparable<PSSessionData>
public:
    /**
     * @author berriers
     * The data fields available for a PhotoScan session object which will
     * become the columns when it is displayed in a table
     */
    #define FIELDS_ENUM(Field) \
        /* Reduced columns */ \
        Field(F_PROJECT_ID,"ID", "Unique identifier parsed from the containing folder") \
        Field(F_PROJECT_NAME,"Name", "Short name of the object being photographed") \
        Field(F_PHOTO_DATE,"Date Shot", "The date the photographs were taken") \
        Field(F_ACTIVE_VERSION,"Version", "Which project file is currently active (if more than one found)") \
        Field(F_ACTIVE_CHUNK,"Chunk", "Which chunk is currently active (if more than one found)") \
        Field(F_IMAGE_COUNT_REAL,"Image Count", "The number of images in the project") \
        /* Extended columns */ \
        Field(F_PROJECT_STATUS,"Status", "Status of this model in the processing pipeline" ) \
        Field(F_IMAGE_ALIGN_LEVEL,"Image Align", "Details from the image alignment phase of PhotoScan") \
        Field(F_DENSE_CLOUD_LEVEL,"Dense Cloud", "Details from the dense cloud generation phase of PhotoScan") \
        Field(F_MODEL_GEN_LEVEL,"Model", "Details from the model generation phase of PhotoScan") \
        Field(F_TEXTURE_GEN_LEVEL,"Texture", "Details from the texture generation phase of PhotoScan") \
        /* Other */ \
        Field(F_PROJECT_FOLDER,"Folder", "The folder that holds the data for this project") \
        Field(F_PROJECT_NOTE,"Note", "A custom note for this project") \
        Field(F_FIELD_COUNT,"!!!!", "Do not use, for internal sizing") \

    DECLARE_ENUM(Field, FIELDS_ENUM)

    // The number of fields shown for base and extended modes
    static const char BASE_LENGTH;
    static const char EXTENDED_LENGTH;

    PSSessionData(QDir pPSProjectFolder, QSettings* settings);
    virtual ~PSSessionData();

    static void setSortBy(Field pNewSortBy);

    void examineProjects(QSettings* settings);
    void extractInfoFromFolderName(QString pFolderName);
    int compareTo(const PSSessionData* o) const;

    bool examineDirectory(QDir pDirToExamine);
	
    void autoSetStatus();
    void setCustomStatus(int statusIndex);
    QString getID();
	
    static int getNextID();
    void setID(QString mID);
    void setSpecialNotes(QString pSpecialNotes);

    void setName(QString pName);

    QFileInfo getPSProjectFile(int which) const;
    QFileInfo getPSProjectFile() const;
	
    QDir getPSProjectFolder() const;
    PSModelData* getModelData() const;
    QFileInfo getModelArchiveFile() const;

    long getRawImageCount() const;
    long getProcessedImageCount() const;
    QFileInfoList getRawFileList() const;
    bool isImageExposureKnown() const;
    const double* getWhiteBalanceMultipliers() const;
    double getBrightnessMultiplier() const;

    QDateTime getDateTakenStart() const;
    QDateTime getDateTakenFinish() const;
    bool areResultsApproved() const;
    QString getSpecialNotes() const;
    QString getName() const;
    QString getNameStrict() const;
    Status getStatus() const;

    void writeGeneralSettings(QSettings* settings) const;
    void readGeneralSettings(QSettings* settings);
    void writeExposureSettings(ExposureSettings pExpSettings, QSettings* settings) const;
    ExposureSettings readExposureSettings(QSettings* settings);

    QString toString() const;

    int getProjectCount() const;
    int getActiveProjectIndex() const;
    PSProjectFileData* getActiveProject() const;

    int getChunkCount() const;
    int getActiveChunkIndex() const;
    PSChunkData* getChunk(int index) const;
    PSChunkData* getActiveChunk() const;

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
    static Field mSortBy;
    static int mNextID;

    // Path to the project file
    QDir mPSProjectFolder;

    // Information about images and sensors
    long mImageCount_raw, mImageCount_processed;
    QFileInfoList mRawFileList;

    // General PS Project information
    QString mID;

    // Image exposure with dcraw
    ExposureSettings mExposure;
    QDateTime mDateTakenStart, mDateTakenFinish;

    // Custom data for this project
    bool mResultsApproved;
    QString mSpecialNotes;
    QString mName;
    Status mStatus;

    // The list of project files in the directory
    QFileInfoList mPSProjectFileList;
    QList<PSProjectFileData*> mPSProjectList;
    int mActiveProject;
};

#endif
