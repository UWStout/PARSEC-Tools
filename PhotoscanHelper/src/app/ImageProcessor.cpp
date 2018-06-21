#include <libraw/libraw.h>
#include <QTemporaryFile>

#include "ImageProcessor.h"

QFileInfo ImageProcessor::developRawImage(QFileInfo pImageFile, ExposureSettings pSettings, bool pAsPreview) {
    // Check the file
    if(!pImageFile.exists() || pImageFile.isDir()) {
        qWarning() << "Error: file given for LibRaw does not exist or is not a file";
    }

    // Construct the command options
    LibRaw* commandOptions;
    commandOptions->imgdata.params.output_tiff = 1;
    if(pAsPreview) commandOptions->imgdata.params.half_size = 1;
    commandOptions->imgdata.params.user_qual = 0;
    // pSettings.toLibRawOptions(commandOptions);

    // Open file using c_str
    commandOptions->open_file(lRawFile.filePath().toLocal8Bit().data());

    // Unpack the image
    commandOptions->unpack();

    // Run DCRAW on the image
    commandOptions->dcraw_process();


    lTempFile.open();
    qDebug() << "lTempFile location: " << lTempFile.fileName();


    // Expose to .tiff
    int lErr = commandOptions->dcraw_ppm_tiff_writer(lTempFile.fileName().toLocal8Bit().data());
    if(lErr != LIBRAW_SUCCESS) {
        qDebug() << "Failed to expose to tiff, error code = " << lErr;
    }

    return QFileInfo(lTempFile.fileName());
}
