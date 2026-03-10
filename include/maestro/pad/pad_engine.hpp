// include/maestro/pad/pad_engine.hpp
#pragma once

#include "maestro/core/types.hpp"
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace maestro {

class MidiEngine;
class AudioEngine;

/**
 * @brief Pad assignment types
 */
enum class PadType {
    MidiNote,       // Trigger MIDI note
    Sample,         // Play audio sample
    Phrase,         // Play MIDI phrase
    Style,          // Trigger style variation
    Scene,          // Recall scene/preset
    Macro           // Execute macro
};

/**
 * @brief Pad configuration
 */
struct PadConfig {
    int index = 0;
    std::string name;
    PadType type = PadType::MidiNote;
    MidiNote midiNote = 60;
    MidiChannel midiChannel = 10;
    int velocity = 127;
    std::string samplePath;
    float sampleStart = 0.0f;
    float sampleEnd = 1.0f;
    bool loopSample = false;
    int loopStart = 0;
    int loopEnd = 0;
    float volume = 1.0f;
    float pan = 0.0f;
    int color = 0xFF808080;  // ARGB
    bool muted = false;
    bool soloed = false;
    std::vector<MidiNote> chordNotes;  // For chord pads
};

/**
 * @brief Multi-pad performance engine
 */
class MAESTRO_API PadEngine {
public:
    PadEngine(MidiEngine& midi, AudioEngine& audio);
    ~PadEngine();

    Result<void> initialize();

    // Pad configuration
    void setPadConfig(int index, const PadConfig& config);
    PadConfig getPadConfig(int index) const;
    std::vector<PadConfig> getAllPadConfigs() const;

    // Pad triggering
    void triggerPad(int index, int velocity = 127);
    void releasePad(int index);
    void stopPad(int index);
    void stopAllPads();

    // Pad groups
    void setPadGroup(int index, int groupId);
    int getPadGroup(int index) const;
    void setGroupExclusive(int groupId, bool exclusive);

    // Bank management
    void setCurrentBank(int bank);
    int getCurrentBank() const;
    int getBankCount() const;
    void setBankSize(int padsPerBank);

    // Sample loading
    Result<void> loadSample(int padIndex, const std::string& path);
    void unloadSample(int padIndex);

    // Recording
    void startPadRecording(int padIndex);
    void stopPadRecording(int padIndex);
    bool isRecording(int padIndex) const;

    // State
    bool isPadPlaying(int index) const;
    int getPlayingPadCount() const;

    // Callbacks
    using PadCallback = std::function<void(int index, int velocity)>;
    void setPadTriggerCallback(PadCallback callback);
    using ReleaseCallback = std::function<void(int index)>;
    void setPadReleaseCallback(ReleaseCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    MidiEngine& midi_;
    AudioEngine& audio_;
    PadCallback triggerCallback_;
    ReleaseCallback releaseCallback_;
};

} // namespace maestro
