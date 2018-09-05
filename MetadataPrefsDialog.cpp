#include "MetadataPrefsDialog.h"
#include "ui_MetadataPrefsDialog.h"

MetadataPrefsDialog::MetadataPrefsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetadataPrefsDialog)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
}

MetadataPrefsDialog::~MetadataPrefsDialog()
{
    delete ui;
}
