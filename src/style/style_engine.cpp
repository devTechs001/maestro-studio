// src/style/style_engine.cpp
#include "maestro/style/style_engine.hpp"
#include "maestro/midi/midi_engine.hpp"
#include "maestro/voice/voice_engine.hpp"
#include "maestro/instruments/instrument.hpp"
#include <algorithm>

namespace maestro {

// ChordInfo implementation
std::vector<MidiNote> ChordInfo::getNotes() const {
    std::vector<MidiNote> notes;
    notes.push_back(root);

    switch (type) {
        case Type::Major:
            notes.push_back(root + 4);
            notes.push_back(root + 7);
            break;
        case Type::Minor:
            notes.push_back(root + 3);
            notes.push_back(root + 7);
            break;
        case Type::Augmented:
            notes.push_back(root + 4);
            notes.push_back(root + 8);
            break;
        case Type::Diminished:
            notes.push_back(root + 3);
            notes.push_back(root + 6);
            break;
        case Type::Seventh:
            notes.push_back(root + 4);
            notes.push_back(root + 7);
            notes.push_back(root + 10);
            break;
        case Type::MajorSeventh:
            notes.push_back(root + 4);
            notes.push_back(root + 7);
            notes.push_back(root + 11);
            break;
        case Type::MinorSeventh:
            notes.push_back(root + 3);
            notes.push_back(root + 7);
            notes.push_back(root + 10);
            break;
        case Type::DiminishedSeventh:
            notes.push_back(root + 3);
            notes.push_back(root + 6);
            notes.push_back(root + 9);
            break;
        case Type::Sixth:
            notes.push_back(root + 4);
            notes.push_back(root + 7);
            notes.push_back(root + 9);
            break;
        case Type::MinorSixth:
            notes.push_back(root + 3);
            notes.push_back(root + 7);
            notes.push_back(root + 9);
            break;
        case Type::Sus4:
            notes.push_back(root + 5);
            notes.push_back(root + 7);
            break;
        case Type::Sus2:
            notes.push_back(root + 2);
            notes.push_back(root + 7);
            break;
        default:
            notes.push_back(root + 4);
            notes.push_back(root + 7);
            break;
    }

    if (hasBass) {
        notes.push_back(bass);
    }

    return notes;
}

ChordInfo ChordInfo::detect(const std::vector<MidiNote>& notes) {
    if (notes.size() < 2) {
        return ChordInfo{};
    }

    ChordInfo chord;
    chord.root = notes[0];

    // Simple detection based on intervals
    std::vector<int> intervals;
    for (size_t i = 1; i < notes.size(); ++i) {
        intervals.push_back((notes[i] - notes[0] + 12) % 12);
    }
    std::sort(intervals.begin(), intervals.end());

    // Detect chord type
    if (intervals == std::vector<int>{3, 7}) {
        chord.type = Type::Minor;
    } else if (intervals == std::vector<int>{4, 7}) {
        chord.type = Type::Major;
    } else if (intervals == std::vector<int>{3, 7, 10}) {
        chord.type = Type::MinorSeventh;
    } else if (intervals == std::vector<int>{4, 7, 10}) {
        chord.type = Type::Seventh;
    } else if (intervals == std::vector<int>{4, 7, 11}) {
        chord.type = Type::MajorSeventh;
    } else if (intervals == std::vector<int>{3, 6}) {
        chord.type = Type::Diminished;
    } else if (intervals == std::vector<int>{4, 8}) {
        chord.type = Type::Augmented;
    } else if (intervals == std::vector<int>{5, 7}) {
        chord.type = Type::Sus4;
    } else if (intervals == std::vector<int>{2, 7}) {
        chord.type = Type::Sus2;
    }

    return chord;
}

std::string ChordInfo::toString() const {
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::string name = noteNames[root % 12];

    switch (type) {
        case Type::Minor: name += "m"; break;
        case Type::Seventh: name += "7"; break;
        case Type::MajorSeventh: name += "maj7"; break;
        case Type::MinorSeventh: name += "m7"; break;
        case Type::Diminished: name += "dim"; break;
        case Type::Augmented: name += "aug"; break;
        case Type::Sixth: name += "6"; break;
        case Type::MinorSixth: name += "m6"; break;
        case Type::Sus4: name += "sus4"; break;
        case Type::Sus2: name += "sus2"; break;
        default: break;
    }

    if (hasBass && bass != root) {
        name += "/" + noteNames[bass % 12];
    }

    return name;
}

ChordInfo ChordInfo::fromString(const std::string& str) {
    ChordInfo chord;
    
    // Parse note name
    static const std::pair<std::string, int> noteMap[] = {
        {"C", 0}, {"C#", 1}, {"Db", 1}, {"D", 2}, {"D#", 3}, {"Eb", 3},
        {"E", 4}, {"F", 5}, {"F#", 6}, {"Gb", 6}, {"G", 7}, {"G#", 8},
        {"Ab", 8}, {"A", 9}, {"A#", 10}, {"Bb", 10}, {"B", 11}
    };

    for (const auto& [noteName, noteNum] : noteMap) {
        if (str.find(noteName) == 0) {
            chord.root = noteNum + 60;  // Middle C octave
            break;
        }
    }

    // Parse chord type
    if (str.find("maj7") != std::string::npos) chord.type = Type::MajorSeventh;
    else if (str.find("m7") != std::string::npos) chord.type = Type::MinorSeventh;
    else if (str.find("7") != std::string::npos) chord.type = Type::Seventh;
    else if (str.find("m6") != std::string::npos) chord.type = Type::MinorSixth;
    else if (str.find("6") != std::string::npos) chord.type = Type::Sixth;
    else if (str.find("sus4") != std::string::npos) chord.type = Type::Sus4;
    else if (str.find("sus2") != std::string::npos) chord.type = Type::Sus2;
    else if (str.find("dim") != std::string::npos) chord.type = Type::Diminished;
    else if (str.find("aug") != std::string::npos) chord.type = Type::Augmented;
    else if (str.find('m') != std::string::npos) chord.type = Type::Minor;
    else chord.type = Type::Major;

    return chord;
}

// StyleEngine implementation
class StyleEngine::Impl {
public:
    instruments::Style currentStyle;
    std::vector<std::pair<int, int>> channelMapping;
};

StyleEngine::StyleEngine(MidiEngine& midi, VoiceEngine& voices)
    : impl_(std::make_unique<Impl>())
    , midi_(midi)
    , voices_(voices) {
}

StyleEngine::~StyleEngine() = default;

Result<void> StyleEngine::initialize() {
    return Result<void>();
}

Result<void> StyleEngine::loadStyle(const std::string& path) {
    // Style loading implementation
    return Result<void>();
}

Result<void> StyleEngine::loadStyleFromMemory(const std::vector<uint8_t>& data) {
    // Style loading from memory
    return Result<void>();
}

void StyleEngine::setStyle(const instruments::Style& style) {
    impl_->currentStyle = style;
}

const instruments::Style& StyleEngine::getCurrentStyle() const {
    return impl_->currentStyle;
}

void StyleEngine::start() {
    playing_ = true;
    paused_ = false;
}

void StyleEngine::stop() {
    playing_ = false;
    paused_ = false;
    position_ = 0;
}

void StyleEngine::pause() {
    paused_ = true;
}

void StyleEngine::resume() {
    paused_ = false;
}

void StyleEngine::setVariation(int variation) {
    currentVariation_ = std::clamp(variation, 0, 3);
}

void StyleEngine::playIntro(int number) {
    // Intro implementation
}

void StyleEngine::playEnding(int number) {
    // Ending implementation
}

void StyleEngine::playFill(int variation) {
    // Fill implementation
}

void StyleEngine::playBreak() {
    // Break implementation
}

void StyleEngine::setTempo(double bpm) {
    tempo_ = std::clamp(bpm, 40.0, 300.0);
}

double StyleEngine::getTempo() const {
    return tempo_;
}

void StyleEngine::setSyncStart(bool enabled) {
    syncStart_ = enabled;
}

bool StyleEngine::isSyncStart() const {
    return syncStart_;
}

void StyleEngine::setChord(const ChordInfo& chord) {
    currentChord_ = chord;
}

ChordInfo StyleEngine::getCurrentChord() const {
    return currentChord_;
}

void StyleEngine::setChordFromNotes(const std::vector<MidiNote>& notes) {
    currentChord_ = ChordInfo::detect(notes);
}

void StyleEngine::setTranspose(int semitones) {
    transpose_ = std::clamp(semitones, -12, 12);
}

int StyleEngine::getTranspose() const {
    return transpose_;
}

void StyleEngine::setChannelMapping(int styleChannel, int midiChannel) {
    auto it = std::find_if(impl_->channelMapping.begin(), impl_->channelMapping.end(),
        [styleChannel](const auto& p) { return p.first == styleChannel; });
    
    if (it != impl_->channelMapping.end()) {
        it->second = midiChannel;
    } else {
        impl_->channelMapping.emplace_back(styleChannel, midiChannel);
    }
}

void StyleEngine::setStyleCallback(StyleCallback callback) {
    callback_ = std::move(callback);
}

} // namespace maestro
