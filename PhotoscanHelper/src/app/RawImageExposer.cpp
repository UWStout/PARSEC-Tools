//package edu.uwstout.berriers.PSHelper.app;

//import java.io.File;
//import java.io.IOException;
//import java.nio.file.Files;
//import java.nio.file.Path;
//import java.nio.file.StandardCopyOption;
//import java.util.Arrays;
//import java.util.List;

//import com.trolltech.qt.core.QFuture;
//import com.trolltech.qt.core.QtConcurrent;

//import edu.uwstout.berriers.PSHelper.Model.PSSessionData;

#include <QThread>
#include <QtConcurrent>
#include <QDir>
#include <QDebug>

#include "PSSessionData.h"
#include "ExposureSettings.h"

#include "RawImageExposer.h"
#include "ImageProcessor.h"

QFileInfo RawImageExposer::msDestination;
ExposureSettings* RawImageExposer::msSettings = nullptr;

RawImageExposer::RawImageExposer(const PSSessionData& pProject, const ExposureSettings &pSettings, const QFileInfo &pDestination) :
    mProjectName(pProject.getName()) {
    updateSettings(pSettings.makeIndependentlyConsistent());
    msDestination = pDestination;
    mRawFiles = pProject.getRawFileList();
}

RawImageExposer::RawImageExposer(const PSSessionData& pProject, const ExposureSettings &pSettings) :
    mProjectName(pProject.getName()) {
    updateSettings(pSettings.makeIndependentlyConsistent());
    msDestination = QFileInfo();
    mRawFiles = pProject.getRawFileList();
}

void RawImageExposer::updateSettings(ExposureSettings* pNewSettings) {
    delete msSettings;
    msSettings = pNewSettings;
}

QString RawImageExposer::describeProcess() {
    return "Raw Exposure - " + mProjectName;
}

QFuture<QFileInfo> RawImageExposer::runProcess() {
    return QtConcurrent::mapped(mRawFiles.begin(), mRawFiles.end(), &RawImageExposer::map);
}

QFileInfo RawImageExposer::map(QFileInfo pRawFile) {
    QFileInfo lDevelopedImage;
    try {
        lDevelopedImage = ImageProcessor::developRawImage(pRawFile, *msSettings, false);
//        QThread::msleep(100);
        //ImageProcessor::compressTIFF(lDevelopedImage);
//        QThread::msleep(100);
        //ImageProcessor::copyMetadataTags(lDevelopedImage, pRawFile);
    } catch (std::exception e) {
        qWarning() << e.what();
    }

    if(msDestination.filePath() != "" && lDevelopedImage.filePath() != "") {
        try {
            qDebug() << QFile::rename(lDevelopedImage.filePath(), msDestination.filePath() + QDir::separator() + lDevelopedImage.fileName());
            return lDevelopedImage;
        } catch (std::exception e) {
            qWarning() << "Error: could not move developed image.";
            qWarning() << e.what();
        }
    }

    return lDevelopedImage;
}

//public class RawImageExposer implements QtConcurrent.MappedFunctor<File, File>, QueueableProcess<File> {

//	final private File mDestination;
//	final private ExposureSettings mSettings;
//	final private String mProjectName;
//	private List<File> mRawFiles;

//	public RawImageExposer(PSSessionData pProject, ExposureSettings pSettings, File pDestination) throws IOException {
//		mRawFiles = Arrays.asList(pProject.getRawFileList());
//		mProjectName = pProject.getName();
//		mSettings = pSettings.makeIndependentlyConsistent();

//		if(pDestination == null || !pDestination.exists()) {
//			mDestination = null;
//		} else {
//			mDestination = pDestination.getCanonicalFile();
//		}
//	}
	
//	@Override
//	public String describeProcess() {
//		return "Raw Exposure - " + mProjectName;
//	}

//	@Override
//	public QFuture<File> runProcess() {
//		return QtConcurrent.mapped(mRawFiles, this);
//	}
	
//	@Override
//	public File map(File pRawFile) {
//		File lDevelopedImage = null;
//		try {
//			lDevelopedImage = ImageProcessorIM4J.developRawImage(pRawFile, mSettings, false);
//			Thread.sleep(100);
//			ImageProcessorIM4J.compressTIFF(lDevelopedImage);
//			Thread.sleep(100);
//			ImageProcessorIM4J.copyMetadataTags(lDevelopedImage, pRawFile);
//		} catch (Exception e) {
//			System.out.println(e.getMessage());
//			e.printStackTrace();
//			return null;
//		}
		
//		if(mDestination != null && lDevelopedImage != null) {
//			try {
//				Path lNewDevImage = Files.move(lDevelopedImage.toPath(),
//						mDestination.toPath().resolve(lDevelopedImage.getName()),
//						StandardCopyOption.REPLACE_EXISTING);
//				return lNewDevImage.toFile();
//			} catch (IOException e) {
//				System.err.println("Error: could not move developed image.");
//				System.err.println(e.getMessage());
//			}
//		}
		
//		return lDevelopedImage;
//	}
//}
