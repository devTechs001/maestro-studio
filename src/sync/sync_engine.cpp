// src/sync/sync_engine.cpp
#include "maestro/sync/sync_engine.hpp"
#include "maestro/midi/midi_engine.hpp"
#include <chrono>

namespace maestro {

class SyncEngine::Impl {
public:
    std::chrono::steady_clock::time_point startTime;
    bool running = false;
};

SyncEngine::SyncEngine() : impl_(std::make_unique<Impl>()) {
    timeSignature_.numerator = 4;
    timeSignature_.denominator = 4;
}

SyncEngine::~SyncEngine() = default;

void SyncEngine::setClockSource(ClockSource source) {
    clockSource_ = source;
}

ClockSource SyncEngine::getClockSource() const {
    return clockSource_;
}

void SyncEngine::setTempo(double bpm) {
    tempo_ = std::clamp(bpm, 20.0, 300.0);
    if (tempoCallback_) {
        tempoCallback_(tempo_);
    }
}

double SyncEngine::getTempo() const {
    return tempo_;
}

void SyncEngine::setTimeSignature(const TimeSignature& ts) {
    timeSignature_ = ts;
}

TimeSignature SyncEngine::getTimeSignature() const {
    return timeSignature_;
}

void SyncEngine::play() {
    if (transportState_ == TransportState::Stopped ||
        transportState_ == TransportState::Paused) {
        impl_->startTime = std::chrono::steady_clock::now();
        impl_->running = true;
    }
    transportState_ = TransportState::Playing;
    if (transportCallback_) {
        transportCallback_(transportState_);
    }
}

void SyncEngine::stop() {
    impl_->running = false;
    position_ = 0;
    timeSeconds_ = 0.0;
    transportState_ = TransportState::Stopped;
    if (transportCallback_) {
        transportCallback_(transportState_);
    }
}

void SyncEngine::pause() {
    impl_->running = false;
    transportState_ = TransportState::Paused;
    if (transportCallback_) {
        transportCallback_(transportState_);
    }
}

void SyncEngine::record() {
    if (transportState_ == TransportState::Stopped ||
        transportState_ == TransportState::Paused) {
        impl_->startTime = std::chrono::steady_clock::now();
        impl_->running = true;
    }
    transportState_ = TransportState::Recording;
    if (transportCallback_) {
        transportCallback_(transportState_);
    }
}

void SyncEngine::rewind() {
    position_ = 0;
    timeSeconds_ = 0.0;
    if (positionCallback_) {
        positionCallback_(position_);
    }
}

void SyncEngine::fastForward() {
    // Fast forward implementation
}

TransportState SyncEngine::getTransportState() const {
    return transportState_;
}

bool SyncEngine::isPlaying() const {
    return transportState_ == TransportState::Playing ||
           transportState_ == TransportState::Recording;
}

bool SyncEngine::isRecording() const {
    return transportState_ == TransportState::Recording;
}

void SyncEngine::setPosition(TickCount ticks) {
    position_ = ticks;
    if (positionCallback_) {
        positionCallback_(position_);
    }
}

TickCount SyncEngine::getPosition() const {
    return position_;
}

MusicalPosition SyncEngine::getMusicalPosition() const {
    return MusicalPosition::fromTicks(position_, 480, timeSignature_);
}

void SyncEngine::setTime(double seconds) {
    timeSeconds_ = seconds;
}

double SyncEngine::getTime() const {
    return timeSeconds_;
}

void SyncEngine::setLoopEnabled(bool enabled) {
    loopEnabled_ = enabled;
}

bool SyncEngine::isLoopEnabled() const {
    return loopEnabled_;
}

void SyncEngine::setLoopPoints(TickCount start, TickCount end) {
    loopStart_ = start;
    loopEnd_ = end;
}

TickCount SyncEngine::getLoopStart() const {
    return loopStart_;
}

TickCount SyncEngine::getLoopEnd() const {
    return loopEnd_;
}

void SyncEngine::setCountInEnabled(bool enabled) {
    countInEnabled_ = enabled;
}

void SyncEngine::setCountInBars(int bars) {
    countInBars_ = std::clamp(bars, 0, 4);
}

void SyncEngine::setMetronomeEnabled(bool enabled) {
    metronomeEnabled_ = enabled;
}

bool SyncEngine::isMetronomeEnabled() const {
    return metronomeEnabled_;
}

void SyncEngine::setMetronomeVolume(float volume) {
    metronomeVolume_ = std::clamp(volume, 0.0f, 1.0f);
}

float SyncEngine::getMetronomeVolume() const {
    return metronomeVolume_;
}

void SyncEngine::sendMIDIClock() {
    // Send MIDI timing clock (0xF8)
}

void SyncEngine::sendMIDIStart() {
    // Send MIDI start (0xFA)
}

void SyncEngine::sendMIDIStop() {
    // Send MIDI stop (0xFC)
}

void SyncEngine::sendMIDIContinue() {
    // Send MIDI continue (0xFB)
}

void SyncEngine::setTempoCallback(TempoCallback callback) {
    tempoCallback_ = std::move(callback);
}

void SyncEngine::setPositionCallback(PositionCallback callback) {
    positionCallback_ = std::move(callback);
}

void SyncEngine::setTransportCallback(TransportCallback callback) {
    transportCallback_ = std::move(callback);
}

} // namespace maestro
