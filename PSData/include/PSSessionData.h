#ifndef PS_SESSION_DATA_H
#define PS_SESSION_DATA_H

#include "psdata_global.h"

#include "PSStatusDescribable.h"
#include "ExposureSettings.h"

#include <QString>
#include <QDateTime>
#include <QDir>
#include <QFileInfoList>
#include <QSettings>

class PSProjectFileData;
class PSChunkData;
class PSModelData;

class PSDATASHARED_EXPORT PSSessionData : public PSStatusDescribable { // Comparable<PSSessionData>
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

    PSSessionData(QDir pPSProjectFolder);
    virtual ~PSSessionData();

    static void setSortBy(Field pNewSortBy);

    void examineProject();
    void extractInfoFromFolderName(QString pFolderName);
    int compareTo(const PSSessionData* o) const;

    bool examineDirectory(QDir pDirToExamine);

    void examineImages();
	
    void autoSetStatus();
    void setCustomStatus(int statusIndex);
    QString getID() const;
	
    static int getNextID();
    void setID(QString mID);
    void addNotes(QString pNotes);

    void setName(QString pName);

    QFileInfo getPSProjectFile() const;
	
    QDir getSessionFolder() const;
    PSModelData* getModelData() const;
    QFileInfo getModelArchiveFile() const;

    size_t getRawImageCount() const;
    size_t getProcessedImageCount() const;
    size_t getMaskImageCount() const;

    QFileInfoList getRawFileList() const;
    QFileInfoList getProcessedFileList() const;
    QFileInfoList getMaskFileList() const;

    void writeGeneralSettings();
    void readGeneralSettings();
    void writeExposureSettings(ExposureSettings pExpSettings);
    ExposureSettings readExposureSettings();

    bool isImageExposureKnown() const;
    const double* getWhiteBalanceMultipliers() const;
    double getBrightnessMultiplier() const;

    QDateTime getDateTimeCaptured() const;
    QStringList getNotes() const;
    QString getName() const;
    QString getNameStrict() const;
    Status getStatus() const;
    QString toString() const;

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
    void setImages(QDir &pDir, QFileInfoList& pImageList, const QStringList pFilter, const QString pFolderName);

    void initSettingsFile();

    static Field mSortBy;
    static int mNextID;

    // INI filename to store all metadata
    QString mSettings;

    // Various folders relevant to the session
    QDir mSessionFolder, mRawFolder, mProcessedFolder, mMasksFolder;

    // Information about images and sensors
    QFileInfoList mRawFileList, mProcessedFileList, mMaskFileList;

    // General PS Project information
    QString mID, mName, mDescription;
    QStringList mNotes;
    Status mStatus;

    // Capture and exposure information
    ExposureSettings mExposure;
    QDateTime mDateTimeCaptured;

    // The list of project files in the directory
    QFileInfo mPSProjectFile;
    PSProjectFileData* mPSProject;
};

#endif
