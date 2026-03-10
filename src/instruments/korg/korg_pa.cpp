// src/instruments/korg/korg_pa.cpp
#include "maestro/instruments/instrument.hpp"

namespace maestro::instruments::korg {

class KorgPA : public Instrument {
public:
    enum class Model { PA5X, PA4X, PA1000, PA700 };

    KorgPA(Model model);
    ~KorgPA() override;

    std::string manufacturer() const override { return "Korg"; }
    std::string model() const override;
    std::string version() const override { return "1.0.0"; }

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

    // PA-specific features
    void setSTS(int number);  // Single Touch Settings
    void setKeyboardSet(int bank, int number);
    void setSongbookEntry(int number);

private:
    Model model_;
    class Impl;
    std::unique_ptr<Impl> impl_;
    bool connected_ = false;
};

class KorgPA::Impl {
public:
    std::vector<Voice> voices;
    std::vector<Style> styles;
    std::vector<Registration> registrations;
    int currentSTS = 0;
    int currentKeyboardSet = 0;
    int currentSongbook = 0;
};

KorgPA::KorgPA(Model model) : model_(model), impl_(std::make_unique<Impl>()) {}
KorgPA::~KorgPA() = default;

std::string KorgPA::model() const {
    switch (model_) {
        case Model::PA5X: return "PA5X";
        case Model::PA4X: return "PA4X";
        case Model::PA1000: return "PA1000";
        case Model::PA700: return "PA700";
        default: return "PA";
    }
}

bool KorgPA::connect(const std::string& midiIn, const std::string& midiOut) {
    connected_ = true;
    return true;
}

void KorgPA::disconnect() {
    connected_ = false;
}

bool KorgPA::isConnected() const {
    return connected_;
}

std::vector<Voice> KorgPA::getVoices() const {
    return impl_->voices;
}

Result<void> KorgPA::setVoice(int part, const Voice& voice) {
    return Result<void>();
}

Voice KorgPA::getVoice(int part) const {
    return Voice{};
}

std::vector<Style> KorgPA::getStyles() const {
    return impl_->styles;
}

Result<void> KorgPA::loadStyle(const std::string& path) {
    return Result<void>();
}

Result<void> KorgPA::setStyle(const Style& style) {
    return Result<void>();
}

Style KorgPA::getCurrentStyle() const {
    return Style{};
}

std::vector<Registration> KorgPA::getRegistrations() const {
    return impl_->registrations;
}

Result<void> KorgPA::loadRegistration(const Registration& reg) {
    return Result<void>();
}

Result<void> KorgPA::saveRegistration(const std::string& name) {
    return Result<void>();
}

void KorgPA::sendNoteOn(MidiChannel ch, MidiNote note, MidiVelocity vel) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::noteOn(ch, note, vel));
    }
}

void KorgPA::sendNoteOff(MidiChannel ch, MidiNote note) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::noteOff(ch, note));
    }
}

void KorgPA::sendCC(MidiChannel ch, MidiCC cc, uint8_t value) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::controlChange(ch, cc, value));
    }
}

void KorgPA::sendProgramChange(MidiChannel ch, uint8_t program) {
    if (midiCallback_) {
        midiCallback_(midi::MidiMessage::programChange(ch, program));
    }
}

void KorgPA::sendSysEx(const std::vector<uint8_t>& data) {
    if (sysExCallback_) {
        midi::SysExMessage msg;
        msg.data = data;
        sysExCallback_(msg);
    }
}

void KorgPA::styleStart() {}
void KorgPA::styleStop() {}
void KorgPA::styleSync() {}
void KorgPA::styleIntro(int num) {}
void KorgPA::styleEnding(int num) {}
void KorgPA::styleMainVariation(int num) {}
void KorgPA::styleFill(int num) {}
void KorgPA::styleBreak() {}

void KorgPA::setSTS(int number) {
    impl_->currentSTS = number;
    sendCC(0, 0x60, static_cast<uint8_t>(number));
}

void KorgPA::setKeyboardSet(int bank, int number) {
    impl_->currentKeyboardSet = bank * 10 + number;
}

void KorgPA::setSongbookEntry(int number) {
    impl_->currentSongbook = number;
}

} // namespace maestro::instruments::korg
