#ifndef EXPOSURE_SETTINGS_H
#define EXPOSURE_SETTINGS_H

#include "psdata_global.h"

// Note that instances of class are deliberately made immutable
class PSDATASHARED_EXPORT ExposureSettings {
public:
    // The different white-balance modes for raw image exposure
    enum WhiteBalanceMode {
        WB_MODE_DEFAULT,
        WB_MODE_CAMERA,
        WB_MODE_AVERAGE,
        WB_MODE_CUSTOM
    };

    // The different brightness modes for raw image exposure
    enum BrightnessMode {
        BRIGHT_MODE_AUTO_HISTOGRAM,
        BRIGHT_MODE_DISABLED,
        BRIGHT_MODE_SCALED
    };

    // A default exposure object to use when a more specific one is not yet set
    const static ExposureSettings DEFAULT_EXPOSURE;

    // Various constructors
    ExposureSettings(const ExposureSettings& copy);
    ExposureSettings(WhiteBalanceMode pWBMode, BrightnessMode pBrightMode);
    ExposureSettings(WhiteBalanceMode pWBMode, const double* pWBCustom, BrightnessMode pBrightMode);
    ExposureSettings(WhiteBalanceMode pWBMode, BrightnessMode pBrightMode, double pBrightScale);
    ExposureSettings(WhiteBalanceMode pWBMode, const double* pWBCustom,
                     BrightnessMode pBrightMode, double pBrightScale);

    // Various accessors
    WhiteBalanceMode getWBMode() const { return mWBMode; }
    BrightnessMode getBrightMode() const { return mBrightMode; }
    double getBrightScale() const { return mBrightScale; }
    const double* getWBCustom() const { return mWBCustom; }

    ExposureSettings* makeIndependentlyConsistent();
	
//	ArrayList<String> argumentList();
//	DCRAWOperation toIM4JOptions();

private:
    const WhiteBalanceMode mWBMode;
    const BrightnessMode mBrightMode;

    const double mBrightScale;
    const double mWBCustom[4];
};

#endif
