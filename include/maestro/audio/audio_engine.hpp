// include/maestro/audio/audio_engine.hpp
#pragma once

#include "maestro/core/types.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <atomic>

namespace maestro {

/**
 * @brief Audio processing callback type
 */
using AudioCallback = std::function<void(
    const float* const* inputChannels,
    float* const* outputChannels,
    uint32_t numFrames,
    uint32_t numInputChannels,
    uint32_t numOutputChannels
)>;

/**
 * @brief Audio device information
 */
struct AudioDevice {
    std::string id;
    std::string name;
    std::string driverName;
    int maxInputChannels;
    int maxOutputChannels;
    std::vector<SampleRate> supportedSampleRates;
    std::vector<BufferSize> supportedBufferSizes;
    bool isDefault;
};

/**
 * @brief Core Audio Engine
 */
class MAESTRO_API AudioEngine {
public:
    struct Config {
        SampleRate sampleRate = 44100;
        BufferSize bufferSize = 256;
        ChannelCount inputChannels = 2;
        ChannelCount outputChannels = 2;
        std::string inputDeviceId;
        std::string outputDeviceId;
        std::string driverName = "auto";
    };

    AudioEngine(SampleRate sampleRate, BufferSize bufferSize);
    ~AudioEngine();

    // Initialization
    Result<void> initialize(const Config& config);

    // Lifecycle
    Result<void> start();
    Result<void> stop();
    bool isRunning() const;

    // Device management
    std::vector<AudioDevice> getAvailableDevices() const;
    Result<void> setInputDevice(const std::string& deviceId);
    Result<void> setOutputDevice(const std::string& deviceId);

    // Configuration
    Result<void> setSampleRate(SampleRate rate);
    Result<void> setBufferSize(BufferSize size);
    SampleRate getSampleRate() const { return config_.sampleRate; }
    BufferSize getBufferSize() const { return config_.bufferSize; }

    // Callback registration
    void setAudioCallback(AudioCallback callback);

    // Performance
    double getCpuUsage() const;
    double getLatency() const;
    uint64_t getUnderrunCount() const;

    // Audio processing chain
    class AudioProcessor;
    void addProcessor(std::shared_ptr<AudioProcessor> processor);
    void removeProcessor(std::shared_ptr<AudioProcessor> processor);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    Config config_;
    AudioCallback callback_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> underruns_{0};
};

/**
 * @brief Base class for audio processors
 */
class MAESTRO_API AudioEngine::AudioProcessor {
public:
    virtual ~AudioProcessor() = default;

    virtual void prepare(SampleRate sampleRate, BufferSize bufferSize) = 0;
    virtual void process(float* const* channels,
                        uint32_t numChannels,
                        uint32_t numFrames) = 0;
    virtual void reset() = 0;

    virtual std::string getName() const = 0;
    virtual bool isBypassed() const { return bypassed_; }
    virtual void setBypassed(bool bypassed) { bypassed_ = bypassed; }

protected:
    bool bypassed_ = false;
};

/**
 * @brief Audio mixer channel
 */
class MAESTRO_API MixerChannel {
public:
    MixerChannel(const std::string& name, int index);

    // Volume and pan
    void setVolume(float volume);  // 0.0 - 1.0
    void setPan(float pan);        // -1.0 (L) to 1.0 (R)
    void setMute(bool mute);
    void setSolo(bool solo);

    float getVolume() const { return volume_; }
    float getPan() const { return pan_; }
    bool isMuted() const { return muted_; }
    bool isSoloed() const { return soloed_; }

    // Effects sends
    void setSendLevel(int sendIndex, float level);
    float getSendLevel(int sendIndex) const;

    void addInsert(std::shared_ptr<AudioEngine::AudioProcessor> effect);
    void removeInsert(int index);

    // Processing
    void process(float* left, float* right, uint32_t numFrames);

    // Metering
    float getPeakLevel(int channel) const;
    float getRmsLevel(int channel) const;

private:
    std::string name_;
    int index_;
    std::atomic<float> volume_{1.0f};
    std::atomic<float> pan_{0.0f};
    std::atomic<bool> muted_{false};
    std::atomic<bool> soloed_{false};
    std::vector<float> sendLevels_;
    std::vector<std::shared_ptr<AudioEngine::AudioProcessor>> inserts_;
    float peakLevels_[2] = {0.0f, 0.0f};
    float rmsLevels_[2] = {0.0f, 0.0f};
};

/**
 * @brief Main audio mixer
 */
class MAESTRO_API AudioMixer {
public:
    AudioMixer(int numChannels = 32, int numBuses = 8);

    // Channel access
    MixerChannel& channel(int index);
    const MixerChannel& channel(int index) const;
    int numChannels() const { return channels_.size(); }

    // Buses
    MixerChannel& bus(int index);
    MixerChannel& masterBus();

    // Routing
    void routeChannelToBus(int channelIndex, int busIndex);

    // Processing
    void process(float* const* outputBuffers, uint32_t numFrames);

private:
    std::vector<std::unique_ptr<MixerChannel>> channels_;
    std::vector<std::unique_ptr<MixerChannel>> buses_;
    std::unique_ptr<MixerChannel> master_;
};

} // namespace maestro
