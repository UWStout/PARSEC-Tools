#include "QuickPreviewDialog.h"
#include "ui_QuickPreviewDialog.h"
#include <QDebug>
#include <QFileDialog>

enum eTextSizeOption {
    TEX_SIZE_256 = 256,
    TEX_SIZE_512 = 512,
    TEX_SIZE_1024 = 1024,
    TEX_SIZE_2048 = 2048,
    TEX_SIZE_4096 = 4096
};

eTextSizeOption texSizeLookup[] = {
    TEX_SIZE_256,
    TEX_SIZE_512,
    TEX_SIZE_1024,
    TEX_SIZE_2048,
    TEX_SIZE_4096
};

QuickPreviewDialog::QuickPreviewDialog(QWidget* parent) : QDialog(parent) {
    mGUI = new Ui::QuickPreviewDialog();
    mGUI->setupUi(this);
    mGUI->toleranceSpinBox->setValue(10);
}

QuickPreviewDialog::~QuickPreviewDialog() {
    delete mGUI;
}

QFileInfo QuickPreviewDialog::getMaskDir() const {
    return QFileInfo(mGUI->pathLineEdit->text());
}

int QuickPreviewDialog::getTollerance() const {
    return mGUI->toleranceSpinBox->value();
}

int QuickPreviewDialog::getTextureSize() const {
    return texSizeLookup[mGUI->texSizeComboBox->currentIndex()];
}

void QuickPreviewDialog::on_browseButton_clicked() {
    QString newDir = QFileDialog::getExistingDirectory(parentWidget(),
                        "Select a collection directory", "");
    if(!newDir.isNull() && !newDir.isEmpty()) {
        mGUI->pathLineEdit->setText(newDir);
    }
}
