#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDateTime>

#include "QtGuiSettings.h"

#include "QtGui.h"
#include "DBApiWrapper.h"
#include "AboutDialog.h"
#include "PlayList.h"

#include <include/callbacks.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include "DBFileDialog.h"

MainWindow *MainWindow::instance = NULL;

template <typename T>
int signum(T val) {
    return (T(0) < val) - (val < T(0));
}

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        trayIcon(nullptr),
        trayMenu(nullptr),
        volumeSlider(this),
        progressBar(this),
        status(this),
#ifdef ARTWORK_ENABLED
        coverArtWidget(this),
#endif
        orderGroup(this),
        loopingGroup(this)
{
    defaultTrayIcon = QIcon(":/root/images/bitmap.png");
    trayIconTheme[QString("Default")] = defaultTrayIcon;
    trayIconTheme[QString("Dark")] = QIcon(":/root/images/tray_dark.png");
    trayIconTheme[QString("Light")] = QIcon(":/root/images/tray_light.png");
    
    ui->setupUi(this);
    
    loadActions();
    loadIcons();
    createToolBars();
    createConnections();

    loadConfig();
    updateTitle();
    
    ui->PlayBackToolBar->show();
    ui->statusBar->addWidget(&status);
}

void MainWindow::Destroy() {
    if (instance != NULL)
        delete instance;
    instance = NULL;
}

MainWindow *MainWindow::Instance() {
    if (instance == NULL) {
        instance = new MainWindow();
    }
    
    return instance;
}

MainWindow::~MainWindow() {
    saveConfig();
    delete ui;
}

void MainWindow::createConnections() {
    connect(DBApiWrapper::Instance(), SIGNAL(trackChanged(DB_playItem_t*,DB_playItem_t*)), this, SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    connect(ui->actionNewPlaylist, SIGNAL(triggered()), ui->playList, SIGNAL(newPlaylist()));
    connect(DBApiWrapper::Instance(), SIGNAL(deadbeefActivated()), this, SLOT(on_deadbeefActivated()));
    connect(&progressBar, &SeekSlider::valueChanged, [this](){
        DB_playItem_s *it = DBAPI->streamer_get_playing_track();
        updateStatusBar(it);
    });
}

void MainWindow::loadIcons() {
    ui->actionPlay->setIcon(getStockIcon(this, "media-playback-start", QStyle::SP_MediaPlay));
    ui->actionPause->setIcon(getStockIcon(this, "media-playback-pause", QStyle::SP_MediaPause));
    ui->actionStop->setIcon(getStockIcon(this, "media-playback-stop", QStyle::SP_MediaStop));
    ui->actionPrev->setIcon(getStockIcon(this, "media-skip-backward", QStyle::SP_MediaSkipBackward));
    ui->actionNext->setIcon(getStockIcon(this, "media-skip-forward", QStyle::SP_MediaSkipForward));
    ui->actionExit->setIcon(getStockIcon(this, "application-exit", QStyle::SP_DialogCloseButton));
    ui->actionClearAll->setIcon(getStockIcon(this, "edit-clear", QStyle::SP_TrashIcon));
    ui->actionAddFiles->setIcon(getStockIcon(this, "document-open", QStyle::SP_FileIcon));
    ui->actionAddFolder->setIcon(getStockIcon(this, "folder-m", QStyle::SP_DirIcon));
    ui->actionAddURL->setIcon(getStockIcon(this, "folder-remote", QStyle::SP_DriveNetIcon));
    ui->actionAddAudioCD->setIcon(getStockIcon(this, "media-optical-audio", QStyle::SP_DriveCDIcon));
    ui->actionNewPlaylist->setIcon(getStockIcon(this, "document-new", QStyle::SP_FileDialogNewFolder));
    ui->actionPreferences->setIcon(getStockIcon(this, "settings-configure", QStyle::SP_CustomBase));
    ui->actionAbout->setIcon(getStockIcon(this, "help-about", QStyle::SP_DialogHelpButton));
}

void MainWindow::loadActions() {
    addAction(ui->actionAddFolder);
    addAction(ui->actionExit);
    addAction(ui->actionPreferences);
    addAction(ui->actionAddFiles);
    addAction(ui->actionAddURL);
    addAction(ui->actionSaveAsPlaylist);
    addAction(ui->actionLoadPlaylist);
    addAction(ui->actionNewPlaylist);
    addAction(ui->actionHideMenuBar);
    addAction(ui->actionFind);
}

void MainWindow::createTray() {
    //trayIcon = new SystemTrayIcon(windowIcon(), this);
    QString variant(SETTINGS->getTrayIconTheme());
    if (trayIconTheme.contains(variant))
        trayIcon = new SystemTrayIcon(trayIconTheme[variant], this);
    else
        trayIcon = new SystemTrayIcon(defaultTrayIcon, this);
    
    
    trayMenu = new QMenu();
    trayMenu->addAction(ui->actionPlay);
    trayMenu->addAction(ui->actionPause);
    trayMenu->addAction(ui->actionStop);
    trayMenu->addAction(ui->actionNext);
    trayMenu->addAction(ui->actionPrev);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->actionPreferences);
    trayMenu->addAction(ui->actionExit);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIcon_activated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon, SIGNAL(wheeled(int)), this, SLOT(trayIcon_wheeled(int)));
}

void MainWindow::titleSettingChanged() {
    updateTitle();
}

void MainWindow::updateTitle(DB_playItem_t *it) {
    char str[256];
    const char *fmt;

    if (!it)
        it = DBAPI->streamer_get_playing_track();
    else
        DBAPI->pl_item_ref(it);

    if (it)
        fmt = SETTINGS->getTitlebarPlaying().toUtf8().constData();
    else
        fmt = SETTINGS->getTitlebarStopped().toUtf8().constData();

    DBAPI->pl_format_title(it, -1, str, sizeof(str), -1, fmt);

    setWindowTitle(QString::fromUtf8(str));
    if (trayIcon)
        trayIcon->setToolTip(QString::fromUtf8(str));

    if (it)
        DBAPI->pl_item_unref(it);
}

void MainWindow::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *e) {
    switch (actionOnClose) {
    case Exit:
        e->accept();
        WRAPPER->Destroy();
        break;
    case Hide:
        e->ignore();
        if (isHidden())
            show();
        else
            hide();
        break;
    case Minimize:
        e->ignore();
        showMinimized();
        break;
    }
}

void MainWindow::on_actionAddFolder_triggered() {
    DBFileDialog fileDialog(this,
                            tr("Add folder(s) to playlist..."),
                            QStringList(),
                            QFileDialog::DirectoryOnly,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    if (!fileDialog.exec())
        return;
    QStringList fileNames = fileDialog.selectedFiles();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void MainWindow::on_actionClearAll_triggered() {
    ui->playList->clearPlayList();
}

void MainWindow::on_actionPlay_triggered() {
    if (DBApiWrapper::Instance()->isPaused)
        DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
    else
    {
        emit DBApiWrapper::Instance()->playbackPaused();
        DBApiWrapper::Instance()->sendPlayMessage(DB_EV_PLAY_CURRENT);
    }
}

void MainWindow::on_actionStop_triggered() {
    DBAPI->sendmessage(DB_EV_STOP, 0, 0, 0);
    updateTitle();
}

void MainWindow::on_actionNext_triggered() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_NEXT);
}

void MainWindow::on_actionPrev_triggered() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_PREV);
}

void MainWindow::on_actionPause_triggered() {
    DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
}

void MainWindow::trayIcon_activated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isHidden())
            show();
        else
            hide();
    }
    if (reason == QSystemTrayIcon::MiddleClick) {
        DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
    }

}

void MainWindow::on_actionExit_triggered() {
    actionOnClose = Exit;
    close();
}

void MainWindow::trayIcon_wheeled(int delta) {
    volumeSlider.setValue(volumeSlider.value() + signum(delta));
}

void MainWindow::updateStatusBar(DB_playItem_t *it)
{
    if (it)
    {
        QString type = QString::fromLatin1(DBAPI->pl_find_meta(it, ":FILETYPE"));
        QString duration = DBAPI->pl_find_meta(it, ":DURATION");

        int all = (duration.split(":")[0].toInt() * 60000) + (duration.split(":")[1].toInt() * 1000);

        QDateTime msec;
        msec.setMSecsSinceEpoch(all * DBAPI->playback_get_pos() / 100);
        status.setText(type + " | " + msec.toString("mm:ss") + " / " + duration);
    }
}

void MainWindow::trackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    if (to != NULL) {
        char str[1024];
        const char *fmt = SETTINGS->getMessageFormat().toUtf8().constData();
        DBAPI->pl_item_ref(to);
        DBAPI->pl_format_title(to, 0, str, sizeof(str), -1, fmt);
        bool showTrayTips = SETTINGS->getShowTrayTips();
        if (trayIcon && showTrayTips) {
            trayIcon->showMessage("DeaDBeeF", QString::fromUtf8(str), QSystemTrayIcon::Information, 2000);
        }
        DBAPI->pl_item_unref(to);
    } else {
        progressBar.setValue(0);
    }
    updateTitle(to);
}


void MainWindow::on_actionLinearOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_LINEAR);
}

void MainWindow::on_actionRandomOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_RANDOM);
}

void MainWindow::on_actionShuffleOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_SHUFFLE_TRACKS);
}

void MainWindow::on_actionLoopAll_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_LOOP_ALL);
}

void MainWindow::on_actionLoopTrack_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_LOOP_SINGLE);
}

void MainWindow::on_actionLoopNothing_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_NOLOOP);
}

void MainWindow::on_actionAbout_triggered() {
    AboutDialog().exec();
}

void MainWindow::on_actionAboutQt_triggered() {
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionPreferences_triggered() {
    //PreferencesDialog *prefDialog = new PreferencesDialog(this);
    if (!prefDialog)
    {
        prefDialog = new PreferencesDialog(this);
        connect(prefDialog, SIGNAL(setCloseOnMinimize(bool)), this, SLOT(setCloseOnMinimized(bool)));
        connect(prefDialog, SIGNAL(setTrayIconHidden(bool)), this, SLOT(setTrayIconHidden(bool)));
        connect(prefDialog, SIGNAL(setTrayIconTheme(const QString &)), this, SLOT(setTrayIconTheme(const QString &)));
        connect(prefDialog, SIGNAL(titlePlayingChanged()), this, SLOT(titleSettingChanged()));
        connect(prefDialog, SIGNAL(titleStoppedChanged()), this, SLOT(titleSettingChanged()));
        prefDialog->exec();
        delete prefDialog;
        prefDialog = nullptr;
    }
    else
        prefDialog->activateWindow();
}

void MainWindow::on_actionSelectAll_triggered() {
    ui->playList->selectAll();
}

void MainWindow::on_actionDeselectAll_triggered() {
    ui->playList->deselectAll();
}

void MainWindow::on_actionRemove_triggered() {
    ui->playList->deleteSelectedTracks();
}

void MainWindow::on_actionAddFiles_triggered() {
    DBFileDialog fileDialog(this,
                            tr("Add file(s) to playlist..."),
                            QStringList(),
                            QFileDialog::ExistingFiles,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    if (!fileDialog.exec())
        return;
    QStringList fileNames = fileDialog.selectedFiles();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void MainWindow::on_actionAddAudioCD_triggered() {
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, SIGNAL(finished()), ui->playList, SLOT(refresh()));
    watcher->setFuture(QtConcurrent::run(loadAudioCD));
}

void MainWindow::on_actionAddURL_triggered() {
    ui->playList->insertByURLAtPosition(QUrl::fromUserInput(QInputDialog::getText(this, tr("Enter URL..."), tr("URL: "), QLineEdit::Normal)));
}

void MainWindow::createToolBars() {
    orderGroup.addAction(ui->actionLinearOrder);
    orderGroup.addAction(ui->actionRandomOrder);
    orderGroup.addAction(ui->actionShuffleOrder);

    loopingGroup.addAction(ui->actionLoopAll);
    loopingGroup.addAction(ui->actionLoopTrack);
    loopingGroup.addAction(ui->actionLoopNothing);

    ui->PlayBackToolBar->addWidget(&progressBar);
    ui->PlayBackToolBar->addWidget(&volumeSlider);
}

QMenu *MainWindow::createPopupMenu() {
    QMenu *popupMenu = new QMenu(this);
    popupMenu->addAction(ui->actionHideMenuBar);
    popupMenu->addSeparator();
    popupMenu->addAction(ui->actionBlockToolbarChanges);
    return popupMenu;
}

void MainWindow::on_actionHideMenuBar_triggered() {
    ui->menuBar->setHidden(!ui->menuBar->isHidden());
    ui->actionHideMenuBar->setChecked(!ui->menuBar->isHidden());
}

void MainWindow::on_actionBlockToolbarChanges_triggered() {
    ui->PlayBackToolBar->setMovable(!ui->actionBlockToolbarChanges->isChecked());
#ifdef ARTWORK_ENABLED
    if (ui->actionBlockToolbarChanges->isChecked())
        coverArtWidget.setTitleBarWidget(new QWidget());
    else
        coverArtWidget.setTitleBarWidget(0);
#endif
}

void MainWindow::on_actionSaveAsPlaylist_triggered() {
    QStringList filters;
    filters << tr("DeaDBeeF playlist files (*.dbpl)");
    DB_playlist_t **plug = deadbeef->plug_get_playlist_list();
    for (int i = 0; plug[i]; i++) {
        if (plug[i]->extensions && plug[i]->load) {
            const char **exts = plug[i]->extensions;
            if (exts && plug[i]->save)
                for (int e = 0; exts[e]; e++)
                    filters << QString("*.%1").arg(exts[e]);
        }
    }
    DBFileDialog fileDialog(this,
                            tr("Save playlist as..."),
                            filters,
                            QFileDialog::AnyFile,
                            QFileDialog::DontUseNativeDialog);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    
    if (!fileDialog.exec())
        return;
    QStringList fileNames = fileDialog.selectedFiles();
    if (fileNames.isEmpty())
        return;
    
    QString destPath = fileNames.last();
    DBPltRef plt;
    if (plt && deadbeef->plt_save(plt, NULL, NULL, destPath.toUtf8().constData(), NULL, NULL, NULL))
        QMessageBox::warning(this,
                             tr("Save playlist as..."),
                             tr("Failed to save playlist to %1: %2").arg(destPath).arg(qt_error_string(errno)));
}

void MainWindow::on_actionLoadPlaylist_triggered() {
    QStringList filters;
    filters << tr("Supported playlist formats (*.dbpl)");
    filters << tr("Other files (*)");
    DBFileDialog fileDialog(this,
                            tr("Load playlist"),
                            filters,
                            QFileDialog::ExistingFile,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    if (!fileDialog.exec())
        return;
    QStringList fileNames = fileDialog.selectedFiles();
    if (fileNames.isEmpty())
        return;

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, SIGNAL(finished()), ui->playList, SLOT(refresh()));
    watcher->setFuture(QtConcurrent::run(loadPlaylist, fileNames.last()));
}

#ifdef ARTWORK_ENABLED
void MainWindow::on_actionHideCoverArt_triggered(bool checked) {
    coverArtWidget.setHidden(!checked);
}

void MainWindow::onCoverartClose() {
    ui->actionHideCoverArt->setChecked(false);
}

#endif

void MainWindow::setCloseOnMinimized(bool minimizeOnClose) {
    bool trayIconIsHidden = SETTINGS->getTrayIconIsHidden();
    configureActionOnClose(minimizeOnClose, trayIconIsHidden);
}

void MainWindow::setTrayIconTheme(const QString &variant) {
    if (trayIconTheme.contains(variant))
        trayIcon->setIcon(trayIconTheme[variant]);
    else
        trayIcon->setIcon(defaultTrayIcon);
}

void MainWindow::setTrayIconHidden(bool hideTrayIcon) {
    // FIXME: crashes when changing from hidden to visible. System bug?
    trayIcon->setVisible(!hideTrayIcon);
    //if (hideTrayIcon) {
    //    delete trayIcon;
    //    trayIcon = nullptr;
    //} else {
    //    createTray();
    //}
    // FIXME: it still crashes in ~MainWindow() after toggling tray icon
    
    bool minimizeOnClose = SETTINGS->getMinimizeOnClose();
    configureActionOnClose(minimizeOnClose, hideTrayIcon);
}

void MainWindow::configureActionOnClose(bool minimizeOnClose, bool hideTrayIcon) {
    actionOnClose = Exit;
    if (minimizeOnClose) {
        if (hideTrayIcon)
            actionOnClose = Minimize;
        else
            actionOnClose = Hide;
    }    
}

void MainWindow::loadConfig() {
    QSize size       = SETTINGS->getWindowSize();
    QPoint point     = SETTINGS->getWindowPosition();
    QByteArray state = SETTINGS->getWindowState();
    bool tbIsLocked  = SETTINGS->getToolbarsIsLocked();
    bool mmIsHidden  = SETTINGS->getMainMenuIsHidden();
    bool trayIconIsHidden = SETTINGS->getTrayIconIsHidden();
    bool minimizeOnClose = SETTINGS->getMinimizeOnClose();
    bool headerIsVisible = SETTINGS->getHeaderIsVisible();
    bool tbIsVisible = SETTINGS->getTabBarIsVisible();

    resize(size);
    move(point);
    ui->actionBlockToolbarChanges->setChecked(tbIsLocked);
    menuBar()->setHidden(mmIsHidden);
    ui->actionHideMenuBar->setChecked(!menuBar()->isHidden());

    ui->actionPlayListHeader->setChecked(headerIsVisible);
    ui->actionHideTabBar->setChecked(tbIsVisible);

    if (!trayIconIsHidden) {
        createTray();
    }
#ifdef ARTWORK_ENABLED
    bool caIsHidden  = SETTINGS->getCoverartIsHidden();
    ui->actionHideCoverArt->setChecked(!caIsHidden);
    if (ui->actionHideCoverArt->isChecked()) {
        addDockWidget(Qt::LeftDockWidgetArea, &coverArtWidget);
        connect(&coverArtWidget, SIGNAL(onCloseEvent()), this, SLOT(onCoverartClose()));
    }
#else
    ui->actionHideCoverArt->setVisible(false);
#endif

    restoreState(state);
    configureActionOnClose(minimizeOnClose, trayIconIsHidden);

    ui->PlayBackToolBar->setMovable(!ui->actionBlockToolbarChanges->isChecked());

#ifdef ARTWORK_ENABLED
    if (ui->actionBlockToolbarChanges->isChecked())
        coverArtWidget.setTitleBarWidget(new QWidget());
    else
        coverArtWidget.setTitleBarWidget(0);
#endif

    switch (DBAPI->conf_get_int("playback.order", PLAYBACK_ORDER_LINEAR)) {
    case PLAYBACK_ORDER_LINEAR:
        ui->actionLinearOrder->setChecked(true);
        break;
    case PLAYBACK_ORDER_RANDOM:
        ui->actionRandomOrder->setChecked(true);
        break;
    case PLAYBACK_ORDER_SHUFFLE_TRACKS:
        ui->actionShuffleOrder->setChecked(true);
        break;
    }

    switch (DBAPI->conf_get_int("playback.loop", PLAYBACK_MODE_NOLOOP)) {
    case PLAYBACK_MODE_LOOP_ALL:
        ui->actionLoopAll->setChecked(true);
        break;
    case PLAYBACK_MODE_LOOP_SINGLE:
        ui->actionLoopTrack->setChecked(true);
        break;
    case PLAYBACK_MODE_NOLOOP:
        ui->actionLoopNothing->setChecked(true);
        break;
    }

    //qDebug() << QString::fromUtf8(DEADBEEF_PREFIX);
}

void MainWindow::saveConfig() {
    SETTINGS->setWindowSize(size());
    SETTINGS->setWindowPosition(pos());
    SETTINGS->setWindowState(saveState());
    SETTINGS->setToolbarsIsLocked(ui->actionBlockToolbarChanges->isChecked());
    SETTINGS->setMainMenuIsHidden(menuBar()->isHidden());
#ifdef ARTWORK_ENABLED
    SETTINGS->setCoverartIsHidden(!ui->actionHideCoverArt->isChecked());
#endif
    ui->playList->saveConfig();
}

void MainWindow::on_actionPlayListHeader_triggered() {
    ui->playList->header();
}

void MainWindow::on_actionHideTabBar_triggered() {
    ui->playList->hideTab();
}

void MainWindow::on_deadbeefActivated() {
    if (isHidden()) show();
}
