// src/core/settings.cpp
#include "maestro/core/settings.hpp"
#include <fstream>
#include <sstream>

namespace maestro {

// Simple JSON helpers (in real implementation, use a proper JSON library)
namespace {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::string extractValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    
    pos = json.find_first_not_of(" \t\n\r", pos + 1);
    if (pos == std::string::npos) return "";
    
    if (json[pos] == '"') {
        size_t end = json.find('"', pos + 1);
        if (end == std::string::npos) return "";
        return json.substr(pos + 1, end - pos - 1);
    }
    
    size_t end = json.find_first_of(",}\n", pos);
    if (end == std::string::npos) end = json.length();
    return trim(json.substr(pos, end - pos));
}

template<typename T>
T parseValue(const std::string& str, T defaultValue) {
    if (str.empty()) return defaultValue;
    if constexpr (std::is_same_v<T, bool>) {
        return str == "true" || str == "1";
    } else if constexpr (std::is_same_v<T, int>) {
        return std::stoi(str);
    } else if constexpr (std::is_same_v<T, float>) {
        return std::stof(str);
    } else if constexpr (std::is_same_v<T, double>) {
        return std::stod(str);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return str;
    }
    return defaultValue;
}

} // anonymous namespace

SettingsManager& SettingsManager::instance() {
    static SettingsManager instance;
    return instance;
}

SettingsManager::SettingsManager() {
    // Initialize default shortcuts
    shortcuts_ = {
        {"file_new", "Ctrl+N", "File", "New Project"},
        {"file_open", "Ctrl+O", "File", "Open Project"},
        {"file_save", "Ctrl+S", "File", "Save Project"},
        {"file_save_as", "Ctrl+Shift+S", "File", "Save As"},
        {"edit_undo", "Ctrl+Z", "Edit", "Undo"},
        {"edit_redo", "Ctrl+Y", "Edit", "Redo"},
        {"edit_cut", "Ctrl+X", "Edit", "Cut"},
        {"edit_copy", "Ctrl+C", "Edit", "Copy"},
        {"edit_paste", "Ctrl+V", "Edit", "Paste"},
        {"transport_play", "Space", "Transport", "Play/Stop"},
        {"transport_stop", "Ctrl+Space", "Transport", "Stop"},
        {"transport_record", "R", "Transport", "Record"},
        {"transport_rewind", "Home", "Transport", "Rewind"},
        {"transport_ff", "End", "Transport", "Fast Forward"},
        {"view_zoom_in", "Ctrl++", "View", "Zoom In"},
        {"view_zoom_out", "Ctrl+-", "View", "Zoom Out"},
        {"view_zoom_fit", "Ctrl+0", "View", "Zoom to Fit"},
        {"tools_metronome", "M", "Tools", "Toggle Metronome"},
        {"tools_mixer", "F9", "View", "Toggle Mixer"},
        {"tools_piano_roll", "F10", "View", "Toggle Piano Roll"},
    };
}

SettingsManager::~SettingsManager() {
    if (initialized_) {
        save();
    }
}

Result<void> SettingsManager::initialize(const std::string& configPath) {
    if (!configPath.empty()) {
        configPath_ = configPath;
    } else {
        configPath_ = getConfigPath();
    }
    
    // Load existing settings or use defaults
    load();
    
    initialized_ = true;
    return Result<void>();
}

Result<void> SettingsManager::save() {
    std::ofstream file(configPath_);
    if (!file) {
        return Result<void>("Cannot open config file for writing: " + configPath_);
    }
    
    file << settingsToJson();
    
    return Result<void>();
}

Result<void> SettingsManager::load() {
    std::ifstream file(configPath_);
    if (!file) {
        // File doesn't exist, use defaults
        return Result<void>();
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    
    return jsonToSettings(json);
}

void SettingsManager::reset() {
    audio_ = AudioSettings{};
    midi_ = MidiSettings{};
    interface_ = InterfaceSettings{};
    paths_ = PathSettings{};
    cloud_ = CloudSettings{};
    instruments_ = InstrumentSettings{};
    advanced_ = AdvancedSettings{};
    resetShortcuts();
}

std::string SettingsManager::getConfigPath() const {
    // Platform-specific config path
#ifdef MAESTRO_PLATFORM_WINDOWS
    return "C:\\ProgramData\\MaestroStudio\\config.json";
#elif defined(MAESTRO_PLATFORM_MACOS)
    return "~/Library/Preferences/MaestroStudio/config.json";
#elif defined(MAESTRO_PLATFORM_LINUX)
    return "~/.config/MaestroStudio/config.json";
#else
    return "maestro_studio_config.json";
#endif
}

std::string SettingsManager::settingsToJson() const {
    std::ostringstream json;
    json << "{\n";
    
    // Audio settings
    json << "  \"audio\": {\n";
    json << "    \"driver\": \"" << audio_.driver << "\",\n";
    json << "    \"sampleRate\": " << audio_.sampleRate << ",\n";
    json << "    \"bufferSize\": " << audio_.bufferSize << ",\n";
    json << "    \"inputChannels\": " << audio_.inputChannels << ",\n";
    json << "    \"outputChannels\": " << audio_.outputChannels << ",\n";
    json << "    \"inputDevice\": \"" << audio_.inputDevice << "\",\n";
    json << "    \"outputDevice\": \"" << audio_.outputDevice << "\",\n";
    json << "    \"enableInputMonitoring\": " << (audio_.enableInputMonitoring ? "true" : "false") << ",\n";
    json << "    \"inputGain\": " << audio_.inputGain << "\n";
    json << "  },\n";
    
    // MIDI settings
    json << "  \"midi\": {\n";
    json << "    \"enableMidiClock\": " << (midi_.enableMidiClock ? "true" : "false") << ",\n";
    json << "    \"sendMidiClock\": " << (midi_.sendMidiClock ? "true" : "false") << ",\n";
    json << "    \"receiveMidiClock\": " << (midi_.receiveMidiClock ? "true" : "false") << ",\n";
    json << "    \"midiInLatency\": " << midi_.midiInLatency << ",\n";
    json << "    \"midiOutLatency\": " << midi_.midiOutLatency << "\n";
    json << "  },\n";
    
    // Interface settings
    json << "  \"interface\": {\n";
    json << "    \"theme\": \"" << interface_.theme << "\",\n";
    json << "    \"uiScale\": " << interface_.uiScale << ",\n";
    json << "    \"fontSize\": " << interface_.fontSize << ",\n";
    json << "    \"fontName\": \"" << interface_.fontName << "\",\n";
    json << "    \"showTooltips\": " << (interface_.showTooltips ? "true" : "false") << ",\n";
    json << "    \"showSplashScreen\": " << (interface_.showSplashScreen ? "true" : "false") << ",\n";
    json << "    \"showOnboarding\": " << (interface_.showOnboarding ? "true" : "false") << ",\n";
    json << "    \"smoothScrolling\": " << (interface_.smoothScrolling ? "true" : "false") << ",\n";
    json << "    \"animateTransitions\": " << (interface_.animateTransitions ? "true" : "false") << ",\n";
    json << "    \"meterDecayTime\": " << interface_.meterDecayTime << ",\n";
    json << "    \"showPeakHold\": " << (interface_.showPeakHold ? "true" : "false") << ",\n";
    json << "    \"peakHoldTime\": " << interface_.peakHoldTime << ",\n";
    json << "    \"compactMode\": " << (interface_.compactMode ? "true" : "false") << ",\n";
    json << "    \"rememberWindowPosition\": " << (interface_.rememberWindowPosition ? "true" : "false") << ",\n";
    json << "    \"rememberWindowSize\": " << (interface_.rememberWindowSize ? "true" : "false") << ",\n";
    json << "    \"recentProjectsCount\": " << interface_.recentProjectsCount << "\n";
    json << "  },\n";
    
    // Paths
    json << "  \"paths\": {\n";
    json << "    \"projectPath\": \"" << paths_.projectPath << "\",\n";
    json << "    \"audioPath\": \"" << paths_.audioPath << "\",\n";
    json << "    \"samplePath\": \"" << paths_.samplePath << "\",\n";
    json << "    \"pluginPathVst2\": \"" << paths_.pluginPathVst2 << "\",\n";
    json << "    \"pluginPathVst3\": \"" << paths_.pluginPathVst3 << "\",\n";
    json << "    \"exportPath\": \"" << paths_.exportPath << "\"\n";
    json << "  },\n";
    
    // Cloud
    json << "  \"cloud\": {\n";
    json << "    \"enabled\": " << (cloud_.enabled ? "true" : "false") << ",\n";
    json << "    \"provider\": \"" << cloud_.provider << "\",\n";
    json << "    \"autoSync\": " << (cloud_.autoSync ? "true" : "false") << ",\n";
    json << "    \"syncInterval\": " << cloud_.syncInterval << "\n";
    json << "  },\n";
    
    // Instruments
    json << "  \"instruments\": {\n";
    json << "    \"defaultManufacturer\": \"" << instruments_.defaultManufacturer << "\",\n";
    json << "    \"defaultModel\": \"" << instruments_.defaultModel << "\",\n";
    json << "    \"autoConnect\": " << (instruments_.autoConnect ? "true" : "false") << ",\n";
    json << "    \"metronomeEnabled\": " << (instruments_.metronomeEnabled ? "true" : "false") << ",\n";
    json << "    \"metronomeVolume\": " << instruments_.metronomeVolume << ",\n";
    json << "    \"countInBars\": " << instruments_.countInBars << "\n";
    json << "  },\n";
    
    // Advanced
    json << "  \"advanced\": {\n";
    json << "    \"enableDebugMode\": " << (advanced_.enableDebugMode ? "true" : "false") << ",\n";
    json << "    \"autosaveEnabled\": " << (advanced_.autosaveEnabled ? "true" : "false") << ",\n";
    json << "    \"autosaveInterval\": " << advanced_.autosaveInterval << ",\n";
    json << "    \"maxUndoLevels\": " << advanced_.maxUndoLevels << ",\n";
    json << "    \"createBackups\": " << (advanced_.createBackups ? "true" : "false") << ",\n";
    json << "    \"maxBackupCount\": " << advanced_.maxBackupCount << ",\n";
    json << "    \"enableCrashReporting\": " << (advanced_.enableCrashReporting ? "true" : "false") << ",\n";
    json << "    \"enableMultithreading\": " << (advanced_.enableMultithreading ? "true" : "false") << ",\n";
    json << "    \"workerThreads\": " << advanced_.workerThreads << ",\n";
    json << "    \"logLevel\": \"" << advanced_.logLevel << "\",\n";
    json << "    \"confirmOnExit\": " << (advanced_.confirmOnExit ? "true" : "false") << ",\n";
    json << "    \"loadLastProject\": " << (advanced_.loadLastProject ? "true" : "false") << "\n";
    json << "  },\n";
    
    // Shortcuts
    json << "  \"shortcuts\": [\n";
    for (size_t i = 0; i < shortcuts_.size(); ++i) {
        const auto& s = shortcuts_[i];
        json << "    {\"action\": \"" << s.action << "\", \"shortcut\": \"" << s.shortcut << "\"}";
        if (i < shortcuts_.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ]\n";
    
    json << "}\n";
    
    return json.str();
}

Result<void> SettingsManager::jsonToSettings(const std::string& json) {
    // Audio
    audio_.driver = parseValue(extractValue(json, "driver"), audio_.driver);
    audio_.sampleRate = parseValue(extractValue(json, "sampleRate"), audio_.sampleRate);
    audio_.bufferSize = parseValue(extractValue(json, "bufferSize"), audio_.bufferSize);
    audio_.enableInputMonitoring = parseValue(extractValue(json, "enableInputMonitoring"), audio_.enableInputMonitoring);
    
    // Interface
    interface_.theme = parseValue(extractValue(json, "theme"), interface_.theme);
    interface_.uiScale = parseValue(extractValue(json, "uiScale"), interface_.uiScale);
    interface_.fontSize = parseValue(extractValue(json, "fontSize"), interface_.fontSize);
    interface_.showTooltips = parseValue(extractValue(json, "showTooltips"), interface_.showTooltips);
    interface_.showSplashScreen = parseValue(extractValue(json, "showSplashScreen"), interface_.showSplashScreen);
    interface_.showOnboarding = parseValue(extractValue(json, "showOnboarding"), interface_.showOnboarding);
    
    // Advanced
    advanced_.autosaveEnabled = parseValue(extractValue(json, "autosaveEnabled"), advanced_.autosaveEnabled);
    advanced_.autosaveInterval = parseValue(extractValue(json, "autosaveInterval"), advanced_.autosaveInterval);
    advanced_.enableDebugMode = parseValue(extractValue(json, "enableDebugMode"), advanced_.enableDebugMode);
    
    return Result<void>();
}

AudioSettings& SettingsManager::audio() { return audio_; }
const AudioSettings& SettingsManager::audio() const { return audio_; }

MidiSettings& SettingsManager::midi() { return midi_; }
const MidiSettings& SettingsManager::midi() const { return midi_; }

InterfaceSettings& SettingsManager::interface() { return interface_; }
const InterfaceSettings& SettingsManager::interface() const { return interface_; }

PathSettings& SettingsManager::paths() { return paths_; }
const PathSettings& SettingsManager::paths() const { return paths_; }

CloudSettings& SettingsManager::cloud() { return cloud_; }
const CloudSettings& SettingsManager::cloud() const { return cloud_; }

InstrumentSettings& SettingsManager::instruments() { return instruments_; }
const InstrumentSettings& SettingsManager::instruments() const { return instruments_; }

AdvancedSettings& SettingsManager::advanced() { return advanced_; }
const AdvancedSettings& SettingsManager::advanced() const { return advanced_; }

std::vector<KeyboardShortcut> SettingsManager::getShortcuts() const {
    return shortcuts_;
}

void SettingsManager::setShortcuts(const std::vector<KeyboardShortcut>& shortcuts) {
    shortcuts_ = shortcuts;
}

std::string SettingsManager::getShortcut(const std::string& action) const {
    for (const auto& s : shortcuts_) {
        if (s.action == action) return s.shortcut;
    }
    return "";
}

void SettingsManager::setShortcut(const std::string& action, const std::string& shortcut) {
    for (auto& s : shortcuts_) {
        if (s.action == action) {
            s.shortcut = shortcut;
            return;
        }
    }
}

void SettingsManager::resetShortcuts() {
    shortcuts_ = {
        {"file_new", "Ctrl+N", "File", "New Project"},
        {"file_open", "Ctrl+O", "File", "Open Project"},
        {"file_save", "Ctrl+S", "File", "Save Project"},
        {"transport_play", "Space", "Transport", "Play/Stop"},
        {"transport_stop", "Ctrl+Space", "Transport", "Stop"},
        {"transport_record", "R", "Transport", "Record"},
    };
}

void SettingsManager::addChangedCallback(SettingsChangedCallback callback) {
    callbacks_.push_back(std::move(callback));
}

void SettingsManager::removeChangedCallback(const SettingsChangedCallback& callback) {
    // Cannot compare std::function directly, so we remove by index or use a different approach
    // For simplicity, we'll just clear all callbacks with the same target
    // In a real implementation, you'd use a proper callback handle
    callbacks_.clear();  // Simplified - just clear all for now
}

Result<void> SettingsManager::exportSettings(const std::string& path) {
    std::ofstream file(path);
    if (!file) {
        return Result<void>("Cannot export settings: " + path);
    }
    file << settingsToJson();
    return Result<void>();
}

Result<void> SettingsManager::importSettings(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return Result<void>("Cannot import settings: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return jsonToSettings(buffer.str());
}

std::vector<std::string> SettingsManager::getAvailablePresets() const {
    return {"Default", "Low Latency", "High Quality", "Live Performance"};
}

Result<void> SettingsManager::savePreset(const std::string& name, const std::string& path) {
    return Result<void>();
}

Result<void> SettingsManager::loadPreset(const std::string& path) {
    return Result<void>();
}

} // namespace maestro
