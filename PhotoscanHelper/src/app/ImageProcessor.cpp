#ifdef  USE_LIB_RAW
#include <libraw/libraw.h>
#endif

#include <QDebug>
#include <QProcessEnvironment>
#include <QProcess>

#include "ImageProcessor.h"

double ImageProcessor::mMultipliers[] = { 1.0, 0.5, 1.0, 0.5 };

QString gDCRawCommand = "/opt/local/bin/dcraw";

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
