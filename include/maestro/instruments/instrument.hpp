// include/maestro/instruments/instrument.hpp
#pragma once

#include "maestro/core/types.hpp"
#include "maestro/midi/midi_types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <array>

namespace maestro::instruments {

/**
 * @brief Voice/Sound definition
 */
struct Voice {
    std::string name;
    uint8_t msb = 0;        // Bank MSB
    uint8_t lsb = 0;        // Bank LSB
    uint8_t program = 0;    // Program Change
    std::string category;
    std::vector<uint8_t> initSysEx;

    bool operator==(const Voice& other) const {
        return msb == other.msb && lsb == other.lsb && program == other.program;
    }
};

/**
 * @brief Registration memory
 */
struct Registration {
    std::string name;
    std::array<Voice, 4> voices;           // Right1, Right2, Right3, Left
    std::array<bool, 4> voiceEnabled;
    std::array<int, 4> voiceVolume;
    std::array<int, 4> voiceOctave;

    int splitPoint = 54;                   // F#3
    bool splitEnabled = false;

    std::string styleName;
    int styleTempo = 120;
    std::string styleVariation;

    std::vector<uint8_t> customSysEx;
};

/**
 * @brief Style part definition
 */
struct StylePart {
    enum Type {
        Intro1, Intro2, Intro3,
        MainA, MainB, MainC, MainD,
        FillA, FillB, FillC, FillD,
        BreakA, BreakB,
        Ending1, Ending2, Ending3
    };

    Type type;
    std::vector<midi::MidiEvent> events;
    int measures = 0;
};

/**
 * @brief Style definition
 */
struct Style {
    std::string name;
    std::string category;
    int tempo = 120;
    TimeSignature timeSignature;

    std::vector<StylePart> parts;

    // Track assignments
    struct Track {
        std::string name;
        MidiChannel channel;
        Voice voice;
        int volume = 100;
        int pan = 64;
        int reverb = 40;
        int chorus = 0;
    };
    std::vector<Track> tracks;

    // Chord detection settings
    bool otsEnabled = true;  // One Touch Setting
    bool syncStart = false;
};

/**
 * @brief Base class for instrument integrations
 */
class MAESTRO_API Instrument {
public:
    virtual ~Instrument() = default;

    // Identity
    virtual std::string manufacturer() const = 0;
    virtual std::string model() const = 0;
    virtual std::string version() const = 0;

    // Connection
    virtual bool connect(const std::string& midiInPort,
                        const std::string& midiOutPort) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    // Voice management
    virtual std::vector<Voice> getVoices() const = 0;
    virtual Result<void> setVoice(int part, const Voice& voice) = 0;
    virtual Voice getVoice(int part) const = 0;

    // Style management
    virtual std::vector<Style> getStyles() const = 0;
    virtual Result<void> loadStyle(const std::string& path) = 0;
    virtual Result<void> setStyle(const Style& style) = 0;
    virtual Style getCurrentStyle() const = 0;

    // Registration
    virtual std::vector<Registration> getRegistrations() const = 0;
    virtual Result<void> loadRegistration(const Registration& reg) = 0;
    virtual Result<void> saveRegistration(const std::string& name) = 0;

    // Real-time control
    virtual void sendNoteOn(MidiChannel ch, MidiNote note, MidiVelocity vel) = 0;
    virtual void sendNoteOff(MidiChannel ch, MidiNote note) = 0;
    virtual void sendCC(MidiChannel ch, MidiCC cc, uint8_t value) = 0;
    virtual void sendProgramChange(MidiChannel ch, uint8_t program) = 0;
    virtual void sendSysEx(const std::vector<uint8_t>& data) = 0;

    // Style playback control
    virtual void styleStart() = 0;
    virtual void styleStop() = 0;
    virtual void styleSync() = 0;
    virtual void styleIntro(int num) = 0;
    virtual void styleEnding(int num) = 0;
    virtual void styleMainVariation(int num) = 0;
    virtual void styleFill(int num) = 0;
    virtual void styleBreak() = 0;

    // Callbacks
    using MidiCallback = std::function<void(const midi::MidiMessage&)>;
    using SysExCallback = std::function<void(const midi::SysExMessage&)>;
    void setMidiCallback(MidiCallback cb) { midiCallback_ = cb; }
    void setSysExCallback(SysExCallback cb) { sysExCallback_ = cb; }

protected:
    MidiCallback midiCallback_;
    SysExCallback sysExCallback_;
};

/**
 * @brief Factory for creating instrument instances
 */
class MAESTRO_API InstrumentFactory {
public:
    static std::unique_ptr<Instrument> create(const std::string& manufacturer,
                                               const std::string& model);
    static std::unique_ptr<Instrument> detect(const std::string& midiInPort,
                                               const std::string& midiOutPort);

    static std::vector<std::pair<std::string, std::string>> supportedInstruments();
};

} // namespace maestro::instruments
