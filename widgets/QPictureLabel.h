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
    double scaleFactor = 1;
    float cw_old = 0;
    float ch_old = 0;
    int scaleTo = 0;
    bool addBorder = false;
    float borderWidth = 0;
    QBrush brush;
    signed int AlignmentX = 0;
    signed int AlignmentY = 0;

public:
    QPictureLabel(QWidget *aParent) : QLabel(aParent) { }
    void setPixmap(QPixmap aPicture);
    void setImage(const QImage *aImage);
    void paintEvent(QPaintEvent *aEvent);
    void setBorder(float bWidth);
    void setBorder(float bWidth, QBrush bBrush);
    void setAlignment(int x, int y);
};
#endif // QPICTURELABEL_H
