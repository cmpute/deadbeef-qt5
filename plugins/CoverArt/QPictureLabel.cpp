#include <QPainter>
//#include <QDebug>

#include "QPictureLabel.h"

//label->setAutoFillBackground(true);

void QPictureLabel::paintEvent(QPaintEvent *aEvent)
{
    QLabel::paintEvent(aEvent);
    scaleFactor = this->devicePixelRatioF();
    //borderWidth = scaleFactor;
    _qpSource.setDevicePixelRatio(scaleFactor);
    _qpCurrent.setDevicePixelRatio(scaleFactor);
    _displayImage();
}

void QPictureLabel::setAlignment(int x, int y)
{
    AlignmentX = x;
    AlignmentY = y;
}

void QPictureLabel::setPixmap(QPixmap aPicture)
{
    _qpSource = _qpCurrent = aPicture;
    cw_old = 0;
    ch_old = 0;
    repaint();
}

void QPictureLabel::setBorder(float bWidth)
{
    borderWidth = bWidth;
    brush = QBrush(Qt::lightGray);
}


void QPictureLabel::setBorder(float bWidth, QBrush bBrush)
{
    borderWidth = bWidth;
    brush = bBrush;
}

void QPictureLabel::_displayImage()
{
    if (_qpSource.isNull()) //no image was set, don't draw anything
    {
        scaleTo = 0;
        return;
    }
    
    float cw = width() * scaleFactor, ch = height() * scaleFactor;
    float pw = _qpCurrent.width(), ph = _qpCurrent.height();
    
    //Qt::TransformationMode::FastTransformation
    bool Repainted = false;
    if ( (cw != cw_old || scaleTo != 1) && ((pw > cw && ph > ch && pw/cw > ph/ch) || (pw > cw && ph <= ch) || (pw < cw && ph < ch && cw/pw < ch/ph)) )
    {
        scaleTo = 1;
        _qpCurrent = _qpSource.scaledToWidth(cw, Qt::TransformationMode::SmoothTransformation);
        Repainted = true;
    }
    else if ( (ch != ch_old || scaleTo != 2 ) && ((pw > cw && ph > ch && pw/cw <= ph/ch) || (ph > ch && pw <= cw) || (pw < cw && ph < ch && cw/pw > ch/ph)) )
    {
        scaleTo = 2;
        _qpCurrent = _qpSource.scaledToHeight(ch, Qt::TransformationMode::SmoothTransformation);
        Repainted = true;
    }
    
    int x;
    if (AlignmentX > 0)
        x = (cw - _qpCurrent.width())/scaleFactor;
    else if (AlignmentX < 0)
        x = 0;
    else
        x = (cw - _qpCurrent.width())/(2*scaleFactor);
    
    int y;
    if (AlignmentY > 0)
        y = (ch - _qpCurrent.height())/scaleFactor;
    else if (AlignmentY < 0)
        y = 0;
    else
        y = (ch - _qpCurrent.height())/(2*scaleFactor);
    
    
    cw_old = cw;
    ch_old = ch;
    
    
    if (Repainted && (borderWidth >= 1))
    {
        QPainter paintPixmap(&_qpCurrent);
        paintPixmap.setPen(QPen(brush, borderWidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
        //paintPixmap.drawRect(0, 0, _qpCurrent.width()/scaleFactor, _qpCurrent.height()/scaleFactor);
        if (scaleTo == 1)
        {
            if (AlignmentY >= 0)
                paintPixmap.drawLine(0, 0, _qpCurrent.width()/scaleFactor, 0); //top
            if (AlignmentY <= 0)
                paintPixmap.drawLine(0, _qpCurrent.width()/scaleFactor, _qpCurrent.height()/scaleFactor, _qpCurrent.width()/scaleFactor); // bottom
        }
        else
        {
            if (AlignmentX >= 0)
                paintPixmap.drawLine(0, 0, 0, _qpCurrent.height()/scaleFactor); //left
            if (AlignmentX <= 0)
                paintPixmap.drawLine(_qpCurrent.width()/scaleFactor, 0, _qpCurrent.width()/scaleFactor, _qpCurrent.height()/scaleFactor); //right
        }
            
    }
    
    
    QPainter paint(this);
    if (borderWidth >= 1)
        paint.fillRect(x, y, _qpCurrent.width()/scaleFactor, _qpCurrent.height()/scaleFactor, QColor(255, 255, 255, 255));
    paint.drawPixmap(x, y, _qpCurrent);
}
