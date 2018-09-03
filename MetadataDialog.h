#ifndef METADATADIALOG_H
#define METADATADIALOG_H

#include <QMenu>
#include <QDialog>
#include <QTableView>
#include <QStandardItem>
#include <QStandardItemModel>

namespace Ui {
class MetadataDialog;
}

class MetadataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MetadataDialog(QWidget *parent = nullptr);
    ~MetadataDialog();
    QTableView *tableViewMeta();
    QTableView *tableViewProps();
    QLineEdit *lineEditPath();
    
private slots:
    void on_btnClose_clicked();

    void on_btnSettings_clicked();
    
    void on_btnApply_clicked();
    
    void Metadata_doubleClicked(const QModelIndex &index);
    
    void metaDataMenuRequested(QPoint p);
    
    void editValueInDialog(QStandardItem *item);
    
private:
    
    Ui::MetadataDialog *ui;
};

#endif // METADATADIALOG_H
