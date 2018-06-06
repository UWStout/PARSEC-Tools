package edu.uwstout.berriers.PSHelper.Model;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Stack;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;

import org.apache.commons.io.FilenameUtils;

public class PSXMLReader {
	
	public static final XMLInputFactory XML_FACTORY = XMLInputFactory.newInstance();

	public static File checkForAndUpdatePath(XMLStreamReader reader, File currentFile) {

		// Is there a path tag we need to follow?
		if(reader.getAttributeValue(null, "path") != null) {

			// Retrieve relative path
			String newFilePath = reader.getAttributeValue(null, "path");

			// Fill in project name if needed
			if(newFilePath.contains("{projectname}")) {
				String baseFilename = FilenameUtils.getBaseName(currentFile.getPath());
				newFilePath = newFilePath.replaceAll("\\{projectname\\}", baseFilename);				
			}

			// Make into an absolute File Path object
			newFilePath = FilenameUtils.getFullPath(currentFile.getPath()) + newFilePath;			
			File newXMLFile = null;
			try { newXMLFile = new File(newFilePath).getCanonicalFile(); }
			catch(IOException e) {}
			
			return newXMLFile;
		}

		// Nothing to follow, so return the old path
		return currentFile;
	}
	
	// Examine the current reader tag for a 'path' attribute and return a new reader
	// to that file if one is found Otherwise, return the same reader.
	public static XMLStreamReader explodeTag(XMLStreamReader reader, Stack<File> currentFileStack)
			throws IOException, XMLStreamException {

		// Is there a path tag we need to follow?
		if(reader.getAttributeValue(null, "path") != null) {

			// Construct the absolute path to the new file and let it become the current file
			File newXMLFile = checkForAndUpdatePath(reader, currentFileStack.peek());
			currentFileStack.push(newXMLFile);

			// Turn into an input stream
			InputStream newXMLStream = getXMLStreamFromFile(newXMLFile);
			XMLStreamReader newReader = XML_FACTORY.createXMLStreamReader(newXMLStream);
			return newReader;
		}

		// Otherwise, stick with the passed in reader
		return reader;
	}
	
	@SuppressWarnings("resource")
	public static InputStream getXMLStreamFromFile(File pFile) throws IOException {

		// Sanity check
		if(pFile == null) { return null; }
	
		// The XML FileStream source
		InputStream lXMLFileStream = null;		
		String ext = FilenameUtils.getExtension(pFile.getName());
		
		switch(ext)
		{
			// Zip files with the XML inside them as doc.xml
			case "psz": case "zip":
			{
				// Declare variables that will be set inside of try-catch block
				ZipFile lZIPFile = null;
		
				// Try to open the PSZ file and obtain a stream for the doc.xml file
				try {
					lZIPFile = new ZipFile(pFile, ZipFile.OPEN_READ);
					ZipEntry lXMLFileEntry = lZIPFile.getEntry("doc.xml");
					if(lXMLFileEntry != null) {
						lXMLFileStream = lZIPFile.getInputStream(lXMLFileEntry);
					} else {
						throw new ZipException("could not find project XML file.");
					}
				} catch (Exception e) {
					if(lZIPFile != null) { lZIPFile.close(); }
					throw new IOException("Project File Error: general error while opening project file\n\t'" 
											+ pFile.getPath() + "'\n\t(" + e.getMessage() +").");
				}
			}
			break;
			
			// A raw PS xml file (probably to be accompanied by a .files directory)
			case "psx":
				lXMLFileStream = new FileInputStream(pFile);
			break;
		}		
		
		return lXMLFileStream;
	}
	
	// Function to read arrays encoded in XML (a la Agisoft's XML file format)
	public void readElementArray(XMLStreamReader reader, String arrayName, String elementName)
			throws XMLStreamException {
		
		// Loop over all elements till the end of the array is reached
		boolean readingElements = true;
		while(readingElements && reader.hasNext()) {

			// Move to the next tag element (START or END)
			reader.next();

			// Parse array element (preserving old reader, just in case)
			if(reader.isStartElement() && reader.getLocalName().equals(elementName))
			{
				XMLStreamReader oldReader = reader;
				processArrayElement(reader, elementName);
				reader = oldReader;
			}
			
			// Check for end of array
			else if(reader.isEndElement() && reader.getLocalName().equals(arrayName))
			{
				readingElements = false;
			}
		}
	}

	// Override this to process the specific array elements you expect to handle
	public void processArrayElement(XMLStreamReader reader, String elementName) {
		System.err.printf("Request to process array element '%s' not handled.\n", elementName);
	}
}
