// include/maestro/studio/automation.hpp
#pragma once

#include "maestro/core/config.hpp"
#include "maestro/core/types.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <set>

namespace maestro::studio {

/**
 * @brief Automation point with curve information
 */
struct AutomationPoint {
    TickCount tick;
    double value;

    enum class Curve {
        Linear,
        Smooth,
        Fast,
        Slow,
        Step,
        SCurve,
        Pulse
    };
    Curve curve = Curve::Linear;

    double controlX = 0.5;
    double controlY = 0.5;

    bool operator<(const AutomationPoint& other) const {
        return tick < other.tick;
    }
};

/**
 * @brief Automation lane for a single parameter
 */
class MAESTRO_API AutomationLane {
public:
    AutomationLane(const std::string& parameterId);

    std::string getParameterId() const { return parameterId_; }
    void setDisplayName(const std::string& name) { displayName_ = name; }
    std::string getDisplayName() const { return displayName_; }

    void setRange(double min, double max);
    double getMinValue() const { return minValue_; }
    double getMaxValue() const { return maxValue_; }

    void addPoint(const AutomationPoint& point);
    void removePoint(int index);
    void movePoint(int index, TickCount newTick, double newValue);
    void setPointCurve(int index, AutomationPoint::Curve curve);

    int getPointCount() const { return static_cast<int>(points_.size()); }
    AutomationPoint& getPoint(int index) { return points_[index]; }
    const AutomationPoint& getPoint(int index) const { return points_[index]; }

    double getValueAt(TickCount tick) const;
    double getValueAtNormalized(TickCount tick) const;

    void clear();
    void scaleValues(double factor);
    void offsetValues(double offset);
    void thin(double tolerance);

    void deleteRegion(TickCount start, TickCount end);
    void copyRegion(TickCount start, TickCount end,
                    AutomationLane& dest, TickCount destStart) const;

    void drawLine(TickCount start, TickCount end, double startValue,
                  double endValue, AutomationPoint::Curve curve);
    void drawFreehand(const std::vector<std::pair<TickCount, double>>& points);

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    void setReadEnabled(bool read) { readEnabled_ = read; }
    bool isReadEnabled() const { return readEnabled_; }
    void setWriteEnabled(bool write) { writeEnabled_ = write; }
    bool isWriteEnabled() const { return writeEnabled_; }

private:
    std::string parameterId_;
    std::string displayName_;
    std::vector<AutomationPoint> points_;
    double minValue_ = 0.0;
    double maxValue_ = 1.0;
    bool enabled_ = true;
    bool readEnabled_ = true;
    bool writeEnabled_ = false;

    double interpolate(const AutomationPoint& a, const AutomationPoint& b,
                       TickCount tick) const;
};

/**
 * @brief Automation for a track or plugin
 */
class MAESTRO_API AutomationContainer {
public:
    AutomationContainer();
    ~AutomationContainer();

    AutomationLane& addLane(const std::string& parameterId);
    void removeLane(const std::string& parameterId);
    AutomationLane* getLane(const std::string& parameterId);
    const AutomationLane* getLane(const std::string& parameterId) const;
    std::vector<std::string> getLaneIds() const;

    void clearAll();
    void enableAll(bool read, bool write);

    void captureSnapshot(TickCount tick);
    void applySnapshot(TickCount tick);

    enum class Mode { Off, Read, Touch, Latch, Write };
    void setMode(Mode mode) { mode_ = mode; }
    Mode getMode() const { return mode_; }

    using ParameterCallback = std::function<void(const std::string&, double)>;
    void setParameterCallback(ParameterCallback callback);

    void process(TickCount tick);

private:
    std::map<std::string, std::unique_ptr<AutomationLane>> lanes_;
    Mode mode_ = Mode::Read;
    ParameterCallback callback_;
};

/**
 * @brief Automation recording
 */
class MAESTRO_API AutomationRecorder {
public:
    AutomationRecorder(AutomationContainer& container);

    void startRecording();
    void stopRecording();
    bool isRecording() const { return recording_; }

    void touchParameter(const std::string& parameterId);
    void releaseParameter(const std::string& parameterId);

    void writeValue(const std::string& parameterId, double value,
                    TickCount tick);

    void setThinningEnabled(bool enabled) { thinning_ = enabled; }
    void setThinningTolerance(double tolerance) { tolerance_ = tolerance; }

    void thinRecordedData();
    void smoothRecordedData(int windowSize);

private:
    AutomationContainer& container_;
    bool recording_ = false;
    bool thinning_ = true;
    double tolerance_ = 0.01;

    std::map<std::string, std::vector<std::pair<TickCount, double>>> recordBuffer_;
    std::set<std::string> touchedParams_;
};

/**
 * @brief LFO-based automation
 */
class MAESTRO_API AutomationLFO {
public:
    AutomationLFO();

    enum class Waveform { Sine, Triangle, Square, Sawtooth, Random, Custom };
    void setWaveform(Waveform waveform);
    void setCustomWaveform(const std::vector<float>& samples);

    void setRate(float hz);
    void setRateSync(float noteValue);
    void setSync(bool sync);
    void setDepth(float depth);
    void setPhase(float phase);
    void setOffset(float offset);

    void setTempo(double bpm);

    float process();
    void reset();

    void generateToLane(AutomationLane& lane, TickCount start,
                        TickCount length, int ppqn);

private:
    Waveform waveform_ = Waveform::Sine;
    float rate_ = 1.0f;
    float depth_ = 1.0f;
    float phase_ = 0.0f;
    float offset_ = 0.0f;
    bool sync_ = false;
    double tempo_ = 120.0;

    float currentPhase_ = 0.0f;
    std::vector<float> customWaveform_;
};

} // namespace maestro::studio
