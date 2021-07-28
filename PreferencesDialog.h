#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>

#include <QStyle>

#include "QtGui.h"

#include "preferencesWidgets/InterfacePreferencesWidget.h"
#include "preferencesWidgets/SoundPreferencesWidget.h"
#include "preferencesWidgets/DspPreferencesWidget.h"
#include "preferencesWidgets/NetworkPreferencesWidget.h"
#include "preferencesWidgets/PluginsPreferencesWidget.h"

#ifdef HOTKEYS_ENABLED
#include "plugins/Hotkeys/HotkeysWidget.h"
#endif

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog {
    Q_OBJECT
public:
    PreferencesDialog(QWidget *parent = 0);

private:
    QVBoxLayout vbox;
    QTabWidget tabWidget;
    QDialogButtonBox buttonBox;

    InterfacePreferencesWidget interfaceWidget;
    SoundPreferencesWidget soundWidget;
    DspPreferencesWidget dspWidget;
    NetworkPreferencesWidget networkWidget;
#ifdef HOTKEYS_ENABLED
    HotkeysWidget hotkeysWidget;
#endif
    PluginsPreferencesWidget pluginsWidget;

    void configureTabs();
    void configureLayout();
    void configureConnections();
    
private Q_SLOTS:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

Q_SIGNALS:
    void setTrayIconHidden(bool);
    void setTrayIconTheme(const QString &);
    void setCloseOnMinimize(bool);
    void titlePlayingChanged();
    void titleStoppedChanged();
    void refreshRateChanged(const QString &);
};

#endif // PREFERENCESDIALOG_H
