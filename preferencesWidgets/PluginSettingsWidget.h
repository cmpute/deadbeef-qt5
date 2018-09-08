#ifndef PLUGINSETTINGSWIDGET_H
#define PLUGINSETTINGSWIDGET_H

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHash>

#include "QtGui.h"

class PluginSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    PluginSettingsWidget(ddb_dialog_t *conf, QWidget *parent = 0);
    PluginSettingsWidget(ddb_dsp_context_t *dsp, QWidget *parent = 0);
    ~PluginSettingsWidget();

private:
    void configureWidgets(ddb_dialog_t *settingsDialog);
    QLabel *label;
    QWidget *prop;
    QLayout *layout;
    QHBoxLayout *hbox;
    QPushButton *btn;
    
    bool isDsp = false;
    
    void addEntryWithLabel(QLayout *layout, QLabel *label, QWidget *prop, bool HLayout = false);
    void addEntry(QLayout *layout, QWidget *prop, bool HLayout = false);

    QHash<QWidget *, QString> keys;

public Q_SLOTS:
    void saveProperty();
    void killDialog();
};

#endif // PLUGINSETTINGSWIDGET_H
