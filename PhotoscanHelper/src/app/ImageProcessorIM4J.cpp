package edu.uwstout.berriers.PSHelper.app;

import java.io.File;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Scanner;

import org.im4java.core.CommandException;
import org.im4java.core.ImageCommand;
import org.im4java.core.MogrifyCmd;
import org.im4java.core.IMOperation;
import org.im4java.core.DcrawCmd;
import org.im4java.core.DCRAWOperation;
import org.im4java.core.ExiftoolCmd;
import org.im4java.core.ETOperation;
import org.im4java.core.Operation;
import org.im4java.process.ArrayListOutputConsumer;
import org.im4java.process.ProcessStarter;

public class ImageProcessorIM4J {
	
	private static boolean USE_GRAPHICS_MAGICK = false;
	private static String searchPath;
	private static String exifToolOverrideBin;
	private static String dcrawOverrideBin;
	private static String imageMagickOverrideBin;

	public static double[] mMultipliers = { 1.0, 1.0, 1.0, 1.0 };

	// The different white-balance modes of dcraw
	public enum WhiteBalanceMode {
		WB_MODE_DEFAULT,
		WB_MODE_CAMERA,
		WB_MODE_AVERAGE,
		WB_MODE_CUSTOM
	}

	// The different brightness modes of dcraw
	public enum BrightnessMode {
		BRIGHT_MODE_AUTO_HISTOGRAM,
		BRIGHT_MODE_DISABLED,
		BRIGHT_MODE_SCALED
	}
	
	/**
	 * Prepare to use the ImageProcessorIM4J class by setting up the search paths.
	 */
	public static void setSearchPath(String newPath) {		
		searchPath = newPath;
		ProcessStarter.setGlobalSearchPath(searchPath);
	}
	
	public static void setExiftoolOverrideBin(String newExiftoolBin) {
		if(newExiftoolBin != null && newExiftoolBin.isEmpty()) {
			exifToolOverrideBin = null;
		} else {
			exifToolOverrideBin = newExiftoolBin;
		}
	}

	public static void setDcrawOverrideBin(String newDcrawBin) {		
		if(newDcrawBin != null && newDcrawBin.isEmpty()) {
			dcrawOverrideBin = null;
		} else {
			dcrawOverrideBin = newDcrawBin;
		}
	}
	
	public static void setImageMagickOverrideBin(String newImageMagickBin) {
		if(newImageMagickBin != null && newImageMagickBin.isEmpty()) {
			imageMagickOverrideBin = null;
		} else {
			imageMagickOverrideBin = newImageMagickBin;
		}
	}

	public static void setUseGraphicsMagick(boolean useGM) {
		USE_GRAPHICS_MAGICK = useGM;
	}

	public static String getSearchPath() {
		return searchPath;
	}

	public static String getExiftoolOverrideBin() {
		return exifToolOverrideBin;
	}

	public static String getDcrawOverrideBin() {
		return dcrawOverrideBin;
	}
	
	public static String getImageMagickOverrideBin() {		
		return imageMagickOverrideBin;
	}
	
	public static boolean getUseGraphicsMagick() {
		return USE_GRAPHICS_MAGICK;
	}

	public static boolean locatePrograms()
	{
		// Setup to test the three commands we use by just asking for version/usage output
		DcrawCmd dcraw = new DcrawCmd();
		if(dcrawOverrideBin != null) {
			dcraw.getCommand().clear();
			dcraw.setCommand(dcrawOverrideBin);
		}
		DCRAWOperation dcrawOptions = new DCRAWOperation();

		MogrifyCmd mogrify = new MogrifyCmd(USE_GRAPHICS_MAGICK);
		if(imageMagickOverrideBin != null) {
			mogrify.getCommand().clear();
			mogrify.setCommand(imageMagickOverrideBin);
		}
		IMOperation mogrifyOptions = new IMOperation();
		mogrifyOptions.version();

		ExiftoolCmd exiftool = new ExiftoolCmd();
		if(exifToolOverrideBin != null) {
			exiftool.getCommand().clear();
			exiftool.setCommand(exifToolOverrideBin);
		}
		ETOperation exiftoolOptions = new ETOperation();
		exiftoolOptions.ver();

		// Pack in an array for easier testing
		ImageCommand commands[] = {
				dcraw, mogrify, exiftool
		};
		
		Operation options[] = {
				dcrawOptions, mogrifyOptions, exiftoolOptions
		};
		
		// Try to run them
		ArrayListOutputConsumer nullOutput = new ArrayListOutputConsumer();
		boolean failed = false;
		for(int i=0; i<commands.length; i++)
		{
			nullOutput.clear();
			try {
				commands[i].setOutputConsumer(nullOutput);
				commands[i].run(options[i]);
			} catch (CommandException e) {
				// Was there an error code returned (dcraw returns 1 anyways)
				if(e.getReturnCode() != 0 && commands[i] != dcraw) {
					failed = true;
				}
			} catch (Exception e) {
				// Something serious is wrong
				failed = true;
			}
		}

		// Return the command status
		return !failed;
	}
		
	public static File developRawImage(File pImageFile, ExposureSettings pSettings, boolean pAsPreview) throws Exception {
		// Check the file
		if(!pImageFile.exists() || pImageFile.isDirectory()) {
			throw new Exception("Error: file given for dcraw does not exist or is not a file.");
		}
		
		// Construct the command options
		DCRAWOperation commandOptions = new DCRAWOperation();
		commandOptions.addOperation(pSettings.toIM4JOptions());

		commandOptions.verbose();
		commandOptions.createTIFF();
		if(pAsPreview) { commandOptions.halfSize(); }
		commandOptions.addImage(pImageFile.getPath());

		// Run the command
		ArrayListOutputConsumer dcrOutput = new ArrayListOutputConsumer();
		DcrawCmd dcraw = new DcrawCmd();
		if(dcrawOverrideBin != null) {
			dcraw.getCommand().clear();
			dcraw.setCommand(dcrawOverrideBin);
		}
		dcraw.setAsyncMode(false);
		dcraw.setOutputConsumer(dcrOutput);
		try {
			dcraw.run(commandOptions);			
		} catch (CommandException ce) {
			ce.printStackTrace();
			ArrayList<String> cmdError = ce.getErrorText();
			for (String line:cmdError) {
				System.err.println(line);
			}
			
			throw new Exception("Error Processing RAW: dcraw execution failed (see standard output for details).", ce);
		} catch (Exception e) {
			throw new Exception("Error Processing RAW: dcraw execution failed.", e);
		}
		
		// Parse output results
		ArrayList<String> output = dcraw.getErrorText();
		for(final String outputLine : output) {
		    if(outputLine.startsWith("multipliers")) {
		    	extractMultipliers(outputLine);
		    }
		}

		// All is well so return the newly exposed file
		int extensionIndex = pImageFile.getPath().lastIndexOf(".");
		return new File(pImageFile.getPath().substring(0, extensionIndex) + ".tiff");
	}
	
	public static boolean compressTIFF(File pImageFile) throws Exception {
		// Sanity checks
		if(!pImageFile.exists() || pImageFile.isDirectory()) {
			throw new Exception("Error: file given for ImageMagick convert does not exist or is not a file.");
		}
		
		// Construct the command line options
		IMOperation commandOptions = new IMOperation();
		commandOptions.compress("LZW");
		commandOptions.addImage(pImageFile.getPath());		
		
		// Run the command
		ArrayListOutputConsumer mogOutput = new ArrayListOutputConsumer();
		MogrifyCmd mogrify = new MogrifyCmd(USE_GRAPHICS_MAGICK);
		if(imageMagickOverrideBin != null) {
			mogrify.getCommand().clear();
			mogrify.setCommand(imageMagickOverrideBin);
		}
		
		mogrify.setOutputConsumer(mogOutput);
		mogrify.setAsyncMode(false);
		try {
			mogrify.run(commandOptions);
		} catch (CommandException ce) {
			ce.printStackTrace();
			ArrayList<String> cmdError = ce.getErrorText();
			for (String line:cmdError) {
				System.err.println(line);
			}
			
			throw new Exception("Error Compressing TIFF: mogrify execution failed (see standard output for details).", ce);
		} catch (Exception e) {
			throw new Exception("Error Compressing TIFF: mogrify execution failed.", e);
		}
		
		return true;
	}

	public static boolean copyMetadataTags(File pDestinationFile, File pSourceFile) throws Exception {
		// Sanity checks
		if(!pSourceFile.exists() || pSourceFile.isDirectory()) {
			throw new Exception("Error: source file given for exiftool does not exist or is not a file.");
		}
		
		if(!pDestinationFile.exists() || pDestinationFile.isDirectory()) {
			throw new Exception("Error: destination file given for exiftool does not exist or is not a file.");
		}

		// Construct the command options
		ETOperation commandOptions = new ETOperation();
		commandOptions.overwrite_original_in_place();
		commandOptions.tagsFromFile(pSourceFile.getPath());
		commandOptions.omitTags("orientation");
		commandOptions.addImage(pDestinationFile.getPath());

		// Run the command
		ArrayListOutputConsumer etOutput = new ArrayListOutputConsumer();
		ExiftoolCmd exiftool = new ExiftoolCmd();
		if(exifToolOverrideBin != null) {
			exiftool.getCommand().clear();
			exiftool.setCommand(exifToolOverrideBin);
		}
		
		exiftool.setAsyncMode(false);
		exiftool.setOutputConsumer(etOutput);
		try {
			exiftool.run(commandOptions);
			exiftool.setAsyncMode(false);
		} catch (CommandException ce) {
			ce.printStackTrace();
			ArrayList<String> cmdError = ce.getErrorText();
			for (String line:cmdError) {
				System.err.println(line);
			}
			
			throw new Exception("Error Copying Tags: exiftool execution failed (see standard output for details).", ce);
		} catch (Exception e) {
			throw new Exception("Error Copying Tags: exiftool execution failed.", e);
		}
		
		return true;
	}

	public static Date getDateFromMetadata(File pImageFile) throws Exception {
		// Make sure file is valid
		if(!pImageFile.exists() || pImageFile.isDirectory()) {
			throw new Exception("Error Extracing Date: File does not exist or is a directory.");
		}

		// Build command line options
		ETOperation commandOptions = new ETOperation();
		commandOptions.getTags("DateTimeOriginal");
		commandOptions.shortOutputFormat();
		commandOptions.veryShortOutputFormat();
		commandOptions.addImage(pImageFile.getPath());

		// Run the command
		ExiftoolCmd exiftool = new ExiftoolCmd();
		if(exifToolOverrideBin != null) {
			exiftool.getCommand().clear();
			exiftool.setCommand(exifToolOverrideBin);
		}
		
		ArrayListOutputConsumer etOutput = new ArrayListOutputConsumer();
		exiftool.setOutputConsumer(etOutput);
		try {
			exiftool.run(commandOptions);
		} catch (CommandException ce) {
			ce.printStackTrace();
			ArrayList<String> cmdError = ce.getErrorText();
			for (String line:cmdError) {
				System.err.println(line);
			}
			
			throw new Exception("Error Extracing Date: Exiftool execution failed (see standard output for details).", ce);
		} catch (Exception e) {
			throw new Exception("Error Extracing Date: Exiftool execution failed.", e);
		}

		// Parse the output
		Date date = new Date();
		ArrayList<String> output = etOutput.getOutput();
		if(output.size() == 1)
		{
			String dateString = output.get(0);
			Scanner dateScanner = new Scanner(dateString);
			dateScanner.useDelimiter("[:\\s]");
			
			int year, month, day, hour, min, sec;
			try {
				year = dateScanner.nextInt();
				month = dateScanner.nextInt();
				day = dateScanner.nextInt();
				hour = dateScanner.nextInt();
				min = dateScanner.nextInt();
				sec = dateScanner.nextInt();
			} catch(Exception e) {
				throw new Exception("Error Extracing Date: Output could not be parsed.", e);				
			} finally {
				dateScanner.close();
			}
			
			Calendar myCal = new GregorianCalendar(year, month, day, hour, min, sec);
			date = myCal.getTime();
		}
		else
		{
			throw new Exception("Error Extracting Date: unexpected output from Exiftool - " + output.toString());
		}
		
		return date;
	}
	
	private static void extractMultipliers(String pS) {
		String[] lValues = pS.split("\\s", 5);
		
		for(int i=0; i<4; i++) {
			mMultipliers[i] = Double.parseDouble(lValues[i+1]);
		}
	}
}
