#ifndef METADATADIALOG_H
#define METADATADIALOG_H

#include <QDialog>
#include <QTableView>

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
    QLineEdit *lineEditPath();
private slots:
    void on_btnClose_clicked();

    void on_btnSettings_clicked();

private:
    Ui::MetadataDialog *ui;
};

#endif // METADATADIALOG_H
