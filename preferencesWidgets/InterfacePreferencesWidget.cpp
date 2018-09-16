#include "InterfacePreferencesWidget.h"
#include "ui_InterfacePreferencesWidget.h"

#include "QtGuiSettings.h"
#include "QtGui.h"
#include "GuiUpdater.h"
#include "MainWindow.h"
#include <QDebug>

InterfacePreferencesWidget::InterfacePreferencesWidget(QWidget *parent, Qt::WindowFlags f):
        QWidget(parent, f),
        ui(new Ui::InterfacePreferencesWidget) {
            
    ui->setupUi(this);
    loadSettings();
    createConnections();
}

void InterfacePreferencesWidget::loadSettings() {
    bool trayIconIsHidden = SETTINGS->getTrayIconIsHidden();
    bool minimizeOnClose = SETTINGS->getMinimizeOnClose();
    bool showTrayTips = SETTINGS->getShowTrayTips();
    int refreshRate = SETTINGS->getRefreshRate();
    QString titlebarPlaying = SETTINGS->getTitlebarPlaying();
    QString titlebarStopped = SETTINGS->getTitlebarStopped();
    QString messageFormat = SETTINGS->getMessageFormat();
    QString TrayIconTheme = SETTINGS->getTrayIconTheme();
    
    ui->DetectCP1251CheckBox->setChecked(DBAPI->conf_get_int("junk.enable_cp1251_detection", 0));
    ui->DetectCP936CheckBox->setChecked(DBAPI->conf_get_int("junk.enable_cp936_detection", 0));
    ui->DetectShiftJISCheckBox->setChecked(DBAPI->conf_get_int("junk.enable_shift_jis_detection", 0));
    
    ui->minimizeCheckBox->setChecked(minimizeOnClose);
    ui->hideTrayCheckBox->setChecked(trayIconIsHidden);
    ui->switchTrackInfoCheckBox->setChecked(showTrayTips);
    ui->trayIconMsgFormatLineEdit->setText(messageFormat);
    ui->refreshRateSlider->setValue(refreshRate);
    ui->refreshRateValueLabel->setText(QString::number(refreshRate));
    ui->titlebarPlayingLineEdit->setText(titlebarPlaying);
    ui->titlebarStoppedLineEdit->setText(titlebarStopped);
    
    ui->TrayIconThemeLabel->setVisible(!trayIconIsHidden);
    ui->TrayIconThemeComboBox->setVisible(!trayIconIsHidden);
    QStringList trayIconTheme = MainWindow::Instance()->trayIconTheme.keys();
    trayIconTheme.sort();
    if (trayIconTheme.contains(QString("Default")))
    {
        trayIconTheme.move(trayIconTheme.indexOf(QString("Default")), 0);
    }
    ui->TrayIconThemeComboBox->addItems(trayIconTheme);
    int trayIconThemeIndex = ui->TrayIconThemeComboBox->findText(TrayIconTheme);
    if ( trayIconThemeIndex != -1 )
        ui->TrayIconThemeComboBox->setCurrentIndex(trayIconThemeIndex);
    else
        ui->TrayIconThemeComboBox->setCurrentIndex(0);
    
    ui->switchTrackInfoCheckBox->setVisible(!trayIconIsHidden);
    ui->trayIconMessageLabel->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    ui->trayIconMsgFormatLineEdit->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    
    DBAPI->conf_lock();
    const char **names = DBAPI->plug_get_gui_names();
    for (int i = 0; names[i]; i++) {
        ui->guiPluginComboBox->addItem(QString::fromUtf8(names[i]));
        if (!strcmp(names[i], DBAPI->conf_get_str_fast("gui_plugin", "Qt"))) {
            ui->guiPluginComboBox->setCurrentIndex(i);
        }
    }
    DBAPI->conf_unlock();
}

void InterfacePreferencesWidget::createConnections() {
    connect(ui->hideTrayCheckBox, SIGNAL(toggled(bool)), SLOT(saveTrayIconHidden(bool)));
    connect(ui->switchTrackInfoCheckBox, SIGNAL(toggled(bool)), SLOT(saveTrackInfoOnSwitch(bool)));
    connect(ui->minimizeCheckBox, SIGNAL(toggled(bool)), SLOT(saveCloseOnMinimize(bool)));
    connect(ui->titlebarPlayingLineEdit, SIGNAL(editingFinished()), SLOT(saveTitlePlaying()));
    connect(ui->titlebarStoppedLineEdit, SIGNAL(editingFinished()), SLOT(saveTitleStopped()));
    connect(ui->trayIconMsgFormatLineEdit, SIGNAL(editingFinished()), SLOT(saveTrayMessageFormat()));
    connect(ui->refreshRateSlider, SIGNAL(valueChanged(int)), SLOT(saveRefreshRate(int)));
    connect(ui->refreshRateSlider, &QSlider::valueChanged, [this]() { ui->refreshRateValueLabel->setText(QString::number(ui->refreshRateSlider->value())); });
    connect(ui->TrayIconThemeComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(saveTrayIconTheme(QString)));
    connect(ui->guiPluginComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(saveGuiPlugin(QString)));
    
    
    
    connect(ui->DetectCP1251CheckBox, SIGNAL(toggled(bool)), SLOT(saveDetectCP1251(bool)));
    connect(ui->DetectCP936CheckBox, SIGNAL(toggled(bool)), SLOT(saveDetectCP936(bool)));
    connect(ui->DetectShiftJISCheckBox, SIGNAL(toggled(bool)), SLOT(saveDetectShiftJIS(bool)));
}

void InterfacePreferencesWidget::saveDetectCP1251(bool enabled) {
    DBAPI->conf_set_int("junk.enable_cp1251_detection", enabled);
}

void InterfacePreferencesWidget::saveDetectCP936(bool enabled) {
    DBAPI->conf_set_int("junk.enable_cp936_detection", enabled);
}

void InterfacePreferencesWidget::saveDetectShiftJIS(bool enabled) {
    DBAPI->conf_set_int("junk.enable_shift_jis_detection", enabled);
}

void InterfacePreferencesWidget::saveTrayIconHidden(bool hidden) {
    SETTINGS->setTrayIconIsHidden(hidden);
    ui->TrayIconThemeLabel->setVisible(!hidden);
    ui->TrayIconThemeComboBox->setVisible(!hidden);
    ui->switchTrackInfoCheckBox->setVisible(!hidden);
    ui->trayIconMessageLabel->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    ui->trayIconMsgFormatLineEdit->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    emit setTrayIconHidden(hidden);
}

void InterfacePreferencesWidget::saveTrayIconTheme(const QString &variant) {
    SETTINGS->setTrayIconTheme(variant);
    emit setTrayIconTheme(variant);
}

void InterfacePreferencesWidget::saveTrackInfoOnSwitch(bool show) {
    SETTINGS->setShowTrayTips(show);
    ui->trayIconMessageLabel->setVisible(show);
    ui->trayIconMsgFormatLineEdit->setVisible(show);
}

void InterfacePreferencesWidget::saveCloseOnMinimize(bool close) {
    SETTINGS->setMinimizeOnClose(close);
    emit setCloseOnMinimize(close);
}

void InterfacePreferencesWidget::saveTitlePlaying() {
    SETTINGS->setTitlebarPlaying(ui->titlebarPlayingLineEdit->text());
    emit titlePlayingChanged();
}

void InterfacePreferencesWidget::saveTitleStopped() {
    SETTINGS->setTitlebarStopped(ui->titlebarStoppedLineEdit->text());
    emit titleStoppedChanged();
}

void InterfacePreferencesWidget::saveTrayMessageFormat() {
    SETTINGS->setMessageFormat(ui->trayIconMsgFormatLineEdit->text());
}

void InterfacePreferencesWidget::saveRefreshRate(int refreshRate) {
    SETTINGS->setRefreshRate(refreshRate);
    GuiUpdater::Instance()->resetTimer(refreshRate);
}

void InterfacePreferencesWidget::saveGuiPlugin(const QString &plugin) {
    DBAPI->conf_set_str("gui_plugin", plugin.toUtf8().constData());
}

void InterfacePreferencesWidget::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
