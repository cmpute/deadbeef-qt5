#include "DBFileDialog.h"
#include "QtGui.h"

#include <QUrl>
#include <QFileInfo>

DBFileDialog::DBFileDialog(QWidget *parent,
                           const QString &caption,
                           const QStringList &filters,
                           FileMode mode, QFileDialog::Options options): QFileDialog(parent, caption, QString())
{
    char buf[PATH_MAX];
    DBAPI->conf_get_str("filechooser.lastdir", "./", buf, sizeof(buf));
    QUrl lasturl = QUrl(QString::fromUtf8(buf));
    setDirectory(lasturl.path());
    setFileMode(mode);
    setOptions(options);
    setNameFilters(filters);
}

int DBFileDialog::exec() {
    int ret = QFileDialog::exec();
    if (!ret)
        return ret;

    if (!selectedFiles().isEmpty()) {
        QFileInfo fileInfo(selectedFiles().last());
        DBAPI->conf_set_str("filechooser.lastdir", fileInfo.path().toUtf8().constData());
    }

    return ret;
}
