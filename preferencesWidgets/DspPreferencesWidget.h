#ifndef DSPPREFERENCESWIDGET_H
#define DSPPREFERENCESWIDGET_H

#include <QDir>
#include <QWidget>
#include <QStandardItemModel>

#include "PluginSettingsWidget.h"
#include "QtGui.h"

namespace Ui {
    class DspPreferencesWidget;
}

class DspPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    DspPreferencesWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~DspPreferencesWidget();
private:
    Ui::DspPreferencesWidget *ui;

    void loadSettings();
    void createConnections();
    
    QStandardItemModel *model;
    
    ddb_dsp_context_t *dsp_clone(ddb_dsp_context_t *from);
    ddb_dsp_context_t *chain;
    int swap_items(int idx);
    void fill_dsp_chain(QStandardItemModel *mdl);
    
    QDir dspPresetDir;
    void fillPresets();
    
    void update_streamer();
    
protected:
    
private Q_SLOTS:
    void openDspConf();
    void addDsp();
    void rmDsp();
    void dspUp();
    void dspDown();
    void deletePreset();
    void savePreset();
    void loadPreset();
};

#endif // DSPPREFERENCESWIDGET_H
