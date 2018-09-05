#include "MetadataDialog.h"
#include "MetadataPrefsDialog.h"
#include "ui_MetadataDialog.h"
#include <QDebug>

#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QInputDialog>
#include <QVariant>

MetadataDialog::MetadataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetadataDialog)
{
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    connect(ui->tableViewMeta, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(Metadata_doubleClicked(const QModelIndex &)));
    ui->tableViewMeta->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableViewMeta, SIGNAL(customContextMenuRequested(QPoint)), SLOT(metaDataMenuRequested(QPoint)));
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

void MetadataDialog::on_btnApply_clicked()
{
    
}

void MetadataDialog::metaDataMenuRequested(QPoint p)
{
    QModelIndex index = ui->tableViewMeta->indexAt(p);
    QStandardItemModel *sModel = dynamic_cast < QStandardItemModel* >(ui->tableViewMeta->model());
    QModelIndex key_index = sModel->index(index.row(), 0);
    QModelIndex value_index = sModel->index(index.row(), 2);
    QStandardItem *key = sModel->itemFromIndex(key_index);
    QStandardItem *item = sModel->itemFromIndex(value_index);
    //qDebug() << key->data().toBool();
    QAction *editInDlgAction = new QAction(tr("Edit"), this);
    connect(editInDlgAction, &QAction::triggered, [=]() { this->editValueInDialog(item); });
    QAction *deleteAction = new QAction(tr("Delete"), this);
    connect(deleteAction, &QAction::triggered, [=]() { 
        item->setText("");
        item->setData(QVariant());
        if (key->data().toBool() == true)
            sModel->removeRow(index.row());
    });
    QAction *addAction = new QAction(tr("Add"), this);
    connect(addAction, &QAction::triggered, [=]() { 
        bool ok;
        QString text = QInputDialog::getText(this, tr("New metadata entry"),
                                         tr("Input key name:"), QLineEdit::Normal,
                                         QString(""), &ok);
        if (ok && !text.isEmpty())
        {
            int i = sModel->rowCount();
            QStandardItem *key = new QStandardItem(text);
            key->setFlags(key->flags()^Qt::ItemIsEditable);
            key->setData(true);
            QStandardItem *keyname = new QStandardItem(text);
            keyname->setFlags(keyname->flags()^Qt::ItemIsEditable);
            QFont keyfont = keyname->font();
            keyfont.setItalic(true);
            keyfont.setUnderline(true);
            keyname->setFont(keyfont);
            sModel->setItem(i,0,key);
            sModel->setItem(i,1,keyname);
        }
    });
    
    QMenu *metaContextMenu = new QMenu(this);
    metaContextMenu->addAction(editInDlgAction);
    metaContextMenu->addAction(addAction);
    metaContextMenu->addAction(deleteAction);
    metaContextMenu->move(ui->tableViewMeta->viewport()->mapToGlobal(p));
    metaContextMenu->show();
}

void MetadataDialog::editValueInDialog(QStandardItem *item)
{
    QStandardItemModel *sModel = item->model();
    QModelIndex keyname_index = ui->tableViewMeta->model()->index(item->row(), 1);
    QStandardItem *keyname = sModel->itemFromIndex(keyname_index);
    
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
    QStandardItemModel *sModel = dynamic_cast < QStandardItemModel* >(ui->tableViewMeta->model());
    QModelIndex value_index = ui->tableViewMeta->model()->index(index.row(), 2);
    QStandardItem *item = sModel->itemFromIndex(value_index);
    
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

QTableView * MetadataDialog::tableViewProps()
{
    return ui->tableViewProps;
}

QTableView * MetadataDialog::tableViewMeta()
{
    return ui->tableViewMeta;
}

QLineEdit * MetadataDialog::lineEditPath()
{
    return ui->lineEditPath;
}
