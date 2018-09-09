#include "QDoubleSlider.h"
#include <QStyleOptionSlider>
#include <QToolTip>
#include <QStyle>

QDoubleSlider::QDoubleSlider(Qt::Orientation orientation, QWidget* pParent /*= NULL*/) :
QSlider(pParent),
m_Multiplier(1000000.0)
{
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));

    setSingleStep(1);

    setOrientation(orientation);
    setFocusPolicy(Qt::NoFocus);
}

void QDoubleSlider::setValue(int Value)
{
    emit valueChanged((double)Value / m_Multiplier);
}

void QDoubleSlider::setValue(double Value, bool BlockSignals)
{
    QSlider::blockSignals(BlockSignals);

    QSlider::setValue(Value * m_Multiplier);

    if (!BlockSignals)
        emit valueChanged(Value);

    QSlider::blockSignals(false);
}

void QDoubleSlider::setRange(double Min, double Max)
{
    QSlider::setRange(Min * m_Multiplier, Max * m_Multiplier);

    emit rangeChanged(Min, Max);
}

void QDoubleSlider::setMinimum(double Min)
{
    QSlider::setMinimum(Min * m_Multiplier);

    emit rangeChanged(minimum(), maximum());
}

double QDoubleSlider::minimum() const
{
    return QSlider::minimum() / m_Multiplier;
}

void QDoubleSlider::setMaximum(double Max)
{
    QSlider::setMaximum(Max * m_Multiplier);

    emit rangeChanged(minimum(), maximum());
}

double QDoubleSlider::maximum() const
{
    return QSlider::maximum() / m_Multiplier;
}

double QDoubleSlider::value() const
{
    int Value = QSlider::value();
    return (double)Value / m_Multiplier;
}

void QDoubleSlider::sliderChange(QAbstractSlider::SliderChange change)
{
    QSlider::sliderChange(change);

    if (change == QAbstractSlider::SliderValueChange )
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        QPoint bottomRightCorner = sr.bottomLeft();

        QToolTip::showText(mapToGlobal( QPoint( bottomRightCorner.x(), bottomRightCorner.y() ) ), QString::number(value()), this);
    }
}
