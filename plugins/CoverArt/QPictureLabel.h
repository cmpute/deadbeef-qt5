#ifndef QPICTURELABEL_H
#define QPICTURELABEL_H

#include <QImage>
#include <QPixmap>
#include <QLabel>

class QPictureLabel : public QLabel
{
private:
    QPixmap _qpSource; //preserve the original, so multiple resize events won't break the quality
    QPixmap _qpCurrent;

    void _displayImage();

public:
    QPictureLabel(QWidget *aParent) : QLabel(aParent) { }
    void setPixmap(QPixmap aPicture);
    void paintEvent(QPaintEvent *aEvent);
};
#endif // QPICTURELABEL_H
