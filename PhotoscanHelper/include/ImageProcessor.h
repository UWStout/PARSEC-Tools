#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <QFileInfo>
#include <ExposureSettings.h>

class ImageProcessor {
public:
    static double mMultipliers[];
    static QFileInfo developRawImage(QFileInfo pImageFile, const ExposureSettings& pSettings, bool pAsPreview);

//    static bool compressTIFF(QFileInfo pImageFile);
//    static bool copyMetadataTags(QFileInfo pDestinationFile, QFileInfo pSourceFile);
//    static QDateTime getDateFromMetadata(QFileInfo pImageFile);
//    static void extractMultipliers(LibRaw* pCommandOptions);
};

#endif // IMAGEPROCESSOR_H
