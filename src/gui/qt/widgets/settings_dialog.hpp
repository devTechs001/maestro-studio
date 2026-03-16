// src/gui/qt/widgets/settings_dialog.hpp
#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QSettings>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include "maestro/core/settings.hpp"

namespace maestro::gui {

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);
    virtual void load() = 0;
    virtual void save() = 0;
    virtual void reset() = 0;
};

class GeneralSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit GeneralSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    
    QCheckBox* showSplashCheck_;
    QCheckBox* showOnboardingCheck_;
    QCheckBox* confirmExitCheck_;
    QCheckBox* loadLastProjectCheck_;
    QSpinBox* recentProjectsSpin_;
    QComboBox* languageCombo_;
    QComboBox* themeCombo_;
};

class AudioSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit AudioSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    void updateDeviceList();

    QComboBox* driverCombo_;
    QComboBox* inputDeviceCombo_;
    QComboBox* outputDeviceCombo_;
    QComboBox* sampleRateCombo_;
    QComboBox* bufferSizeCombo_;
    QSpinBox* inputChannelsSpin_;
    QSpinBox* outputChannelsSpin_;
    QCheckBox* inputMonitorCheck_;
    QDoubleSpinBox* inputGainSpin_;
    QCheckBox* ditherCheck_;
    QPushButton* updateDeviceBtn_;
};

class MidiSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit MidiSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    void updatePortList();
    
    QListWidget* inputList_;
    QListWidget* outputList_;
    QCheckBox* midiClockSendCheck_;
    QCheckBox* midiClockReceiveCheck_;
    QSpinBox* midiInLatencySpin_;
    QSpinBox* midiOutLatencySpin_;
    QCheckBox* runningStatusCheck_;
    QCheckBox* activeSensingCheck_;
};

class InterfaceSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit InterfaceSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    
    QComboBox* themeCombo_;
    QDoubleSpinBox* uiScaleSpin_;
    QSpinBox* fontSizeSpin_;
    QComboBox* fontCombo_;
    QCheckBox* tooltipsCheck_;
    QCheckBox* smoothScrollCheck_;
    QCheckBox* animateCheck_;
    QCheckBox* peakHoldCheck_;
    QSpinBox* meterDecaySpin_;
    QCheckBox* compactModeCheck_;
    QCheckBox* rememberWindowCheck_;
};

class PathSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit PathSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    
    QLineEdit* projectPathEdit_;
    QLineEdit* audioPathEdit_;
    QLineEdit* samplePathEdit_;
    QLineEdit* pluginVst2PathEdit_;
    QLineEdit* pluginVst3PathEdit_;
    QLineEdit* exportPathEdit_;
    QPushButton* projectPathBtn_;
    QPushButton* audioPathBtn_;
    QPushButton* samplePathBtn_;
    QPushButton* pluginVst2PathBtn_;
    QPushButton* pluginVst3PathBtn_;
    QPushButton* exportPathBtn_;
};

class CloudSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit CloudSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    
    QCheckBox* enabledCheck_;
    QComboBox* providerCombo_;
    QLineEdit* syncPathEdit_;
    QCheckBox* autoSyncCheck_;
    QSpinBox* syncIntervalSpin_;
    QCheckBox* syncOnSaveCheck_;
    QPushButton* authButton_;
    QLabel* authStatusLabel_;
};

class ShortcutSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit ShortcutSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    void populateShortcuts();
    
    QTreeWidget* shortcutTree_;
    QPushButton* editButton_;
    QPushButton* resetButton_;
};

class AdvancedSettingsPage : public SettingsPage {
    Q_OBJECT
public:
    explicit AdvancedSettingsPage(QWidget* parent = nullptr);
    
    void load() override;
    void save() override;
    void reset() override;

signals:
    void settingsChanged();

private:
    void setupUI();
    
    QCheckBox* debugModeCheck_;
    QCheckBox* experimentalCheck_;
    QCheckBox* autosaveCheck_;
    QSpinBox* autosaveIntervalSpin_;
    QSpinBox* maxUndoSpin_;
    QCheckBox* backupCheck_;
    QSpinBox* maxBackupSpin_;
    QCheckBox* crashReportCheck_;
    QCheckBox* multithreadCheck_;
    QSpinBox* workerThreadsSpin_;
    QComboBox* logLevelCombo_;
    QCheckBox* confirmExitCheck_;
};

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    
    void setCurrentCategory(SettingsCategory category);

private slots:
    void onCategoryChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onApply();
    void onOk();
    void onCancel();
    void onReset();

private:
    void setupUI();
    void loadAllSettings();
    void saveAllSettings();

    QTreeWidget* categoryTree_;
    QStackedWidget* stackedWidget_;
    
    GeneralSettingsPage* generalPage_;
    AudioSettingsPage* audioPage_;
    MidiSettingsPage* midiPage_;
    InterfaceSettingsPage* interfacePage_;
    PathSettingsPage* pathPage_;
    CloudSettingsPage* cloudPage_;
    ShortcutSettingsPage* shortcutPage_;
    AdvancedSettingsPage* advancedPage_;
    
    QPushButton* applyButton_;
    QPushButton* resetButton_;
    
    bool modified_ = false;
};

} // namespace maestro::gui
