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
    static const uchar BASE_LENGTH;
    static const uchar EXTENDED_LENGTH;

    explicit PSSessionData(QDir pPSProjectFolder);

    // Delete implicity copy and assignment operators
    PSSessionData(const PSSessionData&) = delete;
    PSSessionData& operator=(const PSSessionData&) = delete;

    virtual ~PSSessionData();

    static void setSortBy(Field pNewSortBy);

    void examineProject();
    void extractInfoFromFolderName(QString pFolderName);
    int compareTo(const PSSessionData* o) const;
	
    void autoSetStatus();
    void setCustomStatus(int statusIndex);
    uint64_t getID() const;
	
    static uint64_t getNextID();
    void setID(uint64_t mID);
    void addNotes(QString pNotes);
    void setName(QString pName);
    void setDateTimeCaptured(QDateTime pDateTimeCaptured);

    inline bool hasProject() const { return mPSProjectFile.filePath() != ""; }
    QFileInfo getPSProjectFile() const;
	
    QDir getSessionFolder() const;
    PSModelData* getModelData() const;
    QFileInfo getModelArchiveFile() const;

    int getRawImageCount() const;
    int getProcessedImageCount() const;
    int getMaskImageCount() const;

    const QFileInfoList& getRawFileList() const;
    const QFileInfoList& getProcessedFileList() const;
    const QFileInfoList& getMaskFileList() const;

    void writeGeneralSettings();
    void readGeneralSettings();
    void writeExposureSettings(ExposureSettings pExpSettings);
    ExposureSettings readExposureSettings() const;

    QDateTime getDateTimeCaptured() const;
    QStringList getNotes() const;
    QString getName() const;
    Status getStatus() const;

    PSProjectFileData* getProject() const;

    QString describeImageAlignPhase() const;
    uchar getAlignPhaseStatus() const;

    QString describeDenseCloudPhase() const;
    int getDenseCloudDepthImages() const;
    uchar getDenseCloudPhaseStatus() const;

    QString describeModelGenPhase() const;
    uchar getModelGenPhaseStatus() const;
    long long getModelFaceCount() const;
    long long getModelVertexCount() const;

    QString describeTextureGenPhase() const;
    uchar getTextureGenPhaseStatus() const;

    int getActiveChunkIndex() const;
    int getChunkCount() const;

private:
    void initImageDir(const QDir &pDir, const QStringList& pFilter, const QString& pFolderName);
    void initSettingsFile();

    bool examineDirectory(QDir pDirToExamine);
    void findFilesAndImages();
    void initAsExternalSession();

    void checkSynchronization(QString pProjName, QDateTime pProjTime, QDateTime pRawTime, QDateTime pProcTime, QDateTime pMaskTime);
    void updateOutOfSyncSession();
    void parseProjectXMLAndCache();

    // Session state
    bool isSynchronized;

    // INI filename to store all metadata
    QString mSettings;

    // Various folders relevant to the session
    QDir mSessionFolder, mRawFolder, mProcessedFolder, mMasksFolder;

    // Information about images
    mutable int mRawFileCount, mProcessedFileCount, mMaskFileCount;
    mutable QFileInfoList mRawFileList;
    mutable QFileInfoList mProcessedFileList;
    mutable QFileInfoList mMaskFileList;

    // General PS Project information
    uint64_t mID;
    QString mName, mDescription;
    QStringList mNotes;
    Status mStatus;

    // Chunk Variables
    int mChunkCount, mActiveChunkIndex;
    int mChunkImages, mChunkCameras;

    QString mAlignmentLevelString;
    int mAlignmentFeatureLimit;
    int mAlignmentTieLimit;

    QString mDenseCloudLevelString;
    int mDenseCloudImagesUsed;

    bool mHasMesh;
    long long mMeshFaces, mMeshVerts;

    int mTextureCount;
    int mTextureWidth, mTextureHeight;

    // Capture and exposure information
    ExposureSettings mExposure;
    QDateTime mDateTimeCaptured;

    // The list of project files in the directory
    QFileInfo mPSProjectFile;
    //TODO: Remove/comment out this line
    PSProjectFileData* mPSProject;

    // Class level static values
    static const QString sRawFolderName;
    static const QString sProcessedFolderName;
    static const QString sMasksFolderName;

    static Field mSortBy;
    static uint64_t mNextID;
};

#endif
