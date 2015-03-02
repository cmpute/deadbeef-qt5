#include "PlayListModel.h"

#include "QtGui.h"

PlayListModel::PlayListModel(QObject *parent) : QAbstractItemModel(parent),
    playIcon(":/root/images/play_16.png"),
    pauseIcon(":/root/images/pause_16.png") {
    connect(DBApiWrapper::Instance(), SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)),
            this, SLOT(trackChanged(DB_playItem_t*,DB_playItem_t*)));
    connect(DBApiWrapper::Instance(), SIGNAL(playbackPaused()), this, SLOT(playerPaused()));
    columnNames.insert("%s", tr("Status"));
    columnNames.insert("%n", tr("№"));
    columnNames.insert("%t", tr("Title"));
    columnNames.insert("%a", tr("Artist"));
    columnNames.insert("%b", tr("Album"));
    columnNames.insert("%y", tr("Year"));
    columnNames.insert("%l", tr("Duration"));
    loadConfig();
}

void PlayListModel::loadConfig() {
    QString config = "%s|%n|%t|%a|%b|%y|%l";
    columns = config.split('|');
    status_column = columns.indexOf(QRegExp("^(.*(%s).*)+$"), 0);
}

void PlayListModel::saveConfig() {
    
}

int PlayListModel::columnCount(const QModelIndex &) const {
    return columns.count();
}

Qt::ItemFlags PlayListModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QVariant PlayListModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const {
    switch (role) {
    case Qt::DisplayRole: {
        char title[1024];
        DBItemRef track(DBPltRef().at(index.row()));
        if (index.column() == status_column)
            DBAPI->pl_format_title(track, 0, title, sizeof(title), DB_COLUMN_PLAYING, NULL);
        else
            DBAPI->pl_format_title(track, 0, title, sizeof(title), -1, columns[index.column()].toUtf8().data());
        return QString::fromUtf8(title);
    }
    case Qt::DecorationRole: {
        if (index.column() == status_column) {
            if (playingItemIndex() == index.row()) {
                if (DBApiWrapper::Instance()->isPaused)
                    return pauseIcon;
                else
                    return playIcon;
            }
        }
        break;
    }
    case Qt::SizeHintRole: {
        QSize defSize;
        //TODO: get value from settings
        defSize.setHeight(25);
        return defSize;
    }
    }

    return QVariant();
}

QModelIndex PlayListModel::index(int row, int column, const QModelIndex &parent) const {
    return createIndex(row, column, nullptr);
}

QModelIndex PlayListModel::parent(const QModelIndex &child) const {
    return QModelIndex();
}

int PlayListModel::rowCount(const QModelIndex &parent) const {
    return pltItemCount();
}

QVariant PlayListModel::headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section < columns.count())
                if (columnNames[columns[section]] == tr("Status"))
                    return QVariant("");
                else
                    return QVariant(columnNames[columns[section]]);
            else
                return QVariant(columns[section]);
        }
    }
    return QVariant();
}

void PlayListModel::trackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    if (status_column != -1) {
        emit dataChanged(createIndex(0, status_column, nullptr),
                         createIndex(pltItemCount() - 1, status_column, nullptr));
    }
}

void PlayListModel::playerPaused() {
    QModelIndex index = createIndex(playingItemIndex(), status_column, nullptr);
    emit dataChanged(index, index);
}

void PlayListModel::deleteTracks(const QModelIndexList &tracks) {
    if (tracks.length() == 0)
        return;
    beginRemoveRows(QModelIndex(), tracks.first().row(), tracks.last().row());

    QModelIndex index;
    DBPltRef plt;
    foreach(index, tracks) {
        DBAPI->pl_set_selected(plt.at(index.row()), 1);
    }

    DBAPI->plt_delete_selected(plt);
    endRemoveRows();
}

void PlayListModel::sort(int column, Qt::SortOrder order) {
    if (column == status_column)
        return;
    DBPltRef plt;
    DBAPI->plt_sort(plt, PL_MAIN, -1, columns[column].toUtf8().data(), order);
    emit dataChanged(createIndex(0, 0, nullptr),
                     createIndex(plt.itemCount(), columns.count(), nullptr));
}

void PlayListModel::clearPlayList() {
    DBPltRef plt;
    beginRemoveRows(QModelIndex(), 0, plt.itemCount() - 1);
    DBAPI->plt_clear(plt);
    endRemoveRows();
}

static int addTracksByUrl(const QUrl &url, int position) {
    DBPltRef plt;
    int prev_track_count = plt.itemCount();
    DBApiWrapper::Instance()->addTracksByUrl(url, position);
    return plt.itemCount() - prev_track_count;
}

void PlayListModel::insertByURLAtPosition(const QUrl &url, int position) {
    int count = addTracksByUrl(url, position);
    beginInsertRows(QModelIndex(), position, position + count - 1);
    endInsertRows();
}

static int moveItems(const QList<int> &indices, int before) {
    uint32_t inds[indices.length()];
    for (int i = 0; i < indices.length(); i++) {
        inds[i] = indices[i];
    }

    DBPltRef plt;
    int lastItem = plt.itemCount() - 1;
    DBItemRef bef(before > lastItem
                  ? plt.at(lastItem).next()
                  : plt.at(before));

    DBAPI->pl_lock();
    DBAPI->plt_move_items(plt, PL_MAIN, plt, bef, inds, indices.length());
    DBAPI->pl_unlock();

    return lastItem;
}

void PlayListModel::moveItems(QList<int> indices, int before) {
    int lastItem = ::moveItems(indices, before);

    beginRemoveRows(QModelIndex(), 0, lastItem);
    endRemoveRows();
    beginInsertRows(QModelIndex(), 0, lastItem);
    endInsertRows();
}

QStringList PlayListModel::mimeTypes () const {
    QStringList qstrList;
    qstrList.append("playlist/track");
    qstrList.append("text/uri-list");
    return qstrList;
}

Qt::DropActions PlayListModel::supportedDropActions () const {
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *PlayListModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << index.row() << text;
        }
    }

    mimeData->setData("playlist/track", encodedData);
    return mimeData;
}
