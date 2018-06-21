#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <QFileInfo>

#include <ExposureSettings.h>

class ImageProcessor {
public:
    static QFileInfo developRawImage(QFileInfo pImageFile, ExposureSettings pSettings, bool pAsPreview);
};

#endif // IMAGEPROCESSOR_H
