// src/audio/effects/delay.cpp
#include "maestro/audio/effects/delay.hpp"
#include "maestro/audio/dsp/dsp_common.hpp"
#include <cstring>

namespace maestro::effects {

Delay::Delay() = default;
Delay::~Delay() = default;

void Delay::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    
    // Calculate max delay time (2 seconds)
    maxDelaySamples_ = static_cast<int>(2000.0f * sampleRate / 1000.0f) + 1024;
    
    leftDelay_.resize(maxDelaySamples_, 0.0f);
    rightDelay_.resize(maxDelaySamples_, 0.0f);
    
    reset();
}

void Delay::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;
    
    float* left = channels[0];
    float* right = channels[1];
    
    float delaySamples = timeMs_ * sampleRate_ / 1000.0f;
    float feedbackAmount = feedback_;
    float wetMix = mix_;
    float dryMix = 1.0f - wetMix;
    
    // Apply sync if enabled
    if (sync_) {
        float beatsPerDelay = 4.0f / syncDivision_;
        float secondsPerBeat = 60.0f / 120.0f;  // Assume 120 BPM
        delaySamples = beatsPerDelay * secondsPerBeat * sampleRate_;
    }
    
    // Apply modulation
    float mod = 0.0f;
    if (modDepth_ > 0.0f) {
        modPhase_ += modRate_ / sampleRate_;
        if (modPhase_ > 1.0f) modPhase_ -= 1.0f;
        mod = std::sin(2.0f * M_PI * modPhase_) * modDepth_ * sampleRate_ / 1000.0f;
    }
    
    for (uint32_t i = 0; i < numFrames; ++i) {
        float inputL = left[i];
        float inputR = right[i];
        
        // Calculate read positions with modulation
        float readPosL = leftWritePos_ - delaySamples - mod;
        float readPosR = rightWritePos_ - delaySamples + mod;
        
        // Wrap positions
        while (readPosL < 0) readPosL += maxDelaySamples_;
        while (readPosL >= maxDelaySamples_) readPosL -= maxDelaySamples_;
        while (readPosR < 0) readPosR += maxDelaySamples_;
        while (readPosR >= maxDelaySamples_) readPosR -= maxDelaySamples_;
        
        // Read from delay line (simple interpolation)
        int readIdxL = static_cast<int>(readPosL);
        int readIdxR = static_cast<int>(readPosR);
        float fracL = readPosL - readIdxL;
        float fracR = readPosR - readIdxR;
        
        float delayedL = dsp::lerp(leftDelay_[readIdxL], 
                                    leftDelay_[(readIdxL + 1) % maxDelaySamples_], fracL);
        float delayedR = dsp::lerp(rightDelay_[readIdxR],
                                    rightDelay_[(readIdxR + 1) % maxDelaySamples_], fracR);
        
        // Apply feedback
        float feedbackL = delayedL * feedbackAmount;
        float feedbackR = delayedR * feedbackAmount;
        
        // Ping-pong mode
        if (mode_ == Mode::PingPong) {
            feedbackR = delayedL * feedbackAmount;
            feedbackL = delayedR * feedbackAmount;
        }
        
        // Write to delay line
        int writeIdxL = static_cast<int>(leftWritePos_);
        int writeIdxR = static_cast<int>(rightWritePos_);
        leftDelay_[writeIdxL] = inputL + feedbackL;
        rightDelay_[writeIdxR] = inputR + feedbackR;
        
        // Advance write positions
        leftWritePos_ += 1.0f;
        rightWritePos_ += 1.0f;
        if (leftWritePos_ >= maxDelaySamples_) leftWritePos_ -= maxDelaySamples_;
        if (rightWritePos_ >= maxDelaySamples_) rightWritePos_ -= maxDelaySamples_;
        
        // Mix output
        left[i] = inputL * dryMix + delayedL * wetMix;
        right[i] = inputR * dryMix + delayedR * wetMix;
    }
}

void Delay::reset() {
    std::fill(leftDelay_.begin(), leftDelay_.end(), 0.0f);
    std::fill(rightDelay_.begin(), rightDelay_.end(), 0.0f);
    leftWritePos_ = 0.0f;
    rightWritePos_ = 0.0f;
    leftReadPos_ = 0.0f;
    rightReadPos_ = 0.0f;
    lastLeftOut_ = 0.0f;
    lastRightOut_ = 0.0f;
    modPhase_ = 0.0f;
}

void Delay::setTime(float ms) {
    timeMs_ = std::max(0.0f, ms);
}

void Delay::setFeedback(float amount) {
    feedback_ = std::clamp(amount, 0.0f, 0.95f);
}

void Delay::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void Delay::setMode(Mode mode) {
    mode_ = mode;
}

void Delay::setSync(bool sync) {
    sync_ = sync;
}

void Delay::setSyncDivision(int division) {
    syncDivision_ = division;
}

void Delay::setModRate(float rate) {
    modRate_ = rate;
}

void Delay::setModDepth(float depth) {
    modDepth_ = depth;
}

} // namespace maestro::effects
