// src/pad/pad_engine.cpp
#include "maestro/pad/pad_engine.hpp"
#include "maestro/midi/midi_engine.hpp"
#include "maestro/audio/audio_engine.hpp"

namespace maestro {

class PadEngine::Impl {
public:
    std::vector<std::vector<PadConfig>> banks;
    std::vector<int> padGroups;
    std::vector<bool> groupExclusive;
    std::vector<bool> padPlaying;
    std::vector<bool> padRecording;
    int currentBank = 0;
    int padsPerBank = 16;

    Impl() {
        banks.resize(4);  // 4 banks
        for (auto& bank : banks) {
            bank.resize(16);  // 16 pads per bank
            for (size_t i = 0; i < bank.size(); ++i) {
                bank[i].index = static_cast<int>(i);
                bank[i].name = "Pad " + std::to_string(i + 1);
            }
        }
        padGroups.resize(64, -1);
        groupExclusive.resize(64, false);
        padPlaying.resize(64, false);
        padRecording.resize(64, false);
    }
};

PadEngine::PadEngine(MidiEngine& midi, AudioEngine& audio)
    : impl_(std::make_unique<Impl>())
    , midi_(midi)
    , audio_(audio) {
}

PadEngine::~PadEngine() = default;

Result<void> PadEngine::initialize() {
    return Result<void>();
}

void PadEngine::setPadConfig(int index, const PadConfig& config) {
    int bank = index / impl_->padsPerBank;
    int padInBank = index % impl_->padsPerBank;
    
    if (bank >= 0 && bank < static_cast<int>(impl_->banks.size()) &&
        padInBank >= 0 && padInBank < impl_->padsPerBank) {
        impl_->banks[bank][padInBank] = config;
    }
}

PadConfig PadEngine::getPadConfig(int index) const {
    int bank = index / impl_->padsPerBank;
    int padInBank = index % impl_->padsPerBank;
    
    if (bank >= 0 && bank < static_cast<int>(impl_->banks.size()) &&
        padInBank >= 0 && padInBank < impl_->padsPerBank) {
        return impl_->banks[bank][padInBank];
    }
    return PadConfig{};
}

std::vector<PadConfig> PadEngine::getAllPadConfigs() const {
    std::vector<PadConfig> configs;
    for (const auto& bank : impl_->banks) {
        for (const auto& pad : bank) {
            configs.push_back(pad);
        }
    }
    return configs;
}

void PadEngine::triggerPad(int index, int velocity) {
    if (index < 0 || index >= 64) return;

    int bank = impl_->currentBank;
    int padInBank = index % impl_->padsPerBank;
    const auto& config = impl_->banks[bank][padInBank];

    if (config.muted) return;

    // Handle exclusive groups
    int groupId = impl_->padGroups[index];
    if (groupId >= 0 && impl_->groupExclusive[groupId]) {
        for (int i = 0; i < 64; ++i) {
            if (impl_->padGroups[i] == groupId && i != index) {
                stopPad(i);
            }
        }
    }

    impl_->padPlaying[index] = true;

    switch (config.type) {
        case PadType::MidiNote:
            midi_.send(midi::MidiMessage::noteOn(
                config.midiChannel, config.midiNote, static_cast<MidiVelocity>(velocity)));
            break;

        case PadType::Sample:
            // Sample playback would be handled by audio engine
            break;

        case PadType::Phrase:
            // MIDI phrase playback
            break;

        case PadType::Style:
            // Style trigger
            break;

        case PadType::Scene:
            // Scene recall
            break;

        case PadType::Macro:
            // Macro execution
            break;
    }

    if (triggerCallback_) {
        triggerCallback_(index, velocity);
    }
}

void PadEngine::releasePad(int index) {
    if (index < 0 || index >= 64) return;

    int bank = impl_->currentBank;
    int padInBank = index % impl_->padsPerBank;
    const auto& config = impl_->banks[bank][padInBank];

    if (config.type == PadType::MidiNote) {
        midi_.send(midi::MidiMessage::noteOff(
            config.midiChannel, config.midiNote));
    }

    impl_->padPlaying[index] = false;

    if (releaseCallback_) {
        releaseCallback_(index);
    }
}

void PadEngine::stopPad(int index) {
    if (index < 0 || index >= 64) return;

    int bank = impl_->currentBank;
    int padInBank = index % impl_->padsPerBank;
    const auto& config = impl_->banks[bank][padInBank];

    if (config.type == PadType::MidiNote) {
        midi_.send(midi::MidiMessage::noteOff(
            config.midiChannel, config.midiNote));
    }

    impl_->padPlaying[index] = false;
}

void PadEngine::stopAllPads() {
    for (int i = 0; i < 64; ++i) {
        stopPad(i);
    }
}

void PadEngine::setPadGroup(int index, int groupId) {
    if (index >= 0 && index < 64) {
        impl_->padGroups[index] = groupId;
    }
}

int PadEngine::getPadGroup(int index) const {
    if (index >= 0 && index < 64) {
        return impl_->padGroups[index];
    }
    return -1;
}

void PadEngine::setGroupExclusive(int groupId, bool exclusive) {
    if (groupId >= 0 && groupId < 64) {
        impl_->groupExclusive[groupId] = exclusive;
    }
}

void PadEngine::setCurrentBank(int bank) {
    impl_->currentBank = std::clamp(bank, 0, static_cast<int>(impl_->banks.size()) - 1);
}

int PadEngine::getCurrentBank() const {
    return impl_->currentBank;
}

int PadEngine::getBankCount() const {
    return static_cast<int>(impl_->banks.size());
}

void PadEngine::setBankSize(int padsPerBank) {
    impl_->padsPerBank = std::clamp(padsPerBank, 4, 64);
}

Result<void> PadEngine::loadSample(int padIndex, const std::string& path) {
    int bank = impl_->currentBank;
    int padInBank = padIndex % impl_->padsPerBank;
    
    if (bank >= 0 && bank < static_cast<int>(impl_->banks.size()) &&
        padInBank >= 0 && padInBank < impl_->padsPerBank) {
        impl_->banks[bank][padInBank].samplePath = path;
        impl_->banks[bank][padInBank].type = PadType::Sample;
    }
    
    return Result<void>();
}

void PadEngine::unloadSample(int padIndex) {
    int bank = impl_->currentBank;
    int padInBank = padIndex % impl_->padsPerBank;
    
    if (bank >= 0 && bank < static_cast<int>(impl_->banks.size()) &&
        padInBank >= 0 && padInBank < impl_->padsPerBank) {
        impl_->banks[bank][padInBank].samplePath.clear();
    }
}

void PadEngine::startPadRecording(int padIndex) {
    if (padIndex >= 0 && padIndex < 64) {
        impl_->padRecording[padIndex] = true;
    }
}

void PadEngine::stopPadRecording(int padIndex) {
    if (padIndex >= 0 && padIndex < 64) {
        impl_->padRecording[padIndex] = false;
    }
}

bool PadEngine::isRecording(int padIndex) const {
    if (padIndex >= 0 && padIndex < 64) {
        return impl_->padRecording[padIndex];
    }
    return false;
}

bool PadEngine::isPadPlaying(int index) const {
    if (index >= 0 && index < 64) {
        return impl_->padPlaying[index];
    }
    return false;
}

int PadEngine::getPlayingPadCount() const {
    int count = 0;
    for (bool playing : impl_->padPlaying) {
        if (playing) count++;
    }
    return count;
}

void PadEngine::setPadTriggerCallback(PadCallback callback) {
    triggerCallback_ = std::move(callback);
}

void PadEngine::setPadReleaseCallback(ReleaseCallback callback) {
    releaseCallback_ = std::move(callback);
}

} // namespace maestro
