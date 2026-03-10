// include/maestro/instruments/yamaha/yamaha_instrument.hpp
#pragma once

#include "maestro/instruments/instrument.hpp"

namespace maestro::instruments::yamaha {

/**
 * @brief Yamaha SysEx commands
 */
namespace SysEx {
    constexpr uint8_t MANUFACTURER_ID = 0x43;

    // Model IDs
    constexpr uint8_t TYROS5 = 0x7F;
    constexpr uint8_t GENOS = 0x7F;
    constexpr uint8_t PSR_SX900 = 0x7F;
    constexpr uint8_t MOTIF_XF = 0x7F;
    constexpr uint8_t MODX = 0x7F;
    constexpr uint8_t MONTAGE = 0x7F;

    // Commands
    constexpr uint8_t BULK_DUMP = 0x00;
    constexpr uint8_t PARAMETER_CHANGE = 0x10;
    constexpr uint8_t BULK_REQUEST = 0x20;
    constexpr uint8_t PARAMETER_REQUEST = 0x30;
}

/**
 * @brief Base class for Yamaha instruments
 */
class MAESTRO_API YamahaInstrument : public Instrument {
public:
    std::string manufacturer() const override { return "Yamaha"; }

    // Yamaha-specific features
    virtual void setMultiPadBank(int bank) = 0;
    virtual void triggerMultiPad(int pad) = 0;
    virtual void setOTS(int number) = 0;
    virtual void setRegistrationBank(int bank) = 0;
    virtual void setRegistrationMemory(int memory) = 0;

    // Voice layering
    virtual void enableVoicePart(int part, bool enable) = 0;
    virtual void setVoiceOctave(int part, int octave) = 0;
    virtual void setVoiceVolume(int part, int volume) = 0;
    virtual void setVoiceReverb(int part, int level) = 0;
    virtual void setVoiceChorus(int part, int level) = 0;

    // Display integration
    virtual void requestDisplayCapture() = 0;
    virtual void setDisplayText(const std::string& text) = 0;

protected:
    void parseSysEx(const midi::SysExMessage& msg);
    std::vector<uint8_t> buildSysEx(uint8_t command,
                                     const std::vector<uint8_t>& data);
};

/**
 * @brief Yamaha Genos integration
 */
class MAESTRO_API YamahaGenos : public YamahaInstrument {
public:
    YamahaGenos();
    ~YamahaGenos() override;

    std::string model() const override { return "Genos"; }
    std::string version() const override;

    bool connect(const std::string& midiIn, const std::string& midiOut) override;
    void disconnect() override;
    bool isConnected() const override;

    // Voice management
    std::vector<Voice> getVoices() const override;
    Result<void> setVoice(int part, const Voice& voice) override;
    Voice getVoice(int part) const override;

    // Style management
    std::vector<Style> getStyles() const override;
    Result<void> loadStyle(const std::string& path) override;
    Result<void> setStyle(const Style& style) override;
    Style getCurrentStyle() const override;

    // Registration
    std::vector<Registration> getRegistrations() const override;
    Result<void> loadRegistration(const Registration& reg) override;
    Result<void> saveRegistration(const std::string& name) override;

    // Real-time control
    void sendNoteOn(MidiChannel ch, MidiNote note, MidiVelocity vel) override;
    void sendNoteOff(MidiChannel ch, MidiNote note) override;
    void sendCC(MidiChannel ch, MidiCC cc, uint8_t value) override;
    void sendProgramChange(MidiChannel ch, uint8_t program) override;
    void sendSysEx(const std::vector<uint8_t>& data) override;

    // Style playback control
    void styleStart() override;
    void styleStop() override;
    void styleSync() override;
    void styleIntro(int num) override;
    void styleEnding(int num) override;
    void styleMainVariation(int num) override;
    void styleFill(int num) override;
    void styleBreak() override;

    // Genos-specific features
    void setMultiPadBank(int bank) override;
    void triggerMultiPad(int pad) override;
    void setOTS(int number) override;
    void setRegistrationBank(int bank) override;
    void setRegistrationMemory(int memory) override;
    void enableVoicePart(int part, bool enable) override;
    void setVoiceOctave(int part, int octave) override;
    void setVoiceVolume(int part, int volume) override;
    void setVoiceReverb(int part, int level) override;
    void setVoiceChorus(int part, int level) override;
    void requestDisplayCapture() override;
    void setDisplayText(const std::string& text) override;

    void setLiveControl(int knob, int value);
    void setJoystick(int x, int y);
    void setArticulation(int type);
    void setSuperArticulation(int type);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    bool connected_ = false;
};

/**
 * @brief Yamaha Motif/MODX/Montage integration
 */
class MAESTRO_API YamahaMotif : public YamahaInstrument {
public:
    enum class Model {
        MotifXF,
        MotifXS,
        MODX,
        Montage,
        MontageM
    };

    YamahaMotif(Model model);
    ~YamahaMotif() override;

    std::string model() const override;
    std::string version() const override;

    bool connect(const std::string& midiIn, const std::string& midiOut) override;
    void disconnect() override;
    bool isConnected() const override;

    std::vector<Voice> getVoices() const override;
    Result<void> setVoice(int part, const Voice& voice) override;
    Voice getVoice(int part) const override;

    std::vector<Style> getStyles() const override;
    Result<void> loadStyle(const std::string& path) override;
    Result<void> setStyle(const Style& style) override;
    Style getCurrentStyle() const override;

    std::vector<Registration> getRegistrations() const override;
    Result<void> loadRegistration(const Registration& reg) override;
    Result<void> saveRegistration(const std::string& name) override;

    void sendNoteOn(MidiChannel ch, MidiNote note, MidiVelocity vel) override;
    void sendNoteOff(MidiChannel ch, MidiNote note) override;
    void sendCC(MidiChannel ch, MidiCC cc, uint8_t value) override;
    void sendProgramChange(MidiChannel ch, uint8_t program) override;
    void sendSysEx(const std::vector<uint8_t>& data) override;

    void styleStart() override;
    void styleStop() override;
    void styleSync() override;
    void styleIntro(int num) override;
    void styleEnding(int num) override;
    void styleMainVariation(int num) override;
    void styleFill(int num) override;
    void styleBreak() override;

    void setMultiPadBank(int bank) override;
    void triggerMultiPad(int pad) override;
    void setOTS(int number) override;
    void setRegistrationBank(int bank) override;
    void setRegistrationMemory(int memory) override;
    void enableVoicePart(int part, bool enable) override;
    void setVoiceOctave(int part, int octave) override;
    void setVoiceVolume(int part, int volume) override;
    void setVoiceReverb(int part, int level) override;
    void setVoiceChorus(int part, int level) override;
    void requestDisplayCapture() override;
    void setDisplayText(const std::string& text) override;

    // Motif-specific features
    void setPerformance(int bank, int number);
    void setArpeggiator(bool enable);
    void setArpeggioType(int type);
    void setMotionSequencer(bool enable);
    void setScene(int scene);
    void setSuperKnob(int value);

private:
    Model model_;
    class Impl;
    std::unique_ptr<Impl> impl_;
    bool connected_ = false;
};

/**
 * @brief Yamaha SFF Style format parser
 */
class MAESTRO_API YamahaSFFParser {
public:
    enum class Format {
        SFF1,   // Older styles
        SFF2,   // GE format (Genos, PSR-SX)
        SFFGE   // Genos Enhanced
    };

    static Result<Style> parse(const std::string& path);
    static Result<Style> parseFromMemory(const std::vector<uint8_t>& data);

    static Result<void> save(const Style& style, const std::string& path,
                            Format format = Format::SFF2);

    static Format detectFormat(const std::vector<uint8_t>& data);

private:
    static Result<Style> parseSFF1(const std::vector<uint8_t>& data);
    static Result<Style> parseSFF2(const std::vector<uint8_t>& data);
};

} // namespace maestro::instruments::yamaha
