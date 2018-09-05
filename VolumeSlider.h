#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include <QColor>
#include <QMouseEvent>
#include <QPainter>

class VolumeSlider : public QSlider {
    Q_OBJECT

public:
    VolumeSlider(QWidget *parent = 0);

    void setValue(int value);
    
    void paintEvent(QPaintEvent *aEvent);

private:
    
    QColor tickColor;
    
protected slots:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *e);
};

#endif // VOLUMESLIDER_H
