#ifndef METADATADIALOG_H
#define METADATADIALOG_H

#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>

#include <QMenu>
#include <QFont>
#include <QDialog>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QProgressDialog>
#include "QtGui.h"
#include "DBApiWrapper.h"

namespace Ui {
class MetadataDialog;
}

class MetadataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MetadataDialog(DB_playItem_t *it, QWidget *parent = nullptr);
    ~MetadataDialog();
    
private slots:
    void on_btnClose_clicked();

    void on_btnSettings_clicked();
    
    void on_btnApply_clicked();
    void writeMetadata();
    
    void Metadata_doubleClicked(const QModelIndex &index);
    
    void metaDataMenuRequested(QPoint p);
    
    void editValueInDialog(QStandardItem *item);
    
private:
    QStandardItemModel *modelMetaHeader;
    QStandardItemModel *modelPropsHeader;
    Ui::MetadataDialog *ui;
    
    QStringList metaDataKeys;
    QStringList metaDataCustomKeys;
    QHash<QString, QString> metaDataNames;
    
    QStringList propsKeys;
    QHash<QString, QString> propsNames;
    
    QFutureWatcher<void> metadataWatcher;
    QProgressDialog *metaUpdateProgress = nullptr;
    
    DB_playItem_t *DBItem;
};

#endif // METADATADIALOG_H
