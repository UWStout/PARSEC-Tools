#ifdef  USE_LIB_RAW
#include <libraw/libraw.h>
#endif

#ifdef  USE_EXIV2
#include <exiv2/exiv2.hpp>
#endif

#include <QDebug>
#include <QProcessEnvironment>
#include <QProcess>
#include <QDateTime>
#include <QImageWriter>

#include "ImageProcessor.h"

double ImageProcessor::mMultipliers[] = { 1.0, 0.5, 1.0, 0.5 };

#ifdef Q_OS_WIN32
QString gDCRawCommand = "c:/program files/graphicsmagick/dcraw.exe";
#else
QString gDCRawCommand = "/opt/local/bin/dcraw";
#endif

#ifdef Q_OS_WIN32
QString gGraphicsMagickCommand = "c:/program files/graphicsmagick/gm.exe";
#else
QString gGraphicsMagickCommand = "/opt/local/bin/gm";
#endif

#ifdef Q_OS_WIN32
QString gEXIFToolCommand = "c:/program files/exiftool/exiftool.exe";
#else
QString gEXIFToolCommand = "/opt/local/bin/exiftool";
#endif

inline void extractMultipliersDCRaw(const QString& line) {
    QStringList tokens = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    ImageProcessor::mMultipliers[0] = tokens[1].toDouble();
    ImageProcessor::mMultipliers[1] = tokens[2].toDouble();
    ImageProcessor::mMultipliers[2] = tokens[3].toDouble();
    ImageProcessor::mMultipliers[3] = tokens[4].toDouble();
}

inline QFileInfo developRawImageDCRaw(QFileInfo pImageFile, const ExposureSettings& pSettings, bool pAsPreview) {
    // Check the file
    if(!pImageFile.exists() || pImageFile.isDir()) {
        qWarning("Error: file given for dcraw does not exist or is not a file [%s]", pImageFile.filePath().toLocal8Bit().data());
        return QFileInfo();
    }

    // Construct the command options
    QStringList commandArgs = pSettings.toDCRawArguments();
    commandArgs << "-v" << "-T";

    if(pAsPreview) { commandArgs << "-h"; }
    commandArgs << pImageFile.filePath();

    // Run the command and capture all output from stderr
    QByteArray lOutput;
    QProcess mDCRawProc;
    mDCRawProc.start(gDCRawCommand, commandArgs);

    while(!mDCRawProc.waitForFinished(100)) {}
    lOutput.append(mDCRawProc.readAllStandardError());
    mDCRawProc.close();

    // Parse the multipliers used from the output
    QStringList outputLines = QString(lOutput).split("\n");
    for(QString outputLine : outputLines) {
        if(outputLine.startsWith("multipliers")) {
            extractMultipliersDCRaw(outputLine);
            break;
        }
    }

    // Return the name of the tiff file
    QFileInfo lNewFile(pImageFile.path() + "/" + pImageFile.baseName() + ".tiff");
    qDebug("Tiff file is: %s", lNewFile.filePath().toLocal8Bit().data());
    return lNewFile;
}

#ifdef  USE_LIB_RAW
inline void extractMultipliersLibRaw(LibRaw* pCommandOptions) {
    ImageProcessor::mMultipliers[0] = (double)pCommandOptions->imgdata.color.cam_mul[0];
    ImageProcessor::mMultipliers[1] = (double)pCommandOptions->imgdata.color.cam_mul[1];
    ImageProcessor::mMultipliers[2] = (double)pCommandOptions->imgdata.color.cam_mul[2];
    ImageProcessor::mMultipliers[3] = (double)pCommandOptions->imgdata.color.cam_mul[3];
}

inline QFileInfo developRawImageLibRaw(QFileInfo pImageFile, const ExposureSettings& pSettings, bool pAsPreview) {
    // Check the file
    if(!pImageFile.exists() || pImageFile.isDir()) {
        qWarning() << "Error: file given for LibRaw does not exist or is not a file";
    }

    // Construct the command options
    LibRaw* commandOptions = new LibRaw();
    commandOptions->imgdata.params.output_tiff = 1;
    if(pAsPreview) {
        commandOptions->imgdata.params.half_size = 1;
        commandOptions->imgdata.params.user_qual = 0;
    } else {
        commandOptions->imgdata.params.half_size = 0;
        commandOptions->imgdata.params.user_qual = 12;
    }
    pSettings.toLibRawOptions(commandOptions);

    // Open file using c_str
    commandOptions->open_file(pImageFile.filePath().toLocal8Bit().data());

    // Unpack the image
    commandOptions->unpack();

    // Run DCRAW on the image
    commandOptions->dcraw_process();

    // Create a temporary file to store the image
    QFile lFile(pImageFile.baseName() + ".tiff");
    //qDebug() << "lTempFile location: " << lTempFile.fileName();

    // Expose to .tiff
    int lErr = commandOptions->dcraw_ppm_tiff_writer(lFile.fileName().toLocal8Bit().data());
    if(lErr != LIBRAW_SUCCESS) {
        qDebug() << "Failed to expose to tiff, error code = " << lErr;
    }

    // Get output results
    extractMultipliersLibRaw(commandOptions);
    return QFileInfo(lFile);
}
#endif

QFileInfo ImageProcessor::developRawImage(QFileInfo pImageFile, const ExposureSettings& pSettings, bool pAsPreview) {
#ifdef  USE_LIB_RAW
    return developRawImageLibRaw(pImageFile, pSettings, pAsPreview);
#else
    return developRawImageDCRaw(pImageFile, pSettings, pAsPreview);
#endif
}

QDateTime ImageProcessor::getDateFromMetadata(QFileInfo pImageFile) {
#ifdef  USE_EXIV2
    Exiv2::Image::AutoPtr lImage = Exiv2::ImageFactory::open(pImageFile.filePath().toStdString());
    if(lImage.get() == nullptr || !lImage->good()) {
        qWarning("Error opening %s to read exif data", pImageFile.fileName().toLocal8Bit().data());
        return QDateTime();
    }

    lImage->readMetadata();
    Exiv2::ExifData &exifData = lImage->exifData();
    if (exifData.empty()) {
        qWarning("No exif data found in file %s", pImageFile.fileName().toLocal8Bit().data());
        return QDateTime();
    }

    // Uncomment the chunk below to print ALL exif data found
//    qWarning("EXIF data for %s = ", pImageFile.fileName().toLocal8Bit().data());
//    Exiv2::ExifData::const_iterator end = exifData.end();
//    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
//      qDebug("%s : %s", i->key().c_str(), i->value().toString().c_str());
//    }

    // Locate and extract the original date-time
    if (exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal")) == exifData.end()) {
        qWarning("No creation date found in file %s", pImageFile.fileName().toLocal8Bit().data());
        return QDateTime();
    }

    QString lMetaDataDate = QString::fromStdString(exifData["Exif.Photo.DateTimeOriginal"].toString());
    QString lFormat = "yyyy:MM:dd hh:mm:ss";
    return QDateTime::fromString(lMetaDataDate, lFormat);

#else
    return QDateTime();
#endif
}

bool ImageProcessor::postProcessTIFF(const QString& pRawImage, const QString& pOrigTIFFImage,
                                     const QString& pDestination, bool pKeepOriginal) {
    if (!compressTIFF(pOrigTIFFImage, pDestination, pKeepOriginal)) {
        return false;
    }

    if (!copyTagsFromFile(pRawImage, pDestination)) {
        return false;
    }

    return true;
}

bool ImageProcessor::compressTIFF(const QString& pSourceImage, const QString& pDestination, bool pKeepOriginal) {
    // Construct the command options
    QStringList commandArgs;
    commandArgs << "convert" << pSourceImage << "-compress" << "lzw" << pDestination;

    // Run the command and capture all output from stderr
    QByteArray lOutput;
    QProcess mGraphicsMagickProc;
    mGraphicsMagickProc.start(gGraphicsMagickCommand, commandArgs);

    while(!mGraphicsMagickProc.waitForFinished(100)) {}
    lOutput.append(mGraphicsMagickProc.readAllStandardError());
    mGraphicsMagickProc.close();
    lOutput = lOutput.trimmed();

    // For debugging, show all output
//    qDebug("GM Output ========");
//    qDebug("%s", lOutput.data());
//    qDebug("==================");

    // Empty output means all is well, delete the original (if requested) and return true
    if (lOutput.isEmpty()) {
        if (!pKeepOriginal) { QFile::remove(pSourceImage); }
        return true;
    }

    // Don't delete original regardless of pKeepOriginal as something went wrong
    return false;
}

bool ImageProcessor::copyTagsFromFile(const QString& pSourceImage, const QString& pDestination) {
    // Construct the command options
    QStringList commandArgs;
    commandArgs << "-overwrite_original" << "-tagsfromfile" << pSourceImage << pDestination;

    // Run the command and capture all output from stderr
    QByteArray lOutput;
    QProcess mEXIFToolProc;
    mEXIFToolProc.start(gEXIFToolCommand, commandArgs);

    while(!mEXIFToolProc.waitForFinished(100)) {}
    lOutput.append(mEXIFToolProc.readAllStandardError());
    mEXIFToolProc.close();
    lOutput = lOutput.trimmed();

    // For debugging, show all output
//    qDebug("ET Output ========");
//    qDebug("%s", lOutput.data());
//    qDebug("==================");

    // Empty output means all is well
    return lOutput.isEmpty();
}
