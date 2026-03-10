// include/maestro/ml/ml_engine.hpp
#pragma once

#include "maestro/core/types.hpp"
#include "maestro/midi/midi_types.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <future>
#include <map>

namespace maestro::ml {

/**
 * @brief Tensor representation for ML operations
 */
class MAESTRO_API Tensor {
public:
    enum class DataType { Float32, Float16, Int32, Int64 };

    Tensor();
    Tensor(const std::vector<int64_t>& shape, DataType dtype = DataType::Float32);
    Tensor(const Tensor& other);
    Tensor(Tensor&& other) noexcept;
    ~Tensor();

    Tensor& operator=(const Tensor& other);
    Tensor& operator=(Tensor&& other) noexcept;

    const std::vector<int64_t>& shape() const { return shape_; }
    int64_t size() const;
    int64_t numDimensions() const { return shape_.size(); }
    DataType dataType() const { return dtype_; }

    float* data();
    const float* data() const;
    float& at(const std::vector<int64_t>& indices);
    const float& at(const std::vector<int64_t>& indices) const;

    Tensor reshape(const std::vector<int64_t>& newShape) const;
    Tensor transpose(const std::vector<int64_t>& axes) const;
    Tensor slice(int64_t dim, int64_t start, int64_t end) const;

    static Tensor zeros(const std::vector<int64_t>& shape);
    static Tensor ones(const std::vector<int64_t>& shape);
    static Tensor random(const std::vector<int64_t>& shape, float min = 0.0f, float max = 1.0f);

    std::vector<uint8_t> serialize() const;
    static Tensor deserialize(const std::vector<uint8_t>& data);

private:
    std::vector<int64_t> shape_;
    DataType dtype_;
    std::vector<float> data_;
};

/**
 * @brief ML Model base class
 */
class MAESTRO_API MLModel {
public:
    virtual ~MLModel() = default;

    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    
    virtual std::vector<std::pair<std::string, std::vector<int64_t>>>
        getInputShapes() const = 0;
    virtual std::vector<std::pair<std::string, std::vector<int64_t>>>
        getOutputShapes() const = 0;

    virtual std::vector<Tensor> forward(const std::vector<Tensor>& inputs) = 0;

    virtual Result<void> load(const std::string& path) = 0;
    virtual Result<void> loadFromMemory(const std::vector<uint8_t>& data) = 0;

    enum class Device { CPU, CUDA, Metal, DirectML };
    virtual void setDevice(Device device) = 0;
    virtual Device getDevice() const = 0;

    virtual void optimize() = 0;
    virtual void quantize(int bits = 8) = 0;
};

/**
 * @brief ML Engine - manages models and inference
 */
class MAESTRO_API MLEngine {
public:
    static MLEngine& instance();

    Result<void> initialize();
    void shutdown();

    struct DeviceInfo {
        MLModel::Device type;
        std::string name;
        size_t memoryMB;
        bool available;
    };
    std::vector<DeviceInfo> getAvailableDevices() const;
    void setDefaultDevice(MLModel::Device device);

    std::shared_ptr<MLModel> loadModel(const std::string& path);
    std::shared_ptr<MLModel> getModel(const std::string& name);
    void unloadModel(const std::string& name);

    std::future<std::vector<Tensor>> inferAsync(
        std::shared_ptr<MLModel> model,
        const std::vector<Tensor>& inputs);

    void setNumThreads(int threads);
    int getNumThreads() const;

private:
    MLEngine();
    ~MLEngine();
    
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::map<std::string, std::shared_ptr<MLModel>> models_;
    MLModel::Device defaultDevice_ = MLModel::Device::CPU;
    int numThreads_ = 4;
};

/**
 * @brief Chord generation model
 */
class MAESTRO_API ChordGenerator {
public:
    ChordGenerator();
    ~ChordGenerator();

    Result<void> initialize();
    
    struct ChordGenerationParams {
        std::string key = "C";
        std::string scale = "major";
        int numChords = 4;
        std::string style = "pop";
        float complexity = 0.5f;
        float tension = 0.3f;
        int minVoicing = 3;
        int maxVoicing = 7;
    };

    struct ChordResult {
        std::vector<MidiNote> notes;
        std::string name;
        float confidence;
        TickCount duration;
    };

    std::vector<ChordResult> generate(const ChordGenerationParams& params);
    std::vector<ChordResult> generateProgression(const ChordGenerationParams& params,
                                                  int numBars);

    void setModelPath(const std::string& path);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Melody generation model
 */
class MAESTRO_API MelodyGenerator {
public:
    MelodyGenerator();
    ~MelodyGenerator();

    Result<void> initialize();

    struct MelodyGenerationParams {
        std::vector<MidiNote> chordProgression;
        std::string key = "C";
        std::string scale = "major";
        int numBars = 4;
        int minNote = 48;
        int maxNote = 84;
        float rhythmComplexity = 0.5f;
        float melodicComplexity = 0.5f;
        float repetitionFactor = 0.3f;
        bool useChordTones = true;
        bool usePassingTones = true;
    };

    struct MelodyResult {
        std::vector<midi::NoteEvent> notes;
        float fitness;
        std::string analysis;
    };

    MelodyResult generate(const MelodyGenerationParams& params);
    MelodyResult vary(const MelodyResult& original, float variationAmount = 0.3f);

    void setModelPath(const std::string& path);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Drum pattern generator
 */
class MAESTRO_API DrumPatternGenerator {
public:
    DrumPatternGenerator();
    ~DrumPatternGenerator();

    Result<void> initialize();

    struct DrumPatternParams {
        std::string genre = "pop";
        int tempo = 120;
        int numBars = 4;
        TimeSignature timeSignature;
        float complexity = 0.5f;
        float groove = 0.5f;
        bool useGhostNotes = true;
        bool useFills = true;
        int fillFrequency = 4;  // bars
    };

    struct DrumPatternResult {
        std::vector<midi::MidiEvent> events;
        std::map<std::string, int> instrumentMap;
        float groove;
    };

    DrumPatternResult generate(const DrumPatternParams& params);
    DrumPatternResult addVariation(const DrumPatternResult& original, int variation);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace maestro::ml
