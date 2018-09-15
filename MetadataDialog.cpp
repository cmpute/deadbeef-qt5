#include "MetadataDialog.h"
#include "MetadataPrefsDialog.h"
#include "ui_MetadataDialog.h"
#include <QDebug>

#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QInputDialog>
#include <QVariant>
#include <QMessageBox>
#include <QClipboard>

#include "include/strlcpy.h"

MetadataDialog::MetadataDialog(DB_playItem_t *it, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetadataDialog),
    metadataWatcher(this)
{
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    connect(ui->tableViewMeta, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(Metadata_doubleClicked(const QModelIndex &)));
    ui->tableViewMeta->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableViewMeta, SIGNAL(customContextMenuRequested(QPoint)), SLOT(metaDataMenuRequested(QPoint)));
    ui->tableViewProps->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableViewProps, SIGNAL(customContextMenuRequested(QPoint)), SLOT(propsMenuRequested(QPoint)));
    
    //fill standart metadata keys
    metaDataKeys << "artist" << "title" << "album" << "year" << "genre" << "composer" \
        << "album artist" << "track" << "numtracks" << "disc" << "numdiscs" << "comment";
    metaDataNames.insert("artist", tr("Artist"));
    metaDataNames.insert("title", tr("Title"));
    metaDataNames.insert("album", tr("Album"));
    metaDataNames.insert("year", tr("Year"));
    metaDataNames.insert("genre", tr("Genre"));
    metaDataNames.insert("composer", tr("Composer"));
    metaDataNames.insert("album artist", tr("Album Artist"));
    metaDataNames.insert("track", tr("Track"));
    metaDataNames.insert("numtracks", tr("Total Tracks"));
    metaDataNames.insert("disc", tr("Disc Number"));
    metaDataNames.insert("numdiscs", tr("Total Discs"));
    metaDataNames.insert("comment", tr("Comment"));
    //fill standard properties keys
    propsKeys << ":URI" << ":TRACKNUM" << ":DURATION" << ":TAGS" << ":HAS_EMBEDDED_CUESHEET" << ":FILETYPE";
    propsNames.insert(":URI", tr("Location"));
    propsNames.insert(":TRACKNUM", tr("Subtrack Index"));
    propsNames.insert(":DURATION", tr("Duration"));
    propsNames.insert(":TAGS", tr("Tag Type(s)"));
    propsNames.insert(":HAS_EMBEDDED_CUESHEET", tr("Embedded Cuesheet"));
    propsNames.insert(":FILETYPE", tr("Codec"));
    
    DBAPI->pl_lock();
    DBItem = it;
    DB_metaInfo_t *meta = DBAPI->pl_get_metadata_head(it);
    
    metaDataCustomKeys.clear();
    QHash<QString, QString> metaDataStd;
    foreach (QString v,metaDataKeys){
        metaDataStd[v] = QString("");
    }
    QHash<QString, QString> metaDataCustom;
    
    QStringList propsCustomKeys;
    QHash<QString, QString> propsStd;
    foreach (QString v,propsKeys){
        propsStd[v] = QString("");
    }
    QHash<QString, QString> propsCustom;
    
    while (meta) {
        DB_metaInfo_t *next = meta->next;
        if (meta->key[0] != ':' && meta->key[0] != '!' && meta->key[0] != '_') {
            if (metaDataStd.contains(meta->key))
                metaDataStd[meta->key] = meta->value;
            else
            {
                metaDataCustom[meta->key] = meta->value;
                metaDataCustomKeys << meta->key;
            }
        } else if (meta->key[0] == ':') {
            if (propsStd.contains(meta->key))
                propsStd[meta->key] = meta->value;
            else
            {
                propsCustom[meta->key] = meta->value;
                propsCustomKeys << meta->key;
            }
        }
        meta = next;
    }
    DBAPI->pl_unlock();
    //TODO: metadata editor
    int j;
    char fPath[PATH_MAX];
    DBAPI->pl_format_title(it, -1, fPath, sizeof (fPath), -1, "%F");
    ui->lineEditPath->setText(fPath);
    ui->lineEditPath->setReadOnly(true);
    
    QTableView *tableViewProps = ui->tableViewProps;
    modelPropsHeader = new QStandardItemModel(0,1,this);
    modelPropsHeader->setHorizontalHeaderItem(0, new QStandardItem(tr("Key")));
    modelPropsHeader->setHorizontalHeaderItem(1, new QStandardItem(tr("Value")));
    tableViewProps->setModel(modelPropsHeader);
    //write properties to table
    for (int i=0; i<propsKeys.count(); i++)
    {
        QStandardItem *keyname = new QStandardItem(propsNames[propsKeys.at(i)]);
        keyname->setFlags(keyname->flags()^Qt::ItemIsEditable);
        QStandardItem *value = new QStandardItem(propsStd[propsKeys.at(i)]);
        value->setFlags(value->flags()^Qt::ItemIsEditable);
        modelPropsHeader->setItem(i,0,keyname);
        modelPropsHeader->setItem(i,1,value);
    }
    j = propsKeys.count();
    for (int i=0; i<propsCustomKeys.count(); i++)
    {
        QStandardItem *keyname = new QStandardItem(QString(propsCustomKeys.at(i)).remove(0,1));
        keyname->setFlags(keyname->flags()^Qt::ItemIsEditable);
        QFont keyfont = keyname->font();
        keyfont.setItalic(true);
        keyfont.setUnderline(true);
        keyname->setFont(keyfont);
        QStandardItem *value = new QStandardItem(propsCustom[propsCustomKeys.at(i)]);
        value->setFlags(value->flags()^Qt::ItemIsEditable);
        modelPropsHeader->setItem(i+j,0,keyname);
        modelPropsHeader->setItem(i+j,1,value);
    }
    //tableViewProps->resizeColumnsToContents();
    tableViewProps->setShowGrid(false);
    tableViewProps->setColumnWidth(1, 200);
    tableViewProps->resizeColumnToContents(0);
    tableViewProps->horizontalHeader()->setStretchLastSection(true);
    tableViewProps->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableViewProps->verticalHeader()->hide();
    tableViewProps->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewProps->setSelectionMode(QAbstractItemView::SingleSelection);
    
    
    QTableView *tableViewMeta = ui->tableViewMeta;
    modelMetaHeader = new QStandardItemModel(0,2,this);
    modelMetaHeader->setHorizontalHeaderItem(0, new QStandardItem(QLatin1String("")));
    modelMetaHeader->setHorizontalHeaderItem(1, new QStandardItem(tr("Key")));
    modelMetaHeader->setHorizontalHeaderItem(2, new QStandardItem(tr("Value")));
    tableViewMeta->setModel(modelMetaHeader);
    //write metadata to table
    for (int i=0; i<metaDataKeys.count(); i++)
    {
        QStandardItem *key = new QStandardItem(metaDataKeys.at(i));
        key->setFlags(key->flags()^Qt::ItemIsEditable);
        QStandardItem *keyname = new QStandardItem(metaDataNames[metaDataKeys.at(i)]);
        keyname->setFlags(keyname->flags()^Qt::ItemIsEditable);
        QStandardItem *value = new QStandardItem(metaDataStd[metaDataKeys.at(i)]);
        //value->setFlags(value->flags()^Qt::ItemIsEditable);
        modelMetaHeader->setItem(i,0,key);
        modelMetaHeader->setItem(i,1,keyname);
        modelMetaHeader->setItem(i,2,value);
    }
    j = metaDataKeys.count();
    for (int i=0; i<metaDataCustomKeys.count(); i++)
    {
        QStandardItem *key = new QStandardItem(metaDataCustomKeys.at(i));
        key->setFlags(key->flags()^Qt::ItemIsEditable);
        key->setData(true);
        QStandardItem *keyname = new QStandardItem(metaDataCustomKeys.at(i));
        keyname->setFlags(keyname->flags()^Qt::ItemIsEditable);
        QFont keyfont = keyname->font();
        keyfont.setItalic(true);
        keyfont.setUnderline(true);
        keyname->setFont(keyfont);
        QString valueStr = QString(metaDataCustom[metaDataCustomKeys.at(i)]);
        QStandardItem *value;
        if (valueStr.contains(QChar(10)))
        {
            value = new QStandardItem(valueStr.split(QChar(10))[0] + QString("(...)"));
            value->setData(valueStr);
            value->setFlags(value->flags()^Qt::ItemIsEditable);
        }
        else
            value = new QStandardItem(valueStr);
        //value->setFlags(value->flags()^Qt::ItemIsEditable);
        modelMetaHeader->setItem(i+j,0,key);
        modelMetaHeader->setItem(i+j,1,keyname);
        modelMetaHeader->setItem(i+j,2,value);
    }
    
    tableViewMeta->setShowGrid(false);
    tableViewMeta->setColumnHidden(0, true);
    resizeMetaColumns();
    tableViewMeta->horizontalHeader()->setStretchLastSection(true);
    tableViewMeta->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableViewMeta->verticalHeader()->hide();
    tableViewMeta->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewMeta->setSelectionMode(QAbstractItemView::SingleSelection);
    
    this->ui->btnApply->setEnabled(false);
    connect(modelMetaHeader, &QStandardItemModel::itemChanged, [=](){
        this->ui->btnApply->setEnabled(true);
    });
}

MetadataDialog::~MetadataDialog()
{
    delete ui;
}

void MetadataDialog::on_btnClose_clicked()
{
    if (this->ui->btnApply->isEnabled())
    {
        QMessageBox::StandardButton result = QMessageBox::question(this, "DeaDBeeF",
                tr("Track metadata modified, do you want to save?"),
                QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                QMessageBox::Cancel);
        if (result == QMessageBox::Cancel)
            return;
        else if (result == QMessageBox::Yes)
        {
            on_btnApply_clicked();
            this->accept();
        }
        else
            this->reject();
    }
    else
        this->reject();
}

void MetadataDialog::on_btnApply_clicked()
{
    //QProgressDialog *
    metaUpdateProgress = new QProgressDialog(tr("Updating metadata..."), "", 0, 0, this);
    metaUpdateProgress->setCancelButton(0);
    metaUpdateProgress->setWindowModality(Qt::WindowModal);
    //metaUpdateProgress->setAttribute(Qt::WA_DeleteOnClose);
    metaUpdateProgress->setWindowFlags(Qt::Dialog|Qt::WindowTitleHint|Qt::CustomizeWindowHint);
    connect(&metadataWatcher, &QFutureWatcher<void>::finished, [=](){
        if (metaUpdateProgress)
        {
            metaUpdateProgress->close();
            delete metaUpdateProgress;
            metaUpdateProgress = nullptr;
        }
    });
    metadataWatcher.setFuture(QtConcurrent::run(this, &MetadataDialog::writeMetadata));
    metaUpdateProgress->show();
}

void MetadataDialog::writeMetadata()
{
    this->ui->btnApply->setEnabled(false);
    int row = modelMetaHeader->rowCount();
    DBAPI->pl_lock();
    
    QHash<QString, QString> moddedKeys;
    QStringList moddedKeysList;
    for (int i = 0; i < row ; ++i)
    {
        QString key = modelMetaHeader->item(i, 0)->text();
        QStandardItem *valueItem = modelMetaHeader->item(i, 2);
        QString value("");
        if (valueItem->data().isValid() && static_cast<QMetaType::Type>(valueItem->data().type()) == QMetaType::QString)
            value = valueItem->data().toString();
        else
            value = modelMetaHeader->item(i, 2)->text();
        if (!value.isEmpty())
        {
            moddedKeys[key]=value;
            moddedKeysList << key;
        }
    }
    
    QStringList ReplacedKeys;
    DB_metaInfo_t *meta = deadbeef->pl_get_metadata_head(DBItem);
    while (meta) {
        DB_metaInfo_t *next = meta->next;
        QString Key(meta->key);
        if (Key.front() != QChar(58) && Key.front() != QChar(33) && Key.front() != QChar(95)) {
            if (!moddedKeysList.contains(Key))
                DBAPI->pl_delete_metadata(DBItem, meta);
            else
                ReplacedKeys << Key;
        }
        meta = next;
    }
    
    for (int i = 0; i < moddedKeysList.size(); ++i)
    {
        QString Key = moddedKeysList[i];
        QByteArray KeyInUtf8 = Key.toUtf8();
        const char *skey = KeyInUtf8.constData();
        QString Value = moddedKeys[Key];
        QByteArray ValueInUtf8 = Value.toUtf8();
        const char *svalue = ValueInUtf8.constData();
        if (ReplacedKeys.contains(moddedKeysList[i]))
            DBAPI->pl_delete_meta(DBItem, skey);
        DBAPI->pl_append_meta(DBItem, skey, svalue);
    }
    
    const char *dec = DBAPI->pl_find_meta_raw (DBItem, ":DECODER");
    char decoder_id[100];
    if (dec)
        strlcpy(decoder_id, dec, sizeof(decoder_id));
    
    DBAPI->pl_unlock();
    
    DBAPI->sendmessage(DB_EV_PLAYLISTCHANGED, 0, DDB_PLAYLIST_CHANGE_CONTENT, 0);
    
    if (dec && !(DBAPI->pl_get_item_flags(DBItem) & DDB_IS_SUBTRACK))
    {
        // find decoder
        DB_decoder_t *dec = NULL;
        DB_decoder_t **decoders = DBAPI->plug_get_decoder_list();
        for (int i = 0; decoders[i]; i++) {
            if (!strcmp (decoders[i]->plugin.id, decoder_id)) {
                dec = decoders[i];
                if (dec->write_metadata) {
                    dec->write_metadata(DBItem);
                }
                break;
            }
        }
    }
}

void MetadataDialog::propsMenuRequested(QPoint p)
{
    QModelIndex index = ui->tableViewProps->indexAt(p);
    QModelIndex value_index = modelPropsHeader->index(index.row(), 1);
    QStandardItem *item = modelPropsHeader->itemFromIndex(value_index);
    QAction *copyAction = new QAction(tr("Copy"), this);
    //copyAction->setShortcut(Qt::Key_Copy);
    connect(copyAction, &QAction::triggered, [=]() { 
        QApplication::clipboard()->setText(item->text());
    });
    QMenu *propsContextMenu = new QMenu(this);
    propsContextMenu->addAction(copyAction);
    propsContextMenu->move(ui->tableViewProps->viewport()->mapToGlobal(p));
    propsContextMenu->show();
}

void MetadataDialog::metaDataMenuRequested(QPoint p)
{
    QModelIndex index = ui->tableViewMeta->indexAt(p);
    QModelIndex key_index = modelMetaHeader->index(index.row(), 0);
    QModelIndex value_index = modelMetaHeader->index(index.row(), 2);
    QStandardItem *key = modelMetaHeader->itemFromIndex(key_index);
    QStandardItem *item = modelMetaHeader->itemFromIndex(value_index);
    //qDebug() << key->data().toBool();
    QAction *editInDlgAction = new QAction(tr("Edit"), this);
    connect(editInDlgAction, &QAction::triggered, [=]() { this->editValueInDialog(item); });
    QAction *deleteAction = new QAction(tr("Delete"), this);
    connect(deleteAction, &QAction::triggered, [=]() { 
        item->setText("");
        item->setData(QVariant());
        if (key->data().toBool() == true)
        {
            modelMetaHeader->removeRow(index.row());
            resizeMetaColumns();
        }
    });
    QAction *addAction = new QAction(tr("Add"), this);
    connect(addAction, &QAction::triggered, [=]() { 
        bool ok;
        QString text = QInputDialog::getText(this, tr("New metadata entry"),
                                         tr("Input key name:"), QLineEdit::Normal,
                                         QString(""), &ok);
        if (ok && !text.isEmpty())
        {
            int i = modelMetaHeader->rowCount();
            QStandardItem *key = new QStandardItem(text);
            key->setFlags(key->flags()^Qt::ItemIsEditable);
            key->setData(true);
            QStandardItem *keyname = new QStandardItem(text);
            keyname->setFlags(keyname->flags()^Qt::ItemIsEditable);
            QFont keyfont = keyname->font();
            keyfont.setItalic(true);
            keyfont.setUnderline(true);
            keyname->setFont(keyfont);
            modelMetaHeader->setItem(i,0,key);
            modelMetaHeader->setItem(i,1,keyname);
            resizeMetaColumns();
        }
    });
    
    QMenu *metaContextMenu = new QMenu(this);
    metaContextMenu->addAction(editInDlgAction);
    metaContextMenu->addAction(addAction);
    metaContextMenu->addAction(deleteAction);
    metaContextMenu->move(ui->tableViewMeta->viewport()->mapToGlobal(p));
    metaContextMenu->show();
}

void MetadataDialog::resizeMetaColumns()
{
    //this->ui->tableViewMeta->resizeColumnsToContents();
    this->ui->tableViewMeta->setColumnWidth(2, 200);
    this->ui->tableViewMeta->resizeColumnToContents(1);
    //this->ui->tableViewMeta->horizontalHeader()->setStretchLastSection(true);
}

void MetadataDialog::editValueInDialog(QStandardItem *item)
{
    QModelIndex keyname_index = modelMetaHeader->index(item->row(), 1);
    QStandardItem *keyname = modelMetaHeader->itemFromIndex(keyname_index);
    
    //qDebug() << item->data().toString();
        QDialog *editDialog = new QDialog(this);
        QVBoxLayout *layout = new QVBoxLayout;
        QDialogButtonBox *buttonBox = new QDialogButtonBox(editDialog);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        connect(buttonBox, &QDialogButtonBox::accepted, editDialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, editDialog, &QDialog::reject);
        QPlainTextEdit *editField;
        if (item->data().isValid() && static_cast<QMetaType::Type>(item->data().type()) == QMetaType::QString)
            editField = new QPlainTextEdit(item->data().toString());
        else
            editField = new QPlainTextEdit(item->text());
        layout->addWidget(editField);
        layout->addWidget(buttonBox);
        editDialog->setWindowTitle(tr("Edit Metadata: ") + keyname->text());
        editDialog->setLayout(layout);
        editDialog->resize(600, 600);
        if (editDialog->exec())
        {
            QString valueStr = editField->toPlainText();
            if (valueStr.contains(QChar(10)))
            {
                item->setText(valueStr.split(QChar(10))[0] + QString("(...)"));
                item->setData(valueStr);
            }
            else
            {
                item->setText(valueStr);
                item->setData(QVariant());
                item->setFlags(item->flags()|Qt::ItemIsEditable);
            }
        }
        delete editDialog;
}

void MetadataDialog::Metadata_doubleClicked(const QModelIndex &index)
{
    QModelIndex value_index = modelMetaHeader->index(index.row(), 2);
    QStandardItem *item = modelMetaHeader->itemFromIndex(value_index);
    
    if (item->data().isValid() && static_cast<QMetaType::Type>(item->data().type()) == QMetaType::QString)
    {
        editValueInDialog(item);
    }
    else
    {
        ui->tableViewMeta->edit(value_index);
    }
        
}

void MetadataDialog::on_btnSettings_clicked()
{
    MetadataPrefsDialog *settingsdlg = new MetadataPrefsDialog(this);
    settingsdlg->exec();
}

