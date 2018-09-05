#include "VolumeSlider.h"

#include "QtGuiSettings.h"
#include "QtGui.h"

VolumeSlider::VolumeSlider(QWidget *parent) : QSlider(parent) {
    setRange(-50, 0);
    setOrientation(Qt::Horizontal);
    setFixedWidth(100);
    setValue(DBAPI->volume_get_db());
    //setTickPosition(QSlider::TicksBothSides);
    //setTickInterval(5);
    tickColor = this->palette().color(QPalette::Mid);
    this->setToolTip(QString::number(int(DBAPI->volume_get_db())) + QString("dB"));
    //connect(this, SIGNAL(valueChanged(int)), this, SLOT(setVolumeText()));
}


void VolumeSlider::wheelEvent(QWheelEvent *ev) {
    ev->accept();
    setValue( value() + ev->delta()/20);
}

void VolumeSlider::mousePressEvent(QMouseEvent *ev) {
    mouseMoveEvent(ev);
}

void VolumeSlider::mouseMoveEvent(QMouseEvent *ev) {
    int val = minimum() - ((float)ev->x() / this->width()) * (minimum());
    if(val<=maximum() && val>=minimum()) {
        setValue(val);
    }
}

void VolumeSlider::setValue(int value) {
    QSlider::setValue(value);
    DBAPI->volume_set_db(value);
    this->setToolTip(QString::number(int(value)) + QString("dB"));
}

void VolumeSlider::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(tickColor);
    int y1 = this->height()*3/4;
    int y2 = this->height();
    for (int i=1;i<10;i++)
        painter.drawLine(10*i, y1, 10*i, y2);
    
    QSlider::paintEvent(e);
}
