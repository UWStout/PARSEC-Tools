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

RawImageExposer::RawImageExposer(PSSessionData& pProject, const ExposureSettings &pSettings, const QFileInfo &pDestination) :
    mProjectName(pProject.getName()) {
    updateSettings(pSettings.makeIndependentlyConsistent());
    msDestination = pDestination;
    mRawFiles = pProject.getRawFileList();
}

RawImageExposer::RawImageExposer(PSSessionData& pProject, const ExposureSettings &pSettings) :
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
        // Develop the raw image to a TIFF image
        lDevelopedImage = ImageProcessor::developRawImage(pRawFile, *msSettings, false);

        // Compress the TIFF and copy all metadata tags from the raw file
        ImageProcessor::postProcessTIFF(pRawFile.filePath(), lDevelopedImage.filePath(),
            msDestination.filePath() + QDir::separator() + lDevelopedImage.fileName());
    } catch (std::exception e) {
        qWarning() << e.what();
    }

    return lDevelopedImage;
}
