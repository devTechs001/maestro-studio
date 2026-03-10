// include/maestro/core/engine.hpp
#pragma once

#include "config.hpp"
#include "types.hpp"
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

namespace maestro {

// Forward declarations
class AudioEngine;
class MidiEngine;
class StyleEngine;
class VoiceEngine;
class PadEngine;
class SyncEngine;

/**
 * @brief Main MaestroStudio Engine
 *
 * Central coordinator for all engine subsystems.
 * Thread-safe singleton pattern.
 */
class MAESTRO_API MaestroEngine {
public:
    // Engine state
    enum class State {
        Uninitialized,
        Initializing,
        Ready,
        Running,
        Stopping,
        Error
    };

    // Configuration
    struct Config {
        SampleRate sampleRate = config::DEFAULT_SAMPLE_RATE;
        BufferSize bufferSize = config::DEFAULT_BUFFER_SIZE;
        ChannelCount inputChannels = 2;
        ChannelCount outputChannels = 2;
        std::string audioDriver = "auto";
        std::string midiDriver = "auto";
        bool enablePlugins = true;
        bool enableNetwork = false;
        std::string dataPath;
    };

    // Singleton access
    static MaestroEngine& instance();

    // Lifecycle
    Result<void> initialize(const Config& config);
    Result<void> start();
    Result<void> stop();
    void shutdown();

    // State
    State getState() const { return state_.load(); }
    bool isRunning() const { return state_ == State::Running; }

    // Subsystem access
    AudioEngine& audio();
    MidiEngine& midi();
    StyleEngine& styles();
    VoiceEngine& voices();
    PadEngine& pads();
    SyncEngine& sync();

    // Configuration
    const Config& config() const { return config_; }
    Result<void> setConfig(const Config& config);

    // Performance monitoring
    struct Performance {
        double cpuUsage = 0.0;
        double audioLatency = 0.0;
        double midiLatency = 0.0;
        uint64_t bufferUnderruns = 0;
        uint64_t midiOverflows = 0;
    };
    Performance getPerformance() const;

private:
    MaestroEngine();
    ~MaestroEngine();
    MaestroEngine(const MaestroEngine&) = delete;
    MaestroEngine& operator=(const MaestroEngine&) = delete;

    std::atomic<State> state_{State::Uninitialized};
    Config config_;

    std::unique_ptr<AudioEngine> audioEngine_;
    std::unique_ptr<MidiEngine> midiEngine_;
    std::unique_ptr<StyleEngine> styleEngine_;
    std::unique_ptr<VoiceEngine> voiceEngine_;
    std::unique_ptr<PadEngine> padEngine_;
    std::unique_ptr<SyncEngine> syncEngine_;

    mutable std::mutex mutex_;
    std::thread processingThread_;
};

} // namespace maestro
