#ifndef GUIUPDATER_H
#define GUIUPDATER_H

#include <QObject>
#include <QTimerEvent>

#include "QtGui.h"

class GuiUpdater : public QObject {
    Q_OBJECT
public:
    static GuiUpdater *Instance();
    static void Destroy();

    void resetTimer(int tick);
    
private:
    GuiUpdater(QObject* parent = 0);
    static GuiUpdater *instance;

    bool killTimerAtNextTick;
    void startSpecificTimer(int newTimerTick = -1);
    int timerTick;
    int newPlayingState;
    int oldPlayingState;
    
protected:
    void timerEvent(QTimerEvent *event);
signals:
    void frameUpdate();
    void isPlaying(bool);
};

#endif // GUIUPDATER_H
