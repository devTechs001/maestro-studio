// src/audio/effects/dynamics.cpp
#include "maestro/audio/effects/dynamics.hpp"
#include "maestro/audio/dsp/dsp_common.hpp"
#include <cmath>
#include <algorithm>

namespace maestro::effects {

Compressor::Compressor() = default;
Compressor::~Compressor() = default;

void Compressor::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    
    if (lookahead_ > 0) {
        int delaySamples = static_cast<int>(lookahead_ * sampleRate_ / 1000.0f);
        lookaheadBuffer_.resize(delaySamples + bufferSize, 0.0f);
        lookaheadWritePos_ = 0;
        lookaheadReadPos_ = delaySamples;
    }
    
    reset();
}

void Compressor::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    float attackCoeff = std::exp(-1000.0f / (attack_ * sampleRate_));
    float releaseCoeff = std::exp(-1000.0f / (release_ * sampleRate_));
    float makeupLinear = dsp::dbToGain(makeupGain_);
    
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float input = channel[i];
            float processInput = input;
            
            // Apply lookahead delay
            if (lookahead_ > 0 && !lookaheadBuffer_.empty()) {
                lookaheadBuffer_[lookaheadWritePos_] = input;
                processInput = lookaheadBuffer_[lookaheadReadPos_];
                lookaheadWritePos_ = (lookaheadWritePos_ + 1) % lookaheadBuffer_.size();
                lookaheadReadPos_ = (lookaheadReadPos_ + 1) % lookaheadBuffer_.size();
            }
            
            // Detect level (RMS or peak)
            float detectedLevel = std::abs(processInput);
            
            // Apply sidechain high-pass filter
            if (sidechainEnabled_) {
                float hpCoeff = dsp::onePoleCoeff(sidechainHPFreq_, sampleRate_);
                detectedLevel = processInput - hpCoeff * envelope_;
                envelope_ = hpCoeff * envelope_ + (1.0f - hpCoeff) * detectedLevel;
                detectedLevel = std::abs(detectedLevel);
            }
            
            // Convert to dB
            float levelDb = dsp::gainToDb(detectedLevel);
            
            // Compute gain reduction
            float targetGainDb = computeGain(levelDb);
            float targetGainLinear = dsp::dbToGain(targetGainDb);
            
            // Smooth gain
            if (targetGainLinear < currentGain_) {
                currentGain_ = attackCoeff * currentGain_ + (1.0f - attackCoeff) * targetGainLinear;
            } else {
                currentGain_ = releaseCoeff * currentGain_ + (1.0f - releaseCoeff) * targetGainLinear;
            }
            
            // Apply makeup gain and mix
            float output = processInput * currentGain_ * makeupLinear;
            
            if (mix_ < 1.0f) {
                output = input * (1.0f - mix_) + output * mix_;
            }
            
            channel[i] = output;
            
            // Update meters
            gainReduction_ = dsp::gainToDb(currentGain_);
            outputLevel_ = dsp::gainToDb(std::abs(output));
        }
    }
}

void Compressor::reset() {
    currentGain_ = 1.0f;
    envelope_ = 0.0f;
    gainReduction_ = 0.0f;
    outputLevel_ = 0.0f;
    
    if (!lookaheadBuffer_.empty()) {
        std::fill(lookaheadBuffer_.begin(), lookaheadBuffer_.end(), 0.0f);
    }
}

float Compressor::computeGain(float levelDb) {
    float gainDb = 0.0f;
    
    if (hardKnee_ || knee_ <= 0.001f) {
        // Hard knee
        if (levelDb > threshold_) {
            gainDb = (threshold_ - levelDb) * (1.0f - 1.0f / ratio_);
        }
    } else {
        // Soft knee
        float lowerBound = threshold_ - knee_ / 2.0f;
        float upperBound = threshold_ + knee_ / 2.0f;
        
        if (levelDb < lowerBound) {
            gainDb = 0.0f;
        } else if (levelDb > upperBound) {
            gainDb = (threshold_ - levelDb) * (1.0f - 1.0f / ratio_);
        } else {
            // In knee region
            float t = (levelDb - lowerBound) / knee_;
            float curveGain = (threshold_ - levelDb) * (1.0f - 1.0f / ratio_);
            gainDb = curveGain * t * t * (3.0f - 2.0f * t);
        }
    }
    
    return gainDb;
}

void Compressor::setThreshold(float db) {
    threshold_ = db;
}

void Compressor::setRatio(float ratio) {
    ratio_ = std::max(1.0f, ratio);
}

void Compressor::setKnee(float db) {
    knee_ = std::max(0.0f, db);
}

void Compressor::setHardKnee(bool hard) {
    hardKnee_ = hard;
}

void Compressor::setAttack(float ms) {
    attack_ = std::max(0.01f, ms);
}

void Compressor::setRelease(float ms) {
    release_ = std::max(0.01f, ms);
}

void Compressor::setAutoRelease(bool autoRelease) {
    autoRelease_ = autoRelease;
}

void Compressor::setMakeupGain(float db) {
    makeupGain_ = db;
}

void Compressor::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void Compressor::setType(CompressorType type) {
    type_ = type;
}

void Compressor::enableSidechain(bool enabled) {
    sidechainEnabled_ = enabled;
}

void Compressor::setSidechainHighPass(float freq) {
    sidechainHPFreq_ = freq;
}

void Compressor::setLookahead(float ms) {
    lookahead_ = std::max(0.0f, ms);
}

// Limiter implementation
Limiter::Limiter() = default;
Limiter::~Limiter() = default;

void Limiter::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    
    int delaySamples = static_cast<int>(lookahead_ * sampleRate_ / 1000.0f) + 64;
    lookaheadBuffer_.resize(delaySamples + bufferSize, 0.0f);
    lookaheadWritePos_ = 0;
    lookaheadReadPos_ = delaySamples;
    
    reset();
}

void Limiter::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    float releaseCoeff = std::exp(-1000.0f / (release_ * sampleRate_));
    float ceilingLinear = dsp::dbToGain(ceiling_);
    
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float input = channel[i];
            
            // Write to lookahead buffer
            lookaheadBuffer_[lookaheadWritePos_] = input;
            lookaheadWritePos_ = (lookaheadWritePos_ + 1) % lookaheadBuffer_.size();
            
            // Read delayed sample
            float delayedInput = lookaheadBuffer_[lookaheadReadPos_];
            lookaheadReadPos_ = (lookaheadReadPos_ + 1) % lookaheadBuffer_.size();
            
            // Detect peak
            float peak = std::abs(delayedInput);
            
            // Update envelope
            if (peak > envelope_) {
                envelope_ = peak;
            } else {
                envelope_ = releaseCoeff * envelope_ + (1.0f - releaseCoeff) * peak;
            }
            
            // Compute gain
            float gain = 1.0f;
            if (envelope_ > ceilingLinear) {
                gain = ceilingLinear / envelope_;
            }
            
            channel[i] = delayedInput * gain;
            gainReduction_ = dsp::gainToDb(gain);
        }
    }
}

void Limiter::reset() {
    envelope_ = 0.0f;
    gainReduction_ = 0.0f;
    
    if (!lookaheadBuffer_.empty()) {
        std::fill(lookaheadBuffer_.begin(), lookaheadBuffer_.end(), 0.0f);
    }
}

void Limiter::setCeiling(float db) {
    ceiling_ = db;
}

void Limiter::setRelease(float ms) {
    release_ = std::max(0.01f, ms);
}

void Limiter::setLookahead(float ms) {
    lookahead_ = std::max(0.0f, ms);
}

// Expander implementation
Expander::Expander() = default;
Expander::~Expander() = default;

void Expander::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    reset();
}

void Expander::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    float attackCoeff = std::exp(-1000.0f / (attack_ * sampleRate_));
    float releaseCoeff = std::exp(-1000.0f / (release_ * sampleRate_));
    float rangeLinear = dsp::dbToGain(range_);
    
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float input = channel[i];
            
            // Detect level
            float detectedLevel = std::abs(input);
            float levelDb = dsp::gainToDb(detectedLevel);
            
            // Compute target gain
            float targetGain = 1.0f;
            
            if (mode_ == Mode::Gate) {
                if (levelDb < threshold_) {
                    targetGain = rangeLinear;
                }
            } else {
                if (levelDb < threshold_) {
                    float ratioDb = (threshold_ - levelDb) * (1.0f - 1.0f / ratio_);
                    targetGain = dsp::dbToGain(-ratioDb);
                    targetGain = std::max(targetGain, rangeLinear);
                }
            }
            
            // Smooth gain
            if (targetGain < currentGain_) {
                currentGain_ = attackCoeff * currentGain_ + (1.0f - attackCoeff) * targetGain;
            } else {
                currentGain_ = releaseCoeff * currentGain_ + (1.0f - releaseCoeff) * targetGain;
            }
            
            // Apply hold
            if (currentGain_ > 0.99f) {
                holdCounter_ = static_cast<int>(hold_ * sampleRate_ / 1000.0f);
            }
            if (holdCounter_ > 0) {
                currentGain_ = 1.0f;
                holdCounter_--;
            }
            
            channel[i] = input * currentGain_;
        }
    }
}

void Expander::reset() {
    currentGain_ = 1.0f;
    envelope_ = 0.0f;
    holdCounter_ = 0;
}

void Expander::setThreshold(float db) {
    threshold_ = db;
}

void Expander::setRatio(float ratio) {
    ratio_ = std::max(1.0f, ratio);
}

void Expander::setRange(float db) {
    range_ = db;
}

void Expander::setAttack(float ms) {
    attack_ = std::max(0.01f, ms);
}

void Expander::setRelease(float ms) {
    release_ = std::max(0.01f, ms);
}

void Expander::setHold(float ms) {
    hold_ = std::max(0.0f, ms);
}

void Expander::setMode(Mode mode) {
    mode_ = mode;
}

void Expander::enableSidechain(bool enabled) {
    sidechainEnabled_ = enabled;
}

// DeEsser implementation
DeEsser::DeEsser() = default;
DeEsser::~DeEsser() = default;

void DeEsser::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    reset();
}

void DeEsser::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    float attackCoeff = std::exp(-1000.0f / (10.0f * sampleRate_));
    float releaseCoeff = std::exp(-1000.0f / (50.0f * sampleRate_));
    
    for (uint32_t ch = 0; ch < numChannels; ++ch) {
        float* channel = channels[ch];
        
        for (uint32_t i = 0; i < numFrames; ++i) {
            float input = channel[i];
            
            // High-pass filter for detection (sibilance band)
            float detected = input - detX1_ + 0.9f * detY1_;
            detX1_ = input;
            detY1_ = detected;
            
            float detectedLevel = std::abs(detected);
            float levelDb = dsp::gainToDb(detectedLevel);
            
            // Compute gain reduction
            float targetGain = 1.0f;
            if (levelDb > threshold_) {
                float reductionDb = (levelDb - threshold_) * (1.0f - 1.0f / ratio_);
                targetGain = dsp::dbToGain(-reductionDb);
            }
            
            // Smooth gain
            if (targetGain < currentGain_) {
                currentGain_ = attackCoeff * currentGain_ + (1.0f - attackCoeff) * targetGain;
            } else {
                currentGain_ = releaseCoeff * currentGain_ + (1.0f - releaseCoeff) * targetGain;
            }
            
            // Apply reduction
            if (mode_ == 0) {
                // Wideband - reduce entire signal
                channel[i] = input * currentGain_;
            } else {
                // Split-band - only reduce sibilance band
                channel[i] = input - detected * (1.0f - currentGain_);
            }
            
            gainReduction_ = dsp::gainToDb(currentGain_);
        }
    }
}

void DeEsser::reset() {
    detX1_ = detY1_ = 0.0f;
    currentGain_ = 1.0f;
    gainReduction_ = 0.0f;
}

void DeEsser::setThreshold(float db) {
    threshold_ = db;
}

void DeEsser::setRatio(float ratio) {
    ratio_ = std::max(1.0f, ratio);
}

void DeEsser::setFrequency(float freq) {
    frequency_ = freq;
}

void DeEsser::setBandwidth(float q) {
    q_ = q;
}

void DeEsser::setMode(int mode) {
    mode_ = mode;
}

} // namespace maestro::effects
