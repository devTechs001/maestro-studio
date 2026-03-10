// include/maestro/studio/recording.hpp
#pragma once

#include "maestro/core/types.hpp"
#include "maestro/audio/audio_engine.hpp"
#include <memory>
#include <functional>
#include <atomic>
#include <vector>

namespace maestro::studio {

class AudioEngine;
class MidiEngine;

/**
 * @brief Audio recording engine
 */
class MAESTRO_API AudioRecorder {
public:
    AudioRecorder(AudioEngine& audio);
    ~AudioRecorder();

    // Configuration
    struct Config {
        std::string outputPath;
        enum Format { WAV, AIFF, FLAC, MP3, OGG } format = WAV;
        int bitDepth = 24;
        SampleRate sampleRate = 44100;
        int numChannels = 2;
        bool realtime = true;
        bool createNewFileOnOverflow = true;
        size_t maxFileSizeMB = 2048;
        bool addTimestamp = true;
    };

    void setConfig(const Config& config);
    Config getConfig() const { return config_; }

    // Recording control
    Result<void> arm();
    Result<void> start();
    void stop();
    void pause();
    void resume();

    // State
    enum class State { Idle, Armed, Recording, Paused };
    State getState() const { return state_.load(); }
    bool isRecording() const { return state_ == State::Recording; }

    // Progress
    double getRecordedTime() const;
    uint64_t getRecordedSamples() const;
    size_t getFileSizeBytes() const;

    // Input monitoring
    void setInputMonitoring(bool enabled);
    bool isInputMonitoring() const;
    float getInputLevel(int channel) const;

    // Markers during recording
    void addMarker(const std::string& name);

    // Punch in/out
    void setPunchIn(double timeSeconds);
    void setPunchOut(double timeSeconds);
    void clearPunchPoints();

    // Callbacks
    using RecordingCallback = std::function<void(State, const std::string&)>;
    void setCallback(RecordingCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    AudioEngine& audio_;
    Config config_;
    std::atomic<State> state_{State::Idle};
    RecordingCallback callback_;
};

/**
 * @brief MIDI recording engine
 */
class MAESTRO_API MidiRecorder {
public:
    MidiRecorder(MidiEngine& midi);
    ~MidiRecorder();

    // Recording
    void arm(int trackIndex);
    void disarm(int trackIndex);
    void start();
    void stop();

    // State
    bool isRecording() const;
    bool isArmed(int trackIndex) const;

    // Options
    void setQuantizeOnRecord(bool enabled);
    void setQuantizeValue(int divisions);
    void setCountIn(int bars);
    void setMetronome(bool enabled);

    // Overdub
    enum class OverdubMode { Replace, Merge, TakeLayer };
    void setOverdubMode(OverdubMode mode);

    // Step recording
    void setStepRecording(bool enabled);
    void setStepLength(int divisions);
    void stepForward();
    void stepBackward();

    // Loop recording
    void setLoopRecording(bool enabled);
    void setLoopRegion(TickCount start, TickCount end);
    int getLoopTakeCount() const;
    void selectLoopTake(int take);

    // Get recorded data
    std::vector<midi::MidiEvent> getRecordedEvents() const;
    void clearRecordedEvents();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Multi-take manager
 */
class MAESTRO_API TakeManager {
public:
    struct Take {
        int id;
        std::string name;
        TickCount startTick;
        TickCount endTick;
        std::vector<midi::MidiEvent> midiEvents;
        std::string audioFilePath;
        bool isComped = false;
        float rating = 0.0f;
        int color = 0xFF0080FF;
    };

    struct CompRegion {
        TickCount start;
        TickCount end;
        int takeId;
    };

    TakeManager();
    ~TakeManager();

    // Take management
    int createTake(const std::string& name);
    void deleteTake(int takeId);
    Take& getTake(int takeId);
    std::vector<int> getTakeIds() const;

    // Comping
    void addCompRegion(const CompRegion& region);
    void removeCompRegion(int index);
    std::vector<CompRegion> getCompRegions() const;

    // Generate comped result
    Take generateCompedTake();

    // Quick comp
    void compFromTake(int takeId, TickCount start, TickCount end);
    void quickSwipe(TickCount position, int takeId);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Audio file import/export
 */
class MAESTRO_API AudioFileManager {
public:
    // Import
    struct ImportOptions {
        bool convertSampleRate = true;
        SampleRate targetSampleRate = 44100;
        bool normalize = false;
        bool removeDCOffset = true;
        bool detectTempo = true;
    };

    struct ImportResult {
        std::string path;
        SampleRate sampleRate;
        int numChannels;
        uint64_t numSamples;
        double duration;
        double detectedTempo;
        std::vector<uint32_t> transients;
    };

    static Result<ImportResult> importFile(const std::string& path,
                                            const ImportOptions& options = {});

    // Export
    struct ExportOptions {
        enum Format { WAV, AIFF, FLAC, MP3, OGG, AAC };
        Format format = WAV;
        int bitDepth = 24;
        int bitRate = 320;
        SampleRate sampleRate = 44100;
        bool dither = true;
        bool normalize = false;
        float normalizeLevel = -0.3f;

        // Metadata
        std::string title;
        std::string artist;
        std::string album;
        std::string comment;
    };

    static Result<void> exportFile(const std::string& inputPath,
                                   const std::string& outputPath,
                                   const ExportOptions& options);

    // Batch export
    struct StemExport {
        std::string name;
        std::vector<int> trackIndices;
    };

    static Result<void> exportStems(const Project& project,
                                     const std::string& outputDir,
                                     const std::vector<StemExport>& stems,
                                     const ExportOptions& options);

    // Mixdown
    static Result<void> mixdown(const Project& project,
                                 const std::string& outputPath,
                                 const ExportOptions& options,
                                 TickCount startTick = 0,
                                 TickCount endTick = -1);
};

/**
 * @brief Timestretch and pitch shift
 */
class MAESTRO_API TimeStretchProcessor {
public:
    enum class Algorithm {
        SoundTouch,
        RubberBand,
        Elastique,
        Simple
    };

    TimeStretchProcessor(Algorithm algorithm = Algorithm::RubberBand);
    ~TimeStretchProcessor();

    void setAlgorithm(Algorithm algorithm);
    void prepare(SampleRate sampleRate, int numChannels);

    void setTimeRatio(double ratio);
    void setPitchSemitones(double semitones);
    void setFormantPreservation(bool preserve);

    void process(const float* input, float* output, int numFrames);
    void flush();
    void reset();

    int getLatency() const;

    static Result<void> processFile(const std::string& inputPath,
                                     const std::string& outputPath,
                                     double timeRatio,
                                     double pitchSemitones,
                                     Algorithm algorithm = Algorithm::RubberBand);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace maestro::studio
