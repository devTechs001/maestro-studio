// src/gui/qt/widgets/onboarding_wizard.cpp
#include "onboarding_wizard.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>

namespace maestro::gui {

// WelcomePage implementation
WelcomePage::WelcomePage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Welcome to MaestroStudio");
    setSubTitle("Let's set up your music production environment");
    
    auto* layout = new QVBoxLayout(this);
    
    auto* welcomeLabel = new QLabel(
        "MaestroStudio is a professional music production suite that integrates with "
        "hardware instruments and provides comprehensive DAW functionality.\n\n"
        "This wizard will help you configure:\n"
        "• Audio driver and settings\n"
        "• MIDI input/output devices\n"
        "• Hardware instrument integration\n"
        "• Application preferences");
    welcomeLabel->setWordWrap(true);
    welcomeLabel->setStyleSheet("font-size: 12px; line-height: 1.6;");
    layout->addWidget(welcomeLabel);
    
    layout->addStretch();
    
    registerField("welcomeCompleted", new QCheckBox());
}

int WelcomePage::nextId() const {
    return 1;
}

void WelcomePage::initializePage() {
    field("welcomeCompleted").setBool(true);
}

// AudioSetupPage implementation
AudioSetupPage::AudioSetupPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Audio Configuration");
    setSubTitle("Select your audio driver and settings");
    
    auto* layout = new QVBoxLayout(this);
    
    auto* driverGroup = new QGroupBox("Audio Driver");
    auto* driverLayout = new QFormLayout(driverGroup);
    
    driverCombo_ = new QComboBox();
    driverCombo_->addItems({
        "Auto (Default)",
        "ASIO (Windows)",
        "WASAPI (Windows)",
        "Core Audio (macOS)",
        "ALSA (Linux)",
        "JACK (Linux)"
    });
    driverLayout->addRow("Driver:", driverCombo_);
    layout->addWidget(driverGroup);
    
    auto* settingsGroup = new QGroupBox("Audio Settings");
    auto* settingsLayout = new QFormLayout(settingsGroup);
    
    sampleRateCombo_ = new QComboBox();
    sampleRateCombo_->addItems({"44100 Hz", "48000 Hz", "88200 Hz", "96000 Hz"});
    sampleRateCombo_->setCurrentIndex(1);  // Default 48kHz
    settingsLayout->addRow("Sample Rate:", sampleRateCombo_);
    
    bufferSizeCombo_ = new QComboBox();
    bufferSizeCombo_->addItems({"64", "128", "256", "512", "1024", "2048"});
    bufferSizeCombo_->setCurrentIndex(2);  // Default 256
    settingsLayout->addRow("Buffer Size:", bufferSizeCombo_);
    
    layout->addWidget(settingsGroup);
    
    auto* infoLabel = new QLabel(
        "Lower buffer sizes reduce latency but require more CPU power. "
        "Start with 256 samples and adjust if needed.");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #888; font-size: 11px;");
    layout->addWidget(infoLabel);
    
    layout->addStretch();
    
    registerField("audioDriver", driverCombo_);
    registerField("sampleRate", sampleRateCombo_);
    registerField("bufferSize", bufferSizeCombo_);
}

int AudioSetupPage::nextId() const {
    return 2;
}

bool AudioSetupPage::validatePage() {
    return true;
}

void AudioSetupPage::initializePage() {
}

QString AudioSetupPage::selectedDriver() const {
    return driverCombo_->currentText();
}

int AudioSetupPage::selectedSampleRate() const {
    return sampleRateCombo_->currentText().remove(" Hz").toInt();
}

int AudioSetupPage::selectedBufferSize() const {
    return bufferSizeCombo_->currentText().toInt();
}

// MidiSetupPage implementation
MidiSetupPage::MidiSetupPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("MIDI Configuration");
    setSubTitle("Select your MIDI input and output devices");
    
    auto* layout = new QVBoxLayout(this);
    
    auto* inputsGroup = new QGroupBox("MIDI Inputs");
    auto* inputsLayout = new QVBoxLayout(inputsGroup);
    
    inputList_ = new QListWidget();
    inputList_->setSelectionMode(QAbstractItemView::MultiSelection);
    inputList_->addItems({
        "Microsoft GS Wavetable Synth",
        "Yamaha USB MIDI",
        "Roland USB MIDI",
        "Korg USB MIDI",
        "Virtual Input 1"
    });
    inputsLayout->addWidget(inputList_);
    layout->addWidget(inputsGroup);
    
    auto* outputsGroup = new QGroupBox("MIDI Outputs");
    auto* outputsLayout = new QVBoxLayout(outputsGroup);
    
    outputList_ = new QListWidget();
    outputList_->setSelectionMode(QAbstractItemView::MultiSelection);
    outputList_->addItems({
        "Microsoft GS Wavetable Synth",
        "Yamaha USB MIDI",
        "Roland USB MIDI",
        "Korg USB MIDI",
        "Virtual Output 1"
    });
    outputsLayout->addWidget(outputList_);
    layout->addWidget(outputsGroup);
    
    registerField("midiInputs", inputList_);
    registerField("midiOutputs", outputList_);
}

int MidiSetupPage::nextId() const {
    return 3;
}

bool MidiSetupPage::validatePage() {
    return true;
}

void MidiSetupPage::initializePage() {
}

QStringList MidiSetupPage::selectedInputs() const {
    QStringList inputs;
    for (auto* item : inputList_->selectedItems()) {
        inputs << item->text();
    }
    return inputs;
}

QStringList MidiSetupPage::selectedOutputs() const {
    QStringList outputs;
    for (auto* item : outputList_->selectedItems()) {
        outputs << item->text();
    }
    return outputs;
}

// InstrumentSetupPage implementation
InstrumentSetupPage::InstrumentSetupPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Hardware Instrument");
    setSubTitle("Connect your hardware keyboard or synthesizer");
    
    auto* layout = new QVBoxLayout(this);
    
    auto* instrumentGroup = new QGroupBox("Instrument Selection");
    auto* instrumentLayout = new QFormLayout(instrumentGroup);
    
    manufacturerCombo_ = new QComboBox();
    manufacturerCombo_->addItems({
        "None / Generic MIDI",
        "Yamaha",
        "Roland",
        "Korg",
        "Nord",
        "Kurzweil"
    });
    instrumentLayout->addRow("Manufacturer:", manufacturerCombo_);
    
    modelCombo_ = new QComboBox();
    modelCombo_->addItems({
        "Generic MIDI Device",
        "Yamaha Genos",
        "Yamaha PSR-SX900",
        "Yamaha Motif/MODX",
        "Roland Fantom",
        "Roland Jupiter X",
        "Korg PA5X",
        "Korg Kronos",
        "Nord Stage 3"
    });
    instrumentLayout->addRow("Model:", modelCombo_);
    layout->addWidget(instrumentGroup);
    
    auto* infoLabel = new QLabel(
        "You can change or add instruments later from the Instruments menu.");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #888; font-size: 11px;");
    layout->addWidget(infoLabel);
    
    layout->addStretch();
    
    registerField("manufacturer", manufacturerCombo_);
    registerField("model", modelCombo_);
    
    connect(manufacturerCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                // Update model list based on manufacturer
                modelCombo_->clear();
                switch (index) {
                    case 0:
                        modelCombo_->addItem("Generic MIDI Device");
                        break;
                    case 1:
                        modelCombo_->addItems({"Genos", "PSR-SX900", "PSR-SX700", "Motif XF", "MODX", "Montage"});
                        break;
                    case 2:
                        modelCombo_->addItems({"Fantom", "Jupiter X", "RD-2000"});
                        break;
                    case 3:
                        modelCombo_->addItems({"PA5X", "PA4X", "PA1000", "Kronos", "Nautilus"});
                        break;
                    case 4:
                        modelCombo_->addItems({"Stage 3", "Piano 4", "Wave 2"});
                        break;
                    case 5:
                        modelCombo_->addItems({"PC4", "Forte"});
                        break;
                }
            });
}

int InstrumentSetupPage::nextId() const {
    return 4;
}

bool InstrumentSetupPage::validatePage() {
    return true;
}

void InstrumentSetupPage::initializePage() {
}

QString InstrumentSetupPage::selectedInstrument() const {
    return manufacturerCombo_->currentText() + " " + modelCombo_->currentText();
}

// PreferencesPage implementation
PreferencesPage::PreferencesPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Preferences");
    setSubTitle("Configure application settings");
    
    auto* layout = new QVBoxLayout(this);
    
    auto* prefsGroup = new QGroupBox("General Preferences");
    auto* prefsLayout = new QVBoxLayout(prefsGroup);
    
    autoSaveCheck_ = new QCheckBox("Enable Auto-Save (every 5 minutes)");
    autoSaveCheck_->setChecked(true);
    prefsLayout->addWidget(autoSaveCheck_);
    
    tooltipsCheck_ = new QCheckBox("Show Tooltips");
    tooltipsCheck_->setChecked(true);
    prefsLayout->addWidget(tooltipsCheck_);
    
    cloudSyncCheck_ = new QCheckBox("Enable Cloud Sync for Projects");
    cloudSyncCheck_->setChecked(false);
    prefsLayout->addWidget(cloudSyncCheck_);
    
    layout->addWidget(prefsGroup);
    
    auto* themeGroup = new QGroupBox("Theme");
    auto* themeLayout = new QVBoxLayout(themeGroup);
    
    auto* themeLabel = new QLabel("Theme selection is available in the View menu after setup.");
    themeLabel->setStyleSheet("color: #888;");
    themeLayout->addWidget(themeLabel);
    layout->addWidget(themeGroup);
    
    layout->addStretch();
    
    registerField("autoSave", autoSaveCheck_);
    registerField("tooltips", tooltipsCheck_);
    registerField("cloudSync", cloudSyncCheck_);
}

int PreferencesPage::nextId() const {
    return 5;
}

bool PreferencesPage::validatePage() {
    return true;
}

void PreferencesPage::initializePage() {
}

bool PreferencesPage::autoSaveEnabled() const {
    return autoSaveCheck_->isChecked();
}

bool PreferencesPage::showTooltips() const {
    return tooltipsCheck_->isChecked();
}

bool PreferencesPage::enableCloudSync() const {
    return cloudSyncCheck_->isChecked();
}

// CompletionPage implementation
CompletionPage::CompletionPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Setup Complete!");
    setSubTitle("You're ready to start making music");
    
    auto* layout = new QVBoxLayout(this);
    
    auto* completeLabel = new QLabel(
        "🎉 Congratulations!\n\n"
        "MaestroStudio is now configured and ready to use.\n\n"
        "You can:\n"
        "• Create a new project to start recording\n"
        "• Open an existing project\n"
        "• Connect additional hardware instruments\n"
        "• Customize settings from the Edit menu\n\n"
        "Enjoy making music with MaestroStudio!");
    completeLabel->setAlignment(Qt::AlignCenter);
    completeLabel->setWordWrap(true);
    completeLabel->setStyleSheet("font-size: 12px; line-height: 1.8;");
    layout->addWidget(completeLabel);
    
    layout->addStretch();
    
    setFinalPage(true);
}

void CompletionPage::initializePage() {
}

// OnboardingWizard implementation
OnboardingWizard::OnboardingWizard(QWidget* parent)
    : QWizard(parent) {
    setWindowTitle("MaestroStudio Setup Wizard");
    setMinimumWidth(600);
    setMinimumHeight(500);
    
    // Add pages
    setPage(0, new WelcomePage(this));
    setPage(1, new AudioSetupPage(this));
    setPage(2, new MidiSetupPage(this));
    setPage(3, new InstrumentSetupPage(this));
    setPage(4, new PreferencesPage(this));
    setPage(5, new CompletionPage(this));
    
    setStartId(0);
    
    // Set wizard style
    setWizardStyle(QWizard::ModernStyle);
    setOptions(QWizard::NoBackButtonOnStartPage | QWizard::NoCancelButton);
    
    // Apply stylesheet
    setStyleSheet(R"(
        QWizard {
            background-color: #1e1e1e;
            color: #e0e0e0;
        }
        QWizard QLabel {
            color: #e0e0e0;
        }
        QWizard QGroupBox {
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QWizard QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 5px;
        }
    )");
}

QString OnboardingWizard::audioDriver() const {
    auto* page = qobject_cast<AudioSetupPage*>(page(1));
    return page ? page->selectedDriver() : QString();
}

int OnboardingWizard::sampleRate() const {
    auto* page = qobject_cast<AudioSetupPage*>(page(1));
    return page ? page->selectedSampleRate() : 48000;
}

int OnboardingWizard::bufferSize() const {
    auto* page = qobject_cast<AudioSetupPage*>(page(1));
    return page ? page->selectedBufferSize() : 256;
}

QStringList OnboardingWizard::midiInputs() const {
    auto* page = qobject_cast<MidiSetupPage*>(page(2));
    return page ? page->selectedInputs() : QStringList();
}

QStringList OnboardingWizard::midiOutputs() const {
    auto* page = qobject_cast<MidiSetupPage*>(page(2));
    return page ? page->selectedOutputs() : QStringList();
}

QString OnboardingWizard::instrument() const {
    auto* page = qobject_cast<InstrumentSetupPage*>(page(3));
    return page ? page->selectedInstrument() : QString();
}

bool OnboardingWizard::autoSave() const {
    auto* page = qobject_cast<PreferencesPage*>(page(4));
    return page ? page->autoSaveEnabled() : true;
}

void OnboardingWizard::accept() {
    QVariantMap settings;
    settings["audioDriver"] = audioDriver();
    settings["sampleRate"] = sampleRate();
    settings["bufferSize"] = bufferSize();
    settings["midiInputs"] = midiInputs();
    settings["midiOutputs"] = midiOutputs();
    settings["instrument"] = instrument();
    settings["autoSave"] = autoSave();
    
    emit onboardingCompleted(settings);
    QWizard::accept();
}

} // namespace maestro::gui
