// src/audio/effects/reverb.cpp
#include "maestro/audio/effects/reverb.hpp"
#include <cstring>

namespace maestro::effects {

// Comb filter for reverb
class Reverb::CombFilter {
public:
    CombFilter(int bufferSize, int delay)
        : buffer_(bufferSize, 0.0f)
        , delay_(delay)
        , mask_(bufferSize - 1) {
    }
    
    float process(float input) {
        float output = buffer_[readPos_];
        buffer_[writePos_] = input + output * feedback_;
        writePos_ = (writePos_ + 1) & mask_;
        readPos_ = (readPos_ + 1) & mask_;
        return output;
    }
    
    void setFeedback(float feedback) { feedback_ = feedback; }
    float getFeedback() const { return feedback_; }
    void reset() { std::fill(buffer_.begin(), buffer_.end(), 0.0f); }
    
private:
    std::vector<float> buffer_;
    int delay_;
    int writePos_ = 0;
    int readPos_ = 0;
    int mask_;
    float feedback_ = 0.5f;
};

// All-pass filter for reverb
class Reverb::AllPassFilter {
public:
    AllPassFilter(int bufferSize, int delay)
        : buffer_(bufferSize, 0.0f)
        , delay_(delay)
        , mask_(bufferSize - 1) {
    }
    
    float process(float input) {
        float buffered = buffer_[readPos_];
        float output = -input + buffered;
        buffer_[writePos_] = input + buffered * feedback_;
        writePos_ = (writePos_ + 1) & mask_;
        readPos_ = (readPos_ + 1) & mask_;
        return output;
    }
    
    void setFeedback(float feedback) { feedback_ = feedback; }
    void reset() { std::fill(buffer_.begin(), buffer_.end(), 0.0f); }
    
private:
    std::vector<float> buffer_;
    int delay_;
    int writePos_ = 0;
    int readPos_ = 0;
    int mask_;
    float feedback_ = 0.5f;
};

Reverb::Reverb() {
    // Initialize comb filter delays (Schroeder values)
    combTuning_ = {1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116};
    allPassTuning_ = {556, 441, 341, 225};
}

Reverb::~Reverb() = default;

void Reverb::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    
    // Scale delays based on sample rate
    float scale = sampleRate / 44100.0f;
    
    // Create comb filters
    combFilters_.clear();
    for (size_t i = 0; i < combTuning_.size(); ++i) {
        int delay = static_cast<int>(combTuning_[i] * scale);
        int bufferSize = delay + 4096;
        combFilters_.push_back(std::make_unique<CombFilter>(bufferSize, delay));
    }
    
    // Create all-pass filters
    allPassFilters_.clear();
    for (size_t i = 0; i < allPassTuning_.size(); ++i) {
        int delay = static_cast<int>(allPassTuning_[i] * scale);
        int bufferSize = delay + 1024;
        allPassFilters_.push_back(std::make_unique<AllPassFilter>(bufferSize, delay));
    }
    
    // Allocate stereo buffers
    leftBuffer_.resize(bufferSize);
    rightBuffer_.resize(bufferSize);
    
    reset();
}

void Reverb::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;
    
    float* left = channels[0];
    float* right = channels[1];
    
    // Calculate damping coefficient
    float damp1 = damping_;
    float damp2 = 1.0f - damp1;
    
    for (uint32_t i = 0; i < numFrames; ++i) {
        float input = (left[i] + right[i]) * 0.5f;
        float outL = 0.0f, outR = 0.0f;
        
        // Accumulate comb filters
        for (size_t j = 0; j < combFilters_.size(); ++j) {
            float combOut = combFilters_[j]->process(input);
            if (j % 2 == 0) {
                outL += combOut;
            } else {
                outR += combOut;
            }
        }
        
        // Process through all-pass filters
        for (auto& apf : allPassFilters_) {
            outL = apf->process(outL);
            outR = apf->process(outR);
        }
        
        // Apply width and mix
        float wetL = outL * wet_;
        float wetR = outR * wet_;
        
        left[i] = left[i] * dry_ + wetL;
        right[i] = right[i] * dry_ + wetR;
    }
}

void Reverb::reset() {
    for (auto& cf : combFilters_) {
        cf->reset();
    }
    for (auto& apf : allPassFilters_) {
        apf->reset();
    }
}

void Reverb::setRoomSize(float size) {
    roomSize_ = std::clamp(size, 0.0f, 1.0f);
    float feedback = roomSize_ * 0.9f + 0.1f;
    for (auto& cf : combFilters_) {
        cf->setFeedback(feedback);
    }
}

void Reverb::setDamping(float damping) {
    damping_ = std::clamp(damping, 0.0f, 1.0f);
}

void Reverb::setWet(float wet) {
    wet_ = std::clamp(wet, 0.0f, 1.0f);
}

void Reverb::setDry(float dry) {
    dry_ = std::clamp(dry, 0.0f, 1.0f);
}

void Reverb::setWidth(float width) {
    width_ = std::clamp(width, 0.0f, 1.0f);
}

void Reverb::setFreeze(bool freeze) {
    freeze_ = freeze;
}

void Reverb::setPreset(Preset preset) {
    switch (preset) {
        case Preset::SmallRoom:
            setRoomSize(0.3f);
            setDamping(0.5f);
            setWet(0.25f);
            break;
        case Preset::MediumRoom:
            setRoomSize(0.5f);
            setDamping(0.5f);
            setWet(0.3f);
            break;
        case Preset::LargeRoom:
            setRoomSize(0.7f);
            setDamping(0.6f);
            setWet(0.35f);
            break;
        case Preset::Hall:
            setRoomSize(0.8f);
            setDamping(0.7f);
            setWet(0.4f);
            break;
        case Preset::Cathedral:
            setRoomSize(0.95f);
            setDamping(0.8f);
            setWet(0.5f);
            break;
        case Preset::Plate:
            setRoomSize(0.6f);
            setDamping(0.3f);
            setWet(0.4f);
            break;
        case Preset::Chamber:
            setRoomSize(0.4f);
            setDamping(0.4f);
            setWet(0.3f);
            break;
    }
}

} // namespace maestro::effects
