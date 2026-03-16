// include/maestro/audio/effects/reverb.hpp
#pragma once

#include "maestro/audio/audio_engine.hpp"
#include <array>
#include <vector>

namespace maestro::effects {

/**
 * @brief Algorithmic reverb based on Schroeder/Moorer design
 */
class MAESTRO_API Reverb : public AudioEngine::AudioProcessor {
public:
    Reverb();
    ~Reverb() override;

    // AudioProcessor interface
    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Reverb"; }

    // Parameters
    void setRoomSize(float size);      // 0.0 - 1.0
    void setDamping(float damping);    // 0.0 - 1.0
    void setWet(float wet);            // 0.0 - 1.0
    void setDry(float dry);            // 0.0 - 1.0
    void setWidth(float width);        // 0.0 - 1.0
    void setFreeze(bool freeze);

    // Presets
    enum class Preset {
        SmallRoom,
        MediumRoom,
        LargeRoom,
        Hall,
        Cathedral,
        Plate,
        Chamber
    };
    void setPreset(Preset preset);

    // Getters
    float getRoomSize() const { return roomSize_; }
    float getDamping() const { return damping_; }
    float getWet() const { return wet_; }
    float getDry() const { return dry_; }

private:
    class CombFilter;
    class AllPassFilter;

    std::vector<std::unique_ptr<CombFilter>> combFilters_;
    std::vector<std::unique_ptr<AllPassFilter>> allPassFilters_;

    std::vector<int> combTuning_;
    std::vector<int> allPassTuning_;

    float roomSize_ = 0.5f;
    float damping_ = 0.5f;
    float wet_ = 0.3f;
    float dry_ = 0.7f;
    float width_ = 1.0f;
    bool freeze_ = false;

    SampleRate sampleRate_ = 44100.0f;

    // Stereo buffers
    std::vector<float> leftBuffer_;
    std::vector<float> rightBuffer_;
};

} // namespace maestro::effects
