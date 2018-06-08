//package edu.uwstout.berriers.PSHelper.Model;

//import java.io.File;
//import java.io.FileNotFoundException;
//import java.io.PrintWriter;
//import java.text.DateFormat;
//import java.util.ArrayList;
//import java.util.Collections;
//import java.util.Date;

//import com.trolltech.qt.core.QAbstractItemModel;
//import com.trolltech.qt.core.QModelIndex;
//import com.trolltech.qt.core.QObject;
//import com.trolltech.qt.core.Qt;
//import com.trolltech.qt.core.Qt.ItemDataRole;
//import com.trolltech.qt.core.Qt.ItemFlag;
//import com.trolltech.qt.core.Qt.ItemFlags;
//import com.trolltech.qt.core.Qt.SortOrder;
//import com.trolltech.qt.gui.QBrush;
//import com.trolltech.qt.gui.QColor;

//import edu.uwstout.berriers.PSHelper.Model.PSSessionData.Columns;

//public class PSProjectDataModel extends QAbstractItemModel {

//	// Colors used for indicating status
//	private static final QColor doneColor = new QColor(0x99FF99);		// Light Green
//	private static final QColor okColor = new QColor(0xFFFF00);			// Yellow
//	private static final QColor warningColor = new QColor(0xFF9933);	// Orange
//	private static final QColor badColor = new QColor(0xFF5555);		// Red
//	private static final QColor errorColor = new QColor(0xCC55CC);		// Magenta

//	// The actual data used by the table model
//	private ArrayList<PSSessionData> mData;
	
//	// Are the extended columns visible
//	private boolean mExtendedColsEnabled;
	
//	// Are the status colors enabled
//	private boolean mShowColorForStatus;

//	// Make a PSProjectDataModel with the provided data
//	public PSProjectDataModel(ArrayList<PSSessionData> data, QObject parent) {
//		super(parent);
//		mData = data;

//		setExtendedColsEnabled(false);
//		setShowColorForStatus(false);
//	}

//	public void setExtendedColsEnabled(boolean pExtendedColsEnabled) {
//		layoutAboutToBeChanged.emit();
//		mExtendedColsEnabled = pExtendedColsEnabled;
//		layoutChanged.emit();
//	}

//	public void setShowColorForStatus(boolean pShowColorForStatus) {
//		layoutAboutToBeChanged.emit();
//		mShowColorForStatus = pShowColorForStatus;
//		layoutChanged.emit();
//	}
	
//	// Our model is not hierarchical
//	@Override
//	public QModelIndex parent(QModelIndex child) {
//		return null;
//	}

//	// TODO: check out http://programmingexamples.net/wiki/Qt/Delegates/ComboBoxDelegate
	
//	@Override
//	public ItemFlags flags(QModelIndex index) {
//		if(index.column() == 2 || index.column() == 3) {
//			return new ItemFlags(ItemFlag.ItemIsEnabled, ItemFlag.ItemIsEditable);
//		}
//	    return new ItemFlags(ItemFlag.ItemIsEnabled);
//	}
	
//	@Override
//	public boolean setData(QModelIndex index, Object value, int role) {
//		// Remember to emit data changed signal as appropriate
//		return false;
//	}
	
//	// To change columns edit the column headers above
//	@Override
//	public int columnCount(QModelIndex parent) {
//		if(parent != null) { return 0; }

//		if(mExtendedColsEnabled) { return PSSessionData.Columns.EXTENDED_LENGTH; }
//		else { return PSSessionData.Columns.BASE_LENGTH; }
//	}

//	// The array list of projects holds the rows
//	@Override
//	public int rowCount(QModelIndex arg0) {
//		if(mData == null) return 0;
//		return mData.size();
//	}

//	// Only rows can be retrieved (aka, projects)
//	public PSSessionData getDataAtIndex(int index) {
//		if(index >= 0 && index < mData.size()) { return mData.get(index); }
//		else { return null; }
//	}

//	// Inform the view about the data in the table
//	@Override
//	public Object data(QModelIndex index, int role) {
		
//		// Get the column and row from the index
//		int column = index.column(), row = index.row();
//		if(mData == null || row >= mData.size() || column >= Columns.values().length) return null;

//		// Get the correct data for this row
//		PSSessionData curItem = mData.get(row);

//		// Respond to the requested role differently for each column
//		switch(role)
//		{
//			// Basic display data (usually a string)
//			case ItemDataRole.DisplayRole: {
//				switch(PSSessionData.Columns.values()[column]) {
				
//					// Basic info
//					case PROJECT_ID: return curItem.getID();
//					case PROJECT_NAME: return curItem.getName();
//					case PHOTO_DATE:
//						// A date set to the epoch is meant to be 'unknown' or 'invalid'
//						if(curItem.getDateTakenStart().equals(new Date(0))) {
//							return "Unknown";
//						} else {
//							return DateFormat.getDateInstance().format(curItem.getDateTakenStart());
//						}
					
//					case ACTIVE_VERSION: return String.format("%03d", curItem.getActiveProjectIndex()+1);
// 					case ACTIVE_CHUNK: return String.format("%d of %d", curItem.getActiveChunkIndex()+1, curItem.getChunkCount());
//					case IMAGE_COUNT_REAL: return String.format("%d/%d", curItem.getProcessedImageCount(), curItem.getRawImageCount());

//					// Detailed processing info (extended info)
//					case PROJECT_STATUS: return curItem.getStatus();
//					case IMAGE_ALIGN_LEVEL: return curItem.describeImageAlignPhase();
//					case DENSE_CLOUD_LEVEL: return curItem.describeDenseCloudPhase();
//					case MODEL_GEN_LEVEL: return curItem.describeModelGenPhase();
//					case TEXTURE_GEN_LEVEL: return curItem.describeTextureGenPhase();
					
//					// Optional columns only used for CSV file
//					case PROJECT_FOLDER: return curItem.getPSProjectFolder();
//					case PROJECT_NOTE: return curItem.getSpecialNotes();
					
//					// Other columns should never happen
//					default: return null;
//				}
//			}

//			// Tooltips come from the column or the name of the project
//			case ItemDataRole.ToolTipRole: {
//				if(column == PSSessionData.Columns.PROJECT_NAME.ordinal())
//					{ return curItem.getPSProjectFolder().getName(); }
//				else { return PSSessionData.Columns.values()[column].tooltip; }
//			}

//			// Background colors indicate the quality of the project
//			case ItemDataRole.BackgroundRole:
//			if(mShowColorForStatus) {
				
//				switch(PSSessionData.Columns.values()[column])
//				{
//					// Status column
//					case PROJECT_STATUS:
//					{
//						switch(curItem.getStatus())
//						{
//							// These indicate an error of some sort
//							case UNKNOWN: case UNPROCESSSED:
//								return new QBrush(errorColor);

//							// This means processing is done but it's not yet approved
//							case TEXTURE_GEN_DONE:
//								return new QBrush(okColor);

//							// This indicates it is done and approved
//							case FINAL_APPROVAL:
//								return new QBrush(doneColor);
	
//							// These indicate something is wrong
//							case NEEDS_EXPOSURE_REDO: case NEEDS_ALLIGNMENT_REDO:
//							case NEEDS_MODEL_GEN_REDO: case NEEDS_POINT_CLOUD_REDO:
//							case NEEDS_TEXTURE_GEN_REDO: case NEEDS_GEOMETRY_TOUCHUP:
//							case NEEDS_TEXTURE_TOUCHUP:
//								return new QBrush(badColor);

//							// All other statuses are mid-processing statuses
//							default: return new QBrush(warningColor);
//						}
//					}

//					// Processing phases
//					case IMAGE_ALIGN_LEVEL:
//						switch(curItem.getAlignPhaseStatus())
//						{
//							case 0: return new QBrush(doneColor);
//							case 1: return new QBrush(okColor);
//							case 2: return new QBrush(warningColor);
//							case 3: return new QBrush(badColor);
//							default: return new QBrush(errorColor);
//						}
					
//					case DENSE_CLOUD_LEVEL:
//						switch(curItem.getDenseCloudPhaseStatus())
//						{
//							case 0: return new QBrush(doneColor);
//							case 1: return new QBrush(okColor);
//							case 2: return new QBrush(warningColor);
//							case 3: return new QBrush(badColor);
//							default: return new QBrush(errorColor);
//						}
					
//					case MODEL_GEN_LEVEL:
//						switch(curItem.getModelGenPhaseStatus())
//						{
//							case 0: return new QBrush(doneColor);
//							case 1: return new QBrush(okColor);
//							case 2: return new QBrush(warningColor);
//							case 3: return new QBrush(badColor);
//							default: return new QBrush(errorColor);
//						}
					
//					case TEXTURE_GEN_LEVEL:
//						switch(curItem.getTextureGenPhaseStatus())
//						{
//							case 0: return new QBrush(doneColor);
//							case 1: return new QBrush(okColor);
//							case 2: return new QBrush(warningColor);
//							case 3: return new QBrush(badColor);
//							default: return new QBrush(errorColor);
//						}
					
//					// Ignore all other columns
//					default: return null;
				
//				}
//			} else {
//				return null;
//			}
			
//			// Other roles we do not support
//			default: return null;
//		}
//	}

//	@Override
//	public void sort(int column, Qt.SortOrder order) {
//		PSSessionData.setSortBy(Columns.values()[column]);
//		mData.sort(order==SortOrder.AscendingOrder?Collections.reverseOrder():null);
//		layoutChanged.emit();
//	}
	
//	@Override
//	public Object headerData(int section, Qt.Orientation orientation, int role) {

//		if(orientation == Qt.Orientation.Horizontal) {
//			if(section < 0 || section >= Columns.values().length) return null;
			
//			switch(role) {
//				case ItemDataRole.DisplayRole: return Columns.values()[section].name;
//				case ItemDataRole.ToolTipRole: return Columns.values()[section].tooltip;
//			}
//		}

//		if(orientation == Qt.Orientation.Vertical && role == ItemDataRole.DisplayRole) {
//			return String.format("%03d", (section+1));
//		}
		
//		return null;
//	}
	
//	@Override
//	public QModelIndex index(int row, int column, QModelIndex parent) {
//		if(parent != null) return null;
//		return createIndex(row, column);
//	}

//	// Prepare string values to be written to an RFC 4180 CSV standard file
//	private String encodeForCSV(String input) {
//		// Replace all double-quote characters with two double-quotes in a row
//		String encoded = input.replaceAll("\"", "\"\"");
		
//		// Add a double quote to the start and end of the string
//		encoded = "\"" + encoded + "\"";
		
//		return encoded;
//	}
	
//	public boolean outputToCSVFile(String destFilename) {
		
//		// Try to open the file
//		File outputFile = new File(destFilename);
//		PrintWriter fout = null;
//		try {
//			fout = new PrintWriter(outputFile);
//		} catch (FileNotFoundException e) {
//			e.printStackTrace();
//			return false;
//		}

//		// Output a header row with column titles
//		int colCount = Columns.values().length;
//		for(int col=0; col<colCount; col++) {
//			// Skip the project folder column
//			if(col == Columns.PROJECT_FOLDER.ordinal()) {
//				continue;
//			}
			
//			// Retrieve the header value for this row (as a displayable string)
//			Object headerVal = headerData(col, Qt.Orientation.Horizontal, ItemDataRole.DisplayRole);

//			// Output the value followed by a comma or a newline
//			fout.print(encodeForCSV(headerVal.toString()));
//			if(col < colCount-1) {
//				fout.print(",");
//			} else {
//				// The CRLF line ending is dictated by the RFC 4180 CSV standard
//				fout.print("\r\n");
//			}
//		}
		
//		// Loop over the rows and columns and grab the model data
//		for(int row=0; row<rowCount(); row++) {
//			for(int col=0; col<colCount; col++) {
//				// Skip the project folder column
//				if(col == Columns.PROJECT_FOLDER.ordinal()) {
//					continue;
//				}

//				// Retrieve the data for this row as a displayable string
//				Object dataVal = data(index(row, col), ItemDataRole.DisplayRole);
				
//				// Output the value followed by a comma or a newline
//				fout.print(encodeForCSV(dataVal.toString()));
//				if(col < colCount-1) {
//					fout.print(",");
//				} else {
//					// The CRLF line ending is dictated by the RFC 4180 CSV standard
//					fout.print("\r\n");
//				}
//			}
//		}

//		// Finish the file and close it
//		fout.close();
//		return true;
//	}
//}
