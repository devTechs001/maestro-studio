// src/midi/midi_sequencer.cpp
#include "maestro/midi/midi_engine.hpp"
#include <thread>
#include <chrono>

namespace maestro {

MidiSequencer::MidiSequencer(MidiEngine& engine) : engine_(engine) {}

void MidiSequencer::play() {
    if (state_ == State::Stopped || state_ == State::Paused) {
        shouldStop_ = false;
        playbackThread_ = std::thread(&MidiSequencer::playbackLoop, this);
    }
    state_ = State::Playing;
}

void MidiSequencer::stop() {
    shouldStop_ = true;
    if (playbackThread_.joinable()) {
        playbackThread_.join();
    }
    position_ = 0;
    state_ = State::Stopped;
}

void MidiSequencer::pause() {
    state_ = State::Paused;
}

void MidiSequencer::rewind() {
    position_ = 0;
}

void MidiSequencer::setPosition(TickCount tick) {
    position_ = tick;
}

TickCount MidiSequencer::getPosition() const {
    return position_;
}

void MidiSequencer::setTempo(double bpm) {
    tempo_ = bpm;
}

double MidiSequencer::getTempo() const {
    return tempo_;
}

void MidiSequencer::setTimeSignature(const TimeSignature& ts) {
    timeSignature_ = ts;
}

void MidiSequencer::setLoopPoints(TickCount start, TickCount end) {
    // Loop points implementation
}

void MidiSequencer::setLoopEnabled(bool enabled) {
    // Loop enabled implementation
}

void MidiSequencer::startRecording(int trackIndex) {
    state_ = State::Recording;
}

void MidiSequencer::stopRecording() {
    if (state_ == State::Recording) {
        state_ = State::Stopped;
    }
}

void MidiSequencer::setRecordOverdub(bool overdub) {
    // Overdub implementation
}

void MidiSequencer::load(const MidiFile& file) {
    tracks_.clear();
    for (size_t i = 0; i < file.numTracks(); ++i) {
        tracks_.push_back(file.track(i));
    }
    tempo_ = file.initialTempo();
    timeSignature_ = file.timeSignature();
}

MidiFile MidiSequencer::toFile() const {
    MidiFile file;
    for (const auto& track : tracks_) {
        file.addTrack(track);
    }
    file.setTempo(tempo_);
    return file;
}

void MidiSequencer::clear() {
    tracks_.clear();
    position_ = 0;
    state_ = State::Stopped;
}

int MidiSequencer::addTrack(const std::string& name) {
    MidiFile::Track track;
    track.name = name;
    tracks_.push_back(track);
    return static_cast<int>(tracks_.size() - 1);
}

void MidiSequencer::removeTrack(int index) {
    if (index >= 0 && index < static_cast<int>(tracks_.size())) {
        tracks_.erase(tracks_.begin() + index);
    }
}

void MidiSequencer::setTrackMute(int index, bool mute) {
    // Mute implementation
}

void MidiSequencer::setTrackSolo(int index, bool solo) {
    // Solo implementation
}

void MidiSequencer::setTrackOutputPort(int index, const std::string& portId) {
    // Output port implementation
}

void MidiSequencer::playbackLoop() {
    const int ppqn = 480;
    double ticksPerSecond = tempo_ * ppqn / 60.0;
    
    TickCount lastTick = position_;
    
    while (!shouldStop_) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Process events at current position
        for (const auto& track : tracks_) {
            for (const auto& event : track.events) {
                if (event.tick >= lastTick && event.tick < position_) {
                    engine_.send(event.message);
                }
            }
        }
        
        // Advance position
        position_ += static_cast<TickCount>(ticksPerSecond * 0.01); // 10ms increment
        
        lastTick = position_;
        
        // Sleep to maintain tempo
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(endTime - startTime).count();
        if (elapsed < 0.01) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

} // namespace maestro
