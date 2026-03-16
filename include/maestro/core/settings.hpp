// include/maestro/core/settings.hpp
#pragma once

#include "maestro/core/config.hpp"
#include "maestro/core/types.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace maestro {

/**
 * @brief Settings categories
 */
enum class SettingsCategory {
    General,
    Audio,
    MIDI,
    Interface,
    Paths,
    Instruments,
    Cloud,
    Shortcuts,
    Advanced
};

/**
 * @brief Audio settings
 */
struct AudioSettings {
    std::string driver = "auto";
    SampleRate sampleRate = 48000;
    BufferSize bufferSize = 256;
    int inputChannels = 2;
    int outputChannels = 2;
    std::string inputDevice;
    std::string outputDevice;
    bool enableInputMonitoring = true;
    float inputGain = 1.0f;
    bool ditherEnabled = false;
    int ditherType = 0;  // 0=None, 1=Triangular, 2=Noise-shaped
};

/**
 * @brief MIDI settings
 */
struct MidiSettings {
    std::vector<std::string> inputPorts;
    std::vector<std::string> outputPorts;
    bool enableMidiClock = true;
    bool sendMidiClock = false;
    bool receiveMidiClock = true;
    int midiInLatency = 0;
    int midiOutLatency = 0;
    bool runningStatus = true;
    bool activeSensing = false;
    bool localControl = true;
};

/**
 * @brief Interface settings
 */
struct InterfaceSettings {
    std::string theme = "dark";  // dark, light, system
    double uiScale = 1.0;
    int fontSize = 10;
    std::string fontName = "Segoe UI";
    bool showTooltips = true;
    bool showSplashScreen = true;
    bool showOnboarding = true;
    bool smoothScrolling = true;
    bool animateTransitions = true;
    int meterDecayTime = 300;  // ms
    bool showPeakHold = true;
    int peakHoldTime = 1500;  // ms
    bool horizontalTrackLayout = false;
    bool showTrackIcons = true;
    bool compactMode = false;
    bool darkTitleBar = true;
    bool minimizeToTray = false;
    bool rememberWindowPosition = true;
    bool rememberWindowSize = true;
    int recentProjectsCount = 10;
};

/**
 * @brief Path settings
 */
struct PathSettings {
    std::string projectPath;
    std::string audioPath;
    std::string samplePath;
    std::string stylePath;
    std::string voicePath;
    std::string pluginPathVst2;
    std::string pluginPathVst3;
    std::string pluginPathAu;
    std::string backupPath;
    std::string exportPath;
    bool autoScanPlugins = true;
    bool includeSubfolders = true;
};

/**
 * @brief Cloud settings
 */
struct CloudSettings {
    bool enabled = false;
    std::string provider;  // dropbox, google_drive, onedrive, custom
    std::string syncPath;
    bool autoSync = true;
    int syncInterval = 300;  // seconds
    bool syncOnSave = true;
    bool downloadOnOpen = true;
    std::string apiKey;
    std::string apiSecret;
    std::string refreshToken;
};

/**
 * @brief Instrument settings
 */
struct InstrumentSettings {
    std::string defaultManufacturer;
    std::string defaultModel;
    bool autoConnect = false;
    bool showInstrumentPanel = true;
    int defaultVoicePart = 0;
    int defaultSplitPoint = 54;  // F#3
    bool enableOts = true;  // One Touch Setting
    bool styleSyncStart = false;
    int styleTempo = 120;
    bool metronomeEnabled = false;
    float metronomeVolume = 0.5f;
    int countInBars = 1;
};

/**
 * @brief Keyboard shortcut
 */
struct KeyboardShortcut {
    std::string action;
    std::string shortcut;
    std::string category;
    std::string description;
};

/**
 * @brief Advanced settings
 */
struct AdvancedSettings {
    bool enableDebugMode = false;
    bool enableExperimentalFeatures = false;
    bool enableNetworkFeatures = false;
    int maxUndoLevels = 100;
    int autosaveInterval = 300;  // seconds
    bool autosaveEnabled = true;
    bool createBackups = true;
    int maxBackupCount = 5;
    bool enableCrashReporting = true;
    bool enableUsageStatistics = false;
    int maxCpuUsage = 80;  // percent
    bool enableMultithreading = true;
    int workerThreads = 4;
    bool enableSimd = true;
    bool enableGpuAcceleration = false;
    std::string logLevel = "info";  // debug, info, warning, error
    bool enableMidiLearn = true;
    bool confirmOnExit = true;
    bool loadLastProject = false;
};

/**
 * @brief Settings manager
 */
class MAESTRO_API SettingsManager {
public:
    static SettingsManager& instance();

    // Lifecycle
    Result<void> initialize(const std::string& configPath = "");
    Result<void> save();
    Result<void> load();
    void reset();

    // Category access
    AudioSettings& audio();
    const AudioSettings& audio() const;
    
    MidiSettings& midi();
    const MidiSettings& midi() const;
    
    InterfaceSettings& interface();
    const InterfaceSettings& interface() const;
    
    PathSettings& paths();
    const PathSettings& paths() const;
    
    CloudSettings& cloud();
    const CloudSettings& cloud() const;
    
    InstrumentSettings& instruments();
    const InstrumentSettings& instruments() const;
    
    AdvancedSettings& advanced();
    const AdvancedSettings& advanced() const;

    // Individual settings
    template<typename T>
    void setValue(const std::string& key, const T& value);
    
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue) const;

    // Keyboard shortcuts
    std::vector<KeyboardShortcut> getShortcuts() const;
    void setShortcuts(const std::vector<KeyboardShortcut>& shortcuts);
    std::string getShortcut(const std::string& action) const;
    void setShortcut(const std::string& action, const std::string& shortcut);
    void resetShortcuts();

    // Presets
    Result<void> savePreset(const std::string& name, const std::string& path);
    Result<void> loadPreset(const std::string& path);
    std::vector<std::string> getAvailablePresets() const;

    // Export/Import
    Result<void> exportSettings(const std::string& path);
    Result<void> importSettings(const std::string& path);

    // Callbacks
    using SettingsChangedCallback = std::function<void(const std::string& key)>;
    void addChangedCallback(SettingsChangedCallback callback);
    void removeChangedCallback(const SettingsChangedCallback& callback);

private:
    SettingsManager();
    ~SettingsManager();
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    std::string getConfigPath() const;
    std::string settingsToJson() const;
    Result<void> jsonToSettings(const std::string& json);

    AudioSettings audio_;
    MidiSettings midi_;
    InterfaceSettings interface_;
    PathSettings paths_;
    CloudSettings cloud_;
    InstrumentSettings instruments_;
    AdvancedSettings advanced_;
    
    std::vector<KeyboardShortcut> shortcuts_;
    std::map<std::string, std::string> customValues_;
    std::vector<SettingsChangedCallback> callbacks_;
    
    std::string configPath_;
    bool initialized_ = false;
};

} // namespace maestro
