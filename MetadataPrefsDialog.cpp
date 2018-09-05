#include "MetadataPrefsDialog.h"
#include "ui_MetadataPrefsDialog.h"

#include "QtGui.h"
#include "DBApiWrapper.h"

#include <QMessageBox>

MetadataPrefsDialog::MetadataPrefsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetadataPrefsDialog)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    dirtyBit = false;
    //qDebug() << DBAPI->conf_get_int("mp3.id3v2_version", 3);
    ui->mp3WriteID3v2->setChecked(DBAPI->conf_get_int("mp3.write_id3v2", 1));
    ui->mp3WriteID3v1->setChecked(DBAPI->conf_get_int("mp3.write_id3v1", 0));
    ui->mp3WriteAPEv2->setChecked(DBAPI->conf_get_int("mp3.write_apev2", 0));
    ui->mp3StripID3v2->setChecked(DBAPI->conf_get_int("mp3.strip_id3v2", 0));
    ui->mp3StripID3v1->setChecked(DBAPI->conf_get_int("mp3.strip_id3v1", 0));
    ui->mp3StripAPEv2->setChecked(DBAPI->conf_get_int("mp3.strip_apev2", 0));
    
    ui->apeWriteID3v2->setChecked(DBAPI->conf_get_int("ape.write_id3v2", 0));
    ui->apeWriteAPEv2->setChecked(DBAPI->conf_get_int("ape.write_apev2", 1));
    ui->apeStripID3v2->setChecked(DBAPI->conf_get_int("ape.strip_id3v2", 0));
    ui->apeStripAPEv2->setChecked(DBAPI->conf_get_int("ape.strip_apev2", 0));
    
    ui->wvWriteAPEv2->setChecked(DBAPI->conf_get_int("wv.write_apev2", 1));
    ui->wvWriteID3v1->setChecked(DBAPI->conf_get_int("wv.write_id3v1", 0));
    ui->wvStripAPEv2->setChecked(DBAPI->conf_get_int("wv.strip_apev2", 0));
    ui->wvStripID3v1->setChecked(DBAPI->conf_get_int("wv.strip_id3v1", 0));
    
    ui->comboBoxID3v2Version->addItem(tr("2.3 (Recommended)"), 3);
    ui->comboBoxID3v2Version->addItem(tr("2.4"), 4);
    int ID3v2VerIndex = ui->comboBoxID3v2Version->findData(DBAPI->conf_get_int("mp3.id3v2_version", 3));
    if ( ID3v2VerIndex != -1 )
        ui->comboBoxID3v2Version->setCurrentIndex(ID3v2VerIndex);
    else
        ui->comboBoxID3v2Version->setCurrentIndex(0);
    
    DBAPI->conf_lock();
    ui->lineEditID3v1Encoding->setText(DBAPI->conf_get_str_fast("mp3.id3v1_encoding", "iso8859-1"));
    DBAPI->conf_unlock();
    
    connect(ui->mp3WriteID3v2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->mp3WriteID3v1, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->mp3WriteAPEv2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->mp3StripID3v2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->mp3StripID3v1, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->mp3StripAPEv2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    
    connect(ui->apeWriteID3v2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->apeWriteAPEv2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->apeStripID3v2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->apeStripAPEv2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    
    connect(ui->wvWriteAPEv2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->wvWriteID3v1, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->wvStripAPEv2, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    connect(ui->wvStripID3v1, SIGNAL(toggled(bool)), this, SLOT(setDirty()));
    
    connect(ui->comboBoxID3v2Version, SIGNAL(currentIndexChanged(int)), this, SLOT(setDirty()));
    connect(ui->lineEditID3v1Encoding, SIGNAL(textChanged(const QString &)), this, SLOT(setDirty()));
    
}

MetadataPrefsDialog::~MetadataPrefsDialog()
{
    delete ui;
}

void MetadataPrefsDialog::accept() 
{
    saveConfig();
    delete this;
}

void MetadataPrefsDialog::reject() 
{
    if (dirtyBit)
    {
        QMessageBox::StandardButton result = QMessageBox::question(this, "DeaDBeeF",
                tr("Do you want to save your configurations?"),
                QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                QMessageBox::Cancel);
        if (result == QMessageBox::Cancel)
            return;
        else if (result == QMessageBox::Yes)
            saveConfig();
    }
    delete this;
}

void MetadataPrefsDialog::saveConfig()
{
    DBAPI->conf_set_int("mp3.write_id3v2", (ui->mp3WriteID3v2->checkState() != 0));
    DBAPI->conf_set_int("mp3.write_id3v1", (ui->mp3WriteID3v1->checkState() != 0));
    DBAPI->conf_set_int("mp3.write_apev2", (ui->mp3WriteAPEv2->checkState() != 0));
    DBAPI->conf_set_int("mp3.strip_id3v2", (ui->mp3StripID3v2->checkState() != 0));
    DBAPI->conf_set_int("mp3.strip_id3v1", (ui->mp3StripID3v1->checkState() != 0));
    DBAPI->conf_set_int("mp3.strip_apev2", (ui->mp3StripAPEv2->checkState() != 0));
    
    DBAPI->conf_set_int("ape.write_id3v2", (ui->apeWriteID3v2->checkState() != 0));
    DBAPI->conf_set_int("ape.write_apev2", (ui->apeWriteAPEv2->checkState() != 0));
    DBAPI->conf_set_int("ape.strip_id3v2", (ui->apeStripID3v2->checkState() != 0));
    DBAPI->conf_set_int("ape.strip_apev2", (ui->apeStripAPEv2->checkState() != 0));
    
    DBAPI->conf_set_int("wv.write_apev2", (ui->wvWriteAPEv2->checkState() != 0));
    DBAPI->conf_set_int("wv.write_id3v1", (ui->wvWriteID3v1->checkState() != 0));
    DBAPI->conf_set_int("wv.strip_apev2", (ui->wvStripAPEv2->checkState() != 0));
    DBAPI->conf_set_int("wv.strip_id3v1", (ui->wvStripID3v1->checkState() != 0));
    
    DBAPI->conf_set_int("mp3.id3v2_version", ui->comboBoxID3v2Version->itemData(ui->comboBoxID3v2Version->currentIndex()).toInt());
    
    DBAPI->conf_set_str("mp3.id3v1_encoding", qPrintable(ui->lineEditID3v1Encoding->text()));
    
}

void MetadataPrefsDialog::setDirty() 
{
    dirtyBit = true;
}
