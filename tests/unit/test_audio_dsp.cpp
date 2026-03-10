// tests/unit/test_audio_dsp.cpp
#include <gtest/gtest.h>
#include "maestro/audio/audio_engine.hpp"
#include <vector>
#include <cmath>

using namespace maestro;

class BiquadFilter {
public:
    enum class Type { LowPass, HighPass, BandPass, Notch };
    
    void setCoefficients(Type type, float frequency, float q, float gain, float sampleRate) {
        // Simplified coefficient calculation
        type_ = type;
        frequency_ = frequency;
        q_ = q;
        sampleRate_ = sampleRate;
        
        float w0 = 2.0f * M_PI * frequency / sampleRate;
        float alpha = sinf(w0) / (2.0f * q_);
        
        b0_ = (1.0f - cosf(w0)) / 2.0f;
        b1_ = 1.0f - cosf(w0);
        b2_ = (1.0f - cosf(w0)) / 2.0f;
        a0_ = 1.0f + alpha;
        a1_ = -2.0f * cosf(w0);
        a2_ = 1.0f - alpha;
        
        x1_ = x2_ = y1_ = y2_ = 0.0f;
    }
    
    float process(float input) {
        float y = (b0_/a0_) * input + (b1_/a0_) * x1_ + (b2_/a0_) * x2_
                  - (a1_/a0_) * y1_ - (a2_/a0_) * y2_;
        x2_ = x1_;
        x1_ = input;
        y2_ = y1_;
        y1_ = y;
        return y;
    }
    
    void reset() {
        x1_ = x2_ = y1_ = y2_ = 0.0f;
    }
    
private:
    Type type_ = Type::LowPass;
    float frequency_ = 1000.0f;
    float q_ = 0.707f;
    float sampleRate_ = 44100.0f;
    float b0_, b1_, b2_, a0_, a1_, a2_;
    float x1_, x2_, y1_, y2_;
};

template<typename T, size_t Size>
class CircularBuffer {
public:
    void write(T sample) {
        buffer_[writePos_] = sample;
        writePos_ = (writePos_ + 1) % Size;
    }
    
    T read(size_t delay) const {
        size_t pos = (writePos_ - delay - 1 + Size) % Size;
        return buffer_[pos];
    }
    
    T readInterpolated(float delay) const {
        size_t pos1 = (writePos_ - static_cast<size_t>(delay) - 1 + Size) % Size;
        size_t pos2 = (writePos_ - static_cast<size_t>(delay) - 2 + Size) % Size;
        float frac = delay - static_cast<float>(static_cast<int>(delay));
        return buffer_[pos1] * (1.0f - frac) + buffer_[pos2] * frac;
    }
    
    void clear() {
        buffer_.fill(0);
        writePos_ = 0;
    }
    
private:
    std::array<T, Size> buffer_{};
    size_t writePos_ = 0;
};

class EnvelopeFollower {
public:
    void setAttackTime(float timeMs, float sampleRate) {
        attackCoeff_ = expf(-1000.0f / (timeMs * sampleRate));
    }
    
    void setReleaseTime(float timeMs, float sampleRate) {
        releaseCoeff_ = expf(-1000.0f / (timeMs * sampleRate));
    }
    
    void reset() {
        envelope_ = 0.0f;
    }
    
    float process(float input) {
        float target = std::abs(input);
        if (target > envelope_) {
            envelope_ = attackCoeff_ * envelope_ + (1.0f - attackCoeff_) * target;
        } else {
            envelope_ = releaseCoeff_ * envelope_ + (1.0f - releaseCoeff_) * target;
        }
        return envelope_;
    }
    
private:
    float envelope_ = 0.0f;
    float attackCoeff_ = 0.9f;
    float releaseCoeff_ = 0.999f;
};

class LFO {
public:
    enum class Waveform { Sine, Triangle, Square, Saw };
    
    void setFrequency(float freq, float sampleRate) {
        phaseInc_ = 2.0f * M_PI * freq / sampleRate;
    }
    
    void setWaveform(Waveform waveform) {
        waveform_ = waveform;
    }
    
    void reset() {
        phase_ = 0.0f;
    }
    
    float process() {
        float output = 0.0f;
        
        switch (waveform_) {
            case Waveform::Sine:
                output = sinf(phase_);
                break;
            case Waveform::Triangle:
                output = 2.0f * std::abs(2.0f * (phase_ / (2.0f * M_PI) - std::floor(phase_ / (2.0f * M_PI) + 0.5f))) - 1.0f;
                break;
            case Waveform::Square:
                output = (phase_ < M_PI) ? 1.0f : -1.0f;
                break;
            case Waveform::Saw:
                output = 2.0f * (phase_ / (2.0f * M_PI)) - 1.0f;
                break;
        }
        
        phase_ += phaseInc_;
        if (phase_ > 2.0f * M_PI) {
            phase_ -= 2.0f * M_PI;
        }
        
        return output;
    }
    
private:
    float phase_ = 0.0f;
    float phaseInc_ = 0.01f;
    Waveform waveform_ = Waveform::Sine;
};

TEST(BiquadFilterTest, LowPassAttenuation) {
    BiquadFilter filter;
    filter.setCoefficients(BiquadFilter::Type::LowPass, 1000.0f, 0.707f, 0.0f, 44100.0f);
    
    // Test with frequency well above cutoff
    float freq = 5000.0f;
    float omega = 2.0f * M_PI * freq / 44100.0f;
    
    float maxOutput = 0.0f;
    for (int i = 0; i < 1000; ++i) {
        float input = sinf(i * omega);
        float output = filter.process(input);
        maxOutput = std::max(maxOutput, std::abs(output));
    }
    
    // Output should be significantly attenuated
    EXPECT_LT(maxOutput, 0.2f);
}

TEST(BiquadFilterTest, Reset) {
    BiquadFilter filter;
    filter.setCoefficients(BiquadFilter::Type::LowPass, 1000.0f, 0.707f, 0.0f, 44100.0f);
    
    filter.process(1.0f);
    filter.process(0.5f);
    filter.reset();
    
    EXPECT_NEAR(filter.process(0.0f), 0.0f, 0.0001f);
}

TEST(CircularBufferTest, WriteRead) {
    CircularBuffer<float, 1024> buffer;
    buffer.clear();
    
    buffer.write(1.0f);
    buffer.write(2.0f);
    buffer.write(3.0f);
    
    EXPECT_EQ(buffer.read(0), 3.0f);
    EXPECT_EQ(buffer.read(1), 2.0f);
    EXPECT_EQ(buffer.read(2), 1.0f);
}

TEST(CircularBufferTest, InterpolatedRead) {
    CircularBuffer<float, 1024> buffer;
    buffer.clear();
    
    buffer.write(0.0f);
    buffer.write(1.0f);
    
    EXPECT_NEAR(buffer.readInterpolated(0.5f), 0.5f, 0.0001f);
}

TEST(EnvelopeFollowerTest, AttackRelease) {
    EnvelopeFollower follower;
    follower.setAttackTime(1.0f, 44100.0f);
    follower.setReleaseTime(10.0f, 44100.0f);
    follower.reset();
    
    // Attack phase
    float peak = 0.0f;
    for (int i = 0; i < 100; ++i) {
        float env = follower.process(1.0f);
        EXPECT_GE(env, peak);
        peak = env;
    }
    EXPECT_GT(peak, 0.9f);
    
    // Release phase
    for (int i = 0; i < 1000; ++i) {
        float env = follower.process(0.0f);
        EXPECT_LE(env, peak);
        peak = env;
    }
    EXPECT_LT(peak, 0.1f);
}

TEST(LFOTest, SineWave) {
    LFO lfo;
    lfo.setFrequency(1.0f, 1000.0f);
    lfo.setWaveform(LFO::Waveform::Sine);
    lfo.reset();
    
    std::vector<float> samples;
    for (int i = 0; i < 1000; ++i) {
        samples.push_back(lfo.process());
    }
    
    float minVal = *std::min_element(samples.begin(), samples.end());
    float maxVal = *std::max_element(samples.begin(), samples.end());
    
    EXPECT_NEAR(minVal, -1.0f, 0.01f);
    EXPECT_NEAR(maxVal, 1.0f, 0.01f);
}

TEST(LFOTest, TriangleWave) {
    LFO lfo;
    lfo.setFrequency(1.0f, 1000.0f);
    lfo.setWaveform(LFO::Waveform::Triangle);
    lfo.reset();
    
    for (int i = 0; i < 1000; ++i) {
        float s = lfo.process();
        EXPECT_GE(s, -1.0f);
        EXPECT_LE(s, 1.0f);
    }
}
