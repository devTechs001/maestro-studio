// include/maestro/audio/effects/eq.hpp
#pragma once

#include "maestro/audio/audio_engine.hpp"
#include <array>

namespace maestro::effects {

/**
 * @brief Parametric EQ band
 */
struct EQBand {
    enum class Type {
        LowCut,
        LowShelf,
        Peak,
        HighShelf,
        HighCut,
        Notch,
        LowPass,
        HighPass,
        BandPass,
        AllPass
    };

    Type type = Type::Peak;
    float frequency = 1000.0f;
    float gain = 0.0f;
    float q = 1.0f;
    float slope = 1.0f;
    bool enabled = true;
};

/**
 * @brief Parametric equalizer
 */
class MAESTRO_API ParametricEQ : public AudioEngine::AudioProcessor {
public:
    ParametricEQ(int numBands = 4);
    ~ParametricEQ() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Parametric EQ"; }

    void setBand(int index, const EQBand& band);
    EQBand& getBand(int index);
    const EQBand& getBand(int index) const;
    int getNumBands() const { return numBands_; }

    void setPreset(int preset);

private:
    class BiquadSection;
    std::vector<std::unique_ptr<BiquadSection>> sections_;
    std::array<EQBand, 8> bands_;
    int numBands_ = 4;
    SampleRate sampleRate_ = 44100.0f;
};

/**
 * @brief Graphic equalizer
 */
class MAESTRO_API GraphicEQ : public AudioEngine::AudioProcessor {
public:
    GraphicEQ(int numBands = 31);
    ~GraphicEQ() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Graphic EQ"; }

    void setGain(int band, float gain);
    float getGain(int band) const;
    int getNumBands() const { return numBands_; }
    float getCenterFrequency(int band) const;

    void setPreset(int preset);

private:
    class BiquadSection;
    std::vector<std::unique_ptr<BiquadSection>> sections_;
    std::vector<float> gains_;
    std::vector<float> centerFreqs_;
    int numBands_ = 31;
    SampleRate sampleRate_ = 44100.0f;
};

/**
 * @brief High-pass filter
 */
class MAESTRO_API HighPassFilter : public AudioEngine::AudioProcessor {
public:
    HighPassFilter();

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "High-Pass Filter"; }

    void setFrequency(float freq);
    void setSlope(float slope);
    float getFrequency() const { return frequency_; }

private:
    float frequency_ = 100.0f;
    float slope_ = 12.0f;
    SampleRate sampleRate_ = 44100.0f;
    float x1_ = 0, x2_ = 0, y1_ = 0, y2_ = 0;
    float b0_ = 0, b1_ = 0, b2_ = 0, a1_ = 0, a2_ = 0;
};

/**
 * @brief Low-pass filter
 */
class MAESTRO_API LowPassFilter : public AudioEngine::AudioProcessor {
public:
    LowPassFilter();

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Low-Pass Filter"; }

    void setFrequency(float freq);
    void setResonance(float res);
    float getFrequency() const { return frequency_; }

private:
    float frequency_ = 1000.0f;
    float resonance_ = 0.707f;
    SampleRate sampleRate_ = 44100.0f;
    float x1_ = 0, x2_ = 0, y1_ = 0, y2_ = 0;
    float b0_ = 0, b1_ = 0, b2_ = 0, a1_ = 0, a2_ = 0;
};

} // namespace maestro::effects
