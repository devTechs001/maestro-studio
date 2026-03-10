// src/audio/audio_engine.cpp
#include "maestro/audio/audio_engine.hpp"
#include <RtAudio.h>
#include <chrono>
#include <cstring>

namespace maestro {

class AudioEngine::Impl {
public:
    std::unique_ptr<RtAudio> rtAudio;
    std::vector<std::shared_ptr<AudioProcessor>> processors;
    std::vector<std::vector<float>> processingBuffers;
    std::chrono::high_resolution_clock::time_point lastProcessTime;
    double cpuUsage = 0.0;

    static int audioCallback(void* outputBuffer, void* inputBuffer,
                            unsigned int nFrames,
                            double streamTime,
                            RtAudioStreamStatus status,
                            void* userData) {
        auto* engine = static_cast<AudioEngine*>(userData);

        auto startTime = std::chrono::high_resolution_clock::now();

        if (status & RTAUDIO_OUTPUT_UNDERFLOW) {
            engine->underruns_++;
        }

        float* output = static_cast<float*>(outputBuffer);
        const float* input = static_cast<const float*>(inputBuffer);

        // Clear output
        std::memset(output, 0, nFrames * engine->config_.outputChannels * sizeof(float));

        // Call user callback
        if (engine->callback_) {
            const float* inputs[2] = {input, input ? input + nFrames : nullptr};
            float* outputs[2] = {output, output + nFrames};
            engine->callback_(inputs, outputs, nFrames,
                            engine->config_.inputChannels,
                            engine->config_.outputChannels);
        }

        // Process through processor chain
        float* channels[2] = {output, output + nFrames};
        for (auto& processor : engine->impl_->processors) {
            if (!processor->isBypassed()) {
                processor->process(channels, engine->config_.outputChannels, nFrames);
            }
        }

        // Calculate CPU usage
        auto endTime = std::chrono::high_resolution_clock::now();
        double processTime = std::chrono::duration<double>(endTime - startTime).count();
        double bufferTime = static_cast<double>(nFrames) / engine->config_.sampleRate;
        engine->impl_->cpuUsage = (processTime / bufferTime) * 100.0;

        return 0;
    }
};

AudioEngine::AudioEngine(SampleRate sampleRate, BufferSize bufferSize)
    : impl_(std::make_unique<Impl>()) {
    config_.sampleRate = sampleRate;
    config_.bufferSize = bufferSize;
}

AudioEngine::~AudioEngine() {
    stop();
}

Result<void> AudioEngine::initialize(const Config& config) {
    config_ = config;

    try {
        impl_->rtAudio = std::make_unique<RtAudio>();

        if (impl_->rtAudio->getDeviceCount() == 0) {
            return Result<void>("No audio devices found");
        }

        return Result<void>();
    } catch (const RtAudioError& e) {
        return Result<void>(std::string("RtAudio error: ") + e.what());
    }
}

Result<void> AudioEngine::start() {
    if (running_) {
        return Result<void>("Audio engine already running");
    }

    try {
        RtAudio::StreamParameters outputParams;
        outputParams.deviceId = impl_->rtAudio->getDefaultOutputDevice();
        outputParams.nChannels = config_.outputChannels;

        RtAudio::StreamParameters inputParams;
        inputParams.deviceId = impl_->rtAudio->getDefaultInputDevice();
        inputParams.nChannels = config_.inputChannels;

        unsigned int bufferFrames = config_.bufferSize;

        impl_->rtAudio->openStream(
            &outputParams,
            config_.inputChannels > 0 ? &inputParams : nullptr,
            RTAUDIO_FLOAT32,
            config_.sampleRate,
            &bufferFrames,
            &Impl::audioCallback,
            this
        );

        impl_->rtAudio->startStream();
        running_ = true;

        return Result<void>();
    } catch (const RtAudioError& e) {
        return Result<void>(std::string("Failed to start audio: ") + e.what());
    }
}

Result<void> AudioEngine::stop() {
    if (!running_) {
        return Result<void>();
    }

    try {
        if (impl_->rtAudio->isStreamRunning()) {
            impl_->rtAudio->stopStream();
        }
        if (impl_->rtAudio->isStreamOpen()) {
            impl_->rtAudio->closeStream();
        }
        running_ = false;
        return Result<void>();
    } catch (const RtAudioError& e) {
        return Result<void>(std::string("Failed to stop audio: ") + e.what());
    }
}

std::vector<AudioDevice> AudioEngine::getAvailableDevices() const {
    std::vector<AudioDevice> devices;

    unsigned int count = impl_->rtAudio->getDeviceCount();
    for (unsigned int i = 0; i < count; i++) {
        try {
            auto info = impl_->rtAudio->getDeviceInfo(i);
            AudioDevice device;
            device.id = std::to_string(i);
            device.name = info.name;
            device.maxInputChannels = info.inputChannels;
            device.maxOutputChannels = info.outputChannels;
            device.isDefault = (i == impl_->rtAudio->getDefaultOutputDevice());

            for (auto rate : info.sampleRates) {
                device.supportedSampleRates.push_back(rate);
            }

            devices.push_back(device);
        } catch (...) {
            continue;
        }
    }

    return devices;
}

void AudioEngine::setAudioCallback(AudioCallback callback) {
    callback_ = std::move(callback);
}

void AudioEngine::addProcessor(std::shared_ptr<AudioProcessor> processor) {
    processor->prepare(config_.sampleRate, config_.bufferSize);
    impl_->processors.push_back(processor);
}

void AudioEngine::removeProcessor(std::shared_ptr<AudioProcessor> processor) {
    auto it = std::find(impl_->processors.begin(), impl_->processors.end(), processor);
    if (it != impl_->processors.end()) {
        impl_->processors.erase(it);
    }
}

double AudioEngine::getCpuUsage() const {
    return impl_->cpuUsage;
}

double AudioEngine::getLatency() const {
    return static_cast<double>(config_.bufferSize) / config_.sampleRate * 1000.0;
}

uint64_t AudioEngine::getUnderrunCount() const {
    return underruns_.load();
}

bool AudioEngine::isRunning() const {
    return running_.load();
}

// MixerChannel implementation
MixerChannel::MixerChannel(const std::string& name, int index)
    : name_(name), index_(index) {
    sendLevels_.resize(8, 0.0f);
}

void MixerChannel::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
}

void MixerChannel::setPan(float pan) {
    pan_ = std::clamp(pan, -1.0f, 1.0f);
}

void MixerChannel::setMute(bool mute) {
    muted_ = mute;
}

void MixerChannel::setSolo(bool solo) {
    soloed_ = solo;
}

void MixerChannel::setSendLevel(int sendIndex, float level) {
    if (sendIndex >= 0 && sendIndex < static_cast<int>(sendLevels_.size())) {
        sendLevels_[sendIndex] = std::clamp(level, 0.0f, 1.0f);
    }
}

float MixerChannel::getSendLevel(int sendIndex) const {
    if (sendIndex >= 0 && sendIndex < static_cast<int>(sendLevels_.size())) {
        return sendLevels_[sendIndex];
    }
    return 0.0f;
}

void MixerChannel::addInsert(std::shared_ptr<AudioEngine::AudioProcessor> effect) {
    inserts_.push_back(effect);
}

void MixerChannel::removeInsert(int index) {
    if (index >= 0 && index < static_cast<int>(inserts_.size())) {
        inserts_.erase(inserts_.begin() + index);
    }
}

void MixerChannel::process(float* left, float* right, uint32_t numFrames) {
    // Apply volume and pan
    float leftGain = volume_ * (pan_ < 0 ? 1.0f : 1.0f - pan_);
    float rightGain = volume_ * (pan_ > 0 ? 1.0f : 1.0f + pan_);

    for (uint32_t i = 0; i < numFrames; ++i) {
        left[i] *= leftGain;
        right[i] *= rightGain;
    }

    // Process inserts
    float* channels[2] = {left, right};
    for (auto& insert : inserts_) {
        if (!insert->isBypassed()) {
            insert->process(channels, 2, numFrames);
        }
    }
}

float MixerChannel::getPeakLevel(int channel) const {
    return peakLevels_[channel];
}

float MixerChannel::getRmsLevel(int channel) const {
    return rmsLevels_[channel];
}

// AudioMixer implementation
AudioMixer::AudioMixer(int numChannels, int numBuses)
    : master_(std::make_unique<MixerChannel>("Master", -1)) {
    for (int i = 0; i < numChannels; ++i) {
        channels_.push_back(std::make_unique<MixerChannel>("Ch " + std::to_string(i + 1), i));
    }
    for (int i = 0; i < numBuses; ++i) {
        buses_.push_back(std::make_unique<MixerChannel>("Bus " + std::to_string(i + 1), i));
    }
}

MixerChannel& AudioMixer::channel(int index) {
    return *channels_[index];
}

const MixerChannel& AudioMixer::channel(int index) const {
    return *channels_[index];
}

MixerChannel& AudioMixer::bus(int index) {
    return *buses_[index];
}

MixerChannel& AudioMixer::masterBus() {
    return *master_;
}

void AudioMixer::routeChannelToBus(int channelIndex, int busIndex) {
    // Routing implementation
}

void AudioMixer::process(float* const* outputBuffers, uint32_t numFrames) {
    // Mix all channels to master
    for (auto& channel : channels_) {
        channel->process(outputBuffers[0], outputBuffers[1], numFrames);
    }
}

} // namespace maestro
