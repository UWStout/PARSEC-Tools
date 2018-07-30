#include <stdexcept>

#include "ExposureSettings.h"

#ifdef USE_LIB_RAW
#include "libraw/libraw.h"
#endif

const ExposureSettings ExposureSettings::DEFAULT_EXPOSURE = ExposureSettings(
            ExposureSettings::WB_MODE_DEFAULT, ExposureSettings::BRIGHT_MODE_AUTO_HISTOGRAM);

ExposureSettings::ExposureSettings(const ExposureSettings& copy)
    : mWBMode(copy.mWBMode), mBrightMode(copy.mBrightMode), mBrightScale(copy.mBrightScale),
      mWBCustom{copy.mWBCustom[0], copy.mWBCustom[1], copy.mWBCustom[2], copy.mWBCustom[3] }
{}

ExposureSettings::ExposureSettings(WhiteBalanceMode pWBMode, BrightnessMode pBrightMode)
    : mWBMode(pWBMode), mBrightMode(pBrightMode), mBrightScale(1.0), mWBCustom{1.0, 1.0, 1.0, 1.0} {
    // Consistency check
    if(pWBMode == WB_MODE_CUSTOM || pBrightMode == BRIGHT_MODE_SCALED) {
        throw new std::invalid_argument(
            "Error: do not use this constructor to set the custom/scaled modes."
        );
    }
}

ExposureSettings::ExposureSettings(WhiteBalanceMode pWBMode, const double* pWBCustom, BrightnessMode pBrightMode)
    : mWBMode(pWBMode), mBrightMode(pBrightMode), mBrightScale(1.0),
      mWBCustom{pWBCustom[0], pWBCustom[1], pWBCustom[2], pWBCustom[3]} {
    // Consistency check
    if(pBrightMode == BRIGHT_MODE_SCALED) {
        throw new std::invalid_argument(
            "Error: do not use this constructor to set the scaled brightness mode."
        );
    }

    if(pWBMode == WB_MODE_CUSTOM && (pWBCustom == nullptr)) {
        throw new std::invalid_argument(
            "Error: white balance custom parameters missing."
        );
    }
}

ExposureSettings::ExposureSettings(WhiteBalanceMode pWBMode, BrightnessMode pBrightMode, double pBrightScale)
    : mWBMode(pWBMode), mBrightMode(pBrightMode), mBrightScale(pBrightScale),
      mWBCustom{1.0, 1.0, 1.0, 1.0} {
    // Consistency check
    if(pWBMode == WB_MODE_CUSTOM) {
        throw new std::invalid_argument(
            "Error: do not use this constructor to set the custom white balance modes."
        );
    }
}

ExposureSettings::ExposureSettings(WhiteBalanceMode pWBMode, const double* pWBCustom,
                                   BrightnessMode pBrightMode, double pBrightScale)
    : mWBMode(pWBMode), mBrightMode(pBrightMode), mBrightScale(pBrightScale),
      mWBCustom{pWBCustom[0], pWBCustom[1], pWBCustom[2], pWBCustom[3]} {
}

ExposureSettings* ExposureSettings::makeIndependentlyConsistent() const {
    return new ExposureSettings(WB_MODE_CUSTOM, mWBCustom, BRIGHT_MODE_SCALED, mBrightScale);
}

QStringList ExposureSettings::toDCRawArguments() const {
    QStringList args;

    switch(mBrightMode) {
        default:
        case BRIGHT_MODE_AUTO_HISTOGRAM: break;

        case BRIGHT_MODE_DISABLED: args << "-W"; break;
        case BRIGHT_MODE_SCALED: args << "-W" << "-b" << QString::number(mBrightScale); break;
    }

    switch(mWBMode) {
        default:
        case WB_MODE_DEFAULT: break;

        case WB_MODE_CAMERA: args << "-w"; break;
        case WB_MODE_AVERAGE: args << "-a"; break;
        case WB_MODE_CUSTOM:
            args << "-r" << QString::number(mWBCustom[0])
                 << QString::number(mWBCustom[1])
                 << QString::number(mWBCustom[2])
                 << QString::number(mWBCustom[3]);
        break;
    }

    return args;
}

#ifdef USE_LIB_RAW
void ExposureSettings::toLibRawOptions(LibRaw* pCommandOptions) const {
    switch(mBrightMode) {
        default:
        case BRIGHT_MODE_AUTO_HISTOGRAM: break;

        case BRIGHT_MODE_DISABLED:
            pCommandOptions->imgdata.params.no_auto_bright = 1;
            break;
        case BRIGHT_MODE_SCALED:
            pCommandOptions->imgdata.params.no_auto_bright = 1;
            pCommandOptions->imgdata.params.bright = (float)mBrightScale;
            break;
    }

    switch(mWBMode) {
        default:
        case WB_MODE_DEFAULT: break;

        case WB_MODE_CAMERA:
            pCommandOptions->imgdata.params.use_camera_wb = 1;
            break;
        case WB_MODE_AVERAGE:
            pCommandOptions->imgdata.params.use_auto_wb = 1;
            break;
        case WB_MODE_CUSTOM:
            pCommandOptions->imgdata.params.user_mul[0] = (float)mWBCustom[0];
            pCommandOptions->imgdata.params.user_mul[1] = (float)mWBCustom[1];
            pCommandOptions->imgdata.params.user_mul[2] = (float)mWBCustom[2];
            pCommandOptions->imgdata.params.user_mul[3] = (float)mWBCustom[3];
            break;

    }
}
#endif
