// include/maestro/audio/dsp/dsp_common.hpp
#pragma once

#include "maestro/core/types.hpp"
#include <cmath>
#include <algorithm>
#include <array>
#include <complex>
#include <memory>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef TWO_PI
#define TWO_PI (2.0 * M_PI)
#endif

namespace maestro::dsp {

/**
 * @brief Convert MIDI note to frequency in Hz
 */
inline float midiToFreq(MidiNote note) {
    return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
}

/**
 * @brief Convert frequency to MIDI note
 */
inline MidiNote freqToMidi(float freq) {
    return static_cast<MidiNote>(69.0f + 12.0f * std::log2(freq / 440.0f));
}

/**
 * @brief Convert decibels to linear gain
 */
inline float dbToGain(float db) {
    return std::pow(10.0f, db / 20.0f);
}

/**
 * @brief Convert linear gain to decibels
 */
inline float gainToDb(float gain) {
    if (gain < 0.0001f) return -80.0f;
    return 20.0f * std::log10(gain);
}

/**
 * @brief Linear interpolation
 */
inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

/**
 * @brief Clamp value to range
 */
template<typename T>
inline T clamp(T value, T min, T max) {
    return std::max(min, std::min(max, value));
}

/**
 * @brief Smoothstep interpolation
 */
inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

/**
 * @brief One-pole lowpass filter coefficient
 */
inline float onePoleCoeff(float cutoff, float sampleRate) {
    float rc = 1.0f / (2.0f * M_PI * cutoff);
    float dt = 1.0f / sampleRate;
    return std::exp(-dt / rc);
}

/**
 * @brief Soft clipping (tanh)
 */
inline float softClip(float x) {
    return std::tanh(x);
}

/**
 * @brief Hard clipping
 */
inline float hardClip(float x, float threshold = 1.0f) {
    return std::max(-threshold, std::min(threshold, x));
}

/**
 * @brief DC blocker filter
 */
class DCBlocker {
public:
    void prepare(float sampleRate) {
        sampleRate_ = sampleRate;
        reset();
    }
    
    void reset() {
        x1_ = 0.0f;
        y1_ = 0.0f;
    }
    
    float process(float input) {
        float output = input - x1_ + 0.99f * y1_;
        x1_ = input;
        y1_ = output;
        return output;
    }
    
private:
    float sampleRate_ = 44100.0f;
    float x1_ = 0.0f;
    float y1_ = 0.0f;
};

/**
 * @brief Simple lowpass filter (one-pole)
 */
class OnePoleLPF {
public:
    void setCutoff(float cutoff, float sampleRate) {
        coeff_ = onePoleCoeff(cutoff, sampleRate);
    }
    
    void reset() {
        state_ = 0.0f;
    }
    
    float process(float input) {
        state_ = coeff_ * state_ + (1.0f - coeff_) * input;
        return state_;
    }
    
private:
    float coeff_ = 0.9f;
    float state_ = 0.0f;
};

/**
 * @brief Gain computer with smoothing
 */
class GainComputer {
public:
    void setGain(float gainDb) {
        targetGain_ = dbToGain(gainDb);
    }
    
    void setSmoothing(float ms, float sampleRate) {
        smoothingCoeff_ = std::exp(-1000.0f / (ms * sampleRate));
    }
    
    float process() {
        currentGain_ = lerp(currentGain_, targetGain_, 1.0f - smoothingCoeff_);
        return currentGain_;
    }
    
    float process(float input) {
        return input * process();
    }
    
private:
    float targetGain_ = 1.0f;
    float currentGain_ = 1.0f;
    float smoothingCoeff_ = 0.99f;
};

/**
 * @brief RMS level calculator
 */
class RMSCalculator {
public:
    void setWindowSize(int samples) {
        sum_ = 0.0f;
        count_ = 0;
        size_ = samples;
    }
    
    float process(float sample) {
        sum_ += sample * sample;
        if (++count_ >= size_) {
            count_ = 0;
            sum_ = 0.0f;
        }
        return std::sqrt(sum_ / count_);
    }
    
    float getRMS() const {
        return std::sqrt(sum_ / count_);
    }
    
private:
    float sum_ = 0.0f;
    int count_ = 0;
    int size_ = 1024;
};

/**
 * @brief Peak detector
 */
class PeakDetector {
public:
    void setAttack(float attackMs, float sampleRate) {
        attackCoeff_ = std::exp(-1000.0f / (attackMs * sampleRate));
    }
    
    void setRelease(float releaseMs, float sampleRate) {
        releaseCoeff_ = std::exp(-1000.0f / (releaseMs * sampleRate));
    }
    
    float process(float input) {
        float absInput = std::abs(input);
        if (absInput > peak_) {
            peak_ = attackCoeff_ * peak_ + (1.0f - attackCoeff_) * absInput;
        } else {
            peak_ = releaseCoeff_ * peak_ + (1.0f - releaseCoeff_) * absInput;
        }
        return peak_;
    }
    
    float getPeak() const {
        return peak_;
    }
    
private:
    float peak_ = 0.0f;
    float attackCoeff_ = 0.9f;
    float releaseCoeff_ = 0.999f;
};

/**
 * @brief Simple delay line
 */
template<typename T, size_t MaxSize>
class DelayLine {
public:
    void setDelay(float delayMs, float sampleRate) {
        delaySamples_ = static_cast<size_t>(delayMs * sampleRate / 1000.0f);
        delaySamples_ = std::min(delaySamples_, MaxSize - 1);
    }
    
    void setDelaySamples(size_t samples) {
        delaySamples_ = std::min(samples, MaxSize - 1);
    }
    
    void reset() {
        buffer_.fill(T{});
        writePos_ = 0;
    }
    
    T process(T input) {
        buffer_[writePos_] = input;
        size_t readPos = (writePos_ - delaySamples_ + MaxSize) % MaxSize;
        writePos_ = (writePos_ + 1) % MaxSize;
        return buffer_[readPos];
    }
    
private:
    std::array<T, MaxSize> buffer_{};
    size_t writePos_ = 0;
    size_t delaySamples_ = 0;
};

/**
 * @brief Modulo with wrap for negative numbers
 */
inline float wrap(float value, float min, float max) {
    float range = max - min;
    float result = std::fmod(value - min, range);
    if (result < 0) result += range;
    return result + min;
}

/**
 * @brief Fold (triangle) waveform
 */
inline float fold(float value, float min, float max) {
    float range = max - min;
    float result = std::fmod(value - min, 2.0f * range);
    if (result < 0) result += 2.0f * range;
    if (result > range) result = 2.0f * range - result;
    return result + min;
}

/**
 * @brief Biquad filter for various filter types
 */
class BiquadFilter {
public:
    enum class Type {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        Peak,
        LowShelf,
        HighShelf,
        AllPass
    };

    void setCoefficients(Type type, float frequency, float q, float gain, float sampleRate);
    float process(float input);
    void reset();

    struct Coefficients {
        float b0, b1, b2, a1, a2;
    };
    Coefficients getCoefficients() const { return {b0_, b1_, b2_, a1_, a2_}; }

private:
    float b0_ = 1.0f, b1_ = 0.0f, b2_ = 0.0f;
    float a1_ = 0.0f, a2_ = 0.0f;
    float z1_ = 0.0f, z2_ = 0.0f;
};

/**
 * @brief Envelope follower for dynamics processing
 */
class EnvelopeFollower {
public:
    void setAttackTime(float ms, float sampleRate);
    void setReleaseTime(float ms, float sampleRate);
    float process(float input);
    void reset();

private:
    float attackCoeff_ = 0.9f;
    float releaseCoeff_ = 0.999f;
    float envelope_ = 0.0f;
};

/**
 * @brief Low Frequency Oscillator
 */
class LFO {
public:
    enum class Waveform {
        Sine,
        Triangle,
        Square,
        Sawtooth,
        Random
    };

    void setFrequency(float hz, float sampleRate);
    void setWaveform(Waveform waveform) { waveform_ = waveform; }
    void setPhase(float phase) { phase_ = phase; }
    float process();
    void reset();

private:
    Waveform waveform_ = Waveform::Sine;
    float phase_ = 0.0f;
    float phaseIncrement_ = 0.0f;
    float randomValue_ = 0.0f;
    float randomTarget_ = 0.0f;
};

/**
 * @brief Circular buffer for delay lines
 */
template<typename T, size_t MaxSize>
class CircularBuffer {
public:
    void clear() {
        buffer_.fill(T{});
        writeIndex_ = 0;
    }

    void write(T value) {
        buffer_[writeIndex_] = value;
        writeIndex_ = (writeIndex_ + 1) % MaxSize;
    }

    T read(size_t delayInSamples) const {
        size_t readIndex = (writeIndex_ - delayInSamples + MaxSize) % MaxSize;
        return buffer_[readIndex];
    }

    T readInterpolated(float delayInSamples) const {
        size_t intDelay = static_cast<size_t>(delayInSamples);
        float frac = delayInSamples - intDelay;
        T sample1 = read(intDelay);
        T sample2 = read(intDelay + 1);
        return lerp(sample1, sample2, frac);
    }

    size_t size() const { return MaxSize; }

private:
    std::array<T, MaxSize> buffer_{};
    size_t writeIndex_ = 0;
};

/**
 * @brief FFT processor for spectral analysis
 */
class FFTProcessor {
public:
    enum class Window {
        Rectangular,
        Hann,
        Hamming,
        Blackman,
        FlatTop
    };

    explicit FFTProcessor(size_t fftSize = 2048);
    ~FFTProcessor();

    void process(const float* input, std::complex<float>* output);
    void processInverse(const std::complex<float>* input, float* output);

    size_t getSize() const { return fftSize_; }

    void setWindow(Window window);
    void applyWindow(float* data);

private:
    size_t fftSize_;
    Window windowType_ = Window::Hann;
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace maestro::dsp
