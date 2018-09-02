#ifndef METADATAPREFSDIALOG_H
#define METADATAPREFSDIALOG_H

#include <QDialog>

namespace Ui {
class MetadataPrefsDialog;
}

class MetadataPrefsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MetadataPrefsDialog(QWidget *parent = nullptr);
    ~MetadataPrefsDialog();

private:
    Ui::MetadataPrefsDialog *ui;
};

#endif // METADATAPREFSDIALOG_H
