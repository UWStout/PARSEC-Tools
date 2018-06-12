#include "PSProjectDataModel.h"

#include "PSSessionData.h"

#include <QColor>
#include <QBrush>
#include <QTextStream>

// Colors used for indicating status
const QColor PSProjectDataModel::doneColor = QColor(0x99FF99);		// Light Green
const QColor PSProjectDataModel::okColor = QColor(0xFFFF00);		// Yellow
const QColor PSProjectDataModel::warningColor = QColor(0xFF9933);	// Orange
const QColor PSProjectDataModel::badColor = QColor(0xFF5555);		// Red
const QColor PSProjectDataModel::errorColor = QColor(0xCC55CC);		// Magenta

// Make a PSProjectDataModel with the provided data
PSProjectDataModel::PSProjectDataModel(QVector<PSSessionData*> data, QObject* parent) : QAbstractItemModel(parent) {
    mData = data;
    setExtendedColsEnabled(false);
    setShowColorForStatus(false);
}

PSProjectDataModel::~PSProjectDataModel() {}

void PSProjectDataModel::setExtendedColsEnabled(bool pExtendedColsEnabled) {
    emit layoutAboutToBeChanged();
    mExtendedColsEnabled = pExtendedColsEnabled;
    emit layoutChanged();
}

void PSProjectDataModel::setShowColorForStatus(bool pShowColorForStatus) {
    emit layoutAboutToBeChanged();
    mShowColorForStatus = pShowColorForStatus;
    emit layoutChanged();
}

// Our model is not hierarchical
QModelIndex PSProjectDataModel::parent(const QModelIndex& child) const {
    (void)child;
    return QModelIndex();
}

// TODO: check out http://programmingexamples.net/wiki/Qt/Delegates/ComboBoxDelegate

Qt::ItemFlags PSProjectDataModel::flags(const QModelIndex& index) const {
    if(index.column() == 2 || index.column() == 3) {
        return (Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    return Qt::ItemIsEnabled;
}

bool PSProjectDataModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    (void)index, (void)value, (void)role;
    // Remember to emit data changed signal as appropriate
    return false;
}

// To change columns edit the column headers above
int PSProjectDataModel::columnCount(const QModelIndex& parent) const {
    if(!parent.isValid()) { return 0; }

    if(mExtendedColsEnabled) { return PSSessionData::EXTENDED_LENGTH; }
    else { return PSSessionData::BASE_LENGTH; }
}

// The array list of projects holds the rows
int PSProjectDataModel::rowCount(const QModelIndex& arg0) const {
    (void)arg0;
    return mData.size();
}

// Only rows can be retrieved (aka, projects)
PSSessionData* PSProjectDataModel::getDataAtIndex(int index) {
    if(index >= 0 && index < mData.size()) { return mData[index]; }
    else { return NULL; }
}

// Inform the view about the data in the table
QVariant PSProjectDataModel::data(const QModelIndex& index, int role) const {

    // Get the column and row from the index
    int column = index.column(), row = index.row();
    if(row >= mData.size() || column >= PSSessionData::F_FIELD_COUNT) {
        return QVariant();
    }

    // Get the correct data for this row
    PSSessionData* curItem = mData[row];

    // Respond to the requested role differently for each column
    switch(role)
    {
        // Basic display data (usually a string)
        case Qt::DisplayRole: {
            switch(column) {

                // Basic info
                case PSSessionData::F_PROJECT_ID: return curItem->getID();
                case PSSessionData::F_PROJECT_NAME: return curItem->getName();
                case PSSessionData::F_PHOTO_DATE:
                    if(!curItem->getDateTakenStart().isValid()) {
                        return "Unknown";
                    } else {
                        return curItem->getDateTakenStart().toString("MM/dd/yyyy hh:mm:ss ap");
                    }

                case PSSessionData::F_ACTIVE_VERSION: return QString::asprintf("%03d", curItem->getActiveProjectIndex()+1);
                case PSSessionData::F_ACTIVE_CHUNK: return QString::asprintf("%d of %d", curItem->getActiveChunkIndex()+1, curItem->getChunkCount());
                case PSSessionData::F_IMAGE_COUNT_REAL: return QString::asprintf("%ld/%ld", curItem->getProcessedImageCount(), curItem->getRawImageCount());

                // Detailed processing info (extended info)
                case PSSessionData::F_PROJECT_STATUS: return curItem->getStatus();
                case PSSessionData::F_IMAGE_ALIGN_LEVEL: return curItem->describeImageAlignPhase();
                case PSSessionData::F_DENSE_CLOUD_LEVEL: return curItem->describeDenseCloudPhase();
                case PSSessionData::F_MODEL_GEN_LEVEL: return curItem->describeModelGenPhase();
                case PSSessionData::F_TEXTURE_GEN_LEVEL: return curItem->describeTextureGenPhase();

                // Optional columns only used for CSV file
                case PSSessionData::F_PROJECT_FOLDER: return curItem->getPSProjectFolder().dirName();
                case PSSessionData::F_PROJECT_NOTE: return curItem->getSpecialNotes();

                // Other columns should never happen
                default: return QVariant();
            }
        }

        // Tooltips come from the column or the name of the project
        case Qt::ToolTipRole: {
            if(column == PSSessionData::F_PROJECT_NAME)
                { return curItem->getPSProjectFolder().dirName(); }
            else { return curItem->getDescription((PSSessionData::Field)column); }
        }

        // Background colors indicate the quality of the project
        case Qt::BackgroundRole:
            if(mShowColorForStatus) {

                switch(column)
                {
                    // Status column
                    case PSSessionData::F_PROJECT_STATUS:
                    {
                        switch(curItem->getStatus())
                        {
                            // These indicate an error of some sort
                            case PSSessionData::PSS_UNKNOWN: case PSSessionData::PSS_UNPROCESSSED:
                                return QBrush(errorColor);

                            // This means processing is done but it's not yet approved
                            case PSSessionData::PSS_TEXTURE_GEN_DONE:
                                return QBrush(okColor);

                            // This indicates it is done and approved
                            case PSSessionData::PSS_FINAL_APPROVAL:
                                return QBrush(doneColor);

                            // These indicate something is wrong
                            case PSSessionData::PSS_NEEDS_EXPOSURE_REDO:
                            case PSSessionData::PSS_NEEDS_ALLIGNMENT_REDO:
                            case PSSessionData::PSS_NEEDS_MODEL_GEN_REDO:
                            case PSSessionData::PSS_NEEDS_POINT_CLOUD_REDO:
                            case PSSessionData::PSS_NEEDS_TEXTURE_GEN_REDO:
                            case PSSessionData::PSS_NEEDS_GEOMETRY_TOUCHUP:
                            case PSSessionData::PSS_NEEDS_TEXTURE_TOUCHUP:
                                return QBrush(badColor);

                            // All other statuses are mid-processing statuses
                            default: return QBrush(warningColor);
                        }
                    }

                    // Processing phases
                    case PSSessionData::F_IMAGE_ALIGN_LEVEL:
                        switch(curItem->getAlignPhaseStatus())
                        {
                            case 0: return QBrush(doneColor);
                            case 1: return QBrush(okColor);
                            case 2: return QBrush(warningColor);
                            case 3: return QBrush(badColor);
                            default: return QBrush(errorColor);
                        }

                    case PSSessionData::F_DENSE_CLOUD_LEVEL:
                        switch(curItem->getDenseCloudPhaseStatus())
                        {
                            case 0: return QBrush(doneColor);
                            case 1: return QBrush(okColor);
                            case 2: return QBrush(warningColor);
                            case 3: return QBrush(badColor);
                            default: return QBrush(errorColor);
                        }

                    case PSSessionData::F_MODEL_GEN_LEVEL:
                        switch(curItem->getModelGenPhaseStatus())
                        {
                            case 0: return QBrush(doneColor);
                            case 1: return QBrush(okColor);
                            case 2: return QBrush(warningColor);
                            case 3: return QBrush(badColor);
                            default: return QBrush(errorColor);
                        }

                    case PSSessionData::F_TEXTURE_GEN_LEVEL:
                        switch(curItem->getTextureGenPhaseStatus())
                        {
                            case 0: return QBrush(doneColor);
                            case 1: return QBrush(okColor);
                            case 2: return QBrush(warningColor);
                            case 3: return QBrush(badColor);
                            default: return QBrush(errorColor);
                        }

                    // Ignore all other columns
                    default: return QVariant();
                }
            } else {
                return QVariant();
            }

        // Other roles we do not support
        default: return QVariant();
    }
}

bool greaterThanPSSD(PSSessionData* A, PSSessionData* B) {
    return (A->compareTo(B) > 0);
}

void PSProjectDataModel::sort(int column, Qt::SortOrder order) {
    PSSessionData::setSortBy((PSSessionData::Field)column);
    if (order == Qt::AscendingOrder) {
        std::sort(mData.begin(), mData.end(), greaterThanPSSD);
    } else {
        std::sort(mData.rbegin(), mData.rend(), greaterThanPSSD);
    }
    emit layoutChanged();
}

QVariant PSProjectDataModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if(orientation == Qt::Horizontal) {
        if(section < 0 || section >= PSSessionData::F_FIELD_COUNT) {
            return QVariant();
        }

        switch(role) {
            case Qt::DisplayRole:
                return PSSessionData::getShortName((PSSessionData::Field)section);
            case Qt::ToolTipRole:
                return PSSessionData::getDescription((PSSessionData::Field)section);
        }
    }

    if(orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return QString::number(section + 1);
    }

    return QVariant();
}

QModelIndex PSProjectDataModel::index(int row, int column, const QModelIndex& parent) const {
    if(parent.isValid()) return QModelIndex();
    return createIndex(row, column);
}

// Prepare string values to be written to an RFC 4180 CSV standard file
QString PSProjectDataModel::encodeForCSV(QString input) {
    // Replace all double-quote characters with two double-quotes in a row
    QString encoded = input.replace("\"", "\"\"");

    // Add a double quote to the start and end of the string
    encoded = "\"" + encoded + "\"";

    return encoded;
}

bool PSProjectDataModel::outputToCSVFile(QString destFilename) {
    // Try to open the file
    QFile outFile(destFilename);
    outFile.open(QIODevice::ReadOnly);
    if (!outFile.isOpen()) {
        return false;
    }

    // Wrap with a text stream
    QTextStream fout(&outFile);

    // Output a header row with column titles
    int colCount = PSSessionData::F_FIELD_COUNT;
    for(int col=0; col<colCount; col++) {
        // Skip the project folder column
        if(col == PSSessionData::F_PROJECT_FOLDER) {
            continue;
        }

        // Retrieve the header value for this row (as a displayable string)
        QVariant headerVal = headerData(col, Qt::Horizontal, Qt::DisplayRole);

        // Output the value followed by a comma or a newline
        fout << encodeForCSV(headerVal.toString());
        if(col < colCount-1) {
            fout << ",";
        } else {
            // The CRLF line ending is dictated by the RFC 4180 CSV standard
            fout << "\r\n";
        }
    }

    // Loop over the rows and columns and grab the model data
    for(int row=0; row<rowCount(); row++) {
        for(int col=0; col<colCount; col++) {
            // Skip the project folder column
            if(col == PSSessionData::F_PROJECT_FOLDER) {
                continue;
            }

            // Retrieve the data for this row as a displayable string
            QVariant dataVal = data(index(row, col), Qt::DisplayRole);

            // Output the value followed by a comma or a newline
            fout << encodeForCSV(dataVal.toString());
            if(col < colCount-1) {
                fout << ",";
            } else {
                // The CRLF line ending is dictated by the RFC 4180 CSV standard
                fout << "\r\n";
            }
        }
    }

    // Finish the file and close it
    outFile.close();
    return true;
}

