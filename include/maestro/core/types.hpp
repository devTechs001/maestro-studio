// include/maestro/core/types.hpp
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <optional>
#include <variant>
#include <span>
#include <array>

namespace maestro {

// Basic types
using SampleRate = uint32_t;
using BufferSize = uint32_t;
using ChannelCount = uint16_t;
using MidiChannel = uint8_t;
using MidiNote = uint8_t;
using MidiVelocity = uint8_t;
using MidiCC = uint8_t;
using Timestamp = std::chrono::microseconds;
using TickCount = int64_t;

// Audio types
using AudioSample = float;
using AudioBuffer = std::vector<AudioSample>;
using StereoSample = std::array<AudioSample, 2>;

// Time types
struct TimeSignature {
    uint8_t numerator = 4;
    uint8_t denominator = 4;

    double beatsPerBar() const { return static_cast<double>(numerator); }
};

struct Tempo {
    double bpm = 120.0;

    double ticksPerSecond(int ppqn = 480) const {
        return (bpm * ppqn) / 60.0;
    }

    Timestamp tickToTime(TickCount tick, int ppqn = 480) const {
        double seconds = static_cast<double>(tick) / ticksPerSecond(ppqn);
        return Timestamp(static_cast<int64_t>(seconds * 1000000));
    }
};

// Position in musical time
struct MusicalPosition {
    int32_t bar = 1;
    int32_t beat = 1;
    int32_t tick = 0;

    TickCount toTicks(int ppqn = 480, const TimeSignature& ts = {}) const {
        return (bar - 1) * ts.numerator * ppqn +
               (beat - 1) * ppqn +
               tick;
    }

    static MusicalPosition fromTicks(TickCount ticks, int ppqn = 480,
                                     const TimeSignature& ts = {}) {
        MusicalPosition pos;
        int ticksPerBar = ts.numerator * ppqn;
        pos.bar = static_cast<int32_t>(ticks / ticksPerBar) + 1;
        int remaining = ticks % ticksPerBar;
        pos.beat = remaining / ppqn + 1;
        pos.tick = remaining % ppqn;
        return pos;
    }
};

// Result type for error handling
template<typename T>
class Result {
public:
    Result(T value) : value_(std::move(value)), success_(true) {}
    Result(const char* error) : error_(error), success_(false) {}
    Result(const std::string& error) : error_(error), success_(false) {}

    bool isSuccess() const { return success_; }
    bool isError() const { return !success_; }

    const T& value() const { return value_; }
    T& value() { return value_; }
    const std::string& error() const { return error_; }

    operator bool() const { return success_; }

private:
    T value_;
    std::string error_;
    bool success_;
};

// Specialization for Result<void>
template<>
class Result<void> {
public:
    Result() : success_(true) {}
    Result(const char* error) : error_(error), success_(false) {}
    Result(const std::string& error) : error_(error), success_(false) {}

    bool isSuccess() const { return success_; }
    bool isError() const { return !success_; }

    const std::string& error() const { return error_; }

    operator bool() const { return success_; }

private:
    std::string error_;
    bool success_;
};

// Forward declarations
class AudioEngine;
class MidiEngine;
class StyleEngine;
class VoiceEngine;
class PadEngine;
class StudioEngine;
class PluginHost;
class Project;

} // namespace maestro
