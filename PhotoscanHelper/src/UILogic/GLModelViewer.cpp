package edu.uwstout.berriers.PSHelper.UILogic;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.text.DecimalFormat;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import com.trolltech.qt.core.QFuture;
import com.trolltech.qt.core.QFutureWatcher;
import com.trolltech.qt.core.QtConcurrent;
import com.trolltech.qt.gui.QImage;
import com.trolltech.qt.gui.QPalette;
import com.trolltech.qt.gui.QWidget;
import com.trolltech.qt.opengl.QGLFormat;

import edu.uwstout.berriers.PSHelper.Model.PLYMeshData;
import edu.uwstout.berriers.PSHelper.Model.PSModelData;
import edu.uwstout.berriers.PSHelper.Model.PSSessionData;
import edu.uwstout.berriers.PSHelper.UIForms.Ui_GLModelViewer;

import edu.uwstout.berriers.PSHelper.UILogic.QtModelViewerWidget.RenderMode;

public class GLModelViewer extends QWidget {

	private Ui_GLModelViewer mGUI;	
	
	private PSModelData mModelData;
	private String mName;

	private QImage[] mPngTextures = new QImage[4];
	private PLYMeshData mPlyMesh;
	
	private QFutureWatcher<Boolean> mDataLoading;
	
	private static final DecimalFormat DEC_FMT = new DecimalFormat("#,###");
	
	public GLModelViewer(QWidget parent) {
		super(parent);

        // Prepare Qt to use the right OpenGL context
        // Setup the QtModelViewerWidget to use OpenGL 3.2        
        QGLFormat format = new QGLFormat();
        format.setVersion(2, 1); // Due to bug in Qt 4.8, can't go higher on Mac
        format.setStencil(false);
        format.setSampleBuffers(true);
        format.setSamples(4);
        QGLFormat.setDefaultFormat(format);

		mGUI = new Ui_GLModelViewer();
		mGUI.setupUi(this);
		
		// Rebuild the combobox
		mGUI.renderModeComboBox.clear();
		for(RenderMode mode : QtModelViewerWidget.RenderMode.values()) {
			mGUI.renderModeComboBox.addItem(mode.description);
		}		
		mGUI.renderModeComboBox.insertSeparator(3);
		mGUI.renderModeComboBox.insertSeparator(7);
		mGUI.renderModeComboBox.setCurrentIndex(
				QtModelViewerWidget.RenderMode.RENDER_TEXTURE_COLOR.ordinal());
		 
		// Set color of label for selecting flat rendering color
		QPalette Pal = mGUI.flatColorLabel.palette();
		Pal.setColor(QPalette.ColorRole.Window, mGUI.modelViewer.getFlatColor());
		mGUI.flatColorLabel.setAutoFillBackground(true);
		mGUI.flatColorLabel.setPalette(Pal);
		
		mName = "PS Project";
	}

	public GLModelViewer(PSSessionData pSession, QWidget parent) {
		this(parent);
		if(pSession != null) {
			mName = pSession.getPSProjectFile().getName();		
			if(pSession.getModelData() != null) {
				loadNewModel(pSession.getModelData());				
			}
		}
	}
	
	public GLModelViewer(PSModelData pModel, QWidget parent) {
		this(parent);

		// If we have a model, load it!
		if(pModel != null) {
			loadNewModel(pModel);
		}
	}
	
	@SuppressWarnings("unused")
	private void on_renderModeComboBox_currentIndexChanged(int index) {
		// Account for separators which do affect the index
		if(index < 3) { mGUI.modelViewer.setRenderMode(index); }
		else if(index < 7) { mGUI.modelViewer.setRenderMode(index-1); }
		else { mGUI.modelViewer.setRenderMode(index-2); }
	}
	
	public void loadNewModel(PSSessionData pSession) {
		if(pSession != null) {
			mName = pSession.getPSProjectFile().getName();		
			loadNewModel(pSession.getModelData());
		} else {
			loadNewModel((PSModelData)null);
		}
	}
	
	public void loadNewModel(PSModelData pModel) {
		mModelData = pModel;
		if(mModelData == null) {
			mPngTextures = null;
			mPlyMesh = null;
			mName = "PS Project";
			
			mGUI.modelViewer.setModelData(null, null);
			mGUI.statusLabel.setText("Please load a model.");
		} else {
			mGUI.statusLabel.setText(String.format("Loading mesh for '%s' ...", mName));
	
			mDataLoading = new QFutureWatcher<Boolean>();
			mDataLoading.canceled.connect(this, "dataLoadingFinished()");
			mDataLoading.finished.connect(this, "dataLoadingFinished()");
			
			// Read the mesh and texture data in a separate thread
			try {
				Method myMethod = getClass().getMethod("loadAllData", PSModelData.class);
				QFuture<Boolean> loadingFuture = QtConcurrent.run(this, myMethod, mModelData);
				mDataLoading.setFuture(loadingFuture);
			} catch (NoSuchMethodException | SecurityException e) {
				System.err.println("IO Error occurred reading data: ");
				e.printStackTrace();
			}
		}
	}
	
	public boolean loadAllData(PSModelData pModel) throws IOException {
		
		// Read all the texture files
		mPngTextures[0] = mPngTextures[1] = mPngTextures[2] = mPngTextures[3] = null;
		try {
			for(int texID : pModel.getTextureFiles().keySet()) {
				mPngTextures[texID] = readTextureFromArchive(pModel.getArchiveFile(), pModel.getTextureFile(texID));
			}
		} catch(IOException e) {
			System.err.println(e.getMessage());
		}

		// Read the model
		System.out.printf("Reading model %s\n", pModel.getMeshFilename());
		mPlyMesh = new PLYMeshData();
		
		boolean plyRead = false;
		try {
			plyRead = mPlyMesh.readPLYFile(pModel.getArchiveFile(), pModel.getMeshFilename());
		} catch(IOException e) {
			System.err.println(e.getMessage());			
		}
		
		return plyRead;
	}
	
	private QImage readTextureFromArchive(File pArchiveFile, String pTextureFilename) throws IOException {
		
		if(pArchiveFile != null) {
			// Declare variables that will be set inside of try-catch block
			ZipFile lPSXFile = null;
			InputStream lPNGFileStream = null;
	
			// Try to open the PSZ file and obtain a stream for the doc.xml file
			try {
				lPSXFile = new ZipFile(pArchiveFile, ZipFile.OPEN_READ);
				ZipEntry lPNGFileEntry = lPSXFile.getEntry(pTextureFilename);
				if(lPNGFileEntry != null) {
					System.out.printf("Reading texture %s\n", pTextureFilename);
					lPNGFileStream = lPSXFile.getInputStream(lPNGFileEntry);
					
					// Cache the data locally first
					ByteArrayOutputStream buffer = new ByteArrayOutputStream();
					int nRead;
					byte[] data = new byte[16384];
					while ((nRead = lPNGFileStream.read(data, 0, data.length)) != -1) {
					  buffer.write(data, 0, nRead);
					}
					buffer.flush();

					// Make the image from the cached data
					QImage lTexture = new QImage();
					if(lTexture.loadFromData(buffer.toByteArray())) {
						return lTexture;
					} else {
						return null;
					}
					
				} else {
					throw new ZipException("could not find project texture file.");
				}
			} catch (ZipException e) {
				throw new IOException("PSZ Error: ZIP error while reading PSZ file ("
										+ e.getMessage() +").", e);
			} catch (IOException e) {
				throw new IOException("PSZ Error: IO error reading texture file ("
										+ e.getMessage() + ").", e);
			} catch (Exception e) {
				throw new IOException("PSZ Error: general error while reading texture file ("
										+ e.getMessage() +").", e);
			} finally {
				if(lPNGFileStream != null) {
					lPNGFileStream.close();
				}
				
				if(lPSXFile != null) {
					lPSXFile.close();
				}
			}
		}
		
		return null;
	}
	
	@SuppressWarnings("unused")
	private void dataLoadingFinished() {
		if(!mDataLoading.future().result() || mPlyMesh == null) {
			mGUI.statusLabel.setText("There was an error loading the model.");
		} else {
			
			String modelInfo = String.format("'%s' (%s vertices, %s faces, %d tex)", mName,
					DEC_FMT.format(mPlyMesh.getVertexCount()),
					DEC_FMT.format(mPlyMesh.getFaceCount()),
					mModelData.getTextureFiles().size());
			
			if(mPlyMesh.isMissingData()) {
				modelInfo += String.format(" - Missing: %s%s%s%s",
						(mPngTextures==null?"Texture image,":""),
						(mPlyMesh.withNormals()?"":"surface normals,"),
						(mPlyMesh.withColors()?"":"vertex colors,"),
						(mPlyMesh.withTexCoords()?"":"texture coords,"));
				modelInfo = modelInfo.substring(0, modelInfo.length()-2);
			}
			
			mGUI.statusLabel.setText(modelInfo);
			if(mPngTextures[0] == null) {
				mGUI.modelViewer.setModelData(null, mPlyMesh);
			} else {
				mGUI.modelViewer.setModelData(mPngTextures, mPlyMesh);
			}
		}
	}
}
