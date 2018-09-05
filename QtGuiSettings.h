#ifndef QTGUISETTINGS_H
#define QTGUISETTINGS_H

#define SETTINGS QtGuiSettings::Instance()

#include "config.h"

#include <QVariant>
#include <QSize>
#include <QString>
#include <QSettings>
#include <QPoint>

#define Var(Type, ToType, ValType, Name, Default)  \
    static const QString Name;                          \
    Type get##Name() { return getValue(CUR_GROUP, Name, Default).to##ToType(); }   \
    void set##Name(ValType val) { setValue(CUR_GROUP, Name, val); }

#define VarQ(Type, Name, Default) Var(Q##Type, Type, const Q##Type &, Name, Default)

#define VarBool(Name)   Var(bool, Bool, bool, Name, false)
#define VarTrue(Name)   Var(bool, Bool, bool, Name, true)
#define VarInt(Name, Default)    Var(int,  Int,  int,  Name, Default)
#define VarString(Name, Default) VarQ(String,    Name, Default)
#define VarByteArray(Name)       VarQ(ByteArray, Name, QByteArray())

class QtGuiSettings : public QObject {
    Q_OBJECT
public:
    static QtGuiSettings *Instance();
    static void Destroy();

    QVariant getValue(const QString &group, const QString &key, const QVariant &defaultValue);
    void setValue(const QString &group, const QString &key, const QVariant &value);

//     MainWindow Group
    static const QString MainWindow;

#define CUR_GROUP MainWindow
    VarQ(Size,   WindowSize, QSize(640, 480));
    VarQ(Point,  WindowPosition, QPoint(0, 0));
    VarByteArray(WindowState);

    VarBool(ToolbarsIsLocked);
    VarBool(MainMenuIsHidden);
    VarBool(StatusbarIsHidden);
    VarBool(MinimizeOnClose);
    VarInt (RefreshRate, 10);
    VarString(TitlebarPlaying, "%a - %t - DeaDBeeF-%V");
    VarString(TitlebarStopped, "DeaDBeeF-%V");
    VarInt (TabBarPosition, 0); // TabBar::Top
    VarTrue(TabBarIsVisible);
#ifdef ARTWORK_ENABLED
    VarBool(CoverartIsHidden);
#endif
#undef CUR_GROUP

    //TrayIcon Group
    static const QString TrayIcon;

#define CUR_GROUP TrayIcon
    VarBool(TrayIconIsHidden);
    VarBool(ShowTrayTips);
    VarString(MessageFormat, "%a - %t");
    VarString(TrayIconTheme, "Default");
#undef CUR_GROUP

    //PlayList Group
    static const QString PlayList;

#define CUR_GROUP PlayList
    VarByteArray(HeaderState);
    VarBool(HeaderIsLocked);
    VarTrue(HeaderIsVisible);
#undef CUR_GROUP

private:
    QtGuiSettings();
    static QtGuiSettings *instance;

    QSettings settings;
};

#undef VarString
#undef VarTrue
#undef VarBool
#undef VarByteArray
#undef VarQ
#undef Var

#endif // QTGUISETTINGS_H
