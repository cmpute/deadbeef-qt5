#include "DspPreferencesWidget.h"
#include "ui_DspPreferencesWidget.h"

#include "include/callbacks.h"

#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>

extern ddb_dsp_context_t *dsp_chain;

DspPreferencesWidget::DspPreferencesWidget(QWidget* parent, Qt::WindowFlags f):
        QWidget(parent, f),
        ui(new Ui::DspPreferencesWidget)
{
    ui->setupUi(this);
    model = new QStandardItemModel(this);
    
    loadSettings();
    createConnections();
    ui->dspListView->setModel(model);
}

DspPreferencesWidget::~DspPreferencesWidget() {
    DBAPI->dsp_preset_free(chain);
    dsp_chain = NULL;
}

void DspPreferencesWidget::loadSettings() {
    //ui->enableProxyCheckBox->setChecked(DBAPI->conf_get_int("network.proxy", 0));
    ddb_dsp_context_t *streamer_chain = DBAPI->streamer_get_dsp_chain();
    ddb_dsp_context_t *tail = NULL;
    while (streamer_chain) {
        ddb_dsp_context_t *new_dsp = dsp_clone(streamer_chain);
        if (tail) {
            tail->next = new_dsp;
            tail = new_dsp;
        }
        else {
            chain = tail = new_dsp;
        }
        streamer_chain = streamer_chain->next;
    }
    
    dsp_chain = chain;
    
    fill_dsp_chain(model);
    
    //DBAPI->conf_lock();
    //DBAPI->conf_unlock();
    
    //fill presets
    fillPresets();
}

void DspPreferencesWidget::fillPresets() {
    ui->presetsComboBox->clear();
    dspPresetDir = QDir(QString(DBAPI->get_system_dir(DDB_SYS_DIR_CONFIG))+QString("/presets/dsp/"));
    dspPresetDir.setFilter(QDir::Files|QDir::NoSymLinks);
    dspPresetDir.setNameFilters(QStringList(QString("*.txt")));
    foreach (QString fileName, dspPresetDir.entryList())
        ui->presetsComboBox->addItem(QFileInfo(fileName).completeBaseName());
}

void DspPreferencesWidget::fill_dsp_chain(QStandardItemModel *mdl)
{
    
    ddb_dsp_context_t *dsp = chain;
    model->clear();
    //int i = 0;
    while (dsp) {
        //dsp->plugin->plugin.name
        QStandardItem *item = new QStandardItem(QString(dsp->plugin->plugin.name));
        item->setFlags(item->flags()^Qt::ItemIsEditable);
        //item->setData(i);
        model->appendRow(item);
        //qDebug() << i << dsp->plugin->plugin.name;
        dsp = dsp->next;
        //i++;
    }
}

ddb_dsp_context_t *DspPreferencesWidget::dsp_clone(ddb_dsp_context_t *from) {
    ddb_dsp_context_t *dsp = from->plugin->open();
    char param[2000];
    if (from->plugin->num_params) {
        int n = from->plugin->num_params();
        for (int i = 0; i < n; i++) {
            from->plugin->get_param(from, i, param, sizeof(param));
            dsp->plugin->set_param(dsp, i, param);
        }
    }
    dsp->enabled = from->enabled;
    return dsp;
}

void DspPreferencesWidget::createConnections() {
    connect(ui->dspAddBtn, SIGNAL(clicked()), SLOT(addDsp()));
    connect(ui->dspRmBtn, SIGNAL(clicked()), SLOT(rmDsp()));
    connect(ui->dspConfBtn, SIGNAL(clicked()), SLOT(openDspConf()));
    connect(ui->dspUpBtn, SIGNAL(clicked()), SLOT(dspUp()));
    connect(ui->dspDownBtn, SIGNAL(clicked()), SLOT(dspDown()));
    connect(ui->presetDelBtn, SIGNAL(clicked()), SLOT(deletePreset()));
    connect(ui->presetSaveBtn, SIGNAL(clicked()), SLOT(savePreset()));
    connect(ui->presetLoadBtn, SIGNAL(clicked()), SLOT(loadPreset()));
}

void DspPreferencesWidget::loadPreset() {
    int idx = ui->presetsComboBox->currentIndex();
    QString presetName = ui->presetsComboBox->itemText(idx);
    if (!presetName.isEmpty())
    {
        QString Fname = dspPresetDir.absoluteFilePath(presetName+QString(".txt"));
        QByteArray FnameArr = Fname.toUtf8();
        const char *fname = FnameArr.constData();
        ddb_dsp_context_t *new_chain = NULL;
        int res = DBAPI->dsp_preset_load(fname, &new_chain);
        
        if (!res) {
            DBAPI->dsp_preset_free(chain);
            chain = new_chain;
            fill_dsp_chain(model);
            update_streamer();
        }
    }
}

void DspPreferencesWidget::deletePreset() {
    int removedIndex = ui->presetsComboBox->currentIndex();
    QString presetName = ui->presetsComboBox->itemText(removedIndex);
    if (!presetName.isEmpty())
    {
        dspPresetDir.remove(ui->presetsComboBox->itemText(removedIndex) + QString(".txt"));
        fillPresets();
        //ui->presetsComboBox->removeItem(removedIndex);
    }
}

void DspPreferencesWidget::savePreset() {
    int idx = ui->presetsComboBox->currentIndex();
    QString presetName = ui->presetsComboBox->itemText(idx);
    
    bool ok;
    QString newPreset = QInputDialog::getText(this, tr("Preset name to be save to"),
                                         tr("Type in the preset name:"), QLineEdit::Normal, presetName, &ok);
    if (ok && !newPreset.isEmpty())
    {
        QString Fname = dspPresetDir.absoluteFilePath(newPreset+QString(".txt"));
        QByteArray FnameArr = Fname.toUtf8();
        const char *fname = FnameArr.constData();
        DBAPI->dsp_preset_save(fname, chain);
        fillPresets();
    }
}

void DspPreferencesWidget::dspUp() {
    const auto selected = ui->dspListView->selectionModel()->selectedIndexes();
    int dsp_index;
    if (!selected.isEmpty()) {
        dsp_index = selected[0].row();
    }
    else
        return;
    
    if (dsp_index == 0) {
        return;
    }

    if (-1 == swap_items(dsp_index-1))
        return;
    update_streamer();
    ui->dspListView->setCurrentIndex(model->index(dsp_index-1, 0));
}

void DspPreferencesWidget::dspDown() {
    const auto selected = ui->dspListView->selectionModel()->selectedIndexes();
    int dsp_index;
    if (!selected.isEmpty()) {
        dsp_index = selected[0].row();
    }
    else
        return;
    
    if (dsp_index == model->rowCount()) {
        return;
    }
    
    if (-1 == swap_items(dsp_index))
        return;
    update_streamer();
    ui->dspListView->setCurrentIndex(model->index(dsp_index+1, 0));
    
}

void DspPreferencesWidget::rmDsp() {
    const auto selected = ui->dspListView->selectionModel()->selectedIndexes();
    int dsp_index;
    if (!selected.isEmpty()) {
        dsp_index = selected[0].row();
    }
    else
        return;
    
    ddb_dsp_context_t *p = chain;
    ddb_dsp_context_t *prev = NULL;
    int i = dsp_index;
    while (p && i--) {
        prev = p;
        p = p->next;
    }
    if (p) {
        if (prev) {
            prev->next = p->next;
        }
        else {
            chain = p->next;
        }
        p->plugin->close(p);
        update_streamer();
        model->removeRow(dsp_index);
    }
    
}

void DspPreferencesWidget::addDsp() {
    
    QStringList pluginNames;
    
    struct DB_dsp_s **dsp_list = deadbeef->plug_get_dsp_list();
    int i;
    for (i=0;dsp_list[i];i++) {
        pluginNames << QString(dsp_list[i]->plugin.name);
    }
    //qDebug() << pluginNames;
    bool ok;
    int curr = DBAPI->conf_get_int("converter.last_selected_dsp", 0);
    QString itemText = QInputDialog::getItem(this, tr("Choose dsp plugin"),
                                         tr("Choose dsp plugin to add:"), pluginNames, curr, false, &ok);
    if (ok && !itemText.isEmpty())
    {
        ddb_dsp_context_t *inst = NULL;
        for (int i=0;dsp_list[i];i++) {
            if (QString(dsp_list[i]->plugin.name) == itemText) {
                inst = dsp_list[i]->open();
                break;
            }
        }
        if (inst) {
            // append to DSP chain
            ddb_dsp_context_t *tail = chain;
            while (tail && tail->next) {
                tail = tail->next;
            }
            if (tail) {
                tail->next = inst;
            }
            else {
                chain = inst;
            }
            QStandardItem *item = new QStandardItem(QString(itemText));
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            model->appendRow(item);
            update_streamer();
        }
    }
}

void DspPreferencesWidget::openDspConf() {
    const auto selected = ui->dspListView->selectionModel()->selectedIndexes();
    int dsp_index;
    if (!selected.isEmpty()) {
        dsp_index = selected[0].row();
    }
    else
        return;
    
    
    ddb_dsp_context_t *dsp = chain;
    //ddb_dsp_context_t *prev = NULL;
    
    int i = 0;
    while (i<dsp_index)
    {
        //prev = dsp;
        dsp = dsp->next;
        i++;
    }
    
    
    if (dsp->plugin->configdialog) {
        //qDebug() << dsp_index << dsp->plugin->plugin.name;
        QDialog *dspDialog = new QDialog(this);
        dspDialog->setWindowFlags(dspDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        dspDialog->setAttribute(Qt::WA_DeleteOnClose);
        PluginSettingsWidget *dspPrefsWidget = new PluginSettingsWidget(dsp, dspDialog);
        QVBoxLayout *layout = new QVBoxLayout;
        QDialogButtonBox *buttonBox = new QDialogButtonBox(dspDialog);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        //connect(buttonBox, SIGNAL(accepted()), dspDialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(accepted()), dspPrefsWidget, SLOT(killDialog()));
        layout->addWidget(dspPrefsWidget);
        layout->addWidget(buttonBox);
        dspDialog->setWindowTitle(QString(dsp->plugin->plugin.name));
        dspDialog->setLayout(layout);
        dspDialog->exec();
        //delete dspDialog;
    }
}


int DspPreferencesWidget::swap_items(int idx) {

    ddb_dsp_context_t *prev = NULL;
    ddb_dsp_context_t *p = chain;

    int n = idx;
    while (n > 0 && p) {
        prev = p;
        p = p->next;
        n--;
    }

    if (!p || !p->next)
        return -1;

    ddb_dsp_context_t *moved = p->next;

    if (!moved)
        return -1;

    ddb_dsp_context_t *last = moved ? moved->next : NULL;

    if (prev) {
        p->next = last;
        prev->next = moved;
        moved->next = p;
    }
    else {
        p->next = last;
        chain = moved;
        moved->next = p;
    }
    fill_dsp_chain(model);
    return 0;
}

void DspPreferencesWidget::update_streamer() {
    DBAPI->streamer_set_dsp_chain(chain);
    DBAPI->sendmessage(DB_EV_DSPCHAINCHANGED, 0, 0, 0);
}
