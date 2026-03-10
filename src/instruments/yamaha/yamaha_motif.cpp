// src/instruments/yamaha/yamaha_motif.cpp
#include "maestro/instruments/yamaha/yamaha_instrument.hpp"

namespace maestro::instruments::yamaha {

class YamahaMotif::Impl {
public:
    Model model;
    std::vector<Voice> voices;
    std::vector<Style> styles;
    std::vector<Registration> registrations;
    bool connected = false;
    int currentPerformance = 0;
    bool arpeggiatorEnabled = false;
    int currentScene = 0;
    int superKnobValue = 0;
};

YamahaMotif::YamahaMotif(Model model) : impl_(std::make_unique<Impl>()) {
    impl_->model = model;
}

YamahaMotif::~YamahaMotif() = default;

std::string YamahaMotif::model() const {
    switch (impl_->model) {
        case Model::MotifXF: return "Motif XF";
        case Model::MotifXS: return "Motif XS";
        case Model::MODX: return "MODX";
        case Model::Montage: return "Montage";
        case Model::MontageM: return "Montage M";
        default: return "Unknown";
    }
}

std::string YamahaMotif::version() const {
    return "1.0.0";
}

bool YamahaMotif::connect(const std::string& midiIn, const std::string& midiOut) {
    impl_->connected = true;
    return true;
}

void YamahaMotif::disconnect() {
    impl_->connected = false;
}

bool YamahaMotif::isConnected() const {
    return impl_->connected;
}

std::vector<Voice> YamahaMotif::getVoices() const {
    return impl_->voices;
}

Result<void> YamahaMotif::setVoice(int part, const Voice& voice) {
    return Result<void>();
}

Voice YamahaMotif::getVoice(int part) const {
    return Voice{};
}

std::vector<Style> YamahaMotif::getStyles() const {
    return impl_->styles;
}

Result<void> YamahaMotif::loadStyle(const std::string& path) {
    return Result<void>();
}

Result<void> YamahaMotif::setStyle(const Style& style) {
    return Result<void>();
}

Style YamahaMotif::getCurrentStyle() const {
    return Style{};
}

std::vector<Registration> YamahaMotif::getRegistrations() const {
    return impl_->registrations;
}

Result<void> YamahaMotif::loadRegistration(const Registration& reg) {
    return Result<void>();
}

Result<void> YamahaMotif::saveRegistration(const std::string& name) {
    return Result<void>();
}

void YamahaMotif::sendNoteOn(MidiChannel ch, MidiNote note, MidiVelocity vel) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::noteOn(ch, note, vel));
    }
}

void YamahaMotif::sendNoteOff(MidiChannel ch, MidiNote note) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::noteOff(ch, note));
    }
}

void YamahaMotif::sendCC(MidiChannel ch, MidiCC cc, uint8_t value) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::controlChange(ch, cc, value));
    }
}

void YamahaMotif::sendProgramChange(MidiChannel ch, uint8_t program) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::programChange(ch, program));
    }
}

void YamahaMotif::sendSysEx(const std::vector<uint8_t>& data) {
    if (sysExCallback_) {
        midi::SysExMessage msg;
        msg.data = data;
        sysExCallback_(msg);
    }
}

void YamahaMotif::styleStart() {}
void YamahaMotif::styleStop() {}
void YamahaMotif::styleSync() {}
void YamahaMotif::styleIntro(int num) {}
void YamahaMotif::styleEnding(int num) {}
void YamahaMotif::styleMainVariation(int num) {}
void YamahaMotif::styleFill(int num) {}
void YamahaMotif::styleBreak() {}

void YamahaMotif::setMultiPadBank(int bank) {}
void YamahaMotif::triggerMultiPad(int pad) {}
void YamahaMotif::setOTS(int number) {}
void YamahaMotif::setRegistrationBank(int bank) {}
void YamahaMotif::setRegistrationMemory(int memory) {}
void YamahaMotif::enableVoicePart(int part, bool enable) {}
void YamahaMotif::setVoiceOctave(int part, int octave) {}
void YamahaMotif::setVoiceVolume(int part, int volume) {}
void YamahaMotif::setVoiceReverb(int part, int level) {}
void YamahaMotif::setVoiceChorus(int part, int level) {}
void YamahaMotif::requestDisplayCapture() {}
void YamahaMotif::setDisplayText(const std::string& text) {}

// Motif-specific features
void YamahaMotif::setPerformance(int bank, int number) {
    impl_->currentPerformance = bank * 128 + number;
    sendProgramChange(0, static_cast<uint8_t>(impl_->currentPerformance));
}

void YamahaMotif::setArpeggiator(bool enable) {
    impl_->arpeggiatorEnabled = enable;
    // Send arpeggiator on/off SysEx
}

void YamahaMotif::setArpeggioType(int type) {
    // Set arpeggio type via SysEx
}

void YamahaMotif::setMotionSequencer(bool enable) {
    // Motion sequence on/off
}

void YamahaMotif::setScene(int scene) {
    impl_->currentScene = scene;
    // Send scene change
}

void YamahaMotif::setSuperKnob(int value) {
    impl_->superKnobValue = std::clamp(value, 0, 127);
    sendCC(0, 0x5A, static_cast<uint8_t>(impl_->superKnobValue));
}

} // namespace maestro::instruments::yamaha
