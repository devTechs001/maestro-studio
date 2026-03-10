// include/maestro/midi/midi_types.hpp
#pragma once

#include "maestro/core/types.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <array>

namespace maestro::midi {

// MIDI Status bytes
enum class MessageType : uint8_t {
    NoteOff         = 0x80,
    NoteOn          = 0x90,
    PolyPressure    = 0xA0,
    ControlChange   = 0xB0,
    ProgramChange   = 0xC0,
    ChannelPressure = 0xD0,
    PitchBend       = 0xE0,
    System          = 0xF0
};

// System messages
enum class SystemMessage : uint8_t {
    SysEx           = 0xF0,
    TimeCode        = 0xF1,
    SongPosition    = 0xF2,
    SongSelect      = 0xF3,
    TuneRequest     = 0xF6,
    EndSysEx        = 0xF7,
    TimingClock     = 0xF8,
    Start           = 0xFA,
    Continue        = 0xFB,
    Stop            = 0xFC,
    ActiveSensing   = 0xFE,
    Reset           = 0xFF
};

// Common CC numbers
namespace CC {
    constexpr uint8_t BankSelectMSB     = 0;
    constexpr uint8_t ModWheel          = 1;
    constexpr uint8_t BreathController  = 2;
    constexpr uint8_t FootController    = 4;
    constexpr uint8_t PortamentoTime    = 5;
    constexpr uint8_t DataEntryMSB      = 6;
    constexpr uint8_t Volume            = 7;
    constexpr uint8_t Balance           = 8;
    constexpr uint8_t Pan               = 10;
    constexpr uint8_t Expression        = 11;
    constexpr uint8_t BankSelectLSB     = 32;
    constexpr uint8_t Sustain           = 64;
    constexpr uint8_t Portamento        = 65;
    constexpr uint8_t Sostenuto         = 66;
    constexpr uint8_t SoftPedal         = 67;
    constexpr uint8_t Legato            = 68;
    constexpr uint8_t Hold2             = 69;
    constexpr uint8_t Resonance         = 71;
    constexpr uint8_t ReleaseTime       = 72;
    constexpr uint8_t AttackTime        = 73;
    constexpr uint8_t Cutoff            = 74;
    constexpr uint8_t ReverbSend        = 91;
    constexpr uint8_t ChorusSend        = 93;
    constexpr uint8_t AllSoundOff       = 120;
    constexpr uint8_t ResetAllCtrl      = 121;
    constexpr uint8_t LocalControl      = 122;
    constexpr uint8_t AllNotesOff       = 123;
}

/**
 * @brief MIDI Message structure
 */
struct MidiMessage {
    std::array<uint8_t, 3> data{0, 0, 0};
    uint8_t length = 0;
    Timestamp timestamp{0};

    // Factory methods
    static MidiMessage noteOn(MidiChannel ch, MidiNote note, MidiVelocity vel);
    static MidiMessage noteOff(MidiChannel ch, MidiNote note, MidiVelocity vel = 0);
    static MidiMessage controlChange(MidiChannel ch, MidiCC cc, uint8_t value);
    static MidiMessage programChange(MidiChannel ch, uint8_t program);
    static MidiMessage pitchBend(MidiChannel ch, int16_t value);
    static MidiMessage channelPressure(MidiChannel ch, uint8_t pressure);
    static MidiMessage polyPressure(MidiChannel ch, MidiNote note, uint8_t pressure);

    // Accessors
    MessageType type() const;
    MidiChannel channel() const;
    MidiNote note() const;
    MidiVelocity velocity() const;
    MidiCC ccNumber() const;
    uint8_t ccValue() const;
    uint8_t program() const;
    int16_t pitchBendValue() const;

    // Utilities
    bool isNoteOn() const;
    bool isNoteOff() const;
    bool isNote() const;
    bool isCC() const;
    bool isChannelMessage() const;
    bool isSystemMessage() const;

    std::string toString() const;
};

/**
 * @brief SysEx message container
 */
struct SysExMessage {
    std::vector<uint8_t> data;
    Timestamp timestamp{0};

    // Manufacturer ID helpers
    uint32_t manufacturerId() const;
    bool isYamaha() const { return manufacturerId() == 0x43; }
    bool isRoland() const { return manufacturerId() == 0x41; }
    bool isKorg() const { return manufacturerId() == 0x42; }

    // Model ID for instrument detection
    uint8_t modelId() const;

    std::string toHexString() const;
};

/**
 * @brief MIDI Event for sequencing
 */
struct MidiEvent {
    MidiMessage message;
    TickCount tick = 0;
    int track = 0;

    bool operator<(const MidiEvent& other) const {
        return tick < other.tick;
    }
};

/**
 * @brief Note with duration for display/editing
 */
struct NoteEvent {
    MidiNote note;
    MidiVelocity velocity;
    MidiChannel channel;
    TickCount startTick;
    TickCount duration;

    TickCount endTick() const { return startTick + duration; }
};

} // namespace maestro::midi
