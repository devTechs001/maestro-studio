// src/audio/effects/eq.cpp
#include "maestro/audio/effects/eq.hpp"
#include "maestro/audio/dsp/dsp_common.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace maestro::effects {

// Biquad section for EQ
class ParametricEQ::BiquadSection {
public:
    void setCoefficients(float b0, float b1, float b2, float a1, float a2) {
        b0_ = b0; b1_ = b1; b2_ = b2;
        a1_ = a1; a2_ = a2;
    }
    
    float process(float input) {
        float output = b0_ * input + x1_;
        x1_ = b1_ * input - a1_ * output + x2_;
        x2_ = b2_ * input - a2_ * output;
        return output;
    }
    
    void reset() {
        x1_ = x2_ = 0;
    }
    
private:
    float b0_, b1_, b2_, a1_, a2_;
    float x1_ = 0, x2_ = 0;
};

ParametricEQ::ParametricEQ(int numBands) : numBands_(std::min(numBands, 8)) {
    for (int i = 0; i < numBands_; ++i) {
        bands_[i].frequency = 100.0f * std::pow(10.0f, i);
        bands_[i].q = 1.0f;
    }
}

ParametricEQ::~ParametricEQ() = default;

void ParametricEQ::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    sections_.clear();
    
    for (int i = 0; i < numBands_; ++i) {
        if (bands_[i].enabled) {
            auto section = std::make_unique<BiquadSection>();
            sections_.push_back(std::move(section));
        }
    }
    
    reset();
}

void ParametricEQ::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float sample = channel[i];
            
            for (auto& section : sections_) {
                sample = section->process(sample);
            }
            
            channel[i] = sample;
        }
    }
}

void ParametricEQ::reset() {
    for (auto& section : sections_) {
        section->reset();
    }
}

void ParametricEQ::setBand(int index, const EQBand& band) {
    if (index >= 0 && index < numBands_) {
        bands_[index] = band;
    }
}

EQBand& ParametricEQ::getBand(int index) {
    return bands_[index];
}

const EQBand& ParametricEQ::getBand(int index) const {
    return bands_[index];
}

void ParametricEQ::setPreset(int preset) {
    // Apply EQ preset
    switch (preset) {
        case 0:  // Flat
            for (int i = 0; i < numBands_; ++i) {
                bands_[i].gain = 0.0f;
            }
            break;
        case 1:  // Rock
            bands_[0].gain = 5.0f;
            bands_[1].gain = 3.0f;
            bands_[2].gain = 0.0f;
            bands_[3].gain = 4.0f;
            break;
        case 2:  // Pop
            bands_[0].gain = 2.0f;
            bands_[1].gain = 4.0f;
            bands_[2].gain = 3.0f;
            bands_[3].gain = 2.0f;
            break;
        case 3:  // Jazz
            bands_[0].gain = 3.0f;
            bands_[1].gain = 2.0f;
            bands_[2].gain = -2.0f;
            bands_[3].gain = 3.0f;
            break;
    }
}

// GraphicEQ implementation
class GraphicEQ::BiquadSection {
public:
    void setCoefficients(float b0, float b1, float b2, float a1, float a2) {
        b0_ = b0; b1_ = b1; b2_ = b2;
        a1_ = a1; a2_ = a2;
    }
    
    float process(float input, float gain) {
        float output = b0_ * input + x1_;
        x1_ = b1_ * input - a1_ * output + x2_;
        x2_ = b2_ * input - a2_ * output;
        return output * gain;
    }
    
    void reset() {
        x1_ = x2_ = 0;
    }
    
private:
    float b0_, b1_, b2_, a1_, a2_;
    float x1_ = 0, x2_ = 0;
};

GraphicEQ::GraphicEQ(int numBands) : numBands_(numBands) {
    gains_.resize(numBands, 0.0f);
    
    // Initialize center frequencies (ISO standard)
    static const float stdFreqs[] = {
        20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500,
        630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000,
        10000, 12500, 16000, 20000
    };
    
    for (int i = 0; i < numBands && i < 31; ++i) {
        centerFreqs_.push_back(stdFreqs[i]);
        sections_.push_back(std::make_unique<BiquadSection>());
    }
}

GraphicEQ::~GraphicEQ() = default;

void GraphicEQ::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    reset();
}

void GraphicEQ::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float sample = channel[i];
            float output = 0.0f;
            
            for (size_t b = 0; b < sections_.size(); ++b) {
                float gain = dsp::dbToGain(gains_[b]);
                output += sections_[b]->process(sample, gain);
            }
            
            channel[i] = output;
        }
    }
}

void GraphicEQ::reset() {
    for (auto& section : sections_) {
        section->reset();
    }
}

void GraphicEQ::setGain(int band, float gain) {
    if (band >= 0 && band < static_cast<int>(gains_.size())) {
        gains_[band] = std::clamp(gain, -12.0f, 12.0f);
    }
}

float GraphicEQ::getGain(int band) const {
    if (band >= 0 && band < static_cast<int>(gains_.size())) {
        return gains_[band];
    }
    return 0.0f;
}

float GraphicEQ::getCenterFrequency(int band) const {
    if (band >= 0 && band < static_cast<int>(centerFreqs_.size())) {
        return centerFreqs_[band];
    }
    return 1000.0f;
}

void GraphicEQ::setPreset(int preset) {
    std::fill(gains_.begin(), gains_.end(), 0.0f);
    
    switch (preset) {
        case 0:  // Flat
            break;
        case 1:  // Smile
            gains_[0] = 6.0f;
            gains_[gains_.size()/2] = -3.0f;
            gains_.back() = 6.0f;
            break;
        case 2:  // Vocal boost
            for (size_t i = 0; i < gains_.size(); ++i) {
                float freq = centerFreqs_[i];
                if (freq >= 500 && freq <= 4000) {
                    gains_[i] = 4.0f;
                }
            }
            break;
    }
}

// HighPassFilter implementation
HighPassFilter::HighPassFilter() = default;

void HighPassFilter::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    reset();
    
    float freq = std::clamp(frequency_, 20.0f, sampleRate_ / 2.0f);
    float w0 = 2.0f * M_PI * freq / sampleRate_;
    float alpha = std::sin(w0) / (2.0f * std::pow(2.0f, slope_ / 12.0f));
    float cosw0 = std::cos(w0);
    
    b0_ = (1.0f + cosw0) / 2.0f;
    b1_ = -(1.0f + cosw0);
    b2_ = (1.0f + cosw0) / 2.0f;
    float a0inv = 1.0f / (1.0f + alpha);
    a1_ = -2.0f * cosw0 * a0inv;
    a2_ = (1.0f - alpha) * a0inv;
    b0_ *= a0inv;
    b1_ *= a0inv;
    b2_ *= a0inv;
}

void HighPassFilter::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float x = channel[i];
            float y = b0_ * x + x1_;
            x1_ = b1_ * x - a1_ * y + x2_;
            x2_ = b2_ * x - a2_ * y;
            y2_ = y;
            channel[i] = y;
        }
    }
}

void HighPassFilter::reset() {
    x1_ = x2_ = y1_ = y2_ = 0;
}

void HighPassFilter::setFrequency(float freq) {
    frequency_ = std::clamp(freq, 20.0f, 20000.0f);
}

void HighPassFilter::setSlope(float slope) {
    slope_ = std::clamp(slope, 6.0f, 48.0f);
}

// LowPassFilter implementation
LowPassFilter::LowPassFilter() = default;

void LowPassFilter::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    reset();
    
    float freq = std::clamp(frequency_, 20.0f, sampleRate_ / 2.0f);
    float w0 = 2.0f * M_PI * freq / sampleRate_;
    float alpha = std::sin(w0) / (2.0f * resonance_);
    float cosw0 = std::cos(w0);
    
    b0_ = (1.0f - cosw0) / 2.0f;
    b1_ = 1.0f - cosw0;
    b2_ = (1.0f - cosw0) / 2.0f;
    float a0inv = 1.0f / (1.0f + alpha);
    a1_ = -2.0f * cosw0 * a0inv;
    a2_ = (1.0f - alpha) * a0inv;
    b0_ *= a0inv;
    b1_ *= a0inv;
    b2_ *= a0inv;
}

void LowPassFilter::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float x = channel[i];
            float y = b0_ * x + x1_;
            x1_ = b1_ * x - a1_ * y + x2_;
            x2_ = b2_ * x - a2_ * y;
            y2_ = y;
            channel[i] = y;
        }
    }
}

void LowPassFilter::reset() {
    x1_ = x2_ = y1_ = y2_ = 0;
}

void LowPassFilter::setFrequency(float freq) {
    frequency_ = std::clamp(freq, 20.0f, 20000.0f);
}

void LowPassFilter::setResonance(float res) {
    resonance_ = std::clamp(res, 0.001f, 20.0f);
}

} // namespace maestro::effects
