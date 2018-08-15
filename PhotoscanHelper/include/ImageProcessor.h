#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <QFileInfo>
#include <QImage>
#include <ExposureSettings.h>

class ImageProcessor {
public:
    static double mMultipliers[];
    static QFileInfo developRawImage(QFileInfo pImageFile, const ExposureSettings& pSettings, bool pAsPreview);
    static QDateTime getDateFromMetadata(QFileInfo pImageFile);

    // Post-process tiff will both compress the tiff and copy metadata from the raw file to the final tiff
    static bool postProcessTIFF(const QString& pRawImage, const QString& pOrigTIFFImage,
                                const QString& pDestination, bool pKeepOriginal = false);

    // Compress/Re-compress a tiff file optionally deleting the original
    static bool compressTIFF(const QString& pSourceImage, const QString& pDestination, bool pKeepOriginal = false);

    // Copy all EXIF metadata from the source to the destination
    static bool copyTagsFromFile(const QString& pSourceImage, const QString& pDestination);

//    static bool compressTIFF(QFileInfo pImageFile);
//    static bool copyMetadataTags(QFileInfo pDestinationFile, QFileInfo pSourceFile);
//    static void extractMultipliers(LibRaw* pCommandOptions);
};

#endif // IMAGEPROCESSOR_H
