// include/maestro/voice/voice_engine.hpp
#pragma once

#include "maestro/core/types.hpp"
#include <memory>
#include <vector>
#include <string>

namespace maestro {

class AudioEngine;

/**
 * @brief Voice/Sound definition
 */
struct Voice {
    std::string name;
    uint8_t msb = 0;        // Bank MSB
    uint8_t lsb = 0;        // Bank LSB
    uint8_t program = 0;    // Program Change
    std::string category;
    std::vector<uint8_t> initSysEx;

    bool operator==(const Voice& other) const {
        return msb == other.msb && lsb == other.lsb && program == other.program;
    }
};

/**
 * @brief Voice synthesis and management engine
 */
class MAESTRO_API VoiceEngine {
public:
    VoiceEngine(AudioEngine& audio);
    ~VoiceEngine();

    Result<void> initialize();

    // Voice selection
    void setVoice(int part, const Voice& voice);
    Voice getVoice(int part) const;
    std::vector<Voice> getAvailableVoices() const;

    // Voice parameters
    void setVolume(int part, float volume);
    void setPan(int part, float pan);
    void setOctave(int part, int octave);
    void setReverbSend(int part, float level);
    void setChorusSend(int part, float level);

    // Voice layering
    void enableVoicePart(int part, bool enabled);
    bool isVoicePartEnabled(int part) const;

    // Split point
    void setSplitPoint(MidiNote note);
    MidiNote getSplitPoint() const;
    void enableSplit(bool enabled);
    bool isSplitEnabled() const;

    // SoundFont support
    Result<void> loadSoundFont(const std::string& path);
    void unloadSoundFont();

    // Preset management
    Result<void> savePreset(const std::string& name, const std::string& path);
    Result<void> loadPreset(const std::string& path);

    // Processing
    void process(float* left, float* right, uint32_t numFrames);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    AudioEngine& audio_;
};

} // namespace maestro
