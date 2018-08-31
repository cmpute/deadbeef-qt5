#include "PlayList.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>

#include <QtGui.h>
#include <QtGuiSettings.h>
#include <TabBar.h>

PlayList::PlayList(QWidget *parent) : QTreeView(parent), playListModel(this) {
    setAutoFillBackground(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setDragEnabled(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setIconSize(QSize(16, 16));
    setTextElideMode(Qt::ElideRight);
    setIndentation(0);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setItemsExpandable(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setWordWrap(false);
    setExpandsOnDoubleClick(false);
    setAcceptDrops(true);
    setModel(&playListModel);
    
    header()->setStretchLastSection(false);
    
//###################################################
//     header()->setStretchLastSection(true);
//     header()->setResizeMode(QHeaderView::Stretch);
//###################################################
    
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setSortIndicatorShown(false);
    
    header()->setDefaultSectionSize(80);
    header()->setMinimumSectionSize(10);
    
    createContextMenu();
    createHeaderContextMenu();
    createConnections();

    installEventFilter(this);
}

bool PlayList::eventFilter(QObject *target, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent *)event;
        if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
            emit enterRelease(currentIndex());
            return true;
        }
    }
    return QTreeView::eventFilter(target, event);
}

void PlayList::createConnections() {
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(enterRelease(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));

    connect(header(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(headerContextMenuRequested(QPoint)));
    connect(header(), SIGNAL(sectionResized(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionMoved(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionClicked(int)), SLOT(saveHeaderState()));
    connect(WRAPPER, SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)), this, SLOT(onTrackChanged(DB_playItem_t *, DB_playItem_t *)));
    connect(WRAPPER, SIGNAL(playlistChanged()), SLOT(refresh()));
}

void PlayList::refresh() {
    setModel(NULL);
    setModel(&playListModel);
    goToLastSelection();
    header()->restoreState(headerState);
}

void PlayList::goToLastSelection() {
    int cursor = DBAPI->plt_get_cursor(DBPltRef(), PL_MAIN);
    if (cursor < 0)
        restoreCursor();
    else
        setCurrentIndex(playListModel.index(cursor, 0, QModelIndex()));
}

void PlayList::restoreCursor() {
    int currentPlaylist = DBAPI->plt_get_curr_idx();
    int cursor = DBAPI->conf_get_int(QString("playlist.cursor.%1").arg(currentPlaylist).toUtf8().constData(), -1);
    setCurrentIndex(playListModel.index(cursor, 0, QModelIndex()));
}

void PlayList::storeCursor() {
    DBPltRef plt;
    int cursor = DBAPI->plt_get_cursor(plt, PL_MAIN);
    DBAPI->conf_set_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8().constData(), cursor);
}

void PlayList::saveConfig() {
    SETTINGS->setHeaderIsVisible(!header()->isHidden());
    SETTINGS->setHeaderState(header()->saveState());
    SETTINGS->setHeaderIsLocked(!header()->sectionsMovable() && header()->sectionResizeMode(1) == QHeaderView::Fixed);
    playListModel.saveConfig();
}

void PlayList::loadConfig() {
    bool isVisible = SETTINGS->getHeaderIsVisible();
    bool isLocked = SETTINGS->getHeaderIsLocked();
    headerState = SETTINGS->getHeaderState();
    header()->setHidden(!isVisible);
    header()->restoreState(headerState);
    header()->setSectionsMovable(!isLocked);
    header()->setSectionResizeMode(isLocked ? QHeaderView::Fixed : QHeaderView::Interactive);
    lockColumnsAction->setChecked(isLocked);
}

void PlayList::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls() || event->mimeData()->hasFormat("playlist/track")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void PlayList::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        int count = pltItemCount();;
        int row = indexAt(event->pos()).row();
        int before = (row >= 0) ? row - 1 : count - 1;
        foreach (QUrl url, event->mimeData()->urls()) {
            playListModel.insertByURLAtPosition(url, before);
            before++;
        }
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if (event->mimeData()->hasFormat("playlist/track")) {
        int row = indexAt(event->pos()).row();
        int count = pltItemCount();
        row = (row >= 0) ? row : count;
        QByteArray encodedData = event->mimeData()->data("playlist/track");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QHash<int,QString> newItems;
        while (!stream.atEnd()) {
            int row;
            QString text;
            stream >> row >> text;
            newItems[row] = text;
        }
        QList<int> rows = newItems.keys();
        qSort(rows.begin(), rows.end());
        playListModel.moveItems(rows, row);
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void PlayList::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if (selected == deselected)
        return;

    DBAPI->plt_set_cursor(DBPltRef(), PL_MAIN, selected.indexes().count() == 0 ? -1 : selected.indexes().last().row());
    storeCursor();

    QTreeView::selectionChanged(selected, deselected);
}

void PlayList::trackDoubleClicked(QModelIndex index) {
    DBApiWrapper::Instance()->playTrackByIndex(index.row());
}

void PlayList::createContextMenu() {
    setContextMenuPolicy(Qt::CustomContextMenu);
    //delete tracks
    QAction *delTrack = new QAction(tr("Remove track(s)"), this);
    delTrack->setShortcut(Qt::Key_Delete);
    connect(delTrack, SIGNAL(triggered()), this, SLOT(delSelectedTracks()));
    addAction(delTrack);
    //delete from disk
    QAction *delFile = new QAction(tr("Remove track(s) from disk"), this);
    connect(delFile, SIGNAL(triggered()), this, SLOT(delSelectedFiles()));
    addAction(delFile);
    
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

void PlayList::createHeaderContextMenu() {
    lockColumnsAction = new QAction(tr("Lock columns"), &headerContextMenu);
    lockColumnsAction->setCheckable(true);
    lockColumnsAction->setChecked(header()->sectionsMovable() && header()->sectionResizeMode(0) == QHeaderView::Fixed);
    connect(lockColumnsAction, SIGNAL(toggled(bool)), this, SLOT(lockColumns(bool)));
    
    loadConfig();
    
    QMenu *columnsMenu = new QMenu(tr("Columns"), &headerContextMenu);
    foreach (QString name, playListModel.columnNames.values()) {
        QAction *action = new QAction(name, &headerContextMenu);
        action->setCheckable(true);
        for (int i = 0; i < header()->count(); i++)
            if (playListModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == name ||
                (playListModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == "" && name == tr("Status"))
            ) {
                action->setChecked(!isColumnHidden(i));
                break;
            }
            connect(action, SIGNAL(toggled(bool)), SLOT(setColumnHidden(bool)));
        columnsMenu->addAction(action);
    }
    headerContextMenu.addMenu(columnsMenu);
    headerContextMenu.addAction(lockColumnsAction);
}

void PlayList::showContextMenu(QPoint point) {
    if (indexAt(point).row() < 0)
        return;
    QMenu menu(this);
    menu.addActions(actions());
    menu.exec(mapToGlobal(point));
}

void PlayList::headerContextMenuRequested(QPoint pos) {
    headerContextMenu.move(mapToGlobal(pos));
    headerContextMenu.show();
}

void PlayList::lockColumns(bool locked) {
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);
    headerState = header()->saveState();
}

void PlayList::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    int index = DBAPI->plt_get_item_idx(DBPltRef(), to, PL_MAIN);
    setCurrentIndex(playListModel.index(index, 0, QModelIndex()));

    playListModel.index(index, 0, QModelIndex());
}

void PlayList::delSelectedTracks() {
    playListModel.deleteTracks(selectionModel()->selectedRows(), false);
}

void PlayList::delSelectedFiles() {
    playListModel.deleteTracks(selectionModel()->selectedRows(), true);
}

void PlayList::clearPlayList() {
    playListModel.clearPlayList();
}

void PlayList::insertByURLAtPosition(const QUrl& url, int position) {
    playListModel.insertByURLAtPosition(url, position);
}

void PlayList::toggleHeaderHidden() {
    setHeaderHidden(!isHeaderHidden());
}

void PlayList::setColumnHidden(bool hidden) {
    if (QAction *action = qobject_cast<QAction *>(QObject::sender())) {
        for (int i = 0; i < header()->count(); i++) {
            if (playListModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == action->text() ||
                (playListModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == "" && action->text() == tr("Status"))
            ) {
                QTreeView::setColumnHidden(i, !hidden);
                headerState = header()->saveState();
                break;
            }
        }
    }
}

void PlayList::saveHeaderState() {
    headerState = header()->saveState();
}
