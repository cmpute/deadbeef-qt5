#include "SoundPreferencesWidget.h"
#include "ui_SoundPreferencesWidget.h"

#include "QtGui.h"

static void enumSoundcardCallback(const char *name, const char *desc, void *userdata) {
    SoundPreferencesWidget *dialog = (SoundPreferencesWidget *) (userdata);
    dialog->addDevice(name, desc);
}

SoundPreferencesWidget::SoundPreferencesWidget(QWidget* parent, Qt::WindowFlags f):
        QWidget(parent, f),
        ui(new Ui::SoundPreferencesWidget) {

    ui->setupUi(this);
    loadSettings();
    createConnections();
}

void SoundPreferencesWidget::loadSettings() {
    DBAPI->conf_lock();
    QStringList sampleRates = {"44100", "48000", "88200", "96000", "176400", "192000"};
    QString s = QString::fromUtf8(DBAPI->conf_get_str_fast("alsa_soundcard", "default"));
    const char *outplugname = DBAPI->conf_get_str_fast("output_plugin", "ALSA output plugin");
    ui->addToPlaylistLineEdit->setText(DBAPI->conf_get_str_fast("cli_add_playlist_name", "Default"));
    ui->replaygainModeComboBox->setCurrentIndex(DBAPI->conf_get_int("replaygain_mode", 0));
    ui->peakScaleCheckBox->setChecked(DBAPI->conf_get_int("replaygain_scale", 1));
    ui->preampSlider->setValue(DBAPI->conf_get_int("replaygain_preamp", 0));
    ui->dontAddFromArchCheckBox->setChecked(DBAPI->conf_get_int("ignore_archives", 1));
    int active_1 = DBAPI->conf_get_int("cli_add_to_specific_playlist", 1);
    ui->addToPlaylistCheckBox->setChecked(active_1);
    ui->resumeOnStartupCheckBox->setChecked(DBAPI->conf_get_int("resume_last_session", 0));
    
    QIntValidator* iValid = new QIntValidator(44100, 192000, this);
    ui->checkBox8to16->setChecked(DBAPI->conf_get_int("streamer.8_to_16",1));
    ui->checkBox16to24->setChecked(DBAPI->conf_get_int("streamer.16_to_24",0));
    int active_2 = DBAPI->conf_get_int("streamer.override_samplerate",0);
    ui->checkBoxOverrideSR->setChecked(active_2);
    ui->comboBoxTargetSR->clear();
    ui->comboBoxTargetSR->addItems(sampleRates);
    ui->comboBoxTargetSR->setValidator(iValid);
    ui->comboBoxTargetSR->setEditText(DBAPI->conf_get_str_fast("streamer.samplerate","44100"));
    int active_3 = DBAPI->conf_get_int("streamer.use_dependent_samplerate",0);
    ui->checkBoxDependentSR->setChecked(active_3);
    ui->comboBoxSRMulti48->clear();
    ui->comboBoxSRMulti48->addItems(sampleRates);
    ui->comboBoxSRMulti48->setValidator(iValid);
    ui->comboBoxSRMulti48->setEditText(DBAPI->conf_get_str_fast("streamer.samplerate_mult_48","48000"));
    ui->comboBoxSRMulti44->clear();
    ui->comboBoxSRMulti44->addItems(sampleRates);
    ui->comboBoxSRMulti44->setValidator(iValid);
    ui->comboBoxSRMulti44->setEditText(DBAPI->conf_get_str_fast("streamer.samplerate_mult_44","44100"));
    
    ui->addToPlaylistLineEdit->setEnabled(active_1);
    ui->frameOverrideSR->setEnabled(active_2);
    ui->frameDependentSR->setEnabled(active_3);
    DBAPI->conf_unlock();
    
    alsaDevices.insert("default", "Default Audio Device");
    
    if (DBAPI->get_output()->enum_soundcards) {
        DBAPI->get_output()->enum_soundcards(enumSoundcardCallback, this);
        ui->outputDeviceComboBox->setEnabled(true);
    }
    else
        ui->outputDeviceComboBox->setEnabled(false);
    
    foreach (QString device, alsaDevices.values()) {
        ui->outputDeviceComboBox->addItem(device);
        if (s == alsaDevices.key(device))
            ui->outputDeviceComboBox->setCurrentIndex(ui->outputDeviceComboBox->count() - 1);
    }
    
    DB_output_t **out_plugs = DBAPI->plug_get_output_list();
    for (int i = 0; out_plugs[i]; i++) {
        ui->outputPluginComboBox->addItem(QString::fromUtf8(out_plugs[i]->plugin.name));
        if (!strcmp(outplugname, out_plugs[i]->plugin.name))
            ui->outputPluginComboBox->setCurrentIndex(i);
    }
    
    if (ui->replaygainModeComboBox->currentIndex() == 0) {
        ui->peakScaleCheckBox->setVisible(false);
        ui->preampLabel->setVisible(false);
        ui->preampSlider->setVisible(false);
        ui->preampValueLabel->setVisible(false);
    }
}

void SoundPreferencesWidget::addDevice(const char *name, const char *desc) {
    alsaDevices.insert(QString::fromUtf8(name), QString::fromUtf8(desc));
}

void SoundPreferencesWidget::createConnections() {
    connect(ui->outputDeviceComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeOutputDevice(int)));
    connect(ui->outputPluginComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeOutputPlugin(int)));
    connect(ui->replaygainModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeReplaygainMode(int)));
    connect(ui->peakScaleCheckBox, SIGNAL(toggled(bool)), SLOT(saveReplaygainScale(bool)));
    connect(ui->preampSlider, SIGNAL(sliderReleased()), SLOT(saveReplaygainPreamp()));
    connect(ui->addToPlaylistCheckBox, SIGNAL(toggled(bool)), SLOT(saveAddToDefaultPlaylist(bool)));
    connect(ui->addToPlaylistLineEdit, SIGNAL(editingFinished()), SLOT(saveDefaultPlaylistName()));
    connect(ui->dontAddFromArchCheckBox, SIGNAL(toggled(bool)), SLOT(saveDontAddArchives(bool)));
    connect(ui->resumeOnStartupCheckBox, SIGNAL(toggled(bool)), SLOT(saveResumeOnStartup(bool)));
    
    connect(ui->comboBoxTargetSR, SIGNAL(currentTextChanged(const QString &)), SLOT(saveTargetSR()));
    connect(ui->comboBoxSRMulti48, SIGNAL(currentTextChanged(const QString &)), SLOT(saveSRMulti48()));
    connect(ui->comboBoxSRMulti44, SIGNAL(currentTextChanged(const QString &)), SLOT(saveSRMulti44()));
    connect(ui->checkBox8to16, SIGNAL(toggled(bool)), SLOT(save8to16(bool)));
    connect(ui->checkBox16to24, SIGNAL(toggled(bool)), SLOT(save16to24(bool)));
    connect(ui->checkBoxOverrideSR, SIGNAL(toggled(bool)), SLOT(saveOverrideSR(bool)));
    connect(ui->checkBoxDependentSR, SIGNAL(toggled(bool)), SLOT(saveDependentSR(bool)));
}

void SoundPreferencesWidget::save8to16(bool enabled) {
    DBAPI->conf_set_int("streamer.8_to_16", enabled);
}
void SoundPreferencesWidget::save16to24(bool enabled) {
    DBAPI->conf_set_int("streamer.16_to_24", enabled);
}
void SoundPreferencesWidget::saveOverrideSR(bool enabled) {
    ui->frameOverrideSR->setEnabled(enabled);
    DBAPI->conf_set_int("streamer.override_samplerate", enabled);
}
void SoundPreferencesWidget::saveTargetSR() {
    DBAPI->conf_set_str("streamer.samplerate", ui->comboBoxTargetSR->currentText().toUtf8().constData());
}
void SoundPreferencesWidget::saveDependentSR(bool enabled) {
    ui->frameDependentSR->setEnabled(enabled);
    DBAPI->conf_set_int("streamer.use_dependent_samplerate", enabled);
}
void SoundPreferencesWidget::saveSRMulti48() {
    DBAPI->conf_set_str("streamer.samplerate_mult_48", ui->comboBoxSRMulti48->currentText().toUtf8().constData());
}
void SoundPreferencesWidget::saveSRMulti44() {
    DBAPI->conf_set_str("streamer.samplerate_mult_44", ui->comboBoxSRMulti44->currentText().toUtf8().constData());
}
    
void SoundPreferencesWidget::changeOutputDevice(int deviceNum) {
    DBAPI->conf_set_str("alsa_soundcard", alsaDevices.key(ui->outputDeviceComboBox->currentText()).toUtf8().constData());
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::changeOutputPlugin(int pluginNum) {
    DB_output_t **out_plugs = DBAPI->plug_get_output_list();
    DBAPI->conf_set_str("output_plugin", out_plugs[pluginNum]->plugin.name);
    DBAPI->sendmessage(DB_EV_REINIT_SOUND, 0, 0, 0);
}

void SoundPreferencesWidget::changeReplaygainMode(int index) {
    ui->peakScaleCheckBox->setVisible(index > 0);
    ui->preampLabel->setVisible(index > 0);
    ui->preampSlider->setVisible(index > 0);
    ui->preampValueLabel->setVisible(index > 0);
    DBAPI->conf_set_int("replaygain_mode", index);
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::saveReplaygainScale(bool enabled) {
    DBAPI->conf_set_int("replaygain_scale", enabled);
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::saveReplaygainPreamp() {
    DBAPI->conf_set_float("replaygain_preamp", ui->preampSlider->value());
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::saveAddToDefaultPlaylist(bool enabled) {
    DBAPI->conf_set_int("cli_add_to_specific_playlist", enabled);
    ui->addToPlaylistLineEdit->setEnabled(enabled);
}

void SoundPreferencesWidget::saveDefaultPlaylistName() {
    DBAPI->conf_set_str("cli_add_playlist_name", ui->addToPlaylistLineEdit->text().toUtf8().constData());
}

void SoundPreferencesWidget::saveDontAddArchives(bool enabled) {
    DBAPI->conf_set_int("ignore_archives", enabled);
}

void SoundPreferencesWidget::saveResumeOnStartup(bool enabled) {
    DBAPI->conf_set_int("resume_last_session", enabled);
}

void SoundPreferencesWidget::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
