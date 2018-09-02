#include "MetadataPrefsDialog.h"
#include "ui_MetadataPrefsDialog.h"

MetadataPrefsDialog::MetadataPrefsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetadataPrefsDialog)
{
    ui->setupUi(this);
}

MetadataPrefsDialog::~MetadataPrefsDialog()
{
    delete ui;
}
