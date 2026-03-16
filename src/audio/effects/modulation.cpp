// src/audio/effects/modulation.cpp
#include "maestro/audio/effects/modulation.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace maestro::effects {

// ==================== Chorus Implementation ====================

Chorus::Chorus() = default;
Chorus::~Chorus() = default;

void Chorus::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;

    // Initialize LFOs with different phase offsets
    for (int i = 0; i < numVoices_; ++i) {
        voicesL_[i].lfo.setFrequency(rate_, sampleRate);
        voicesL_[i].lfo.setWaveform(dsp::LFO::Waveform::Sine);
        voicesL_[i].lfo.setPhase(static_cast<float>(i) / numVoices_);

        voicesR_[i].lfo.setFrequency(rate_, sampleRate);
        voicesR_[i].lfo.setWaveform(dsp::LFO::Waveform::Sine);
        voicesR_[i].lfo.setPhase(static_cast<float>(i) / numVoices_ + 0.5f);

        voicesL_[i].phaseOffset = 0.0f;
        voicesR_[i].phaseOffset = stereoWidth_ * 0.5f;
    }

    reset();
}

void Chorus::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;

    float* left = channels[0];
    float* right = channels[1];

    float baseDelaySamples = baseDelayMs_ * sampleRate_ / 1000.0f;
    float modDepthSamples = depth_ * baseDelaySamples;
    float feedbackAmount = feedback_;
    float wetMix = mix_;
    float dryMix = 1.0f - wetMix;

    for (uint32_t i = 0; i < numFrames; ++i) {
        float inputL = left[i];
        float inputR = right[i];

        float chorusL = 0.0f, chorusR = 0.0f;

        // Process each voice
        for (int v = 0; v < numVoices_; ++v) {
            // Get LFO modulation
            float lfoL = voicesL_[v].lfo.process();
            float lfoR = voicesR_[v].lfo.process();

            // Calculate delay positions with modulation
            float delayModL = lfoL * modDepthSamples + voicesL_[v].phaseOffset * 100.0f;
            float delayModR = lfoR * modDepthSamples + voicesR_[v].phaseOffset * 100.0f;

            float totalDelayL = baseDelaySamples + delayModL;
            float totalDelayR = baseDelaySamples + delayModR;

            // Read from delay lines with interpolation
            float delayedL = voicesL_[v].delayLine.readInterpolated(totalDelayL);
            float delayedR = voicesR_[v].delayLine.readInterpolated(totalDelayR);

            // Write to delay lines with feedback
            voicesL_[v].delayLine.write(inputL + delayedL * feedbackAmount);
            voicesR_[v].delayLine.write(inputR + delayedR * feedbackAmount);

            chorusL += delayedL;
            chorusR += delayedR;
        }

        // Average the voices
        chorusL /= numVoices_;
        chorusR /= numVoices_;

        // Mix dry and wet
        left[i] = inputL * dryMix + chorusL * wetMix;
        right[i] = inputR * dryMix + chorusR * wetMix;
    }
}

void Chorus::reset() {
    for (int i = 0; i < numVoices_; ++i) {
        voicesL_[i].delayLine.clear();
        voicesR_[i].delayLine.clear();
        voicesL_[i].lfo.reset();
        voicesR_[i].lfo.reset();
    }
}

void Chorus::setRate(float hz) {
    rate_ = std::clamp(hz, 0.1f, 10.0f);
    for (int i = 0; i < numVoices_; ++i) {
        voicesL_[i].lfo.setFrequency(rate_, sampleRate_);
        voicesR_[i].lfo.setFrequency(rate_, sampleRate_);
    }
}

void Chorus::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
}

void Chorus::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void Chorus::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.9f);
}

void Chorus::setDelay(float ms) {
    baseDelayMs_ = std::clamp(ms, 1.0f, 30.0f);
}

void Chorus::setVoices(int voices) {
    numVoices_ = std::clamp(voices, 1, 4);
}

void Chorus::setStereoWidth(float width) {
    stereoWidth_ = std::clamp(width, 0.0f, 1.0f);
}

void Chorus::setMode(Mode mode) {
    switch (mode) {
        case Mode::Classic:
            setRate(1.0f);
            setDepth(0.5f);
            setDelay(7.0f);
            setVoices(2);
            break;
        case Mode::Ensemble:
            setRate(0.5f);
            setDepth(0.7f);
            setDelay(10.0f);
            setVoices(4);
            break;
        case Mode::Vintage:
            setRate(1.5f);
            setDepth(0.3f);
            setDelay(5.0f);
            setVoices(1);
            break;
        case Mode::Shimmer:
            setRate(2.0f);
            setDepth(0.8f);
            setDelay(15.0f);
            setVoices(4);
            break;
    }
}

// ==================== Phaser Implementation ====================

Phaser::Phaser() = default;
Phaser::~Phaser() = default;

void Phaser::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;

    lfoL_.setFrequency(rate_, sampleRate);
    lfoL_.setWaveform(dsp::LFO::Waveform::Sine);
    lfoR_.setFrequency(rate_, sampleRate);
    lfoR_.setWaveform(dsp::LFO::Waveform::Sine);
    lfoR_.setPhase(0.5f);  // 180 degree phase difference

    reset();
}

void Phaser::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;

    float* left = channels[0];
    float* right = channels[1];

    float lfoDepthL = lfoL_.process();
    float lfoDepthR = lfoR_.process();

    for (uint32_t i = 0; i < numFrames; ++i) {
        float inputL = left[i];
        float inputR = right[i];

        float outputL = inputL;
        float outputR = inputR;

        // Process all-pass stages
        for (int s = 0; s < numStages_; ++s) {
            // Modulate center frequency with LFO
            float modFreq = centerFreq_ * std::pow(2.0f, lfoDepthL * freqRange_);
            allpassL_[s].setCoefficients(dsp::BiquadFilter::Type::AllPass,
                                         modFreq, 1.0f, 0.0f, sampleRate_);
            outputL = allpassL_[s].process(outputL);

            modFreq = centerFreq_ * std::pow(2.0f, lfoDepthR * freqRange_);
            allpassR_[s].setCoefficients(dsp::BiquadFilter::Type::AllPass,
                                         modFreq, 1.0f, 0.0f, sampleRate_);
            outputR = allpassR_[s].process(outputR);
        }

        // Mix with feedback
        outputL = inputL + outputL * feedback_;
        outputR = inputR + outputR * feedback_;

        // Apply mix and stereo width
        float mixedL = inputL * (1.0f - mix_) + outputL * mix_;
        float mixedR = inputR * (1.0f - mix_) + outputR * mix_;

        // Apply stereo width
        float center = (mixedL + mixedR) * 0.5f;
        float side = (mixedL - mixedR) * 0.5f * stereoWidth_;

        left[i] = center + side;
        right[i] = center - side;

        lastOutputL_ = outputL;
        lastOutputR_ = outputR;
    }
}

void Phaser::reset() {
    for (int i = 0; i < MAX_STAGES; ++i) {
        allpassL_[i].reset();
        allpassR_[i].reset();
    }
    lfoL_.reset();
    lfoR_.reset();
    lastOutputL_ = 0.0f;
    lastOutputR_ = 0.0f;
}

void Phaser::setRate(float hz) {
    rate_ = std::clamp(hz, 0.1f, 10.0f);
}

void Phaser::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
}

void Phaser::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.9f);
}

void Phaser::setStages(int stages) {
    numStages_ = std::clamp(stages / 2, 1, 6) * 2;  // Ensure even number
}

void Phaser::setCenterFreq(float hz) {
    centerFreq_ = std::clamp(hz, 20.0f, 10000.0f);
}

void Phaser::setFreqRange(float octaves) {
    freqRange_ = std::clamp(octaves, 0.5f, 4.0f);
}

void Phaser::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void Phaser::setStereo(float width) {
    stereoWidth_ = std::clamp(width, 0.0f, 1.0f);
}

// ==================== Flanger Implementation ====================

Flanger::Flanger() = default;
Flanger::~Flanger() = default;

void Flanger::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;

    lfoL_.setFrequency(rate_, sampleRate);
    lfoL_.setWaveform(dsp::LFO::Waveform::Sine);
    lfoR_.setFrequency(rate_, sampleRate);
    lfoR_.setWaveform(dsp::LFO::Waveform::Sine);
    lfoR_.setPhase(0.5f);

    reset();
}

void Flanger::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;

    float* left = channels[0];
    float* right = channels[1];

    float baseDelaySamples = delayMs_ * sampleRate_ / 1000.0f;
    float modDepthSamples = depth_ * baseDelaySamples;

    for (uint32_t i = 0; i < numFrames; ++i) {
        float inputL = left[i];
        float inputR = right[i];

        // Get LFO values
        float lfoL = lfoL_.process();
        float lfoR = lfoR_.process();

        // Calculate delay with modulation
        float delayL = baseDelaySamples + lfoL * modDepthSamples + manual_ * 100.0f;
        float delayR = baseDelaySamples + lfoR * modDepthSamples + manual_ * 100.0f;

        // Through zero mode
        if (mode_ == Mode::ThroughZero) {
            delayL = std::abs(delayL);
            delayR = std::abs(delayR);
        }

        // Read from delay lines
        float delayedL = delayL_.readInterpolated(delayL);
        float delayedR = delayR_.readInterpolated(delayR);

        // Write with feedback
        delayL_.write(inputL + delayedL * feedback_);
        delayR_.write(inputR + delayedR * feedback_);

        // Mix
        float outputL = inputL * (1.0f - mix_) + delayedL * mix_;
        float outputR = inputR * (1.0f - mix_) + delayedR * mix_;

        // Barber pole mode
        if (mode_ == Mode::Barber) {
            outputL = inputL + delayedL * feedback_;
            outputR = inputR - delayedR * feedback_;
        }

        left[i] = stereo_ ? outputL : (outputL + outputR) * 0.5f;
        right[i] = stereo_ ? outputR : (outputL + outputR) * 0.5f;
    }
}

void Flanger::reset() {
    delayL_.clear();
    delayR_.clear();
    lfoL_.reset();
    lfoR_.reset();
}

void Flanger::setRate(float hz) {
    rate_ = std::clamp(hz, 0.01f, 5.0f);
}

void Flanger::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
}

void Flanger::setFeedback(float feedback) {
    feedback_ = std::clamp(feedback, 0.0f, 0.9f);
}

void Flanger::setDelay(float ms) {
    delayMs_ = std::clamp(ms, 0.1f, 10.0f);
}

void Flanger::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void Flanger::setStereo(bool stereo) {
    stereo_ = stereo;
}

void Flanger::setManual(float position) {
    manual_ = std::clamp(position, 0.0f, 1.0f);
}

void Flanger::setMode(Mode mode) {
    mode_ = mode;
}

// ==================== Tremolo Implementation ====================

Tremolo::Tremolo() = default;
Tremolo::~Tremolo() = default;

void Tremolo::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;

    lfoL_.setFrequency(rate_, sampleRate);
    lfoL_.setWaveform(dsp::LFO::Waveform::Sine);
    lfoR_.setFrequency(rate_, sampleRate);
    lfoR_.setWaveform(dsp::LFO::Waveform::Sine);
    lfoR_.setPhase(stereoPhase_ / 360.0f);

    reset();
}

void Tremolo::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;

    float* left = channels[0];
    float* right = channels[1];

    for (uint32_t i = 0; i < numFrames; ++i) {
        float lfoL = (lfoL_.process() + 1.0f) * 0.5f;  // 0 to 1
        float lfoR = (lfoR_.process() + 1.0f) * 0.5f;

        float gainL = 1.0f - depth_ * lfoL;
        float gainR = 1.0f - depth_ * lfoR;

        left[i] *= gainL;
        right[i] *= gainR;
    }
}

void Tremolo::reset() {
    lfoL_.reset();
    lfoR_.reset();
}

void Tremolo::setRate(float hz) {
    rate_ = std::clamp(hz, 0.1f, 20.0f);
}

void Tremolo::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
}

void Tremolo::setWaveform(dsp::LFO::Waveform waveform) {
    lfoL_.setWaveform(waveform);
    lfoR_.setWaveform(waveform);
}

void Tremolo::setStereoPhase(float degrees) {
    stereoPhase_ = std::fmod(degrees, 360.0f);
}

void Tremolo::setSync(bool sync) {
    sync_ = sync;
    if (sync_) {
        float beatFreq = static_cast<float>(tempo_) / 60.0f;
        rate_ = beatFreq / noteValue_;
    }
}

void Tremolo::setTempo(double bpm) {
    tempo_ = bpm;
    if (sync_) {
        setSync(true);
    }
}

void Tremolo::setNoteValue(float note) {
    noteValue_ = note;
    if (sync_) {
        setSync(true);
    }
}

// ==================== RotarySpeaker Implementation ====================

RotarySpeaker::RotarySpeaker() = default;
RotarySpeaker::~RotarySpeaker() = default;

void RotarySpeaker::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;

    hornLFO_.setFrequency(hornCurrentRate_, sampleRate);
    hornLFO_.setWaveform(dsp::LFO::Waveform::Sine);
    drumLFO_.setFrequency(drumCurrentRate_, sampleRate);
    drumLFO_.setWaveform(dsp::LFO::Waveform::Sine);

    // Crossover at 500Hz
    lowpassL_.setCoefficients(dsp::BiquadFilter::Type::LowPass, 500.0f, 0.707f, 0.0f, sampleRate);
    lowpassR_.setCoefficients(dsp::BiquadFilter::Type::LowPass, 500.0f, 0.707f, 0.0f, sampleRate);
    highpassL_.setCoefficients(dsp::BiquadFilter::Type::HighPass, 500.0f, 0.707f, 0.0f, sampleRate);
    highpassR_.setCoefficients(dsp::BiquadFilter::Type::HighPass, 500.0f, 0.707f, 0.0f, sampleRate);

    reset();
}

void RotarySpeaker::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;

    float* left = channels[0];
    float* right = channels[1];

    // Update rotation speed
    float targetRate = (speed_ == Speed::Fast) ? fastRate_ : slowRate_;
    float accelCoeff = std::exp(-1.0f / (acceleration_ * sampleRate_));
    hornCurrentRate_ = dsp::lerp(hornCurrentRate_, targetRate, 1.0f - accelCoeff);
    drumCurrentRate_ = dsp::lerp(drumCurrentRate_, targetRate * 0.8f, 1.0f - accelCoeff);

    hornLFO_.setFrequency(hornCurrentRate_, sampleRate_);
    drumLFO_.setFrequency(drumCurrentRate_, sampleRate_);

    for (uint32_t i = 0; i < numFrames; ++i) {
        float input = (left[i] + right[i]) * 0.5f;

        // Split into horn (high) and drum (low)
        float drum = lowpassL_.process(input);
        float horn = highpassL_.process(input);

        // Apply Doppler and amplitude modulation
        float hornMod = hornLFO_.process();
        float drumMod = drumLFO_.process();

        // Distance-based amplitude modulation
        float hornAmp = 1.0f - 0.3f * (hornMod + 1.0f) * 0.5f;
        float drumAmp = 1.0f - 0.2f * (drumMod + 1.0f) * 0.5f;

        // Apply drive to horn
        if (drive_ > 0.0f) {
            horn = dsp::softClip(horn * (1.0f + drive_));
        }

        // Mix to stereo with rotation effect
        float hornAngle = (hornMod + 1.0f) * 0.5f * M_PI;
        float drumAngle = (drumMod + 1.0f) * 0.5f * M_PI;

        float leftOut = drum * drumAmp * std::cos(drumAngle) +
                       horn * hornAmp * std::cos(hornAngle);
        float rightOut = drum * drumAmp * std::sin(drumAngle) +
                        horn * hornAmp * std::sin(hornAngle);

        // Apply levels
        left[i] = leftOut * hornLevel_ + drum * drumLevel_ * drumAmp;
        right[i] = rightOut * hornLevel_ + drum * drumLevel_ * drumAmp;
    }
}

void RotarySpeaker::reset() {
    hornLFO_.reset();
    drumLFO_.reset();
    lowpassL_.reset();
    lowpassR_.reset();
    highpassL_.reset();
    highpassR_.reset();
    dopplerL_.clear();
    dopplerR_.clear();
}

void RotarySpeaker::setSpeed(Speed speed) {
    speed_ = speed;
}

void RotarySpeaker::setSlowRate(float hz) {
    slowRate_ = hz;
}

void RotarySpeaker::setFastRate(float hz) {
    fastRate_ = hz;
}

void RotarySpeaker::setAcceleration(float time) {
    acceleration_ = std::max(0.1f, time);
}

void RotarySpeaker::setHornLevel(float level) {
    hornLevel_ = std::clamp(level, 0.0f, 1.0f);
}

void RotarySpeaker::setDrumLevel(float level) {
    drumLevel_ = std::clamp(level, 0.0f, 1.0f);
}

void RotarySpeaker::setDistance(float meters) {
    distance_ = std::max(0.5f, meters);
}

void RotarySpeaker::setDrive(float amount) {
    drive_ = std::clamp(amount, 0.0f, 1.0f);
}

// ==================== Vibrato Implementation ====================

Vibrato::Vibrato() = default;
Vibrato::~Vibrato() = default;

void Vibrato::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    lfo_.setFrequency(rate_, sampleRate);
    lfo_.setWaveform(dsp::LFO::Waveform::Sine);
    reset();
}

void Vibrato::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;

    float* left = channels[0];
    float* right = channels[1];

    float baseDelaySamples = delayMs_ * sampleRate_ / 1000.0f;
    float modDepthSamples = depth_ * baseDelaySamples;

    for (uint32_t i = 0; i < numFrames; ++i) {
        float lfo = lfo_.process();
        float delayMod = baseDelaySamples + lfo * modDepthSamples;

        float delayedL = delayL_.readInterpolated(delayMod);
        float delayedR = delayR_.readInterpolated(delayMod);

        delayL_.write(left[i]);
        delayR_.write(right[i]);

        left[i] = delayedL;
        right[i] = delayedR;
    }
}

void Vibrato::reset() {
    delayL_.clear();
    delayR_.clear();
    lfo_.reset();
}

void Vibrato::setRate(float hz) {
    rate_ = std::clamp(hz, 0.1f, 20.0f);
}

void Vibrato::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
}

void Vibrato::setDelay(float ms) {
    delayMs_ = std::clamp(ms, 1.0f, 20.0f);
}

void Vibrato::setWaveform(dsp::LFO::Waveform waveform) {
    lfo_.setWaveform(waveform);
}

// ==================== PitchShifter Implementation ====================

PitchShifter::PitchShifter() = default;
PitchShifter::~PitchShifter() = default;

void PitchShifter::prepare(SampleRate sampleRate, BufferSize bufferSize) {
    sampleRate_ = sampleRate;
    reset();
}

void PitchShifter::process(float* const* channels, uint32_t numChannels, uint32_t numFrames) {
    if (numChannels < 2) return;

    float* left = channels[0];
    float* right = channels[1];

    for (uint32_t i = 0; i < numFrames; ++i) {
        // Write input to all grains
        for (auto& grain : grainsL_) {
            grain.delayLine.write(left[i]);
        }
        for (auto& grain : grainsR_) {
            grain.delayLine.write(right[i]);
        }

        // Read from active grain with pitch shift
        float shiftedL = grainsL_[activeGrain_].delayLine.readInterpolated(
            grainsL_[activeGrain_].position * pitchRatio_);
        float shiftedR = grainsR_[activeGrain_].delayLine.readInterpolated(
            grainsR_[activeGrain_].position * pitchRatio_);

        // Apply envelope
        shiftedL *= grainsL_[activeGrain_].envelope;
        shiftedR *= grainsR_[activeGrain_].envelope;

        // Mix
        left[i] = left[i] * (1.0f - mix_) + shiftedL * mix_;
        right[i] = right[i] * (1.0f - mix_) + shiftedR * mix_;

        // Advance grain position
        for (int g = 0; g < 4; ++g) {
            grainsL_[g].position += 1.0f;
            grainsR_[g].position += 1.0f;

            // Update envelope
            if (grainsL_[g].position < GRAIN_SIZE / 2) {
                grainsL_[g].envelope = grainsL_[g].position / (GRAIN_SIZE / 2);
            } else {
                grainsL_[g].envelope = 1.0f - (grainsL_[g].position - GRAIN_SIZE / 2) / (GRAIN_SIZE / 2);
            }
            grainsR_[g].envelope = grainsL_[g].envelope;

            // Reset grain when done
            if (grainsL_[g].position >= GRAIN_SIZE) {
                grainsL_[g].position = 0;
                grainsR_[g].position = 0;
            }
        }

        // Cycle active grain
        activeGrain_ = (activeGrain_ + 1) % 4;
    }
}

void PitchShifter::reset() {
    for (int i = 0; i < 4; ++i) {
        grainsL_[i].delayLine.clear();
        grainsR_[i].delayLine.clear();
        grainsL_[i].position = 0;
        grainsR_[i].position = 0;
        grainsL_[i].envelope = 0;
        grainsR_[i].envelope = 0;
    }
    activeGrain_ = 0;
}

void PitchShifter::setSemitones(float semitones) {
    pitchRatio_ = std::pow(2.0f, semitones / 12.0f);
}

void PitchShifter::setMix(float mix) {
    mix_ = std::clamp(mix, 0.0f, 1.0f);
}

void PitchShifter::setQuality(int quality) {
    quality_ = std::clamp(quality, 0, 2);
}

} // namespace maestro::effects
