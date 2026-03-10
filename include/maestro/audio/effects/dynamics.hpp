// include/maestro/audio/effects/dynamics.hpp
#pragma once

#include "maestro/audio/audio_engine.hpp"

namespace maestro::effects {

/**
 * @brief Compressor types
 */
enum class CompressorType {
    FeedForward,
    FeedBack,
    Opto,
    VariMu,
    FET
};

/**
 * @brief Dynamics compressor
 */
class MAESTRO_API Compressor : public AudioEngine::AudioProcessor {
public:
    Compressor();
    ~Compressor() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Compressor"; }

    // Threshold and ratio
    void setThreshold(float db);
    void setRatio(float ratio);
    void setKnee(float db);
    void setHardKnee(bool hard);

    // Timing
    void setAttack(float ms);
    void setRelease(float ms);
    void setAutoRelease(bool autoRelease);

    // Output
    void setMakeupGain(float db);
    void setMix(float mix);

    // Type
    void setType(CompressorType type);

    // Sidechain
    void enableSidechain(bool enabled);
    void setSidechainHighPass(float freq);

    // Meters
    float getGainReduction() const { return gainReduction_; }
    float getOutputLevel() const { return outputLevel_; }

    // Lookahead
    void setLookahead(float ms);

private:
    float detectLevel(float sample);
    float computeGain(float level);
    float smoothGain(float targetGain);

    float threshold_ = -20.0f;
    float ratio_ = 4.0f;
    float knee_ = 6.0f;
    bool hardKnee_ = false;
    
    float attack_ = 10.0f;
    float release_ = 100.0f;
    bool autoRelease_ = false;
    
    float makeupGain_ = 0.0f;
    float mix_ = 1.0f;
    
    CompressorType type_ = CompressorType::FeedForward;
    
    bool sidechainEnabled_ = false;
    float sidechainHPFreq_ = 100.0f;
    
    float lookahead_ = 0.0f;
    
    float gainReduction_ = 0.0f;
    float outputLevel_ = 0.0f;
    
    float currentGain_ = 1.0f;
    float envelope_ = 0.0f;
    
    SampleRate sampleRate_ = 44100.0f;
    
    // Lookahead buffer
    std::vector<float> lookaheadBuffer_;
    size_t lookaheadWritePos_ = 0;
    size_t lookaheadReadPos_ = 0;
};

/**
 * @brief Limiter (brickwall)
 */
class MAESTRO_API Limiter : public AudioEngine::AudioProcessor {
public:
    Limiter();
    ~Limiter() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Limiter"; }

    void setCeiling(float db);
    void setRelease(float ms);
    void setLookahead(float ms);

    float getGainReduction() const { return gainReduction_; }

private:
    float ceiling_ = -0.3f;
    float release_ = 50.0f;
    float lookahead_ = 1.0f;
    
    float gainReduction_ = 0.0f;
    float envelope_ = 0.0f;
    
    SampleRate sampleRate_ = 44100.0f;
    
    std::vector<float> lookaheadBuffer_;
    size_t lookaheadWritePos_ = 0;
    size_t lookaheadReadPos_ = 0;
};

/**
 * @brief Expander/Gate
 */
class MAESTRO_API Expander : public AudioEngine::AudioProcessor {
public:
    enum class Mode { Expand, Gate };

    Expander();
    ~Expander() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "Expander"; }

    void setThreshold(float db);
    void setRatio(float ratio);
    void setRange(float db);
    void setAttack(float ms);
    void setRelease(float ms);
    void setHold(float ms);
    void setMode(Mode mode);

    void enableSidechain(bool enabled);

private:
    float threshold_ = -40.0f;
    float ratio_ = 2.0f;
    float range_ = -80.0f;
    float attack_ = 10.0f;
    float release_ = 100.0f;
    float hold_ = 50.0f;
    Mode mode_ = Mode::Expand;
    
    bool sidechainEnabled_ = false;
    
    float currentGain_ = 1.0f;
    float envelope_ = 0.0f;
    int holdCounter_ = 0;
    
    SampleRate sampleRate_ = 44100.0f;
};

/**
 * @brief De-esser
 */
class MAESTRO_API DeEsser : public AudioEngine::AudioProcessor {
public:
    DeEsser();
    ~DeEsser() override;

    void prepare(SampleRate sampleRate, BufferSize bufferSize) override;
    void process(float* const* channels, uint32_t numChannels, uint32_t numFrames) override;
    void reset() override;
    std::string getName() const override { return "De-Esser"; }

    void setThreshold(float db);
    void setRatio(float ratio);
    void setFrequency(float freq);
    void setBandwidth(float q);
    void setMode(int mode);  // 0=Wideband, 1=Split-band

    float getGainReduction() const { return gainReduction_; }

private:
    float threshold_ = -20.0f;
    float ratio_ = 3.0f;
    float frequency_ = 6000.0f;
    float q_ = 1.0f;
    int mode_ = 1;
    
    float gainReduction_ = 0.0f;
    float currentGain_ = 1.0f;
    
    SampleRate sampleRate_ = 44100.0f;
    
    // Detection filter
    float detX1_ = 0, detY1_ = 0;
};

} // namespace maestro::effects
