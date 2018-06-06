package edu.uwstout.berriers.PSHelper.app;

import java.io.File;
import java.io.FileFilter;

public class ExtensionFileFilter implements FileFilter {

	private String[] mGoodExtensions;
	private boolean mAcceptDirs;

	public ExtensionFileFilter(boolean pAcceptDirs) {
		mAcceptDirs = pAcceptDirs;
		mGoodExtensions = null;
	}

	public ExtensionFileFilter(String[] extensions, boolean pAcceptDirs) {
		mAcceptDirs = pAcceptDirs;
		mGoodExtensions = extensions;
	}

	public ExtensionFileFilter(String[] extensions) {
		mAcceptDirs = false;
		mGoodExtensions = extensions;
	}

	@Override
	public boolean accept(File pathname) {
		// Ignore all files and folders that being with a '.'
		if(pathname.getName().startsWith(".")) return false;

		if(mAcceptDirs && pathname.isDirectory()) return true;
		if(mGoodExtensions == null) return false;
		
		String filename = pathname.getName();
		for(final String lExtension : mGoodExtensions) {
			if(filename.endsWith(lExtension) || filename.endsWith(lExtension.toUpperCase())) {
				return true;
			}
		}
		return false;
	}

}
