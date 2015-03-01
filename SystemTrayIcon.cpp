#include "SystemTrayIcon.h"

SystemTrayIcon::SystemTrayIcon(const QIcon &icon, QObject *parent) :
    QSystemTrayIcon(icon, parent) {
}

bool SystemTrayIcon::event(QEvent *event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelevent = (QWheelEvent *)event;
        emit wheeled(wheelevent->delta());
        event->accept();
        return true;
    }
    else
        return QSystemTrayIcon::event(event);
}
