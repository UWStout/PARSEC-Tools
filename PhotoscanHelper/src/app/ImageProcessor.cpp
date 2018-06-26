#include <libraw/libraw.h>
#include <QDebug>
#include <QFile>

#include "ImageProcessor.h"

double ImageProcessor::mMultipliers[] = {1.0, 1.0, 1.0, 1.0};

QFileInfo ImageProcessor::developRawImage(QFileInfo pImageFile, const ExposureSettings& pSettings, bool pAsPreview) {
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
    QTemporaryFile lTempFile("XXXXXX.tiff");
    lTempFile.setAutoRemove(false);
    lTempFile.open();
    qDebug() << "lTempFile location: " << lTempFile.fileName();

    // Expose to .tiff
    int lErr = commandOptions->dcraw_ppm_tiff_writer(lTempFile.fileName().toLocal8Bit().data());
    if(lErr != LIBRAW_SUCCESS) {
        qDebug() << "Failed to expose to tiff, error code = " << lErr;
    }

    // Get output results
    extractMultipliers(commandOptions);

    if(pAsPreview) {
        return QFileInfo(lTempFile.fileName());
    } else {
        QString lNewName = pImageFile.baseName() + ".tiff";
        QFile::rename(lTempFile.fileName(), lNewName);
        return QFileInfo(lNewName);
    }
}

void ImageProcessor::extractMultipliers(LibRaw* pCommandOptions) {
    mMultipliers[0] = (double)pCommandOptions->imgdata.color.cam_mul[0];
    mMultipliers[1] = (double)pCommandOptions->imgdata.color.cam_mul[1];
    mMultipliers[2] = (double)pCommandOptions->imgdata.color.cam_mul[2];
    mMultipliers[3] = (double)pCommandOptions->imgdata.color.cam_mul[3];
}
