// src/instruments/yamaha/yamaha_genos.cpp
#include "maestro/instruments/yamaha/yamaha_instrument.hpp"
#include "maestro/midi/midi_engine.hpp"
#include <cstring>

namespace maestro::instruments::yamaha {

class YamahaGenos::Impl {
public:
    std::vector<Voice> voices;
    std::vector<Style> styles;
    std::vector<Registration> registrations;
    Style currentStyle;
    int currentVariation = 0;
    bool stylePlaying = false;
    int registrationBank = 0;
    int registrationMemory = 1;
    int multiPadBank = 0;
    std::vector<bool> voicePartsEnabled;
    std::vector<int> voiceOctaves;
    std::vector<int> voiceVolumes;

    Impl() {
        voicePartsEnabled.resize(4, true);
        voiceOctaves.resize(4, 0);
        voiceVolumes.resize(4, 100);
    }
};

YamahaGenos::YamahaGenos() : impl_(std::make_unique<Impl>()) {
}

YamahaGenos::~YamahaGenos() = default;

std::string YamahaGenos::version() const {
    return "1.0.0";
}

bool YamahaGenos::connect(const std::string& midiIn, const std::string& midiOut) {
    // Connection implementation
    connected_ = true;
    return true;
}

void YamahaGenos::disconnect() {
    connected_ = false;
}

bool YamahaGenos::isConnected() const {
    return connected_;
}

std::vector<Voice> YamahaGenos::getVoices() const {
    return impl_->voices;
}

Result<void> YamahaGenos::setVoice(int part, const Voice& voice) {
    if (part >= 0 && part < 4) {
        // Send voice change SysEx to Genos
        std::vector<uint8_t> sysex{0xF0, 0x43, 0x10, 0x00};
        sysex.push_back(static_cast<uint8_t>(part));
        sysex.push_back(voice.msb);
        sysex.push_back(voice.lsb);
        sysex.push_back(voice.program);
        sysex.push_back(0xF7);
        sendSysEx(sysex);
    }
    return Result<void>();
}

Voice YamahaGenos::getVoice(int part) const {
    if (part >= 0 && part < static_cast<int>(impl_->voices.size())) {
        return impl_->voices[part];
    }
    return Voice{};
}

std::vector<Style> YamahaGenos::getStyles() const {
    return impl_->styles;
}

Result<void> YamahaGenos::loadStyle(const std::string& path) {
    auto result = YamahaSFFParser::parse(path);
    if (result.isSuccess()) {
        impl_->styles.push_back(result.value());
        return Result<void>();
    }
    return Result<void>(result.error());
}

Result<void> YamahaGenos::setStyle(const Style& style) {
    impl_->currentStyle = style;
    return Result<void>();
}

Style YamahaGenos::getCurrentStyle() const {
    return impl_->currentStyle;
}

std::vector<Registration> YamahaGenos::getRegistrations() const {
    return impl_->registrations;
}

Result<void> YamahaGenos::loadRegistration(const Registration& reg) {
    // Load registration to Genos
    return Result<void>();
}

Result<void> YamahaGenos::saveRegistration(const std::string& name) {
    // Save registration to Genos
    return Result<void>();
}

void YamahaGenos::sendNoteOn(MidiChannel ch, MidiNote note, MidiVelocity vel) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::noteOn(ch, note, vel));
    }
}

void YamahaGenos::sendNoteOff(MidiChannel ch, MidiNote note) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::noteOff(ch, note));
    }
}

void YamahaGenos::sendCC(MidiChannel ch, MidiCC cc, uint8_t value) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::controlChange(ch, cc, value));
    }
}

void YamahaGenos::sendProgramChange(MidiChannel ch, uint8_t program) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::programChange(ch, program));
    }
}

void YamahaGenos::sendSysEx(const std::vector<uint8_t>& data) {
    if (sysExCallback_) {
        midi::SysExMessage msg;
        msg.data = data;
        sysExCallback_(msg);
    }
}

void YamahaGenos::styleStart() {
    // Send style start command
    impl_->stylePlaying = true;
    sendCC(0, 0x83, 0x7F);  // Start/Stop
}

void YamahaGenos::styleStop() {
    impl_->stylePlaying = false;
    sendCC(0, 0x83, 0x00);
}

void YamahaGenos::styleSync() {
    // Sync start
    sendCC(0, 0x83, 0x20);
}

void YamahaGenos::styleIntro(int num) {
    sendCC(0, 0x85, static_cast<uint8_t>(num - 1));
}

void YamahaGenos::styleEnding(int num) {
    sendCC(0, 0x86, static_cast<uint8_t>(num - 1));
}

void YamahaGenos::styleMainVariation(int num) {
    impl_->currentVariation = num;
    sendCC(0, 0x84, static_cast<uint8_t>(num));
}

void YamahaGenos::styleFill(int num) {
    sendCC(0, 0x88, static_cast<uint8_t>(num));
}

void YamahaGenos::styleBreak() {
    sendCC(0, 0x89, 0x00);
}

void YamahaGenos::setMultiPadBank(int bank) {
    impl_->multiPadBank = bank;
    sendCC(0, 0x8A, static_cast<uint8_t>(bank));
}

void YamahaGenos::triggerMultiPad(int pad) {
    sendCC(0, 0x8B, static_cast<uint8_t>(pad));
}

void YamahaGenos::setOTS(int number) {
    sendCC(0, 0x8C, static_cast<uint8_t>(number));
}

void YamahaGenos::setRegistrationBank(int bank) {
    impl_->registrationBank = bank;
    sendProgramChange(0, static_cast<uint8_t>(bank * 8));
}

void YamahaGenos::setRegistrationMemory(int memory) {
    impl_->registrationMemory = memory;
    sendProgramChange(0, static_cast<uint8_t>(impl_->registrationBank * 8 + memory - 1));
}

void YamahaGenos::enableVoicePart(int part, bool enable) {
    if (part >= 0 && part < 4) {
        impl_->voicePartsEnabled[part] = enable;
    }
}

void YamahaGenos::setVoiceOctave(int part, int octave) {
    if (part >= 0 && part < 4) {
        impl_->voiceOctaves[part] = std::clamp(octave, -2, 2);
    }
}

void YamahaGenos::setVoiceVolume(int part, int volume) {
    if (part >= 0 && part < 4) {
        impl_->voiceVolumes[part] = std::clamp(volume, 0, 127);
    }
}

void YamahaGenos::setVoiceReverb(int part, int level) {
    // Reverb send level
}

void YamahaGenos::setVoiceChorus(int part, int level) {
    // Chorus send level
}

void YamahaGenos::requestDisplayCapture() {
    // Request display capture via SysEx
}

void YamahaGenos::setDisplayText(const std::string& text) {
    // Send text to Genos display
}

void YamahaGenos::setLiveControl(int knob, int value) {
    sendCC(0, static_cast<MidiCC>(0x40 + knob), static_cast<uint8_t>(value));
}

void YamahaGenos::setJoystick(int x, int y) {
    sendCC(0, 0x60, static_cast<uint8_t>(x));
    sendCC(0, 0x61, static_cast<uint8_t>(y));
}

void YamahaGenos::setArticulation(int type) {
    sendCC(0, 0x62, static_cast<uint8_t>(type));
}

void YamahaGenos::setSuperArticulation(int type) {
    sendCC(0, 0x63, static_cast<uint8_t>(type));
}

void YamahaInstrument::parseSysEx(const midi::SysExMessage& msg) {
    if (msg.isYamaha()) {
        // Parse Yamaha SysEx message
    }
}

std::vector<uint8_t> YamahaInstrument::buildSysEx(uint8_t command,
                                                   const std::vector<uint8_t>& data) {
    std::vector<uint8_t> sysex;
    sysex.push_back(0xF0);
    sysex.push_back(SysEx::MANUFACTURER_ID);
    sysex.push_back(command);
    sysex.insert(sysex.end(), data.begin(), data.end());
    sysex.push_back(0xF7);
    return sysex;
}

} // namespace maestro::instruments::yamaha
