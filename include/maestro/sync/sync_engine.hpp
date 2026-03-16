// include/maestro/sync/sync_engine.hpp
#pragma once

#include "maestro/core/config.hpp"
#include "maestro/core/types.hpp"
#include <memory>
#include <functional>

namespace maestro {

/**
 * @brief Clock source types
 */
enum class ClockSource {
    Internal,
    MIDI,
    USB,
    Network,
    WordClock,
    ADAT,
    SPDIF
};

/**
 * @brief Transport state
 */
enum class TransportState {
    Stopped,
    Playing,
    Recording,
    Paused,
    FastForward,
    Rewind
};

/**
 * @brief Synchronization engine
 */
class MAESTRO_API SyncEngine {
public:
    SyncEngine();
    ~SyncEngine();

    // Clock configuration
    void setClockSource(ClockSource source);
    ClockSource getClockSource() const;

    void setTempo(double bpm);
    double getTempo() const;

    void setTimeSignature(const TimeSignature& ts);
    TimeSignature getTimeSignature() const;

    // Transport control
    void play();
    void stop();
    void pause();
    void record();
    void rewind();
    void fastForward();

    TransportState getTransportState() const;
    bool isPlaying() const;
    bool isRecording() const;

    // Position
    void setPosition(TickCount ticks);
    TickCount getPosition() const;
    MusicalPosition getMusicalPosition() const;

    // Time
    void setTime(double seconds);
    double getTime() const;

    // Loop
    void setLoopEnabled(bool enabled);
    bool isLoopEnabled() const;
    void setLoopPoints(TickCount start, TickCount end);
    TickCount getLoopStart() const;
    TickCount getLoopEnd() const;

    // Count-in
    void setCountInEnabled(bool enabled);
    void setCountInBars(int bars);

    // Metronome
    void setMetronomeEnabled(bool enabled);
    bool isMetronomeEnabled() const;
    void setMetronomeVolume(float volume);
    float getMetronomeVolume() const;

    // MIDI sync
    void sendMIDIClock();
    void sendMIDIStart();
    void sendMIDIStop();
    void sendMIDIContinue();

    // Callbacks
    using TempoCallback = std::function<void(double)>;
    using PositionCallback = std::function<void(TickCount)>;
    using TransportCallback = std::function<void(TransportState)>;

    void setTempoCallback(TempoCallback callback);
    void setPositionCallback(PositionCallback callback);
    void setTransportCallback(TransportCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    ClockSource clockSource_ = ClockSource::Internal;
    double tempo_ = 120.0;
    TimeSignature timeSignature_;
    TransportState transportState_ = TransportState::Stopped;
    TickCount position_ = 0;
    double timeSeconds_ = 0.0;
    bool loopEnabled_ = false;
    TickCount loopStart_ = 0;
    TickCount loopEnd_ = 480 * 4 * 4;  // 4 bars
    bool countInEnabled_ = false;
    int countInBars_ = 1;
    bool metronomeEnabled_ = false;
    float metronomeVolume_ = 0.5f;
    TempoCallback tempoCallback_;
    PositionCallback positionCallback_;
    TransportCallback transportCallback_;
};

} // namespace maestro
