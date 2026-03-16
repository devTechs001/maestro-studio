// include/maestro/midi/midi_engine.hpp
#pragma once

#include "maestro/core/config.hpp"
#include "maestro/core/types.hpp"
#include "maestro/midi/midi_types.hpp"
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

namespace maestro {

struct MidiPort {
    std::string id;
    std::string name;
    bool isInput;
    bool isOutput;
    bool isVirtual;
};

using MidiInputCallback = std::function<void(const midi::MidiMessage&)>;
using SysExCallback = std::function<void(const midi::SysExMessage&)>;

/**
 * @brief Core MIDI Engine
 */
class MAESTRO_API MidiEngine {
public:
    MidiEngine();
    ~MidiEngine();

    // Initialization
    Result<void> initialize();
    Result<void> start();
    Result<void> stop();

    // Port management
    std::vector<MidiPort> getInputPorts() const;
    std::vector<MidiPort> getOutputPorts() const;

    Result<void> openInput(const std::string& portId);
    Result<void> openOutput(const std::string& portId);
    Result<void> closeInput(const std::string& portId);
    Result<void> closeOutput(const std::string& portId);

    // Virtual ports
    Result<void> createVirtualInput(const std::string& name);
    Result<void> createVirtualOutput(const std::string& name);

    // Sending MIDI
    Result<void> send(const midi::MidiMessage& msg,
                     const std::string& portId = "");
    Result<void> sendSysEx(const midi::SysExMessage& msg,
                          const std::string& portId = "");
    void sendToAll(const midi::MidiMessage& msg);

    // Callbacks
    void setInputCallback(MidiInputCallback callback);
    void setSysExCallback(SysExCallback callback);

    // MIDI Learn
    struct LearnResult {
        midi::MidiMessage message;
        std::string portId;
    };
    void startLearn(std::function<void(const LearnResult&)> callback);
    void stopLearn();
    bool isLearning() const;

    // Filtering
    void setChannelFilter(MidiChannel channel, bool enabled);
    void setMessageTypeFilter(midi::MessageType type, bool enabled);

    // Clock
    void sendClock();
    void sendStart();
    void sendStop();
    void sendContinue();
    void sendSongPosition(uint16_t position);

    // Performance
    double getLatency() const;
    uint64_t getOverflowCount() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief MIDI Router for complex routing scenarios
 */
class MAESTRO_API MidiRouter {
public:
    struct Route {
        std::string sourcePort;
        std::string destPort;
        MidiChannel sourceChannel = 0;  // 0 = all
        MidiChannel destChannel = 0;    // 0 = no change
        bool filterNotes = false;
        bool filterCC = false;
        bool filterProgramChange = false;
        int transpose = 0;
        int velocityOffset = 0;
    };

    void addRoute(const Route& route);
    void removeRoute(size_t index);
    void clearRoutes();

    void process(const midi::MidiMessage& msg, const std::string& sourcePort);

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

private:
    std::vector<Route> routes_;
    bool enabled_ = true;
};

/**
 * @brief MIDI File parser and player
 */
class MAESTRO_API MidiFile {
public:
    struct Track {
        std::string name;
        std::vector<midi::MidiEvent> events;
        MidiChannel channel = 0;
    };

    // Loading
    static Result<MidiFile> load(const std::string& path);
    static Result<MidiFile> loadFromMemory(const std::vector<uint8_t>& data);

    // Saving
    Result<void> save(const std::string& path) const;
    std::vector<uint8_t> toMemory() const;

    // Properties
    int format() const { return format_; }
    int ticksPerQuarterNote() const { return ppqn_; }
    double initialTempo() const { return initialTempo_; }
    TimeSignature timeSignature() const { return timeSignature_; }
    TickCount duration() const;

    // Track access
    size_t numTracks() const { return tracks_.size(); }
    Track& track(size_t index) { return tracks_[index]; }
    const Track& track(size_t index) const { return tracks_[index]; }

    // Editing
    void addTrack(const Track& track);
    void removeTrack(size_t index);
    void mergeTrack(size_t sourceIndex, size_t destIndex);

    // Utilities
    void transpose(int semitones);
    void quantize(int gridTicks);
    void setTempo(double bpm);

private:
    int format_ = 1;
    int ppqn_ = 480;
    double initialTempo_ = 120.0;
    TimeSignature timeSignature_;
    std::vector<Track> tracks_;
};

/**
 * @brief MIDI Sequencer for playback and recording
 */
class MAESTRO_API MidiSequencer {
public:
    enum class State {
        Stopped,
        Playing,
        Recording,
        Paused
    };

    MidiSequencer(MidiEngine& engine);

    // Playback control
    void play();
    void stop();
    void pause();
    void rewind();
    void setPosition(TickCount tick);
    TickCount getPosition() const;

    // Tempo and timing
    void setTempo(double bpm);
    double getTempo() const;
    void setTimeSignature(const TimeSignature& ts);
    void setLoopPoints(TickCount start, TickCount end);
    void setLoopEnabled(bool enabled);

    // Recording
    void startRecording(int trackIndex);
    void stopRecording();
    void setRecordOverdub(bool overdub);

    // File operations
    void load(const MidiFile& file);
    MidiFile toFile() const;
    void clear();

    // Track management
    int addTrack(const std::string& name);
    void removeTrack(int index);
    void setTrackMute(int index, bool mute);
    void setTrackSolo(int index, bool solo);
    void setTrackOutputPort(int index, const std::string& portId);

    // State
    State getState() const { return state_; }
    bool isPlaying() const { return state_ == State::Playing; }
    bool isRecording() const { return state_ == State::Recording; }

    // Callbacks
    using PositionCallback = std::function<void(TickCount, const MusicalPosition&)>;
    void setPositionCallback(PositionCallback callback);

private:
    MidiEngine& engine_;
    State state_ = State::Stopped;
    std::vector<MidiFile::Track> tracks_;
    TickCount position_ = 0;
    double tempo_ = 120.0;
    TimeSignature timeSignature_;

    // Playback thread
    void playbackLoop();
    std::thread playbackThread_;
    std::atomic<bool> shouldStop_{false};
};

} // namespace maestro
