// include/maestro/style/style_engine.hpp
#pragma once

#include "maestro/core/config.hpp"
#include "maestro/core/types.hpp"
#include "maestro/midi/midi_types.hpp"
#include <memory>
#include <vector>
#include <string>

namespace maestro {

class MidiEngine;
class VoiceEngine;

namespace instruments {
    struct Style;
    struct Voice;
}

/**
 * @brief Chord information
 */
struct ChordInfo {
    enum class Type {
        Major, Minor, Augmented, Diminished,
        Seventh, MajorSeventh, MinorSeventh,
        DiminishedSeventh, AugmentedSeventh,
        Sixth, MinorSixth, Ninth, Eleventh, Thirteenth,
        Sus2, Sus4, Add9, NoThird
    };

    MidiNote root = 60;
    Type type = Type::Major;
    MidiNote bass = 0;
    bool hasBass = false;

    std::vector<MidiNote> getNotes() const;
    static ChordInfo detect(const std::vector<MidiNote>& notes);
    std::string toString() const;
    static ChordInfo fromString(const std::string& str);
};

/**
 * @brief Style playback engine
 */
class MAESTRO_API StyleEngine {
public:
    StyleEngine(MidiEngine& midi, VoiceEngine& voices);
    ~StyleEngine();

    Result<void> initialize();

    // Style loading
    Result<void> loadStyle(const std::string& path);
    Result<void> loadStyleFromMemory(const std::vector<uint8_t>& data);

    // Style selection
    void setStyle(const instruments::Style& style);
    const instruments::Style& getCurrentStyle() const;

    // Playback control
    void start();
    void stop();
    void pause();
    void resume();

    // Variations
    void setVariation(int variation);  // 0=MainA, 1=MainB, 2=MainC, 3=MainD
    void playIntro(int number = 1);
    void playEnding(int number = 1);
    void playFill(int variation = 0);
    void playBreak();

    // Tempo and sync
    void setTempo(double bpm);
    double getTempo() const;
    void setSyncStart(bool enabled);
    bool isSyncStart() const;

    // Chord detection
    void setChord(const ChordInfo& chord);
    ChordInfo getCurrentChord() const;
    void setChordFromNotes(const std::vector<MidiNote>& notes);

    // Transposition
    void setTranspose(int semitones);
    int getTranspose() const;

    // Channel routing
    void setChannelMapping(int styleChannel, int midiChannel);

    // State
    bool isPlaying() const { return playing_; }
    bool isPaused() const { return paused_; }
    int getCurrentVariation() const { return currentVariation_; }
    TickCount getPosition() const { return position_; }

    // Callbacks
    using StyleCallback = std::function<void(int variation, TickCount position)>;
    void setStyleCallback(StyleCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    MidiEngine& midi_;
    VoiceEngine& voices_;
    bool playing_ = false;
    bool paused_ = false;
    bool syncStart_ = false;
    int currentVariation_ = 0;
    TickCount position_ = 0;
    double tempo_ = 120.0;
    int transpose_ = 0;
    ChordInfo currentChord_;
    StyleCallback callback_;
};

} // namespace maestro
