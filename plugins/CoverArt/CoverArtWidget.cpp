#include "CoverArtWidget.h"
#include "CoverArtWrapper.h"

#include <QStyle>
#include <QPixmap>

#include "QtGui.h"
#include "CoverArtCache.h"
#include <include/callbacks.h>
#include <QEvent>
#include <QVBoxLayout>

CoverArtWidget::CoverArtWidget(QWidget *parent):
        QDockWidget(parent),
        label(this),
        updateCoverAction(tr("Update cover"), &label) {
    setObjectName("CoverArt Widget");
    
    //label.setFrameStyle(QFrame::StyledPanel|QFrame::Raised);
    
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 3, 0, 0);
    verticalLayout->addWidget(&label);
    QWidget *parentWidget = new QWidget(this);
    parentWidget->setLayout(verticalLayout);
    setWidget(parentWidget);
    
    //setWidget(&label);
    
    //double scaleFactor = this->devicePixelRatioF();
    setMinimumWidth(120);
    setMinimumHeight(120);
    setMaximumWidth(CoverArtWrapper::Instance()->defaultWidth);
    setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    label.setContextMenuPolicy(Qt::ActionsContextMenu);
    label.addAction(&updateCoverAction);
    updateCoverAction.setIcon(getStockIcon(&label, "view-refresh", QStyle::SP_MediaPlay));
    connect(DBApiWrapper::Instance(), SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)), SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    connect(CoverArtCache::Instance(this), SIGNAL(coverIsReady(const QImage &)), SLOT(setCover(const QImage &)));
    connect(&updateCoverAction, SIGNAL(triggered(bool)), SLOT(reloadCover()));
    CACHE->getDefaultCoverArt();
}

CoverArtWidget::~CoverArtWidget() {
    CACHE->Destroy();
}

void CoverArtWidget::trackChanged(DB_playItem_t *, DB_playItem_t *to) {
    if (isVisible())
        updateCover(to);
}

void CoverArtWidget::setCover(const QImage &aCover) {
    label.setBorder((((aCover.pixel(0, 0) & 0xFF000000) >> 24) > 0x40) ? this->devicePixelRatioF() : 0, QBrush(Qt::lightGray));
    label.setAlignment(1, -1);
    label.setPixmap(QPixmap::fromImage(aCover));
    
    //QPixmap coverPixmap = QPixmap::fromImage(aCover);
    //int length = std::min(coverPixmap.width(), coverPixmap.height());
    //label.setPixmap(coverPixmap.copy(QRect((coverPixmap.width()-length)/2, (coverPixmap.height()-length)/2, length, length)));
    
    //setMaximumWidth(aCover.width() + 5);
    //setMaximumHeight(aCover.height() + 25);
}

void CoverArtWidget::reloadCover() {
    DB_playItem_t *track = DBAPI->streamer_get_playing_track();
    if (!track)
        return;
    const char *album = DBAPI->pl_find_meta(track, "album");
    const char *artist = DBAPI->pl_find_meta(track, "artist");
    if (!album || !*album) {
        album = DBAPI->pl_find_meta(track, "title");
    }
    CACHE->removeCoverArt(artist, album);
    CACHE->getCoverArt(DBAPI->pl_find_meta(track, ":URI"), artist, album);
    if (track)
        DBAPI->pl_item_unref(track);
}

void CoverArtWidget::updateCover(DB_playItem_t *track) {
    if (!track)
        track = DBAPI->streamer_get_playing_track();
    else
        DBAPI->pl_item_ref(track);

    if (!track)
        return;

    const char *album = DBAPI->pl_find_meta(track, "album");
    const char *artist = DBAPI->pl_find_meta(track, "artist");
    if (!album || !*album) {
        album = DBAPI->pl_find_meta(track, "title");
    }

    CACHE->getCoverArt(DBAPI->pl_find_meta(track, ":URI"), artist, album);

    if (track)
        DBAPI->pl_item_unref(track);
}

void CoverArtWidget::closeEvent(QCloseEvent *event) {
    emit onCloseEvent();
    QDockWidget::closeEvent(event);
}

void CoverArtWidget::showEvent(QShowEvent *event) {
    updateCover();
    QDockWidget::showEvent(event);
}
