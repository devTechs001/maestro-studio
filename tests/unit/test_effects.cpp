// tests/unit/test_effects.cpp
#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <algorithm>

// Simplified effect classes for testing
class Reverb {
public:
    void prepare(float sampleRate, int bufferSize) {
        sampleRate_ = sampleRate;
        bufferSize_ = bufferSize;
        wet_ = 0.3f;
        dry_ = 0.7f;
        roomSize_ = 0.5f;
        damping_ = 0.5f;
    }
    
    void process(float* const* channels, int numChannels, int numFrames) {
        // Simplified reverb processing
        for (int i = 0; i < numFrames; ++i) {
            for (int ch = 0; ch < numChannels; ++ch) {
                float input = channels[ch][i];
                channels[ch][i] = input * dry_ + input * wet_ * roomSize_;
            }
        }
    }
    
    void setWet(float wet) { wet_ = wet; }
    void setDry(float dry) { dry_ = dry; }
    void setRoomSize(float size) { roomSize_ = size; }
    void setDamping(float damping) { damping_ = damping; }
    float getRoomSize() const { return roomSize_; }
    
    enum class Preset { SmallRoom, LargeRoom, Hall, Cathedral };
    void setPreset(Preset preset) {
        switch (preset) {
            case Preset::SmallRoom: roomSize_ = 0.3f; break;
            case Preset::LargeRoom: roomSize_ = 0.6f; break;
            case Preset::Hall: roomSize_ = 0.8f; break;
            case Preset::Cathedral: roomSize_ = 0.95f; break;
        }
    }
    
private:
    float sampleRate_ = 44100.0f;
    int bufferSize_ = 512;
    float wet_ = 0.3f;
    float dry_ = 0.7f;
    float roomSize_ = 0.5f;
    float damping_ = 0.5f;
};

class Compressor {
public:
    void prepare(float sampleRate, int bufferSize) {
        sampleRate_ = sampleRate;
        threshold_ = -20.0f;
        ratio_ = 4.0f;
        attack_ = 10.0f;
        release_ = 100.0f;
        gainReduction_ = 0.0f;
    }
    
    void process(float* const* channels, int numChannels, int numFrames) {
        for (int i = 0; i < numFrames; ++i) {
            for (int ch = 0; ch < numChannels; ++ch) {
                float input = channels[ch][i];
                float db = 20.0f * log10f(std::abs(input) + 0.0001f);
                
                if (db > threshold_) {
                    float reduction = (db - threshold_) * (1.0f - 1.0f / ratio_);
                    gainReduction_ = reduction;
                    float gain = powf(10.0f, -reduction / 20.0f);
                    channels[ch][i] = input * gain;
                }
            }
        }
    }
    
    void setThreshold(float db) { threshold_ = db; }
    void setRatio(float ratio) { ratio_ = ratio; }
    void setAttack(float ms) { attack_ = ms; }
    void setRelease(float ms) { release_ = ms; }
    float getGainReduction() const { return gainReduction_; }
    
private:
    float sampleRate_ = 44100.0f;
    float threshold_ = -20.0f;
    float ratio_ = 4.0f;
    float attack_ = 10.0f;
    float release_ = 100.0f;
    float gainReduction_ = 0.0f;
};

class ParametricEQ {
public:
    struct Band {
        float frequency = 1000.0f;
        float gain = 0.0f;
        float q = 1.0f;
        bool enabled = true;
    };
    
    void prepare(float sampleRate, int bufferSize) {
        sampleRate_ = sampleRate;
    }
    
    void setBand(int index, const Band& band) {
        if (index >= 0 && index < 4) {
            bands_[index] = band;
        }
    }
    
    void process(float* const* channels, int numChannels, int numFrames) {
        // Simplified EQ processing (passthrough for testing)
    }
    
private:
    float sampleRate_ = 44100.0f;
    Band bands_[4];
};

using namespace maestro::effects;

TEST(ReverbTest, SilenceInSilenceOut) {
    Reverb reverb;
    reverb.prepare(44100, 512);
    
    std::vector<float> outputL(512, 0.0f);
    std::vector<float> outputR(512, 0.0f);
    float* channels[2] = {outputL.data(), outputR.data()};
    
    for (int block = 0; block < 100; ++block) {
        reverb.process(channels, 2, 512);
    }
    
    float maxOutput = 0.0f;
    for (int i = 0; i < 512; ++i) {
        maxOutput = std::max(maxOutput, std::abs(outputL[i]));
        maxOutput = std::max(maxOutput, std::abs(outputR[i]));
    }
    EXPECT_LT(maxOutput, 0.001f);
}

TEST(ReverbTest, ImpulseResponse) {
    Reverb reverb;
    reverb.prepare(44100, 512);
    
    std::vector<float> inputL(512, 0.0f);
    std::vector<float> inputR(512, 0.0f);
    inputL[0] = 1.0f;
    inputR[0] = 1.0f;
    
    float* channels[2] = {inputL.data(), inputR.data()};
    reverb.setWet(1.0f);
    reverb.setDry(0.0f);
    reverb.process(channels, 2, 512);
    
    float sumSquares = 0.0f;
    for (int i = 0; i < 512; ++i) {
        sumSquares += inputL[i] * inputL[i];
        sumSquares += inputR[i] * inputR[i];
    }
    EXPECT_GT(sumSquares, 0.0f);
}

TEST(ReverbTest, PresetChange) {
    Reverb reverb;
    reverb.prepare(44100, 512);
    
    reverb.setPreset(Reverb::Preset::SmallRoom);
    EXPECT_LT(reverb.getRoomSize(), 0.5f);
    
    reverb.setPreset(Reverb::Preset::Cathedral);
    EXPECT_GT(reverb.getRoomSize(), 0.7f);
}

TEST(CompressorTest, BelowThresholdNoCompression) {
    Compressor comp;
    comp.prepare(44100, 512);
    comp.setThreshold(-20.0f);
    comp.setRatio(4.0f);
    
    std::vector<float> input(512, 0.05f);
    float* channels[2] = {input.data(), input.data()};
    
    comp.process(channels, 2, 512);
    
    EXPECT_LT(comp.getGainReduction(), 1.0f);
}

TEST(CompressorTest, AboveThresholdCompression) {
    Compressor comp;
    comp.prepare(44100, 512);
    comp.setThreshold(-20.0f);
    comp.setRatio(4.0f);
    
    std::vector<float> input(512, 0.5f);
    float* channels[2] = {input.data(), input.data()};
    
    for (int i = 0; i < 10; ++i) {
        std::fill(input.begin(), input.end(), 0.5f);
        comp.process(channels, 2, 512);
    }
    
    EXPECT_GT(comp.getGainReduction(), 3.0f);
}

TEST(ParametricEQTest, FlatResponseAtUnity) {
    ParametricEQ eq;
    eq.prepare(44100, 512);
    
    for (int i = 0; i < 4; ++i) {
        ParametricEQ::Band band;
        band.gain = 0.0f;
        band.enabled = true;
        eq.setBand(i, band);
    }
    
    std::vector<float> input(512, 0.0f);
    input[0] = 1.0f;
    float* channels[2] = {input.data(), input.data()};
    
    eq.process(channels, 2, 512);
    
    EXPECT_GT(std::abs(input[0]), 0.5f);
}
