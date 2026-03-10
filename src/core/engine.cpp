// src/core/engine.cpp
#include "maestro/core/engine.hpp"
#include "maestro/audio/audio_engine.hpp"
#include "maestro/midi/midi_engine.hpp"
#include "maestro/style/style_engine.hpp"
#include "maestro/voice/voice_engine.hpp"
#include "maestro/pad/pad_engine.hpp"
#include "maestro/sync/sync_engine.hpp"
#include <iostream>

namespace maestro {

MaestroEngine& MaestroEngine::instance() {
    static MaestroEngine instance;
    return instance;
}

MaestroEngine::MaestroEngine() = default;

MaestroEngine::~MaestroEngine() {
    shutdown();
}

Result<void> MaestroEngine::initialize(const Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != State::Uninitialized) {
        return Result<void>("Engine already initialized");
    }

    state_ = State::Initializing;
    config_ = config;

    try {
        // Initialize subsystems in order
        syncEngine_ = std::make_unique<SyncEngine>();
        audioEngine_ = std::make_unique<AudioEngine>(config.sampleRate,
                                                       config.bufferSize);
        midiEngine_ = std::make_unique<MidiEngine>();
        voiceEngine_ = std::make_unique<VoiceEngine>(*audioEngine_);
        styleEngine_ = std::make_unique<StyleEngine>(*midiEngine_, *voiceEngine_);
        padEngine_ = std::make_unique<PadEngine>(*midiEngine_, *audioEngine_);

        // Initialize audio
        AudioEngine::Config audioConfig;
        audioConfig.sampleRate = config.sampleRate;
        audioConfig.bufferSize = config.bufferSize;
        audioConfig.inputChannels = config.inputChannels;
        audioConfig.outputChannels = config.outputChannels;
        audioConfig.driverName = config.audioDriver;

        auto audioResult = audioEngine_->initialize(audioConfig);
        if (!audioResult) {
            state_ = State::Error;
            return Result<void>("Audio initialization failed: " + audioResult.error());
        }

        // Initialize MIDI
        auto midiResult = midiEngine_->initialize();
        if (!midiResult) {
            state_ = State::Error;
            return Result<void>("MIDI initialization failed: " + midiResult.error());
        }

        // Initialize other engines
        voiceEngine_->initialize();
        styleEngine_->initialize();
        padEngine_->initialize();

        state_ = State::Ready;
        return Result<void>();

    } catch (const std::exception& e) {
        state_ = State::Error;
        return Result<void>(std::string("Initialization exception: ") + e.what());
    }
}

Result<void> MaestroEngine::start() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != State::Ready) {
        return Result<void>("Engine not ready to start");
    }

    auto result = audioEngine_->start();
    if (!result) {
        return result;
    }

    result = midiEngine_->start();
    if (!result) {
        audioEngine_->stop();
        return result;
    }

    state_ = State::Running;
    return Result<void>();
}

Result<void> MaestroEngine::stop() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != State::Running) {
        return Result<void>("Engine not running");
    }

    state_ = State::Stopping;

    midiEngine_->stop();
    audioEngine_->stop();

    state_ = State::Ready;
    return Result<void>();
}

void MaestroEngine::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ == State::Running) {
        midiEngine_->stop();
        audioEngine_->stop();
    }

    padEngine_.reset();
    styleEngine_.reset();
    voiceEngine_.reset();
    midiEngine_.reset();
    audioEngine_.reset();
    syncEngine_.reset();

    state_ = State::Uninitialized;
}

AudioEngine& MaestroEngine::audio() { return *audioEngine_; }
MidiEngine& MaestroEngine::midi() { return *midiEngine_; }
StyleEngine& MaestroEngine::styles() { return *styleEngine_; }
VoiceEngine& MaestroEngine::voices() { return *voiceEngine_; }
PadEngine& MaestroEngine::pads() { return *padEngine_; }
SyncEngine& MaestroEngine::sync() { return *syncEngine_; }

Result<void> MaestroEngine::setConfig(const Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ == State::Running) {
        return Result<void>("Cannot change config while running");
    }
    
    config_ = config;
    return Result<void>();
}

MaestroEngine::Performance MaestroEngine::getPerformance() const {
    Performance perf;
    if (audioEngine_) {
        perf.cpuUsage = audioEngine_->getCpuUsage();
        perf.audioLatency = audioEngine_->getLatency();
        perf.bufferUnderruns = audioEngine_->getUnderrunCount();
    }
    if (midiEngine_) {
        perf.midiLatency = midiEngine_->getLatency();
        perf.midiOverflows = midiEngine_->getOverflowCount();
    }
    return perf;
}

} // namespace maestro
