#include "MetadataDialog.h"
#include "MetadataPrefsDialog.h"
#include "ui_MetadataDialog.h"

MetadataDialog::MetadataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetadataDialog)
{
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    //ui->tableViewProps
    //ui->tableViewMeta
}

MetadataDialog::~MetadataDialog()
{
    delete ui;
}

void MetadataDialog::on_btnClose_clicked()
{
    this->reject();
    this->close();
}

void MetadataDialog::on_btnSettings_clicked()
{
    MetadataPrefsDialog *settingsdlg = new MetadataPrefsDialog(this);
    settingsdlg->exec();
}

QTableView * MetadataDialog::tableViewMeta()
{
    return ui->tableViewMeta;
}

QLineEdit * MetadataDialog::lineEditPath()
{
    return ui->lineEditPath;
}
