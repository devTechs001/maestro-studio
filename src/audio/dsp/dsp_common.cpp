// src/audio/dsp/dsp_common.cpp
#include "maestro/audio/dsp/dsp_common.hpp"
#include <complex>
#include <vector>
#include <cstring>

namespace maestro::dsp {

// FFT implementation using Cooley-Tukey algorithm
class FFTProcessor::Impl {
public:
    std::vector<std::complex<float>> twiddleFactors;
    std::vector<int> bitReversedIndices;
    size_t fftSize = 0;
    std::vector<float> window;

    void init(size_t size) {
        fftSize = size;
        twiddleFactors.resize(size);
        bitReversedIndices.resize(size);

        // Compute twiddle factors
        float angleStep = -2.0f * M_PI / size;
        for (size_t k = 0; k < size; ++k) {
            float angle = k * angleStep;
            twiddleFactors[k] = std::polar(1.0f, angle);
        }

        // Compute bit-reversed indices
        size_t bits = 0;
        while ((1u << bits) < size) bits++;
        for (size_t i = 0; i < size; ++i) {
            bitReversedIndices[i] = bitReverse(i, bits);
        }
    }

    static size_t bitReverse(size_t n, size_t bits) {
        size_t result = 0;
        for (size_t i = 0; i < bits; ++i) {
            result = (result << 1) | (n & 1);
            n >>= 1;
        }
        return result;
    }

    void fft(std::complex<float>* data) {
        // Bit-reversal permutation
        for (size_t i = 0; i < fftSize; ++i) {
            size_t j = bitReversedIndices[i];
            if (i < j) {
                std::swap(data[i], data[j]);
            }
        }

        // Cooley-Tukey FFT
        for (size_t len = 2; len <= fftSize; len *= 2) {
            size_t halfLen = len / 2;
            size_t twiddleStep = fftSize / len;

            for (size_t i = 0; i < fftSize; i += len) {
                for (size_t j = 0; j < halfLen; ++j) {
                    std::complex<float> w = twiddleFactors[j * twiddleStep];
                    std::complex<float> t = w * data[i + j + halfLen];
                    std::complex<float> u = data[i + j];
                    data[i + j] = u + t;
                    data[i + j + halfLen] = u - t;
                }
            }
        }
    }

    void ifft(std::complex<float>* data) {
        // Conjugate input
        for (size_t i = 0; i < fftSize; ++i) {
            data[i] = std::conj(data[i]);
        }

        // Forward FFT
        fft(data);

        // Conjugate and scale output
        for (size_t i = 0; i < fftSize; ++i) {
            data[i] = std::conj(data[i]) / static_cast<float>(fftSize);
        }
    }

    void applyWindow(float* data, Window type) {
        if (window.size() != fftSize) {
            window.resize(fftSize);
            computeWindow(window.data(), fftSize, type);
        }

        for (size_t i = 0; i < fftSize; ++i) {
            data[i] *= window[i];
        }
    }

    static void computeWindow(float* window, size_t size, Window type) {
        switch (type) {
            case Window::Hann:
                for (size_t i = 0; i < size; ++i) {
                    window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
                }
                break;

            case Window::Hamming:
                for (size_t i = 0; i < size; ++i) {
                    window[i] = 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (size - 1));
                }
                break;

            case Window::Blackman:
                for (size_t i = 0; i < size; ++i) {
                    window[i] = 0.42f - 0.5f * std::cos(2.0f * M_PI * i / (size - 1))
                                      + 0.08f * std::cos(4.0f * M_PI * i / (size - 1));
                }
                break;

            case Window::FlatTop:
                for (size_t i = 0; i < size; ++i) {
                    double n = static_cast<double>(i) / (size - 1);
                    window[i] = 1.0f
                              - 1.93f * std::cos(2.0f * M_PI * n)
                              + 1.29f * std::cos(4.0f * M_PI * n)
                              - 0.388f * std::cos(6.0f * M_PI * n)
                              + 0.028f * std::cos(8.0f * M_PI * n);
                }
                break;

            case Window::Rectangular:
            default:
                std::fill(window, window + size, 1.0f);
                break;
        }
    }
};

FFTProcessor::FFTProcessor(size_t fftSize) : fftSize_(fftSize) {
    // Ensure power of 2
    size_t size = 1;
    while (size < fftSize_) size *= 2;
    fftSize_ = size;

    impl_ = std::make_unique<Impl>();
    impl_->init(fftSize_);
}

FFTProcessor::~FFTProcessor() = default;

void FFTProcessor::process(const float* input, std::complex<float>* output) {
    // Copy input to complex array
    std::vector<std::complex<float>> data(fftSize_);
    for (size_t i = 0; i < fftSize_; ++i) {
        data[i] = (i < fftSize_) ? std::complex<float>(input[i], 0.0f) : 0.0f;
    }

    // Perform FFT
    impl_->fft(data.data());

    // Copy output
    std::memcpy(output, data.data(), fftSize_ * sizeof(std::complex<float>));
}

void FFTProcessor::processInverse(const std::complex<float>* input, float* output) {
    // Copy input
    std::vector<std::complex<float>> data(fftSize_);
    std::memcpy(data.data(), input, fftSize_ * sizeof(std::complex<float>));

    // Perform IFFT
    impl_->ifft(data.data());

    // Copy real output
    for (size_t i = 0; i < fftSize_; ++i) {
        output[i] = data[i].real();
    }
}

void FFTProcessor::setWindow(Window window) {
    windowType_ = window;
    impl_->window.clear();  // Force recomputation
}

void FFTProcessor::applyWindow(float* data) {
    impl_->applyWindow(data, windowType_);
}

// EnvelopeFollower implementation
void EnvelopeFollower::setAttackTime(float ms, float sampleRate) {
    attackCoeff_ = std::exp(-1000.0f / (ms * sampleRate));
}

void EnvelopeFollower::setReleaseTime(float ms, float sampleRate) {
    releaseCoeff_ = std::exp(-1000.0f / (ms * sampleRate));
}

float EnvelopeFollower::process(float input) {
    float absInput = std::abs(input);
    if (absInput > envelope_) {
        envelope_ = attackCoeff_ * envelope_ + (1.0f - attackCoeff_) * absInput;
    } else {
        envelope_ = releaseCoeff_ * envelope_ + (1.0f - releaseCoeff_) * absInput;
    }
    return envelope_;
}

void EnvelopeFollower::reset() {
    envelope_ = 0.0f;
}

// LFO implementation
void LFO::setFrequency(float hz, float sampleRate) {
    phaseIncrement_ = hz / sampleRate;
}

float LFO::process() {
    float output = 0.0f;

    switch (waveform_) {
        case Waveform::Sine:
            output = std::sin(TWO_PI * phase_);
            break;

        case Waveform::Triangle:
            output = 4.0f * std::abs(phase_ - 0.5f) - 1.0f;
            break;

        case Waveform::Square:
            output = phase_ < 0.5f ? 1.0f : -1.0f;
            break;

        case Waveform::Sawtooth:
            output = 2.0f * phase_ - 1.0f;
            break;

        case Waveform::Random:
            if (phase_ < phaseIncrement_ || phase_ >= 1.0f) {
                // Generate new random value at start of each cycle
                static thread_local std::random_device rd;
                static thread_local std::mt19937 gen(rd());
                static thread_local std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
                randomTarget_ = dist(gen);
            }
            // Smooth transition to target
            output = lerp(randomValue_, randomTarget_, 0.1f);
            randomValue_ = output;
            break;
    }

    phase_ += phaseIncrement_;
    if (phase_ >= 1.0f) phase_ -= 1.0f;

    return output;
}

void LFO::reset() {
    phase_ = 0.0f;
}

// BiquadFilter implementation
void BiquadFilter::setCoefficients(Type type, float frequency, float q, float gain, float sampleRate) {
    float omega = TWO_PI * frequency / sampleRate;
    float sinOmega = std::sin(omega);
    float cosOmega = std::cos(omega);
    float alpha = sinOmega / (2.0f * q);
    float A = std::sqrt(dbToGain(gain));

    float a0;

    switch (type) {
        case Type::LowPass:
            b0_ = (1.0f - cosOmega) / 2.0f;
            b1_ = 1.0f - cosOmega;
            b2_ = (1.0f - cosOmega) / 2.0f;
            a0 = 1.0f + alpha;
            a1_ = -2.0f * cosOmega;
            a2_ = 1.0f - alpha;
            break;

        case Type::HighPass:
            b0_ = (1.0f + cosOmega) / 2.0f;
            b1_ = -(1.0f + cosOmega);
            b2_ = (1.0f + cosOmega) / 2.0f;
            a0 = 1.0f + alpha;
            a1_ = -2.0f * cosOmega;
            a2_ = 1.0f - alpha;
            break;

        case Type::BandPass:
            b0_ = alpha;
            b1_ = 0.0f;
            b2_ = -alpha;
            a0 = 1.0f + alpha;
            a1_ = -2.0f * cosOmega;
            a2_ = 1.0f - alpha;
            break;

        case Type::Notch:
            b0_ = 1.0f;
            b1_ = -2.0f * cosOmega;
            b2_ = 1.0f;
            a0 = 1.0f + alpha;
            a1_ = -2.0f * cosOmega;
            a2_ = 1.0f - alpha;
            break;

        case Type::Peak:
            b0_ = 1.0f + alpha * A;
            b1_ = -2.0f * cosOmega;
            b2_ = 1.0f - alpha * A;
            a0 = 1.0f + alpha / A;
            a1_ = -2.0f * cosOmega;
            a2_ = 1.0f - alpha / A;
            break;

        case Type::LowShelf: {
            float sqrtA = std::sqrt(A);
            b0_ = A * ((A + 1.0f) - (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha);
            b1_ = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosOmega);
            b2_ = A * ((A + 1.0f) - (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha);
            a0 = (A + 1.0f) + (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha;
            a1_ = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosOmega);
            a2_ = (A + 1.0f) + (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha;
            break;
        }

        case Type::HighShelf: {
            float sqrtA = std::sqrt(A);
            b0_ = A * ((A + 1.0f) + (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha);
            b1_ = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosOmega);
            b2_ = A * ((A + 1.0f) + (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha);
            a0 = (A + 1.0f) - (A - 1.0f) * cosOmega + 2.0f * sqrtA * alpha;
            a1_ = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosOmega);
            a2_ = (A + 1.0f) - (A - 1.0f) * cosOmega - 2.0f * sqrtA * alpha;
            break;
        }

        case Type::AllPass:
            b0_ = 1.0f - alpha;
            b1_ = -2.0f * cosOmega;
            b2_ = 1.0f + alpha;
            a0 = 1.0f + alpha;
            a1_ = -2.0f * cosOmega;
            a2_ = 1.0f - alpha;
            break;
    }

    // Normalize coefficients
    b0_ /= a0;
    b1_ /= a0;
    b2_ /= a0;
    a1_ /= a0;
    a2_ /= a0;
}

float BiquadFilter::process(float input) {
    // Direct Form II Transposed
    float output = b0_ * input + z1_;
    z1_ = b1_ * input - a1_ * output + z2_;
    z2_ = b2_ * input - a2_ * output;
    return output;
}

void BiquadFilter::reset() {
    z1_ = z2_ = 0.0f;
}

// CircularBuffer implementation (already in header, but adding explicit methods)
template class CircularBuffer<float, 8192>;
template class CircularBuffer<float, 16384>;
template class CircularBuffer<float, 32768>;
template class CircularBuffer<float, 65536>;

} // namespace maestro::dsp
