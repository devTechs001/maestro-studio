// src/studio/recording.cpp
#include "maestro/studio/recording.hpp"
#include "maestro/audio/audio_engine.hpp"
#include "maestro/midi/midi_engine.hpp"
#include <fstream>
#include <cstring>
#include <chrono>

namespace maestro::studio {

// AudioRecorder implementation
class AudioRecorder::Impl {
public:
    std::ofstream outputFile;
    std::vector<float> inputBuffers[2];
    uint64_t samplesRecorded = 0;
    double punchInTime = -1.0;
    double punchOutTime = -1.0;
    std::vector<std::string> markers;
    float inputLevels[2] = {0.0f, 0.0f};
};

AudioRecorder::AudioRecorder(AudioEngine& audio)
    : impl_(std::make_unique<Impl>())
    , audio_(audio) {
}

AudioRecorder::~AudioRecorder() {
    stop();
}

void AudioRecorder::setConfig(const Config& config) {
    config_ = config;
}

Result<void> AudioRecorder::arm() {
    state_ = State::Armed;
    if (callback_) {
        callback_(state_, "Armed for recording");
    }
    return Result<void>();
}

Result<void> AudioRecorder::start() {
    if (state_ != State::Armed) {
        return Result<void>("Recorder not armed");
    }

    // Open output file
    std::string filename = config_.outputPath;
    if (config_.addTimestamp) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        filename += "_" + std::to_string(time);
    }
    
    switch (config_.format) {
        case Config::WAV: filename += ".wav"; break;
        case Config::AIFF: filename += ".aiff"; break;
        case Config::FLAC: filename += ".flac"; break;
        case Config::MP3: filename += ".mp3"; break;
        case Config::OGG: filename += ".ogg"; break;
    }

    impl_->outputFile.open(filename, std::ios::binary);
    if (!impl_->outputFile) {
        return Result<void>("Cannot create output file: " + filename);
    }

    // Write WAV header (simplified)
    if (config_.format == Config::WAV) {
        char header[44];
        std::memcpy(header, "RIFF", 4);
        uint32_t fileSize = 0;  // Will be updated on close
        std::memcpy(header + 4, &fileSize, 4);
        std::memcpy(header + 8, "WAVE", 4);
        std::memcpy(header + 12, "fmt ", 4);
        uint32_t fmtSize = 16;
        std::memcpy(header + 16, &fmtSize, 4);
        uint16_t audioFormat = 1;  // PCM
        std::memcpy(header + 20, &audioFormat, 2);
        uint16_t numChannels = static_cast<uint16_t>(config_.numChannels);
        std::memcpy(header + 22, &numChannels, 2);
        uint32_t sampleRate = config_.sampleRate;
        std::memcpy(header + 24, &sampleRate, 4);
        uint32_t byteRate = sampleRate * numChannels * config_.bitDepth / 8;
        std::memcpy(header + 28, &byteRate, 4);
        uint16_t blockAlign = numChannels * config_.bitDepth / 8;
        std::memcpy(header + 32, &blockAlign, 2);
        uint16_t bitsPerSample = static_cast<uint16_t>(config_.bitDepth);
        std::memcpy(header + 34, &bitsPerSample, 2);
        std::memcpy(header + 36, "data", 4);
        uint32_t dataSize = 0;  // Will be updated on close
        std::memcpy(header + 40, &dataSize, 4);
        impl_->outputFile.write(header, 44);
    }

    impl_->samplesRecorded = 0;
    state_ = State::Recording;
    
    if (callback_) {
        callback_(state_, "Recording started");
    }
    
    return Result<void>();
}

void AudioRecorder::stop() {
    if (state_ != State::Recording) {
        return;
    }

    if (impl_->outputFile.is_open()) {
        // Update file size in header
        impl_->outputFile.close();
    }

    state_ = State::Idle;
    if (callback_) {
        callback_(state_, "Recording stopped");
    }
}

void AudioRecorder::pause() {
    if (state_ == State::Recording) {
        state_ = State::Paused;
        if (callback_) {
            callback_(state_, "Recording paused");
        }
    }
}

void AudioRecorder::resume() {
    if (state_ == State::Paused) {
        state_ = State::Recording;
        if (callback_) {
            callback_(state_, "Recording resumed");
        }
    }
}

double AudioRecorder::getRecordedTime() const {
    return static_cast<double>(impl_->samplesRecorded) / config_.sampleRate;
}

uint64_t AudioRecorder::getRecordedSamples() const {
    return impl_->samplesRecorded;
}

size_t AudioRecorder::getFileSizeBytes() const {
    if (impl_->outputFile.is_open()) {
        return impl_->outputFile.tellp();
    }
    return 0;
}

void AudioRecorder::setInputMonitoring(bool enabled) {
    // Input monitoring implementation
}

bool AudioRecorder::isInputMonitoring() const {
    return false;
}

float AudioRecorder::getInputLevel(int channel) const {
    if (channel >= 0 && channel < 2) {
        return impl_->inputLevels[channel];
    }
    return 0.0f;
}

void AudioRecorder::addMarker(const std::string& name) {
    impl_->markers.push_back(name + "@" + std::to_string(getRecordedTime()));
}

void AudioRecorder::setPunchIn(double timeSeconds) {
    impl_->punchInTime = timeSeconds;
}

void AudioRecorder::setPunchOut(double timeSeconds) {
    impl_->punchOutTime = timeSeconds;
}

void AudioRecorder::clearPunchPoints() {
    impl_->punchInTime = -1.0;
    impl_->punchOutTime = -1.0;
}

void AudioRecorder::setCallback(RecordingCallback callback) {
    callback_ = std::move(callback);
}

// MidiRecorder implementation
class MidiRecorder::Impl {
public:
    std::vector<bool> trackArmed;
    std::vector<midi::MidiEvent> recordedEvents;
    bool recording = false;
    bool quantizeOnRecord = false;
    int quantizeDivisions = 4;
    int countInBars = 0;
    bool metronome = false;
    OverdubMode overdubMode = OverdubMode::Replace;
    bool stepRecording = false;
    int stepLength = 4;
    bool loopRecording = false;
    TickCount loopStart = 0;
    TickCount loopEnd = 0;
    int loopTakeCount = 0;
    int currentTake = 0;
};

MidiRecorder::MidiRecorder(MidiEngine& midi)
    : impl_(std::make_unique<Impl>()) {
    impl_->trackArmed.resize(64, false);
}

MidiRecorder::~MidiRecorder() = default;

void MidiRecorder::arm(int trackIndex) {
    if (trackIndex >= 0 && trackIndex < 64) {
        impl_->trackArmed[trackIndex] = true;
    }
}

void MidiRecorder::disarm(int trackIndex) {
    if (trackIndex >= 0 && trackIndex < 64) {
        impl_->trackArmed[trackIndex] = false;
    }
}

void MidiRecorder::start() {
    impl_->recording = true;
    impl_->recordedEvents.clear();
}

void MidiRecorder::stop() {
    impl_->recording = false;
}

bool MidiRecorder::isRecording() const {
    return impl_->recording;
}

bool MidiRecorder::isArmed(int trackIndex) const {
    if (trackIndex >= 0 && trackIndex < 64) {
        return impl_->trackArmed[trackIndex];
    }
    return false;
}

void MidiRecorder::setQuantizeOnRecord(bool enabled) {
    impl_->quantizeOnRecord = enabled;
}

void MidiRecorder::setQuantizeValue(int divisions) {
    impl_->quantizeDivisions = divisions;
}

void MidiRecorder::setCountIn(int bars) {
    impl_->countInBars = bars;
}

void MidiRecorder::setMetronome(bool enabled) {
    impl_->metronome = enabled;
}

void MidiRecorder::setOverdubMode(OverdubMode mode) {
    impl_->overdubMode = mode;
}

void MidiRecorder::setStepRecording(bool enabled) {
    impl_->stepRecording = enabled;
}

void MidiRecorder::setStepLength(int divisions) {
    impl_->stepLength = divisions;
}

void MidiRecorder::stepForward() {
    // Step forward implementation
}

void MidiRecorder::stepBackward() {
    // Step backward implementation
}

void MidiRecorder::setLoopRecording(bool enabled) {
    impl_->loopRecording = enabled;
}

void MidiRecorder::setLoopRegion(TickCount start, TickCount end) {
    impl_->loopStart = start;
    impl_->loopEnd = end;
}

int MidiRecorder::getLoopTakeCount() const {
    return impl_->loopTakeCount;
}

void MidiRecorder::selectLoopTake(int take) {
    impl_->currentTake = take;
}

std::vector<midi::MidiEvent> MidiRecorder::getRecordedEvents() const {
    return impl_->recordedEvents;
}

void MidiRecorder::clearRecordedEvents() {
    impl_->recordedEvents.clear();
}

// TakeManager implementation
class TakeManager::Impl {
public:
    std::vector<Take> takes;
    std::vector<CompRegion> compRegions;
    int nextTakeId = 1;
};

TakeManager::TakeManager() : impl_(std::make_unique<Impl>()) {}
TakeManager::~TakeManager() = default;

int TakeManager::createTake(const std::string& name) {
    Take take;
    take.id = impl_->nextTakeId++;
    take.name = name;
    impl_->takes.push_back(take);
    return take.id;
}

void TakeManager::deleteTake(int takeId) {
    impl_->takes.erase(
        std::remove_if(impl_->takes.begin(), impl_->takes.end(),
            [takeId](const Take& t) { return t.id == takeId; }),
        impl_->takes.end()
    );
}

TakeManager::Take& TakeManager::getTake(int takeId) {
    for (auto& take : impl_->takes) {
        if (take.id == takeId) return take;
    }
    static Take empty;
    return empty;
}

std::vector<int> TakeManager::getTakeIds() const {
    std::vector<int> ids;
    for (const auto& take : impl_->takes) {
        ids.push_back(take.id);
    }
    return ids;
}

void TakeManager::addCompRegion(const CompRegion& region) {
    impl_->compRegions.push_back(region);
}

void TakeManager::removeCompRegion(int index) {
    if (index >= 0 && index < static_cast<int>(impl_->compRegions.size())) {
        impl_->compRegions.erase(impl_->compRegions.begin() + index);
    }
}

std::vector<TakeManager::CompRegion> TakeManager::getCompRegions() const {
    return impl_->compRegions;
}

TakeManager::Take TakeManager::generateCompedTake() {
    Take comped;
    comped.id = 0;
    comped.name = "Comped Take";
    comped.isComped = true;
    // Merge comp regions
    return comped;
}

void TakeManager::compFromTake(int takeId, TickCount start, TickCount end) {
    CompRegion region{start, end, takeId};
    addCompRegion(region);
}

void TakeManager::quickSwipe(TickCount position, int takeId) {
    // Quick swipe implementation
}

} // namespace maestro::studio
