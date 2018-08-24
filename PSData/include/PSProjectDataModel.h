#ifndef PS_PROJECT_DATA_MODEL_H
#define PS_PROJECT_DATA_MODEL_H

#include "psdata_global.h"

#include <QAbstractItemModel>
#include <QVector>
class PSSessionData;

class PSDATASHARED_EXPORT PSProjectDataModel : public QAbstractItemModel {

    // Colors used for indicating status
    static const QColor doneColor;
    static const QColor okColor;
    static const QColor warningColor;
    static const QColor badColor;
    static const QColor errorColor;

    // The actual data used by the table model
    QVector<PSSessionData*> mData;
	
    // Are the extended columns visible
    bool mExtendedColsEnabled;
	
    // Are the status colors enabled
    bool mShowColorForStatus;

public:
    // Make a PSProjectDataModel with the provided data
    PSProjectDataModel(QVector<PSSessionData*> data, QObject* parent);
    ~PSProjectDataModel();

    void appendNewSession(PSSessionData* pSession);

    void setExtendedColsEnabled(bool pExtendedColsEnabled);
    void setShowColorForStatus(bool pShowColorForStatus);

    int countUniqueDirs() const;
    int countDirsWithoutProjects() const;
    int countDirsWithoutImageAlign() const;
    int countDirsWithoutDenseCloud() const;
    int countDirsWithoutModels() const;

    QModelIndex parent(const QModelIndex& child) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& arg0 = QModelIndex()) const;

    // Only rows can be retrieved (aka, projects)
    PSSessionData* getDataAtIndex(int index);

    // Inform the view about the data in the table
    QVariant data(const QModelIndex& index, int role) const;
    void sort(int column, Qt::SortOrder order);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

    // Prepare string values to be written to an RFC 4180 CSV standard file
    QString encodeForCSV(QString input);
    bool outputToCSVFile(QString destFilename);

    QVector<PSSessionData*> getData();
};

#endif
