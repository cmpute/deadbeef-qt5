#include "QtGui.h"

#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include <QLocale>

#include "DBApiWrapper.h"
#include "MainWindow.h"
#include "QtGuiSettings.h"

static int pluginStart();
static int pluginStop();
static int pluginConnect();
static int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);
void MainThreadRun(void *);

static intptr_t thread;

DB_functions_t *deadbeef;
DB_gui_t plugin;

#ifdef HOTKEYS_ENABLED
DB_hotkeys_plugin_t *hotkeys_plugin;
#endif
#ifdef ARTWORK_ENABLED
DB_artwork_plugin_t *coverart_plugin;
#endif

extern "C" {
    DB_plugin_t *ddb_gui_Qt_load(DB_functions_t *api) {
        deadbeef = api;
        plugin.plugin.api_vmajor = DB_API_VERSION_MAJOR;
        plugin.plugin.api_vminor = DB_API_VERSION_MINOR;
        plugin.plugin.version_major = 1;
        plugin.plugin.version_minor = 0;
        plugin.plugin.type = DB_PLUGIN_MISC;
        plugin.plugin.id = "qtui";
        plugin.plugin.name = "Qt user interface";
        plugin.plugin.descr = "Qt user interface";
        plugin.plugin.copyright = "Anton Novikov <tonn.post@gmail.com>\nSemen Minyushov <semikmsv@gmail.com>\nRoman Karjavin<redpunk231@gmail.com>\nZiyun Ling<ohmygod19993@gmail.com>";
        plugin.plugin.website = "https://github.com/maz-1/deadbeef-qt";
        plugin.plugin.start = pluginStart;
        plugin.plugin.stop = pluginStop;
        plugin.plugin.connect = pluginConnect;
        plugin.plugin.message = pluginMessage;
        return DB_PLUGIN(&plugin);
    }
}

static int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    switch (id) {
    case DB_EV_SONGCHANGED:
        WRAPPER->onSongChanged((ddb_event_trackchange_t *)ctx);
        break;
    case DB_EV_PAUSED:
        WRAPPER->onPause();
        break;
    case DB_EV_PLAYLISTCHANGED:
        WRAPPER->onPlaylistChanged();
        break;
    case DB_EV_ACTIVATED:
        WRAPPER->onDeadbeefActivated();
        break;
    }
    return 0;
}

static int pluginStart() {
    thread =  deadbeef->thread_start(MainThreadRun, NULL);
    return 0;
}

static int pluginStop() {
    QApplication::quit();
    qDebug() << "waiting for Qt thread to finish";
    deadbeef->thread_join(thread);
    qDebug() << "Qt thread finished";
    thread = 0;
    qDebug() << "QtGui_stop completed";
    return 0;
}

static int pluginConnect() {
#ifdef ARTWORK_ENABLED
    coverart_plugin = (DB_artwork_plugin_t *)DBAPI->plug_get_for_id("artwork");
    if (coverart_plugin)
        qDebug() << "qtui: found cover-art plugin";

#endif

#ifdef HOTKEYS_ENABLED
    hotkeys_plugin = (DB_hotkeys_plugin_t *)DBAPI->plug_get_for_id("hotkeys");
    if (hotkeys_plugin)
        qDebug() << "qtui: found global hotkeys plugin";
#endif
    return 0;
}


void MainThreadRun(void *) {
    int argc = 1;
    char name[] = "deadbeef";
    char *argv[] = { name, nullptr };
    QApplication app(argc, argv);
    QApplication::setOrganizationName("deadbeef");
    QApplication::setApplicationName("deadbeef-qt");
    
    if (app.devicePixelRatio() > 1)
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QString locale = QLocale::system().name();
    QTranslator translator;
    QString qmFile = QString::fromUtf8(DEADBEEF_PREFIX) + QString("/share/deadbeef/translations/QtGui_") + locale;
    if (!translator.load(qmFile))
    {
        qDebug() << "Failed to load " << qmFile;
    }
    else
    {
        if (!app.installTranslator(&translator))
        {
            qDebug()<< "Failed to install translator";
        }
    }

    //MainWindow w;
    //w.show();
    
    //qDebug() << MainWindow::Instance();
    MainWindow::Instance()->show();
    app.exec();
    MainWindow::Destroy();
}
