// include/maestro/studio/project.hpp
#pragma once

#include "maestro/core/types.hpp"
#include "maestro/midi/midi_types.hpp"
#include "maestro/audio/audio_engine.hpp"
#include <vector>
#include <memory>
#include <string>
#include <array>

namespace maestro::studio {

/**
 * @brief Audio/MIDI clip
 */
struct Clip {
    enum class Type { Audio, Midi };

    std::string name;
    Type type;
    TickCount startTick;
    TickCount length;
    TickCount offset;    // Start offset within source
    double gain = 1.0;
    bool muted = false;
    int color = 0xFF0080FF;  // ARGB

    // MIDI data (if MIDI clip)
    std::vector<midi::NoteEvent> notes;
    std::vector<midi::MidiEvent> events;

    // Audio data (if Audio clip)
    std::string audioFilePath;
    double audioStartTime;
    double audioDuration;
};

/**
 * @brief Track in the project
 */
class Track {
public:
    enum class Type { Midi, Audio, Instrument, Bus, Master };

    Track(const std::string& name, Type type);

    std::string name() const { return name_; }
    Type type() const { return type_; }

    // Clips
    void addClip(const Clip& clip);
    void removeClip(size_t index);
    Clip& clip(size_t index);
    size_t clipCount() const;

    // Mixer settings
    float volume() const { return volume_; }
    void setVolume(float vol) { volume_ = vol; }
    float pan() const { return pan_; }
    void setPan(float p) { pan_ = p; }
    bool isMuted() const { return muted_; }
    void setMuted(bool m) { muted_ = m; }
    bool isSoloed() const { return soloed_; }
    void setSoloed(bool s) { soloed_ = s; }
    bool isArmed() const { return armed_; }
    void setArmed(bool a) { armed_ = a; }

    // Routing
    void setOutput(const std::string& busName);
    void setMidiOutput(const std::string& portId, MidiChannel channel);

    // Effects
    void addEffect(std::shared_ptr<AudioEngine::AudioProcessor> effect);
    void removeEffect(size_t index);

    // Automation
    struct AutomationPoint {
        TickCount tick;
        double value;
        enum class Curve { Linear, Smooth, Step } curve = Curve::Linear;
    };

    struct AutomationLane {
        std::string parameter;
        std::vector<AutomationPoint> points;
    };

    void addAutomationLane(const std::string& parameter);
    AutomationLane& automationLane(const std::string& parameter);

private:
    std::string name_;
    Type type_;
    std::vector<Clip> clips_;
    float volume_ = 1.0f;
    float pan_ = 0.0f;
    bool muted_ = false;
    bool soloed_ = false;
    bool armed_ = false;
    std::string output_;
    std::string midiPort_;
    MidiChannel midiChannel_ = 0;
    std::vector<std::shared_ptr<AudioEngine::AudioProcessor>> effects_;
    std::vector<AutomationLane> automationLanes_;
};

/**
 * @brief Project container
 */
class MAESTRO_API Project {
public:
    Project();

    // File operations
    static Result<Project> load(const std::string& path);
    Result<void> save(const std::string& path);

    // Properties
    std::string name() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    double tempo() const { return tempo_; }
    void setTempo(double bpm) { tempo_ = bpm; }
    TimeSignature timeSignature() const { return timeSig_; }
    void setTimeSignature(const TimeSignature& ts) { timeSig_ = ts; }
    SampleRate sampleRate() const { return sampleRate_; }

    // Tracks
    Track& addTrack(const std::string& name, Track::Type type);
    void removeTrack(size_t index);
    Track& track(size_t index);
    const Track& track(size_t index) const;
    size_t trackCount() const;

    // Markers
    struct Marker {
        std::string name;
        TickCount tick;
        int color;
    };
    void addMarker(const Marker& marker);
    std::vector<Marker>& markers() { return markers_; }

    // Time selection
    void setLoopRegion(TickCount start, TickCount end);
    std::pair<TickCount, TickCount> loopRegion() const;
    bool isLoopEnabled() const { return loopEnabled_; }
    void setLoopEnabled(bool enabled) { loopEnabled_ = enabled; }

private:
    std::string name_ = "Untitled";
    std::string path_;
    double tempo_ = 120.0;
    TimeSignature timeSig_;
    SampleRate sampleRate_ = 44100;
    std::vector<std::unique_ptr<Track>> tracks_;
    std::vector<Marker> markers_;
    TickCount loopStart_ = 0;
    TickCount loopEnd_ = 0;
    bool loopEnabled_ = false;
};

/**
 * @brief Plugin host for VST/AU plugins
 */
class MAESTRO_API PluginHost {
public:
    struct PluginInfo {
        std::string id;
        std::string name;
        std::string manufacturer;
        std::string path;
        enum class Type { VST2, VST3, AU, CLAP } type;
        bool isInstrument;
        bool hasEditor;
        int numInputs;
        int numOutputs;
    };

    PluginHost();
    ~PluginHost();

    // Scanning
    void scanPluginDirectories();
    void addPluginDirectory(const std::string& path);
    std::vector<PluginInfo> getAvailablePlugins() const;

    // Loading
    class PluginInstance;
    std::unique_ptr<PluginInstance> loadPlugin(const std::string& id);

    // Processing
    void prepareToPlay(SampleRate sampleRate, BufferSize bufferSize);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class MAESTRO_API PluginHost::PluginInstance {
public:
    virtual ~PluginInstance() = default;

    virtual const PluginInfo& info() const = 0;

    // Processing
    virtual void process(float* const* audioChannels, int numChannels,
                        int numFrames) = 0;
    virtual void processMidi(const midi::MidiMessage& msg) = 0;

    // Parameters
    virtual int getNumParameters() const = 0;
    virtual std::string getParameterName(int index) const = 0;
    virtual float getParameter(int index) const = 0;
    virtual void setParameter(int index, float value) = 0;

    // State
    virtual std::vector<uint8_t> getState() const = 0;
    virtual void setState(const std::vector<uint8_t>& state) = 0;

    // Editor
    virtual bool hasEditor() const = 0;
    virtual void openEditor(void* parentWindow) = 0;
    virtual void closeEditor() = 0;
    virtual std::pair<int, int> getEditorSize() const = 0;
};

} // namespace maestro::studio
