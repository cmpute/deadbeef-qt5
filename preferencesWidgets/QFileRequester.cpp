#include "QFileRequester.h"

//label->setAutoFillBackground(true);

QFileRequester::QFileRequester(QString str, QWidget *aParent) :
QWidget(aParent)
{
    hbox = new QHBoxLayout(this);
    btn = new QPushButton("...", this);
    prop = new QLineEdit(str, this);
    hbox->addWidget(prop);
    hbox->addWidget(btn);
    this->setLayout(hbox);
    connect(btn, SIGNAL(clicked()), this, SLOT(openDialog()));
    connect(prop, SIGNAL(editingFinished()), this, SLOT(textChanged()));
}

void QFileRequester::setText(QString str)
{
    this->prop->setText(str);
}

QString QFileRequester::text()
{
    return this->prop->text();
}

void QFileRequester::textChanged()
{
    emit this->changed();
}

void QFileRequester::openDialog()
{
    QString fileNames;
    QFileDialog dialog(this);
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles().join(":");
        this->setText(fileNames);
        emit this->changed();
    }
}
