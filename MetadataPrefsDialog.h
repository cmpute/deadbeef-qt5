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
    
public slots:
    void reject();
    void accept();

private:
    Ui::MetadataPrefsDialog *ui;
    bool dirtyBit = false;
private slots:
    void setDirty();
    void saveConfig();
};

#endif // METADATAPREFSDIALOG_H
