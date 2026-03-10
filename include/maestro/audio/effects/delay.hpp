// include/maestro/audio/effects/delay.hpp
#pragma once

#include "maestro/audio/audio_engine.hpp"
#include <vector>

namespace maestro::effects {

/**
 * @brief Stereo delay with feedback and modulation
 */
class MAESTRO_API Delay : public AudioEngine::AudioProcessor {
public:
    enum class Mode {
        Mono,
        Stereo,
        PingPong,
        Dual
    };

    Delay();
    ~Delay() override;

    // AudioProcessor interface
    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Delay"; }

    // Parameters
    void setTime(float ms);           // Delay time in milliseconds
    void setFeedback(float amount);   // Feedback amount 0.0 - 1.0
    void setMix(float mix);           // Wet/dry mix 0.0 - 1.0
    void setMode(Mode mode);
    void setSync(bool sync);
    void setSyncDivision(int division);  // 1=whole, 2=half, 4=quarter, 8=eighth, etc.
    
    // Modulation
    void setModRate(float rate);      // LFO rate in Hz
    void setModDepth(float depth);    // Modulation depth in ms

private:
    float getDelayTimeMs() const;
    float readDelay(float* delayLine, float readPos);
    void writeDelay(float* delayLine, float writePos, float value);

    std::vector<float> leftDelay_;
    std::vector<float> rightDelay_;
    
    float timeMs_ = 250.0f;
    float feedback_ = 0.3f;
    float mix_ = 0.5f;
    Mode mode_ = Mode::Stereo;
    bool sync_ = false;
    int syncDivision_ = 4;
    
    float modRate_ = 0.5f;
    float modDepth_ = 0.0f;
    float modPhase_ = 0.0f;
    
    SampleRate sampleRate_ = 44100.0f;
    int maxDelaySamples_ = 0;
    
    float leftWritePos_ = 0.0f;
    float rightWritePos_ = 0.0f;
    float leftReadPos_ = 0.0f;
    float rightReadPos_ = 0.0f;
    
    float lastLeftOut_ = 0.0f;
    float lastRightOut_ = 0.0f;
};

} // namespace maestro::effects
