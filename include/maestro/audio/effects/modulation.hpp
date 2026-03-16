// include/maestro/audio/effects/modulation.hpp
#pragma once

#include "maestro/audio/audio_engine.hpp"
#include "maestro/audio/dsp/dsp_common.hpp"
#include <array>

namespace maestro::effects {

/**
 * @brief Stereo Chorus effect
 */
class MAESTRO_API Chorus : public AudioEngine::AudioProcessor {
public:
    Chorus();
    ~Chorus() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;

    std::string getName() const override { return "Chorus"; }

    // Parameters
    void setRate(float hz);            // 0.1 - 10 Hz
    void setDepth(float depth);        // 0.0 - 1.0
    void setMix(float mix);            // 0.0 - 1.0
    void setFeedback(float feedback);  // 0.0 - 0.9
    void setDelay(float ms);           // 1.0 - 30.0 ms (base delay)
    void setVoices(int voices);        // 1 - 4
    void setStereoWidth(float width);  // 0.0 - 1.0

    // Presets
    enum class Mode { Classic, Ensemble, Vintage, Shimmer };
    void setMode(Mode mode);

private:
    static constexpr size_t MAX_DELAY = 8192;

    struct Voice {
        dsp::CircularBuffer<float, MAX_DELAY> delayLine;
        dsp::LFO lfo;
        float phaseOffset = 0.0f;
    };

    std::array<Voice, 4> voicesL_, voicesR_;
    int numVoices_ = 2;

    float rate_ = 1.0f;
    float depth_ = 0.5f;
    float mix_ = 0.5f;
    float feedback_ = 0.0f;
    float baseDelayMs_ = 7.0f;
    float stereoWidth_ = 1.0f;

    float sampleRate_ = 44100.0f;
};

/**
 * @brief Phaser effect
 */
class MAESTRO_API Phaser : public AudioEngine::AudioProcessor {
public:
    Phaser();
    ~Phaser() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;

    std::string getName() const override { return "Phaser"; }

    void setRate(float hz);
    void setDepth(float depth);
    void setFeedback(float feedback);
    void setStages(int stages);        // 2, 4, 6, 8, 10, 12
    void setCenterFreq(float hz);
    void setFreqRange(float octaves);
    void setMix(float mix);
    void setStereo(float width);

private:
    static constexpr int MAX_STAGES = 12;

    std::array<dsp::BiquadFilter, MAX_STAGES> allpassL_, allpassR_;
    dsp::LFO lfoL_, lfoR_;

    int numStages_ = 6;
    float rate_ = 0.5f;
    float depth_ = 0.7f;
    float feedback_ = 0.5f;
    float centerFreq_ = 1000.0f;
    float freqRange_ = 2.0f;
    float mix_ = 0.5f;
    float stereoWidth_ = 0.5f;

    float lastOutputL_ = 0.0f, lastOutputR_ = 0.0f;

    float sampleRate_ = 44100.0f;
};

/**
 * @brief Flanger effect
 */
class MAESTRO_API Flanger : public AudioEngine::AudioProcessor {
public:
    Flanger();
    ~Flanger() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;

    std::string getName() const override { return "Flanger"; }

    void setRate(float hz);
    void setDepth(float depth);
    void setFeedback(float feedback);
    void setDelay(float ms);           // 0.1 - 10 ms
    void setMix(float mix);
    void setStereo(bool stereo);
    void setManual(float position);    // For manual flanging

    // Modes
    enum class Mode { Normal, ThroughZero, Barber };
    void setMode(Mode mode);

private:
    static constexpr size_t MAX_DELAY = 2048;

    dsp::CircularBuffer<float, MAX_DELAY> delayL_, delayR_;
    dsp::LFO lfoL_, lfoR_;

    float rate_ = 0.2f;
    float depth_ = 0.7f;
    float feedback_ = 0.5f;
    float delayMs_ = 2.0f;
    float mix_ = 0.5f;
    float manual_ = 0.5f;
    bool stereo_ = true;
    Mode mode_ = Mode::Normal;

    float sampleRate_ = 44100.0f;
};

/**
 * @brief Tremolo effect
 */
class MAESTRO_API Tremolo : public AudioEngine::AudioProcessor {
public:
    Tremolo();
    ~Tremolo() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;

    std::string getName() const override { return "Tremolo"; }

    void setRate(float hz);
    void setDepth(float depth);
    void setWaveform(dsp::LFO::Waveform waveform);
    void setStereoPhase(float degrees);
    void setSync(bool sync);
    void setTempo(double bpm);
    void setNoteValue(float note);

private:
    dsp::LFO lfoL_, lfoR_;

    float rate_ = 4.0f;
    float depth_ = 0.5f;
    float stereoPhase_ = 0.0f;
    bool sync_ = false;
    double tempo_ = 120.0;
    float noteValue_ = 0.25f;

    float sampleRate_ = 44100.0f;
};

/**
 * @brief Rotary Speaker (Leslie) simulation
 */
class MAESTRO_API RotarySpeaker : public AudioEngine::AudioProcessor {
public:
    RotarySpeaker();
    ~RotarySpeaker() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;

    std::string getName() const override { return "Rotary Speaker"; }

    enum class Speed { Slow, Fast, Stop };
    void setSpeed(Speed speed);
    Speed getSpeed() const { return speed_; }

    void setSlowRate(float hz);
    void setFastRate(float hz);
    void setAcceleration(float time);   // seconds to change speed
    void setHornLevel(float level);
    void setDrumLevel(float level);
    void setDistance(float meters);
    void setDrive(float amount);

private:
    Speed speed_ = Speed::Slow;

    float slowRate_ = 0.8f;
    float fastRate_ = 6.7f;
    float acceleration_ = 2.0f;
    float hornLevel_ = 1.0f;
    float drumLevel_ = 0.7f;
    float distance_ = 2.0f;
    float drive_ = 0.0f;

    // Horn simulation
    dsp::LFO hornLFO_;
    float hornAngle_ = 0.0f;
    float hornTargetRate_ = 0.8f;
    float hornCurrentRate_ = 0.8f;

    // Drum simulation
    dsp::LFO drumLFO_;
    float drumAngle_ = 0.0f;
    float drumTargetRate_ = 0.67f;
    float drumCurrentRate_ = 0.67f;

    // Crossover
    dsp::BiquadFilter lowpassL_, lowpassR_;
    dsp::BiquadFilter highpassL_, highpassR_;

    // Delay lines for doppler
    static constexpr size_t MAX_DELAY = 512;
    dsp::CircularBuffer<float, MAX_DELAY> dopplerL_, dopplerR_;

    float sampleRate_ = 44100.0f;
};

/**
 * @brief Vibrato effect (pitch modulation)
 */
class MAESTRO_API Vibrato : public AudioEngine::AudioProcessor {
public:
    Vibrato();
    ~Vibrato() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;

    std::string getName() const override { return "Vibrato"; }

    void setRate(float hz);
    void setDepth(float depth);        // 0.0 - 1.0
    void setDelay(float ms);           // Base delay for modulation
    void setWaveform(dsp::LFO::Waveform waveform);

private:
    static constexpr size_t MAX_DELAY = 4096;

    dsp::CircularBuffer<float, MAX_DELAY> delayL_, delayR_;
    dsp::LFO lfo_;

    float rate_ = 5.0f;
    float depth_ = 0.3f;
    float delayMs_ = 5.0f;

    float sampleRate_ = 44100.0f;
};

/**
 * @brief Pitch shifter using delay-line modulation
 */
class MAESTRO_API PitchShifter : public AudioEngine::AudioProcessor {
public:
    PitchShifter();
    ~PitchShifter() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;

    std::string getName() const override { return "Pitch Shifter"; }

    void setSemitones(float semitones);  // -12 to +12
    void setMix(float mix);
    void setQuality(int quality);        // 0=fast, 1=good, 2=best

private:
    static constexpr size_t MAX_DELAY = 8192;
    static constexpr size_t GRAIN_SIZE = 1024;

    struct Grain {
        dsp::CircularBuffer<float, MAX_DELAY> delayLine;
        float position = 0.0f;
        float envelope = 0.0f;
        bool active = false;
    };

    std::array<Grain, 4> grainsL_, grainsR_;
    int activeGrain_ = 0;
    float pitchRatio_ = 1.0f;
    float mix_ = 1.0f;
    int quality_ = 1;

    float sampleRate_ = 44100.0f;
};

} // namespace maestro::effects
