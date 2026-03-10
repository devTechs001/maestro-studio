// src/gui/qt/widgets/settings_dialog.cpp
#include "settings_dialog.hpp"
#include "maestro/core/settings.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QSplitter>

namespace maestro::gui {

// SettingsPage base implementation
SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent) {}

// GeneralSettingsPage implementation
GeneralSettingsPage::GeneralSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void GeneralSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Startup group
    auto* startupGroup = new QGroupBox("Startup");
    auto* startupLayout = new QVBoxLayout(startupGroup);
    
    showSplashCheck_ = new QCheckBox("Show splash screen on startup");
    startupLayout->addWidget(showSplashCheck_);
    
    showOnboardingCheck_ = new QCheckBox("Show onboarding wizard on first run");
    startupLayout->addWidget(showOnboardingCheck_);
    
    loadLastProjectCheck_ = new QCheckBox("Load last project on startup");
    startupLayout->addWidget(loadLastProjectCheck_);
    
    layout->addWidget(startupGroup);
    
    // Behavior group
    auto* behaviorGroup = new QGroupBox("Behavior");
    auto* behaviorLayout = new QVBoxLayout(behaviorGroup);
    
    confirmExitCheck_ = new QCheckBox("Confirm on exit");
    behaviorLayout->addWidget(confirmExitCheck_);
    
    auto* recentLayout = new QHBoxLayout();
    recentLayout->addWidget(new QLabel("Recent projects to show:"));
    recentProjectsSpin_ = new QSpinBox();
    recentProjectsSpin_->setRange(0, 50);
    recentProjectsSpin_->setValue(10);
    recentLayout->addWidget(recentProjectsSpin_);
    behaviorLayout->addLayout(recentLayout);
    
    layout->addWidget(behaviorGroup);
    
    // Appearance group
    auto* appearanceGroup = new QGroupBox("Appearance");
    auto* appearanceLayout = new QVBoxLayout(appearanceGroup);
    
    auto* themeLayout = new QHBoxLayout();
    themeLayout->addWidget(new QLabel("Theme:"));
    themeCombo_ = new QComboBox();
    themeCombo_->addItems({"Dark", "Light", "System"});
    themeLayout->addWidget(themeCombo_);
    themeLayout->addStretch();
    appearanceLayout->addLayout(themeLayout);
    
    auto* languageLayout = new QHBoxLayout();
    languageLayout->addWidget(new QLabel("Language:"));
    languageCombo_ = new QComboBox();
    languageCombo_->addItems({"English", "Deutsch", "Français", "Español", "日本語", "中文"});
    languageLayout->addWidget(languageCombo_);
    languageLayout->addStretch();
    appearanceLayout->addLayout(languageLayout);
    
    layout->addWidget(appearanceGroup);
    
    layout->addStretch();
}

void GeneralSettingsPage::load() {
    auto& settings = maestro::SettingsManager::instance().interface();
    showSplashCheck_->setChecked(settings.showSplashScreen);
    showOnboardingCheck_->setChecked(settings.showOnboarding);
    loadLastProjectCheck_->setChecked(maestro::SettingsManager::instance().advanced().loadLastProject);
    recentProjectsSpin_->setValue(settings.recentProjectsCount);
    confirmExitCheck_->setChecked(maestro::SettingsManager::instance().advanced().confirmOnExit);
    
    if (settings.theme == "dark") themeCombo_->setCurrentIndex(0);
    else if (settings.theme == "light") themeCombo_->setCurrentIndex(1);
    else themeCombo_->setCurrentIndex(2);
}

void GeneralSettingsPage::save() {
    auto& settings = maestro::SettingsManager::instance().interface();
    settings.showSplashScreen = showSplashCheck_->isChecked();
    settings.showOnboarding = showOnboardingCheck_->isChecked();
    settings.recentProjectsCount = recentProjectsSpin_->value();
    
    if (themeCombo_->currentIndex() == 0) settings.theme = "dark";
    else if (themeCombo_->currentIndex() == 1) settings.theme = "light";
    else settings.theme = "system";
    
    maestro::SettingsManager::instance().advanced().loadLastProject = loadLastProjectCheck_->isChecked();
    maestro::SettingsManager::instance().advanced().confirmOnExit = confirmExitCheck_->isChecked();
    
    emit settingsChanged();
}

void GeneralSettingsPage::reset() {
    showSplashCheck_->setChecked(true);
    showOnboardingCheck_->setChecked(true);
    loadLastProjectCheck_->setChecked(false);
    recentProjectsSpin_->setValue(10);
    confirmExitCheck_->setChecked(true);
    themeCombo_->setCurrentIndex(0);
}

// AudioSettingsPage implementation
AudioSettingsPage::AudioSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void AudioSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Driver group
    auto* driverGroup = new QGroupBox("Audio Driver");
    auto* driverLayout = new QFormLayout(driverGroup);
    
    driverCombo_ = new QComboBox();
    driverCombo_->addItems({"Auto", "ASIO", "WASAPI", "Core Audio", "ALSA", "JACK"});
    driverLayout->addRow("Driver:", driverCombo_);
    
    updateDeviceBtn_ = new QPushButton("Refresh Devices");
    connect(updateDeviceBtn_, &QPushButton::clicked, this, &AudioSettingsPage::updateDeviceList);
    driverLayout->addRow("", updateDeviceBtn_);
    
    layout->addWidget(driverGroup);
    
    // Devices group
    auto* devicesGroup = new QGroupBox("Devices");
    auto* devicesLayout = new QFormLayout(devicesGroup);
    
    inputDeviceCombo_ = new QComboBox();
    devicesLayout->addRow("Input Device:", inputDeviceCombo_);
    
    outputDeviceCombo_ = new QComboBox();
    devicesLayout->addRow("Output Device:", outputDeviceCombo_);
    
    layout->addWidget(devicesGroup);
    
    // Settings group
    auto* settingsGroup = new QGroupBox("Audio Settings");
    auto* settingsLayout = new QFormLayout(settingsGroup);
    
    sampleRateCombo_ = new QComboBox();
    sampleRateCombo_->addItems({"44100", "48000", "88200", "96000", "176400", "192000"});
    sampleRateCombo_->setCurrentIndex(1);
    settingsLayout->addRow("Sample Rate:", sampleRateCombo_);
    
    bufferSizeCombo_ = new QComboBox();
    bufferSizeCombo_->addItems({"64", "128", "256", "512", "1024", "2048"});
    bufferSizeCombo_->setCurrentIndex(2);
    settingsLayout->addRow("Buffer Size:", bufferSizeCombo_);
    
    inputChannelsSpin_ = new QSpinBox();
    inputChannelsSpin_->setRange(0, 64);
    inputChannelsSpin_->setValue(2);
    settingsLayout->addRow("Input Channels:", inputChannelsSpin_);
    
    outputChannelsSpin_ = new QSpinBox();
    outputChannelsSpin_->setRange(1, 64);
    outputChannelsSpin_->setValue(2);
    settingsLayout->addRow("Output Channels:", outputChannelsSpin_);
    
    layout->addWidget(settingsGroup);
    
    // Monitoring group
    auto* monitoringGroup = new QGroupBox("Monitoring");
    auto* monitoringLayout = new QFormLayout(monitoringGroup);
    
    inputMonitorCheck_ = new QCheckBox("Enable input monitoring");
    monitoringLayout->addRow(inputMonitorCheck_);
    
    inputGainSpin_ = new QDoubleSpinBox();
    inputGainSpin_->setRange(0.0, 2.0);
    inputGainSpin_->setValue(1.0);
    inputGainSpin_->setSingleStep(0.1);
    monitoringLayout->addRow("Input Gain:", inputGainSpin_);
    
    layout->addWidget(monitoringGroup);
    
    // Advanced group
    auto* advancedGroup = new QGroupBox("Advanced");
    auto* advancedLayout = new QVBoxLayout(advancedGroup);
    
    ditherCheck_ = new QCheckBox("Enable dither on export");
    advancedLayout->addWidget(ditherCheck_);
    
    layout->addWidget(advancedGroup);
    
    layout->addStretch();
}

void AudioSettingsPage::updateDeviceList() {
    inputDeviceCombo_->clear();
    outputDeviceCombo_->clear();
    
    inputDeviceCombo_->addItems({"Default Input", "USB Audio Interface", "Built-in Microphone"});
    outputDeviceCombo_->addItems({"Default Output", "USB Audio Interface", "Built-in Speakers", "HDMI Output"});
}

void AudioSettingsPage::load() {
    auto& settings = maestro::SettingsManager::instance().audio();
    
    if (settings.driver == "auto") driverCombo_->setCurrentIndex(0);
    else if (settings.driver == "asio") driverCombo_->setCurrentIndex(1);
    else if (settings.driver == "wasapi") driverCombo_->setCurrentIndex(2);
    
    sampleRateCombo_->setCurrentText(QString::number(settings.sampleRate));
    bufferSizeCombo_->setCurrentText(QString::number(settings.bufferSize));
    inputChannelsSpin_->setValue(settings.inputChannels);
    outputChannelsSpin_->setValue(settings.outputChannels);
    inputMonitorCheck_->setChecked(settings.enableInputMonitoring);
    inputGainSpin_->setValue(settings.inputGain);
    ditherCheck_->setChecked(settings.ditherEnabled);
    
    updateDeviceList();
}

void AudioSettingsPage::save() {
    auto& settings = maestro::SettingsManager::instance().audio();
    
    switch (driverCombo_->currentIndex()) {
        case 0: settings.driver = "auto"; break;
        case 1: settings.driver = "asio"; break;
        case 2: settings.driver = "wasapi"; break;
        case 3: settings.driver = "coreaudio"; break;
        case 4: settings.driver = "alsa"; break;
        case 5: settings.driver = "jack"; break;
    }
    
    settings.sampleRate = sampleRateCombo_->currentText().toUInt();
    settings.bufferSize = bufferSizeCombo_->currentText().toUInt();
    settings.inputChannels = inputChannelsSpin_->value();
    settings.outputChannels = outputChannelsSpin_->value();
    settings.enableInputMonitoring = inputMonitorCheck_->isChecked();
    settings.inputGain = static_cast<float>(inputGainSpin_->value());
    settings.ditherEnabled = ditherCheck_->isChecked();
    
    emit settingsChanged();
}

void AudioSettingsPage::reset() {
    driverCombo_->setCurrentIndex(0);
    sampleRateCombo_->setCurrentIndex(1);
    bufferSizeCombo_->setCurrentIndex(2);
    inputChannelsSpin_->setValue(2);
    outputChannelsSpin_->setValue(2);
    inputMonitorCheck_->setChecked(true);
    inputGainSpin_->setValue(1.0);
    ditherCheck_->setChecked(false);
}

// MidiSettingsPage implementation
MidiSettingsPage::MidiSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void MidiSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    auto* splitter = new QSplitter(Qt::Vertical);
    
    // Ports group
    auto* portsGroup = new QGroupBox("MIDI Ports");
    auto* portsLayout = new QHBoxLayout(portsGroup);
    
    auto* inputGroup = new QGroupBox("Inputs");
    auto* inputLayout = new QVBoxLayout(inputGroup);
    inputList_ = new QListWidget();
    inputList_->setSelectionMode(QAbstractItemView::MultiSelection);
    inputLayout->addWidget(inputList_);
    
    auto* outputGroup = new QGroupBox("Outputs");
    auto* outputLayout = new QVBoxLayout(outputGroup);
    outputList_ = new QListWidget();
    outputList_->setSelectionMode(QAbstractItemView::MultiSelection);
    outputLayout->addWidget(outputList_);
    
    portsLayout->addWidget(inputGroup);
    portsLayout->addWidget(outputGroup);
    
    splitter->addWidget(portsGroup);
    
    // Settings group
    auto* settingsGroup = new QGroupBox("MIDI Settings");
    auto* settingsLayout = new QFormLayout(settingsGroup);
    
    midiClockSendCheck_ = new QCheckBox("Send MIDI Clock");
    settingsLayout->addRow(midiClockSendCheck_);
    
    midiClockReceiveCheck_ = new QCheckBox("Receive MIDI Clock");
    settingsLayout->addRow(midiClockReceiveCheck_);
    
    auto* latencyLayout = new QHBoxLayout();
    midiInLatencySpin_ = new QSpinBox();
    midiInLatencySpin_->setRange(0, 500);
    midiInLatencySpin_->setSuffix(" ms");
    latencyLayout->addWidget(new QLabel("Input Latency:"));
    latencyLayout->addWidget(midiInLatencySpin_);
    
    midiOutLatencySpin_ = new QSpinBox();
    midiOutLatencySpin_->setRange(0, 500);
    midiOutLatencySpin_->setSuffix(" ms");
    latencyLayout->addWidget(new QLabel("Output Latency:"));
    latencyLayout->addWidget(midiOutLatencySpin_);
    settingsLayout->addRow(latencyLayout);
    
    runningStatusCheck_ = new QCheckBox("Enable Running Status");
    settingsLayout->addRow(runningStatusCheck_);
    
    activeSensingCheck_ = new QCheckBox("Send Active Sensing");
    settingsLayout->addRow(activeSensingCheck_);
    
    splitter->addWidget(settingsGroup);
    
    layout->addWidget(splitter);
    layout->addStretch();
    
    updatePortList();
}

void MidiSettingsPage::updatePortList() {
    inputList_->clear();
    outputList_->clear();
    
    inputList_->addItems({
        "Microsoft GS Wavetable Synth",
        "Yamaha USB MIDI",
        "Roland USB MIDI",
        "Korg USB MIDI",
        "Virtual Input 1"
    });
    
    outputList_->addItems({
        "Microsoft GS Wavetable Synth",
        "Yamaha USB MIDI",
        "Roland USB MIDI",
        "Korg USB MIDI",
        "Virtual Output 1"
    });
}

void MidiSettingsPage::load() {
    auto& settings = maestro::SettingsManager::instance().midi();
    midiClockSendCheck_->setChecked(settings.sendMidiClock);
    midiClockReceiveCheck_->setChecked(settings.receiveMidiClock);
    midiInLatencySpin_->setValue(settings.midiInLatency);
    midiOutLatencySpin_->setValue(settings.midiOutLatency);
    runningStatusCheck_->setChecked(settings.runningStatus);
    activeSensingCheck_->setChecked(settings.activeSensing);
}

void MidiSettingsPage::save() {
    auto& settings = maestro::SettingsManager::instance().midi();
    settings.sendMidiClock = midiClockSendCheck_->isChecked();
    settings.receiveMidiClock = midiClockReceiveCheck_->isChecked();
    settings.midiInLatency = midiInLatencySpin_->value();
    settings.midiOutLatency = midiOutLatencySpin_->value();
    settings.runningStatus = runningStatusCheck_->isChecked();
    settings.activeSensing = activeSensingCheck_->isChecked();
    
    emit settingsChanged();
}

void MidiSettingsPage::reset() {
    midiClockSendCheck_->setChecked(false);
    midiClockReceiveCheck_->setChecked(true);
    midiInLatencySpin_->setValue(0);
    midiOutLatencySpin_->setValue(0);
    runningStatusCheck_->setChecked(true);
    activeSensingCheck_->setChecked(false);
}

// InterfaceSettingsPage implementation
InterfaceSettingsPage::InterfaceSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void InterfaceSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Theme group
    auto* themeGroup = new QGroupBox("Theme & Appearance");
    auto* themeLayout = new QFormLayout(themeGroup);
    
    themeCombo_ = new QComboBox();
    themeCombo_->addItems({"Dark", "Light", "System"});
    themeLayout->addRow("Theme:", themeCombo_);
    
    uiScaleSpin_ = new QDoubleSpinBox();
    uiScaleSpin_->setRange(0.75, 2.0);
    uiScaleSpin_->setValue(1.0);
    uiScaleSpin_->setSingleStep(0.25);
    uiScaleSpin_->setSuffix("x");
    themeLayout->addRow("UI Scale:", uiScaleSpin_);
    
    fontSizeSpin_ = new QSpinBox();
    fontSizeSpin_->setRange(8, 24);
    fontSizeSpin_->setValue(10);
    themeLayout->addRow("Font Size:", fontSizeSpin_);
    
    fontCombo_ = new QComboBox();
    fontCombo_->addItems({"Segoe UI", "Arial", "Helvetica", "Roboto", "System"});
    themeLayout->addRow("Font:", fontCombo_);
    
    layout->addWidget(themeGroup);
    
    // Behavior group
    auto* behaviorGroup = new QGroupBox("Interface Behavior");
    auto* behaviorLayout = new QVBoxLayout(behaviorGroup);
    
    tooltipsCheck_ = new QCheckBox("Show tooltips");
    behaviorLayout->addWidget(tooltipsCheck_);
    
    smoothScrollCheck_ = new QCheckBox("Enable smooth scrolling");
    behaviorLayout->addWidget(smoothScrollCheck_);
    
    animateCheck_ = new QCheckBox("Animate transitions");
    behaviorLayout->addWidget(animateCheck_);
    
    compactModeCheck_ = new QCheckBox("Compact mode");
    behaviorLayout->addWidget(compactModeCheck_);
    
    rememberWindowCheck_ = new QCheckBox("Remember window position and size");
    behaviorLayout->addWidget(rememberWindowCheck_);
    
    layout->addWidget(behaviorGroup);
    
    // Meters group
    auto* metersGroup = new QGroupBox("Level Meters");
    auto* metersLayout = new QFormLayout(metersGroup);
    
    peakHoldCheck_ = new QCheckBox("Show peak hold");
    metersLayout->addRow(peakHoldCheck_);
    
    meterDecaySpin_ = new QSpinBox();
    meterDecaySpin_->setRange(100, 5000);
    meterDecaySpin_->setValue(300);
    meterDecaySpin_->setSuffix(" ms");
    metersLayout->addRow("Meter decay time:", meterDecaySpin_);
    
    layout->addWidget(metersGroup);
    
    layout->addStretch();
}

void InterfaceSettingsPage::load() {
    auto& settings = maestro::SettingsManager::instance().interface();
    
    if (settings.theme == "dark") themeCombo_->setCurrentIndex(0);
    else if (settings.theme == "light") themeCombo_->setCurrentIndex(1);
    else themeCombo_->setCurrentIndex(2);
    
    uiScaleSpin_->setValue(settings.uiScale);
    fontSizeSpin_->setValue(settings.fontSize);
    tooltipsCheck_->setChecked(settings.showTooltips);
    smoothScrollCheck_->setChecked(settings.smoothScrolling);
    animateCheck_->setChecked(settings.animateTransitions);
    peakHoldCheck_->setChecked(settings.showPeakHold);
    meterDecaySpin_->setValue(settings.meterDecayTime);
    compactModeCheck_->setChecked(settings.compactMode);
    rememberWindowCheck_->setChecked(settings.rememberWindowPosition);
}

void InterfaceSettingsPage::save() {
    auto& settings = maestro::SettingsManager::instance().interface();
    
    switch (themeCombo_->currentIndex()) {
        case 0: settings.theme = "dark"; break;
        case 1: settings.theme = "light"; break;
        case 2: settings.theme = "system"; break;
    }
    
    settings.uiScale = uiScaleSpin_->value();
    settings.fontSize = fontSizeSpin_->value();
    settings.showTooltips = tooltipsCheck_->isChecked();
    settings.smoothScrolling = smoothScrollCheck_->isChecked();
    settings.animateTransitions = animateCheck_->isChecked();
    settings.showPeakHold = peakHoldCheck_->isChecked();
    settings.meterDecayTime = meterDecaySpin_->value();
    settings.compactMode = compactModeCheck_->isChecked();
    settings.rememberWindowPosition = rememberWindowCheck_->isChecked();
    
    emit settingsChanged();
}

void InterfaceSettingsPage::reset() {
    themeCombo_->setCurrentIndex(0);
    uiScaleSpin_->setValue(1.0);
    fontSizeSpin_->setValue(10);
    tooltipsCheck_->setChecked(true);
    smoothScrollCheck_->setChecked(true);
    animateCheck_->setChecked(true);
    peakHoldCheck_->setChecked(true);
    meterDecaySpin_->setValue(300);
    compactModeCheck_->setChecked(false);
    rememberWindowCheck_->setChecked(true);
}

// PathSettingsPage implementation
PathSettingsPage::PathSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void PathSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    auto* pathsGroup = new QGroupBox("Default Paths");
    auto* pathsLayout = new QFormLayout(pathsGroup);
    
    // Project path
    auto* projectLayout = new QHBoxLayout();
    projectPathEdit_ = new QLineEdit();
    projectPathBtn_ = new QPushButton("...");
    projectPathBtn_->setFixedWidth(40);
    connect(projectPathBtn_, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Project Folder");
        if (!path.isEmpty()) projectPathEdit_->setText(path);
    });
    projectLayout->addWidget(projectPathEdit_);
    projectLayout->addWidget(projectPathBtn_);
    pathsLayout->addRow("Projects:", projectLayout);
    
    // Audio path
    auto* audioLayout = new QHBoxLayout();
    audioPathEdit_ = new QLineEdit();
    audioPathBtn_ = new QPushButton("...");
    audioPathBtn_->setFixedWidth(40);
    connect(audioPathBtn_, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Audio Folder");
        if (!path.isEmpty()) audioPathEdit_->setText(path);
    });
    audioLayout->addWidget(audioPathEdit_);
    audioLayout->addWidget(audioPathBtn_);
    pathsLayout->addRow("Audio Files:", audioLayout);
    
    // Sample path
    auto* sampleLayout = new QHBoxLayout();
    samplePathEdit_ = new QLineEdit();
    samplePathBtn_ = new QPushButton("...");
    samplePathBtn_->setFixedWidth(40);
    connect(samplePathBtn_, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Samples Folder");
        if (!path.isEmpty()) samplePathEdit_->setText(path);
    });
    sampleLayout->addWidget(samplePathEdit_);
    sampleLayout->addWidget(samplePathBtn_);
    pathsLayout->addRow("Samples:", sampleLayout);
    
    // Plugin VST2 path
    auto* pluginVst2Layout = new QHBoxLayout();
    pluginVst2PathEdit_ = new QLineEdit();
    pluginVst2PathBtn_ = new QPushButton("...");
    pluginVst2PathBtn_->setFixedWidth(40);
    connect(pluginVst2PathBtn_, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select VST2 Folder");
        if (!path.isEmpty()) pluginVst2PathEdit_->setText(path);
    });
    pluginVst2Layout->addWidget(pluginVst2PathEdit_);
    pluginVst2Layout->addWidget(pluginVst2PathBtn_);
    pathsLayout->addRow("VST2 Plugins:", pluginVst2Layout);
    
    // Plugin VST3 path
    auto* pluginVst3Layout = new QHBoxLayout();
    pluginVst3PathEdit_ = new QLineEdit();
    pluginVst3PathBtn_ = new QPushButton("...");
    pluginVst3PathBtn_->setFixedWidth(40);
    connect(pluginVst3PathBtn_, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select VST3 Folder");
        if (!path.isEmpty()) pluginVst3PathEdit_->setText(path);
    });
    pluginVst3Layout->addWidget(pluginVst3PathEdit_);
    pluginVst3Layout->addWidget(pluginVst3PathBtn_);
    pathsLayout->addRow("VST3 Plugins:", pluginVst3Layout);
    
    // Export path
    auto* exportLayout = new QHBoxLayout();
    exportPathEdit_ = new QLineEdit();
    exportPathBtn_ = new QPushButton("...");
    exportPathBtn_->setFixedWidth(40);
    connect(exportPathBtn_, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Export Folder");
        if (!path.isEmpty()) exportPathEdit_->setText(path);
    });
    exportLayout->addWidget(exportPathEdit_);
    exportLayout->addWidget(exportPathBtn_);
    pathsLayout->addRow("Exports:", exportLayout);
    
    layout->addWidget(pathsGroup);
    layout->addStretch();
}

void PathSettingsPage::load() {
    auto& settings = maestro::SettingsManager::instance().paths();
    projectPathEdit_->setText(QString::fromStdString(settings.projectPath));
    audioPathEdit_->setText(QString::fromStdString(settings.audioPath));
    samplePathEdit_->setText(QString::fromStdString(settings.samplePath));
    pluginVst2PathEdit_->setText(QString::fromStdString(settings.pluginPathVst2));
    pluginVst3PathEdit_->setText(QString::fromStdString(settings.pluginPathVst3));
    exportPathEdit_->setText(QString::fromStdString(settings.exportPath));
}

void PathSettingsPage::save() {
    auto& settings = maestro::SettingsManager::instance().paths();
    settings.projectPath = projectPathEdit_->text().toStdString();
    settings.audioPath = audioPathEdit_->text().toStdString();
    settings.samplePath = samplePathEdit_->text().toStdString();
    settings.pluginPathVst2 = pluginVst2PathEdit_->text().toStdString();
    settings.pluginPathVst3 = pluginVst3PathEdit_->text().toStdString();
    settings.exportPath = exportPathEdit_->text().toStdString();
    
    emit settingsChanged();
}

void PathSettingsPage::reset() {
    projectPathEdit_->clear();
    audioPathEdit_->clear();
    samplePathEdit_->clear();
    pluginVst2PathEdit_->clear();
    pluginVst3PathEdit_->clear();
    exportPathEdit_->clear();
}

// CloudSettingsPage implementation
CloudSettingsPage::CloudSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void CloudSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Enable group
    enabledCheck_ = new QCheckBox("Enable Cloud Sync");
    connect(enabledCheck_, &QCheckBox::toggled, [this](bool checked) {
        providerCombo_->setEnabled(checked);
        syncPathEdit_->setEnabled(checked);
        autoSyncCheck_->setEnabled(checked);
        syncIntervalSpin_->setEnabled(checked);
        syncOnSaveCheck_->setEnabled(checked);
        authButton_->setEnabled(checked);
    });
    layout->addWidget(enabledCheck_);
    
    // Provider group
    auto* providerGroup = new QGroupBox("Cloud Provider");
    auto* providerLayout = new QFormLayout(providerGroup);
    
    providerCombo_ = new QComboBox();
    providerCombo_->addItems({"Dropbox", "Google Drive", "OneDrive", "iCloud", "Custom (WebDAV)"});
    providerLayout->addRow("Provider:", providerCombo_);
    
    syncPathEdit_ = new QLineEdit();
    syncPathEdit_->setPlaceholderText("/MaestroStudio/Projects");
    providerLayout->addRow("Sync Folder:", syncPathEdit_);
    
    layout->addWidget(providerGroup);
    
    // Sync settings
    auto* syncGroup = new QGroupBox("Sync Settings");
    auto* syncLayout = new QVBoxLayout(syncGroup);
    
    autoSyncCheck_ = new QCheckBox("Automatic sync");
    syncLayout->addWidget(autoSyncCheck_);
    
    auto* intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(new QLabel("Sync interval:"));
    syncIntervalSpin_ = new QSpinBox();
    syncIntervalSpin_->setRange(30, 3600);
    syncIntervalSpin_->setValue(300);
    syncIntervalSpin_->setSuffix(" seconds");
    intervalLayout->addWidget(syncIntervalSpin_);
    intervalLayout->addStretch();
    syncLayout->addLayout(intervalLayout);
    
    syncOnSaveCheck_ = new QCheckBox("Sync immediately after save");
    syncLayout->addWidget(syncOnSaveCheck_);
    
    layout->addWidget(syncGroup);
    
    // Authentication
    auto* authGroup = new QGroupBox("Authentication");
    auto* authLayout = new QVBoxLayout(authGroup);
    
    authButton_ = new QPushButton("Connect to Cloud Service");
    authLayout->addWidget(authButton_);
    
    authStatusLabel_ = new QLabel("Not connected");
    authStatusLabel_->setStyleSheet("color: #888;");
    authLayout->addWidget(authStatusLabel_);
    
    layout->addWidget(authGroup);
    
    layout->addStretch();
    
    // Initially disable controls
    providerCombo_->setEnabled(false);
    syncPathEdit_->setEnabled(false);
    autoSyncCheck_->setEnabled(false);
    syncIntervalSpin_->setEnabled(false);
    syncOnSaveCheck_->setEnabled(false);
    authButton_->setEnabled(false);
}

void CloudSettingsPage::load() {
    auto& settings = maestro::SettingsManager::instance().cloud();
    enabledCheck_->setChecked(settings.enabled);
    
    if (settings.provider == "dropbox") providerCombo_->setCurrentIndex(0);
    else if (settings.provider == "google_drive") providerCombo_->setCurrentIndex(1);
    else if (settings.provider == "onedrive") providerCombo_->setCurrentIndex(2);
    else if (settings.provider == "icloud") providerCombo_->setCurrentIndex(3);
    else providerCombo_->setCurrentIndex(4);
    
    syncPathEdit_->setText(QString::fromStdString(settings.syncPath));
    autoSyncCheck_->setChecked(settings.autoSync);
    syncIntervalSpin_->setValue(settings.syncInterval);
    syncOnSaveCheck_->setChecked(settings.syncOnSave);
    
    if (!settings.refreshToken.empty()) {
        authStatusLabel_->setText("Connected");
        authStatusLabel_->setStyleSheet("color: #00aa00;");
    }
}

void CloudSettingsPage::save() {
    auto& settings = maestro::SettingsManager::instance().cloud();
    settings.enabled = enabledCheck_->isChecked();
    
    switch (providerCombo_->currentIndex()) {
        case 0: settings.provider = "dropbox"; break;
        case 1: settings.provider = "google_drive"; break;
        case 2: settings.provider = "onedrive"; break;
        case 3: settings.provider = "icloud"; break;
        default: settings.provider = "custom"; break;
    }
    
    settings.syncPath = syncPathEdit_->text().toStdString();
    settings.autoSync = autoSyncCheck_->isChecked();
    settings.syncInterval = syncIntervalSpin_->value();
    settings.syncOnSave = syncOnSaveCheck_->isChecked();
    
    emit settingsChanged();
}

void CloudSettingsPage::reset() {
    enabledCheck_->setChecked(false);
    providerCombo_->setCurrentIndex(0);
    syncPathEdit_->clear();
    autoSyncCheck_->setChecked(true);
    syncIntervalSpin_->setValue(300);
    syncOnSaveCheck_->setChecked(true);
}

// ShortcutSettingsPage implementation
ShortcutSettingsPage::ShortcutSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void ShortcutSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    shortcutTree_ = new QTreeWidget();
    shortcutTree_->setHeaderLabels({"Action", "Shortcut", "Category"});
    shortcutTree_->setColumnWidth(0, 250);
    shortcutTree_->setColumnWidth(1, 150);
    shortcutTree_->setColumnWidth(2, 150);
    shortcutTree_->setAlternatingRowColors(true);
    layout->addWidget(shortcutTree_);
    
    auto* buttonLayout = new QHBoxLayout();
    
    editButton_ = new QPushButton("Edit Shortcut");
    connect(editButton_, &QPushButton::clicked, [this]() {
        auto* item = shortcutTree_->currentItem();
        if (item) {
            // Open shortcut editor dialog
        }
    });
    buttonLayout->addWidget(editButton_);
    
    resetButton_ = new QPushButton("Reset to Defaults");
    connect(resetButton_, &QPushButton::clicked, [this]() {
        maestro::SettingsManager::instance().resetShortcuts();
        populateShortcuts();
    });
    buttonLayout->addWidget(resetButton_);
    
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
}

void ShortcutSettingsPage::populateShortcuts() {
    shortcutTree_->clear();
    
    auto shortcuts = maestro::SettingsManager::instance().getShortcuts();
    std::map<std::string, QTreeWidgetItem*> categories;
    
    for (const auto& s : shortcuts) {
        if (categories.find(s.category) == categories.end()) {
            auto* catItem = new QTreeWidgetItem(shortcutTree_);
            catItem->setText(0, s.category);
            catItem->setExpanded(true);
            categories[s.category] = catItem;
        }
        
        auto* item = new QTreeWidgetItem(categories[s.category]);
        item->setText(0, s.action);
        item->setText(1, s.shortcut);
        item->setText(2, s.category);
    }
}

void ShortcutSettingsPage::load() {
    populateShortcuts();
}

void ShortcutSettingsPage::save() {
    std::vector<maestro::KeyboardShortcut> shortcuts;
    
    for (int i = 0; i < shortcutTree_->topLevelItemCount(); ++i) {
        auto* catItem = shortcutTree_->topLevelItem(i);
        for (int j = 0; j < catItem->childCount(); ++j) {
            auto* item = catItem->child(j);
            maestro::KeyboardShortcut s;
            s.action = item->text(0).toStdString();
            s.shortcut = item->text(1).toStdString();
            s.category = item->text(2).toStdString();
            shortcuts.push_back(s);
        }
    }
    
    maestro::SettingsManager::instance().setShortcuts(shortcuts);
    emit settingsChanged();
}

void ShortcutSettingsPage::reset() {
    maestro::SettingsManager::instance().resetShortcuts();
    populateShortcuts();
}

// AdvancedSettingsPage implementation
AdvancedSettingsPage::AdvancedSettingsPage(QWidget* parent) : SettingsPage(parent) {
    setupUI();
}

void AdvancedSettingsPage::setupUI() {
    auto* layout = new QVBoxLayout(this);
    auto* splitter = new QSplitter(Qt::Vertical);
    
    // General advanced
    auto* generalGroup = new QGroupBox("General");
    auto* generalLayout = new QVBoxLayout(generalGroup);
    
    debugModeCheck_ = new QCheckBox("Enable debug mode");
    generalLayout->addWidget(debugModeCheck_);
    
    experimentalCheck_ = new QCheckBox("Enable experimental features");
    generalLayout->addWidget(experimentalCheck_);
    
    confirmExitCheck_ = new QCheckBox("Confirm on exit");
    generalLayout->addWidget(confirmExitCheck_);
    
    splitter->addWidget(generalGroup);
    
    // Autosave & Backup
    auto* autosaveGroup = new QGroupBox("Auto-Save & Backup");
    auto* autosaveLayout = new QFormLayout(autosaveGroup);
    
    autosaveCheck_ = new QCheckBox("Enable auto-save");
    autosaveLayout->addRow(autosaveCheck_);
    
    auto* intervalLayout = new QHBoxLayout();
    autosaveIntervalSpin_ = new QSpinBox();
    autosaveIntervalSpin_->setRange(60, 3600);
    autosaveIntervalSpin_->setValue(300);
    autosaveIntervalSpin_->setSuffix(" seconds");
    intervalLayout->addWidget(new QLabel("Interval:"));
    intervalLayout->addWidget(autosaveIntervalSpin_);
    intervalLayout->addStretch();
    autosaveLayout->addRow(intervalLayout);
    
    backupCheck_ = new QCheckBox("Create backup before save");
    autosaveLayout->addRow(backupCheck_);
    
    auto* backupLayout = new QHBoxLayout();
    maxBackupSpin_ = new QSpinBox();
    maxBackupSpin_->setRange(1, 20);
    maxBackupSpin_->setValue(5);
    backupLayout->addWidget(new QLabel("Max backups:"));
    backupLayout->addWidget(maxBackupSpin_);
    backupLayout->addStretch();
    autosaveLayout->addRow(backupLayout);
    
    splitter->addWidget(autosaveGroup);
    
    // Performance
    auto* perfGroup = new QGroupBox("Performance");
    auto* perfLayout = new QFormLayout(perfGroup);
    
    multithreadCheck_ = new QCheckBox("Enable multi-threading");
    perfLayout->addRow(multithreadCheck_);
    
    workerThreadsSpin_ = new QSpinBox();
    workerThreadsSpin_->setRange(1, 32);
    workerThreadsSpin_->setValue(4);
    perfLayout->addRow("Worker threads:", workerThreadsSpin_);
    
    auto* logLayout = new QHBoxLayout();
    logLevelCombo_ = new QComboBox();
    logLevelCombo_->addItems({"Debug", "Info", "Warning", "Error"});
    logLayout->addWidget(new QLabel("Log level:"));
    logLayout->addWidget(logLevelCombo_);
    logLayout->addStretch();
    perfLayout->addRow(logLayout);
    
    splitter->addWidget(perfGroup);
    
    // Crash reporting
    auto* crashGroup = new QGroupBox("Crash Reporting");
    auto* crashLayout = new QVBoxLayout(crashGroup);
    
    crashReportCheck_ = new QCheckBox("Send crash reports automatically");
    crashLayout->addWidget(crashReportCheck_);
    
    auto* infoLabel = new QLabel(
        "Crash reports help us improve MaestroStudio. "
        "No personal data is collected.");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #888; font-size: 10px;");
    crashLayout->addWidget(infoLabel);
    
    splitter->addWidget(crashGroup);
    
    layout->addWidget(splitter);
    layout->addStretch();
}

void AdvancedSettingsPage::load() {
    auto& settings = maestro::SettingsManager::instance().advanced();
    debugModeCheck_->setChecked(settings.enableDebugMode);
    experimentalCheck_->setChecked(settings.enableExperimentalFeatures);
    autosaveCheck_->setChecked(settings.autosaveEnabled);
    autosaveIntervalSpin_->setValue(settings.autosaveInterval);
    backupCheck_->setChecked(settings.createBackups);
    maxBackupSpin_->setValue(settings.maxBackupCount);
    crashReportCheck_->setChecked(settings.enableCrashReporting);
    multithreadCheck_->setChecked(settings.enableMultithreading);
    workerThreadsSpin_->setValue(settings.workerThreads);
    confirmExitCheck_->setChecked(settings.confirmOnExit);
    
    if (settings.logLevel == "debug") logLevelCombo_->setCurrentIndex(0);
    else if (settings.logLevel == "info") logLevelCombo_->setCurrentIndex(1);
    else if (settings.logLevel == "warning") logLevelCombo_->setCurrentIndex(2);
    else logLevelCombo_->setCurrentIndex(3);
}

void AdvancedSettingsPage::save() {
    auto& settings = maestro::SettingsManager::instance().advanced();
    settings.enableDebugMode = debugModeCheck_->isChecked();
    settings.enableExperimentalFeatures = experimentalCheck_->isChecked();
    settings.autosaveEnabled = autosaveCheck_->isChecked();
    settings.autosaveInterval = autosaveIntervalSpin_->value();
    settings.createBackups = backupCheck_->isChecked();
    settings.maxBackupCount = maxBackupSpin_->value();
    settings.enableCrashReporting = crashReportCheck_->isChecked();
    settings.enableMultithreading = multithreadCheck_->isChecked();
    settings.workerThreads = workerThreadsSpin_->value();
    settings.confirmOnExit = confirmExitCheck_->isChecked();
    
    switch (logLevelCombo_->currentIndex()) {
        case 0: settings.logLevel = "debug"; break;
        case 1: settings.logLevel = "info"; break;
        case 2: settings.logLevel = "warning"; break;
        case 3: settings.logLevel = "error"; break;
    }
    
    emit settingsChanged();
}

void AdvancedSettingsPage::reset() {
    debugModeCheck_->setChecked(false);
    experimentalCheck_->setChecked(false);
    autosaveCheck_->setChecked(true);
    autosaveIntervalSpin_->setValue(300);
    backupCheck_->setChecked(true);
    maxBackupSpin_->setValue(5);
    crashReportCheck_->setChecked(true);
    multithreadCheck_->setChecked(true);
    workerThreadsSpin_->setValue(4);
    logLevelCombo_->setCurrentIndex(1);
    confirmExitCheck_->setChecked(true);
}

// SettingsDialog implementation
SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Settings");
    setMinimumSize(900, 600);
    resize(1000, 700);
    
    setupUI();
    loadAllSettings();
}

void SettingsDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);
    auto* splitter = new QSplitter(Qt::Horizontal);
    
    // Category tree
    categoryTree_ = new QTreeWidget();
    categoryTree_->setHeaderHidden(true);
    categoryTree_->setFixedWidth(200);
    categoryTree_->setSelectionMode(QAbstractItemView::SingleSelection);
    
    QStringList categories = {
        "General", "Audio", "MIDI", "Interface",
        "Paths", "Cloud", "Keyboard Shortcuts", "Advanced"
    };
    
    for (const auto& cat : categories) {
        auto* item = new QTreeWidgetItem(categoryTree_);
        item->setText(0, cat);
    }
    
    categoryTree_->setCurrentItem(categoryTree_->topLevelItem(0));
    connect(categoryTree_, &QTreeWidget::currentItemChanged,
            this, &SettingsDialog::onCategoryChanged);
    
    splitter->addWidget(categoryTree_);
    
    // Settings pages
    stackedWidget_ = new QStackedWidget();
    
    generalPage_ = new GeneralSettingsPage(this);
    audioPage_ = new AudioSettingsPage(this);
    midiPage_ = new MidiSettingsPage(this);
    interfacePage_ = new InterfaceSettingsPage(this);
    pathPage_ = new PathSettingsPage(this);
    cloudPage_ = new CloudSettingsPage(this);
    shortcutPage_ = new ShortcutSettingsPage(this);
    advancedPage_ = new AdvancedSettingsPage(this);
    
    stackedWidget_->addWidget(generalPage_);
    stackedWidget_->addWidget(audioPage_);
    stackedWidget_->addWidget(midiPage_);
    stackedWidget_->addWidget(interfacePage_);
    stackedWidget_->addWidget(pathPage_);
    stackedWidget_->addWidget(cloudPage_);
    stackedWidget_->addWidget(shortcutPage_);
    stackedWidget_->addWidget(advancedPage_);
    
    splitter->addWidget(stackedWidget_);
    splitter->setStretchFactor(1, 1);
    
    layout->addWidget(splitter);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    
    resetButton_ = new QPushButton("Reset to Defaults");
    connect(resetButton_, &QPushButton::clicked, this, &SettingsDialog::onReset);
    buttonLayout->addWidget(resetButton_);
    
    buttonLayout->addStretch();
    
    applyButton_ = new QPushButton("Apply");
    connect(applyButton_, &QPushButton::clicked, this, &SettingsDialog::onApply);
    buttonLayout->addWidget(applyButton_);
    
    auto* okButton = new QPushButton("OK");
    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::onOk);
    buttonLayout->addWidget(okButton);
    
    auto* cancelButton = new QPushButton("Cancel");
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    layout->addLayout(buttonLayout);
}

void SettingsDialog::onCategoryChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    if (current) {
        stackedWidget_->setCurrentIndex(current->row());
    }
}

void SettingsDialog::loadAllSettings() {
    generalPage_->load();
    audioPage_->load();
    midiPage_->load();
    interfacePage_->load();
    pathPage_->load();
    cloudPage_->load();
    shortcutPage_->load();
    advancedPage_->load();
}

void SettingsDialog::saveAllSettings() {
    generalPage_->save();
    audioPage_->save();
    midiPage_->save();
    interfacePage_->save();
    pathPage_->save();
    cloudPage_->save();
    shortcutPage_->save();
    advancedPage_->save();
    
    maestro::SettingsManager::instance().save();
}

void SettingsDialog::onApply() {
    saveAllSettings();
}

void SettingsDialog::onOk() {
    saveAllSettings();
    accept();
}

void SettingsDialog::onCancel() {
    reject();
}

void SettingsDialog::onReset() {
    generalPage_->reset();
    audioPage_->reset();
    midiPage_->reset();
    interfacePage_->reset();
    pathPage_->reset();
    cloudPage_->reset();
    shortcutPage_->reset();
    advancedPage_->reset();
}

void SettingsDialog::setCurrentCategory(SettingsCategory category) {
    categoryTree_->setCurrentItem(categoryTree_->topLevelItem(static_cast<int>(category)));
}

} // namespace maestro::gui
