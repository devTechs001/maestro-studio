// src/gui/qt/widgets/onboarding_wizard.hpp
#pragma once

#include <QWizard>
#include <QWizardPage>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>

namespace maestro::gui {

// Welcome Page
class WelcomePage : public QWizardPage {
    Q_OBJECT
public:
    explicit WelcomePage(QWidget* parent = nullptr);
    
    int nextId() const override;
    
private:
    void initializePage() override;
};

// Audio Setup Page
class AudioSetupPage : public QWizardPage {
    Q_OBJECT
public:
    explicit AudioSetupPage(QWidget* parent = nullptr);
    
    int nextId() const override;
    bool validatePage() override;
    
    QString selectedDriver() const;
    int selectedSampleRate() const;
    int selectedBufferSize() const;
    
private:
    void initializePage() override;
    
    QComboBox* driverCombo_;
    QComboBox* sampleRateCombo_;
    QComboBox* bufferSizeCombo_;
};

// MIDI Setup Page
class MidiSetupPage : public QWizardPage {
    Q_OBJECT
public:
    explicit MidiSetupPage(QWidget* parent = nullptr);
    
    int nextId() const override;
    bool validatePage() override;
    
    QStringList selectedInputs() const;
    QStringList selectedOutputs() const;
    
private:
    void initializePage() override;
    
    QListWidget* inputList_;
    QListWidget* outputList_;
};

// Instrument Setup Page
class InstrumentSetupPage : public QWizardPage {
    Q_OBJECT
public:
    explicit InstrumentSetupPage(QWidget* parent = nullptr);
    
    int nextId() const override;
    bool validatePage() override;
    
    QString selectedInstrument() const;
    
private:
    void initializePage() override;
    
    QComboBox* manufacturerCombo_;
    QComboBox* modelCombo_;
};

// Preferences Page
class PreferencesPage : public QWizardPage {
    Q_OBJECT
public:
    explicit PreferencesPage(QWidget* parent = nullptr);
    
    int nextId() const override;
    bool validatePage() override;
    
    bool autoSaveEnabled() const;
    bool showTooltips() const;
    bool enableCloudSync() const;
    
private:
    void initializePage() override;
    
    QCheckBox* autoSaveCheck_;
    QCheckBox* tooltipsCheck_;
    QCheckBox* cloudSyncCheck_;
};

// Completion Page
class CompletionPage : public QWizardPage {
    Q_OBJECT
public:
    explicit CompletionPage(QWidget* parent = nullptr);
    
private:
    void initializePage() override;
};

// Main Onboarding Wizard
class OnboardingWizard : public QWizard {
    Q_OBJECT
public:
    explicit OnboardingWizard(QWidget* parent = nullptr);
    
    // Get configured settings
    QString audioDriver() const;
    int sampleRate() const;
    int bufferSize() const;
    QStringList midiInputs() const;
    QStringList midiOutputs() const;
    QString instrument() const;
    bool autoSave() const;
    
signals:
    void onboardingCompleted(const QVariantMap& settings);
    
private:
    void accept() override;
};

} // namespace maestro::gui
