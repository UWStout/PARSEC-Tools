package edu.uwstout.berriers.PSHelper.app;

import java.util.ArrayList;

import org.im4java.core.DCRAWOperation;

import edu.uwstout.berriers.PSHelper.app.ImageProcessorIM4J.*;

public class ExposureSettings {

	final private WhiteBalanceMode mWBMode;
	final private BrightnessMode mBrightMode;

	final private double mBrightScale;
	final private double[] mWBCustom;
	
	final static public ExposureSettings DEFAULT_EXPOSURE =
			new ExposureSettings(WhiteBalanceMode.WB_MODE_DEFAULT, BrightnessMode.BRIGHT_MODE_AUTO_HISTOGRAM);
	
	public ExposureSettings(WhiteBalanceMode pWBMode, BrightnessMode pBrightMode) {
		mWBMode = pWBMode;
		mBrightMode = pBrightMode;
					
		mBrightScale = 1.0;
		mWBCustom = new double[4];
		mWBCustom[0] = mWBCustom[1] = mWBCustom[2] = mWBCustom[3] = 1.0;
		
		if(pWBMode == WhiteBalanceMode.WB_MODE_CUSTOM || pBrightMode == BrightnessMode.BRIGHT_MODE_SCALED) {
			throw new IllegalArgumentException("Error: do not use this constructor to set the custom/scaled modes.");
		}
	}

	public ExposureSettings(WhiteBalanceMode pWBMode, double[] pWBCustom, BrightnessMode pBrightMode) {
		mWBMode = pWBMode;
		mBrightMode = pBrightMode;
					
		mBrightScale = 1.0;
		mWBCustom = pWBCustom;
		
		if(pBrightMode == BrightnessMode.BRIGHT_MODE_SCALED) {
			throw new IllegalArgumentException("Error: do not use this constructor to set the scaled brightness mode.");
		}
		
		if(pWBMode == WhiteBalanceMode.WB_MODE_CUSTOM && (pWBCustom == null || pWBCustom.length != 4)) {
			throw new IllegalArgumentException("Error: white balance custom parameters are not correct.");				
		}
	}

	public ExposureSettings(WhiteBalanceMode pWBMode, BrightnessMode pBrightMode, double pBrightScale) {
		mWBMode = pWBMode;
		mBrightMode = pBrightMode;
				
		mBrightScale = pBrightScale;
		mWBCustom = new double[4];
		mWBCustom[0] = mWBCustom[1] = mWBCustom[2] = mWBCustom[3] = 1.0;
		
		if(pWBMode == WhiteBalanceMode.WB_MODE_CUSTOM) {
			throw new IllegalArgumentException("Error: do not use this constructor to set the custom white balance modes.");
		}
	}

	public ExposureSettings(WhiteBalanceMode pWBMode, double[] pWBCustom,
						    BrightnessMode pBrightMode, double pBrightScale) {
		mWBMode = pWBMode;
		mBrightMode = pBrightMode;
				
		mBrightScale = pBrightScale;
		mWBCustom = pWBCustom;

		if(pWBMode == WhiteBalanceMode.WB_MODE_CUSTOM && (pWBCustom == null || pWBCustom.length != 4)) {
			throw new IllegalArgumentException("Error: white balance custom parameters are not correct.");				
		}
	}
	
	public WhiteBalanceMode getWBMode() { return mWBMode; }
	public BrightnessMode getBrightMode() { return mBrightMode; }
	public double getBrightScale() { return mBrightScale; }
	public double[] getWBCustom() { return mWBCustom; }

	public ExposureSettings makeIndependentlyConsistent() {
		try {
			return new ExposureSettings(WhiteBalanceMode.WB_MODE_CUSTOM, mWBCustom,
						BrightnessMode.BRIGHT_MODE_SCALED, mBrightScale);
		} catch (Exception e) { return null; }
	}
	
	public ArrayList<String> argumentList() {
		ArrayList<String> commandFlags = new ArrayList<String>();
		
		switch(mBrightMode) {
			default:
			case BRIGHT_MODE_AUTO_HISTOGRAM: break;
			
			case BRIGHT_MODE_DISABLED: commandFlags.add("-W"); break;
			case BRIGHT_MODE_SCALED:
				commandFlags.add("-W");
				commandFlags.add("-b");
				commandFlags.add(String.format("%.6f", mBrightScale));
			break;
		}
		
		switch(mWBMode) {
			default:
			case WB_MODE_DEFAULT: break;
			
			case WB_MODE_CAMERA: commandFlags.add("-w"); break;
			case WB_MODE_AVERAGE: commandFlags.add("-a"); break;
			case WB_MODE_CUSTOM:
				commandFlags.add("-r");
				commandFlags.add(String.format("%.6f", mWBCustom[0]));
				commandFlags.add(String.format("%.6f", mWBCustom[1]));
				commandFlags.add(String.format("%.6f", mWBCustom[2]));
				commandFlags.add(String.format("%.6f", mWBCustom[3]));
			break;
		}
		
		return commandFlags;
	}

	public DCRAWOperation toIM4JOptions() {
		DCRAWOperation options = new DCRAWOperation();
		
		switch(mBrightMode) {
			default:
			case BRIGHT_MODE_AUTO_HISTOGRAM: break;
			
			case BRIGHT_MODE_DISABLED: options.fixedWhiteLevel(); break;
			case BRIGHT_MODE_SCALED:
				options.fixedWhiteLevel();
				options.brightness(mBrightScale);
			break;
		}
		
		switch(mWBMode) {
			default:
			case WB_MODE_DEFAULT: break;
			
			case WB_MODE_CAMERA: options.useCameraWB(); break;
			case WB_MODE_AVERAGE: options.useAverageWB(); break;
			case WB_MODE_CUSTOM:
				options.setWB(mWBCustom[0], mWBCustom[1], mWBCustom[2], mWBCustom[3]);
			break;
		}
		
		return options;
	}
}
