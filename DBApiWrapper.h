#ifndef DBAPIWRAPPER_H
#define DBAPIWRAPPER_H

#include <deadbeef/deadbeef.h>
#include <QObject>

#define WRAPPER DBApiWrapper::Instance()

class QUrl;

class DBApiWrapper : public QObject {

    Q_OBJECT

public:
    static DBApiWrapper *Instance();
    static void Destroy();

    bool isPaused;

    void sendPlayMessage(uint32_t id);

    void playTrackByIndex(int index);

    void addTracksByUrl(const QUrl &url, int position = -1);
    
    static int onSongChanged(ddb_event_trackchange_t *ev);
    static int onPause();
    static int onPlaylistChanged();
    static int onDeadbeefActivated();
    
private:
    DBApiWrapper();
    static DBApiWrapper *instance;

signals:
    void playlistChanged();
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void playbackPaused();
    void deadbeefActivated();
};

#define MOVE_ONLY(Class)                        \
    Class(const Class &) = delete;              \
    Class &operator=(const Class &) = delete;   \
    Class(Class &&) = default;                  \
    Class &operator=(Class &&) = default;

class DBItemRef {
public:
    MOVE_ONLY(DBItemRef);
    explicit DBItemRef(DB_playItem_t *track) : item(track) {}
    ~DBItemRef();

    operator DB_playItem_t*() { return item; }
    DBItemRef next() const;

    static DBItemRef playing();
private:
    DB_playItem_t *item;
};

class DBPltRef {
public:
    MOVE_ONLY(DBPltRef);
    DBPltRef();
    ~DBPltRef();
    int itemCount() const;
    DBItemRef at(int idx) const;

    operator ddb_playlist_t*() { return plt; }
private:
    ddb_playlist_t *plt;
};

extern DB_functions_t *deadbeef;

static inline int playingItemIndex() {
    return deadbeef->plt_get_item_idx(DBPltRef(), DBItemRef::playing(), PL_MAIN);
}

static inline int pltItemCount() {
    return DBPltRef().itemCount();
}

// DBItemRef implementation
inline DBItemRef::~DBItemRef() {
    if (item)
        deadbeef->pl_item_unref(item);
}
inline DBItemRef DBItemRef::next() const {
    return DBItemRef(deadbeef->pl_get_next(item, PL_MAIN));
}
inline DBItemRef DBItemRef::playing() {
    return DBItemRef(deadbeef->streamer_get_playing_track());
}

// DBPltRef implementation
inline DBPltRef::DBPltRef() : plt(deadbeef->plt_get_curr()) {
}
inline DBPltRef::~DBPltRef() {
    if (plt)
        deadbeef->plt_unref(plt);
}
inline int DBPltRef::itemCount() const {
    return deadbeef->plt_get_item_count(plt, PL_MAIN);
}
inline DBItemRef DBPltRef::at(int idx) const {
    return DBItemRef(deadbeef->plt_get_item_for_idx(plt, idx, PL_MAIN));
}

#endif // DBAPIWRAPPER_H
