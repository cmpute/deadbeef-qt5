#include <QPainter>
//#include <QDebug>

#include "QPictureLabel.h"

//label->setAutoFillBackground(true);

void QPictureLabel::paintEvent(QPaintEvent *aEvent)
{
    QLabel::paintEvent(aEvent);
    scaleFactor = this->devicePixelRatioF();
    _qpSource.setDevicePixelRatio(scaleFactor);
    _qpCurrent.setDevicePixelRatio(scaleFactor);
    _displayImage();
}

void QPictureLabel::setPixmap(QPixmap aPicture)
{
    _qpSource = _qpCurrent = aPicture;
    cw_old = 0;
    ch_old = 0;
    repaint();
}

void QPictureLabel::_displayImage()
{
    if (_qpSource.isNull()) //no image was set, don't draw anything
        return;
    
    float y_offset = scaleFactor + 1;
    
    float cw = width() * scaleFactor, ch = height() * scaleFactor;
    float pw = _qpCurrent.width(), ph = _qpCurrent.height();
    
    //Qt::TransformationMode::FastTransformation
    if ( (cw != cw_old || scaleTo != 1) && ((pw > cw && ph > ch && pw/cw > ph/ch) || (pw > cw && ph <= ch) || (pw < cw && ph < ch && cw/pw < ch/ph)) )
    {
        scaleTo = 1;
        _qpCurrent = _qpSource.scaledToWidth(cw, Qt::TransformationMode::SmoothTransformation);
    }
    else if ( (ch != ch_old || scaleTo != 2 ) && ((pw > cw && ph > ch && pw/cw <= ph/ch) || (ph > ch && pw <= cw) || (pw < cw && ph < ch && cw/pw > ch/ph)) )
    {
        scaleTo = 2;
        _qpCurrent = _qpSource.scaledToHeight(ch, Qt::TransformationMode::SmoothTransformation);
    }
    
    int x = (cw - _qpCurrent.width())/(2*scaleFactor);
    //int x = 0;
    //int x = (cw - _qpCurrent.width())/scaleFactor;
    
    //int y = (ch - _qpCurrent.height())/scaleFactor;
    int y = y_offset;
    //int y = _qpCurrent.height();
    
    cw_old = cw;
    ch_old = ch;
    
    QPainter paint(this);
    //paint.fillRect(x, y, _qpCurrent.width()/scaleFactor, _qpCurrent.height()/scaleFactor, QColor(255, 255, 255, 255));
    paint.drawPixmap(x, y, _qpCurrent);
}
