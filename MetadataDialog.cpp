#include "MetadataDialog.h"
#include "MetadataPrefsDialog.h"
#include "ui_MetadataDialog.h"
//#include <QDebug>

#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QVariant>

MetadataDialog::MetadataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetadataDialog)
{
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    connect(ui->tableViewMeta, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(Metadata_doubleClicked(const QModelIndex &)));
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

void MetadataDialog::editValueInDialog(QStandardItem *item, QString title)
{
    //qDebug() << item->data().toString();
        QDialog *editDialog = new QDialog(this);
        QVBoxLayout *layout = new QVBoxLayout;
        QDialogButtonBox *buttonBox = new QDialogButtonBox(editDialog);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        connect(buttonBox, &QDialogButtonBox::accepted, editDialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, editDialog, &QDialog::reject);
        QPlainTextEdit *editField = new QPlainTextEdit(item->data().toString());
        layout->addWidget(editField);
        layout->addWidget(buttonBox);
        editDialog->setWindowTitle(title);
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
    QModelIndex keyname_index = ui->tableViewMeta->model()->index(index.row(), 1);
    QModelIndex value_index = ui->tableViewMeta->model()->index(index.row(), 2);
    QStandardItem *keyname = sModel->itemFromIndex(keyname_index);
    QStandardItem *item = sModel->itemFromIndex(value_index);
    
    if (item->data().isValid() && static_cast<QMetaType::Type>(item->data().type()) == QMetaType::QString)
    {
        editValueInDialog(item, tr("Edit Metadata: ") + keyname->text());
    }
    else
    {
        ui->tableViewMeta->edit(value_index);
    }
        
}

void MetadataDialog::on_btnSettings_clicked()
{
    //MetadataPrefsDialog *settingsdlg = new MetadataPrefsDialog(this);
    //settingsdlg->exec();
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
