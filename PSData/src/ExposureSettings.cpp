#include <stdexcept>

#include "ExposureSettings.h"

const ExposureSettings ExposureSettings::DEFAULT_EXPOSURE = ExposureSettings(
            ExposureSettings::WB_MODE_DEFAULT, ExposureSettings::BRIGHT_MODE_AUTO_HISTOGRAM);

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

    if(pWBMode == WB_MODE_CUSTOM && (pWBCustom == NULL)) {
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

ExposureSettings* ExposureSettings::makeIndependentlyConsistent() {
    try {
        return new ExposureSettings(WB_MODE_CUSTOM, mWBCustom, BRIGHT_MODE_SCALED, mBrightScale);
    } catch (...) { return NULL; }
}

//	ArrayList<String> argumentList() {
//		ArrayList<String> commandFlags = new ArrayList<String>();

//		switch(mBrightMode) {
//			default:
//			case BRIGHT_MODE_AUTO_HISTOGRAM: break;

//			case BRIGHT_MODE_DISABLED: commandFlags.add("-W"); break;
//			case BRIGHT_MODE_SCALED:
//				commandFlags.add("-W");
//				commandFlags.add("-b");
//				commandFlags.add(String.format("%.6f", mBrightScale));
//			break;
//		}

//		switch(mWBMode) {
//			default:
//			case WB_MODE_DEFAULT: break;

//			case WB_MODE_CAMERA: commandFlags.add("-w"); break;
//			case WB_MODE_AVERAGE: commandFlags.add("-a"); break;
//			case WB_MODE_CUSTOM:
//				commandFlags.add("-r");
//				commandFlags.add(String.format("%.6f", mWBCustom[0]));
//				commandFlags.add(String.format("%.6f", mWBCustom[1]));
//				commandFlags.add(String.format("%.6f", mWBCustom[2]));
//				commandFlags.add(String.format("%.6f", mWBCustom[3]));
//			break;
//		}

//		return commandFlags;
//	}

//	DCRAWOperation toIM4JOptions() {
//		DCRAWOperation options = new DCRAWOperation();

//		switch(mBrightMode) {
//			default:
//			case BRIGHT_MODE_AUTO_HISTOGRAM: break;

//			case BRIGHT_MODE_DISABLED: options.fixedWhiteLevel(); break;
//			case BRIGHT_MODE_SCALED:
//				options.fixedWhiteLevel();
//				options.brightness(mBrightScale);
//			break;
//		}

//		switch(mWBMode) {
//			default:
//			case WB_MODE_DEFAULT: break;

//			case WB_MODE_CAMERA: options.useCameraWB(); break;
//			case WB_MODE_AVERAGE: options.useAverageWB(); break;
//			case WB_MODE_CUSTOM:
//				options.setWB(mWBCustom[0], mWBCustom[1], mWBCustom[2], mWBCustom[3]);
//			break;
//		}

//		return options;
//	}