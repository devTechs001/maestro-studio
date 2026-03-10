// src/studio/automation.cpp
#include "maestro/studio/automation.hpp"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace maestro::studio {

// AutomationLane implementation
AutomationLane::AutomationLane(const std::string& parameterId)
    : parameterId_(parameterId) {
}

void AutomationLane::setRange(double min, double max) {
    minValue_ = min;
    maxValue_ = max;
}

void AutomationLane::addPoint(const AutomationPoint& point) {
    points_.push_back(point);
    std::sort(points_.begin(), points_.end());
}

void AutomationLane::removePoint(int index) {
    if (index >= 0 && index < static_cast<int>(points_.size())) {
        points_.erase(points_.begin() + index);
    }
}

void AutomationLane::movePoint(int index, TickCount newTick, double newValue) {
    if (index >= 0 && index < static_cast<int>(points_.size())) {
        points_[index].tick = newTick;
        points_[index].value = newValue;
        std::sort(points_.begin(), points_.end());
    }
}

void AutomationLane::setPointCurve(int index, AutomationPoint::Curve curve) {
    if (index >= 0 && index < static_cast<int>(points_.size())) {
        points_[index].curve = curve;
    }
}

double AutomationLane::getValueAt(TickCount tick) const {
    if (points_.empty()) return minValue_;
    
    // Before first point
    if (tick <= points_.front().tick) {
        return points_.front().value * (maxValue_ - minValue_) + minValue_;
    }
    
    // After last point
    if (tick >= points_.back().tick) {
        return points_.back().value * (maxValue_ - minValue_) + minValue_;
    }
    
    // Find surrounding points
    auto it = std::lower_bound(points_.begin(), points_.end(),
        AutomationPoint{tick, 0.0},
        [](const AutomationPoint& a, const AutomationPoint& b) {
            return a.tick < b.tick;
        });
    
    if (it == points_.begin()) return points_.front().value;
    
    const auto& prev = *(it - 1);
    const auto& curr = *it;
    
    return interpolate(prev, curr, tick) * (maxValue_ - minValue_) + minValue_;
}

double AutomationLane::getValueAtNormalized(TickCount tick) const {
    if (points_.empty()) return 0.5;
    
    if (tick <= points_.front().tick) return points_.front().value;
    if (tick >= points_.back().tick) return points_.back().value;
    
    auto it = std::lower_bound(points_.begin(), points_.end(),
        AutomationPoint{tick, 0.0},
        [](const AutomationPoint& a, const AutomationPoint& b) {
            return a.tick < b.tick;
        });
    
    if (it == points_.begin()) return points_.front().value;
    
    const auto& prev = *(it - 1);
    const auto& curr = *it;
    
    return interpolate(prev, curr, tick);
}

double AutomationLane::interpolate(const AutomationPoint& a,
                                    const AutomationPoint& b,
                                    TickCount tick) const {
    if (b.tick == a.tick) return a.value;
    
    double t = static_cast<double>(tick - a.tick) / (b.tick - a.tick);
    
    switch (a.curve) {
        case AutomationPoint::Curve::Linear:
            return a.value + t * (b.value - a.value);
            
        case AutomationPoint::Curve::Step:
            return a.value;
            
        case AutomationPoint::Curve::Smooth:
        case AutomationPoint::Curve::SCurve:
            // Smoothstep interpolation
            t = t * t * (3.0 - 2.0 * t);
            return a.value + t * (b.value - a.value);
            
        case AutomationPoint::Curve::Fast:
            // Exponential attack
            t = 1.0 - std::exp(-5.0 * t);
            return a.value + t * (b.value - a.value);
            
        case AutomationPoint::Curve::Slow:
            // Logarithmic attack
            t = std::log(1.0 + 9.0 * t) / std::log(10.0);
            return a.value + t * (b.value - a.value);
            
        case AutomationPoint::Curve::Pulse:
            return (t < 0.1) ? b.value : a.value;
    }
    
    return a.value + t * (b.value - a.value);
}

void AutomationLane::clear() {
    points_.clear();
}

void AutomationLane::scaleValues(double factor) {
    for (auto& point : points_) {
        point.value *= factor;
    }
}

void AutomationLane::offsetValues(double offset) {
    for (auto& point : points_) {
        point.value += offset;
    }
}

void AutomationLane::thin(double tolerance) {
    if (points_.size() < 3) return;
    
    std::vector<AutomationPoint> thinned;
    thinned.push_back(points_[0]);
    
    for (size_t i = 1; i < points_.size() - 1; ++i) {
        double expected = interpolate(points_[i-1], points_[i+1], points_[i].tick);
        if (std::abs(points_[i].value - expected) > tolerance) {
            thinned.push_back(points_[i]);
        }
    }
    
    thinned.push_back(points_.back());
    points_ = std::move(thinned);
}

void AutomationLane::deleteRegion(TickCount start, TickCount end) {
    points_.erase(
        std::remove_if(points_.begin(), points_.end(),
            [start, end](const AutomationPoint& p) {
                return p.tick >= start && p.tick <= end;
            }),
        points_.end()
    );
}

void AutomationLane::copyRegion(TickCount start, TickCount end,
                                 AutomationLane& dest, TickCount destStart) const {
    for (const auto& point : points_) {
        if (point.tick >= start && point.tick <= end) {
            AutomationPoint newPoint = point;
            newPoint.tick = destStart + (point.tick - start);
            dest.addPoint(newPoint);
        }
    }
}

void AutomationLane::drawLine(TickCount start, TickCount end,
                               double startValue, double endValue,
                               AutomationPoint::Curve curve) {
    AutomationPoint p1{start, startValue};
    p1.curve = curve;
    AutomationPoint p2{end, endValue};
    addPoint(p1);
    addPoint(p2);
}

void AutomationLane::drawFreehand(
        const std::vector<std::pair<TickCount, double>>& points) {
    for (const auto& p : points) {
        AutomationPoint point{p.first, p.second};
        point.curve = AutomationPoint::Curve::Linear;
        addPoint(point);
    }
}

// AutomationContainer implementation
AutomationContainer::AutomationContainer() = default;
AutomationContainer::~AutomationContainer() = default;

AutomationLane& AutomationContainer::addLane(const std::string& parameterId) {
    if (lanes_.find(parameterId) == lanes_.end()) {
        lanes_[parameterId] = std::make_unique<AutomationLane>(parameterId);
    }
    return *lanes_[parameterId];
}

void AutomationContainer::removeLane(const std::string& parameterId) {
    lanes_.erase(parameterId);
}

AutomationLane* AutomationContainer::getLane(const std::string& parameterId) {
    auto it = lanes_.find(parameterId);
    return it != lanes_.end() ? it->second.get() : nullptr;
}

const AutomationLane* AutomationContainer::getLane(const std::string& parameterId) const {
    auto it = lanes_.find(parameterId);
    return it != lanes_.end() ? it->second.get() : nullptr;
}

std::vector<std::string> AutomationContainer::getLaneIds() const {
    std::vector<std::string> ids;
    for (const auto& lane : lanes_) {
        ids.push_back(lane.first);
    }
    return ids;
}

void AutomationContainer::clearAll() {
    lanes_.clear();
}

void AutomationContainer::enableAll(bool read, bool write) {
    for (auto& lane : lanes_) {
        lane.second->setReadEnabled(read);
        lane.second->setWriteEnabled(write);
    }
}

void AutomationContainer::captureSnapshot(TickCount tick) {
    // Capture current values
}

void AutomationContainer::applySnapshot(TickCount tick) {
    // Apply captured values
}

void AutomationContainer::process(TickCount tick) {
    if (mode_ == Mode::Off) return;
    
    for (auto& lane : lanes_) {
        if (lane.second->isReadEnabled()) {
            double value = lane.second->getValueAt(tick);
            if (callback_) {
                callback_(lane.first, value);
            }
        }
    }
}

void AutomationContainer::setParameterCallback(ParameterCallback callback) {
    callback_ = std::move(callback);
}

// AutomationRecorder implementation
AutomationRecorder::AutomationRecorder(AutomationContainer& container)
    : container_(container) {
}

void AutomationRecorder::startRecording() {
    recording_ = true;
    recordBuffer_.clear();
}

void AutomationRecorder::stopRecording() {
    recording_ = false;
    
    // Write buffered data to lanes
    for (auto& [paramId, values] : recordBuffer_) {
        auto* lane = container_.getLane(paramId);
        if (lane) {
            for (const auto& [tick, value] : values) {
                AutomationPoint point{tick, value};
                lane->addPoint(point);
            }
        }
    }
    
    if (thinning_) {
        thinRecordedData();
    }
}

void AutomationRecorder::touchParameter(const std::string& parameterId) {
    touchedParams_.insert(parameterId);
}

void AutomationRecorder::releaseParameter(const std::string& parameterId) {
    touchedParams_.erase(parameterId);
}

void AutomationRecorder::writeValue(const std::string& parameterId,
                                     double value, TickCount tick) {
    if (!recording_) return;
    recordBuffer_[parameterId].emplace_back(tick, value);
}

void AutomationRecorder::thinRecordedData() {
    // Remove redundant points
}

void AutomationRecorder::smoothRecordedData(int windowSize) {
    // Apply smoothing
}

// AutomationLFO implementation
AutomationLFO::AutomationLFO() = default;

void AutomationLFO::setWaveform(Waveform waveform) {
    waveform_ = waveform;
}

void AutomationLFO::setCustomWaveform(const std::vector<float>& samples) {
    customWaveform_ = samples;
}

void AutomationLFO::setRate(float hz) {
    rate_ = hz;
}

void AutomationLFO::setRateSync(float noteValue) {
    // Calculate rate based on tempo and note value
    rate_ = static_cast<float>(tempo_ / 60.0 / noteValue);
}

void AutomationLFO::setSync(bool sync) {
    sync_ = sync;
}

void AutomationLFO::setDepth(float depth) {
    depth_ = std::clamp(depth, 0.0f, 1.0f);
}

void AutomationLFO::setPhase(float phase) {
    phase_ = phase;
}

void AutomationLFO::setOffset(float offset) {
    offset_ = std::clamp(offset, -1.0f, 1.0f);
}

void AutomationLFO::setTempo(double bpm) {
    tempo_ = bpm;
}

float AutomationLFO::process() {
    float output = 0.0f;
    
    switch (waveform_) {
        case Waveform::Sine:
            output = std::sin(2.0f * M_PI * currentPhase_);
            break;
        case Waveform::Triangle:
            output = 2.0f * std::abs(2.0f * currentPhase_ - 1.0f) - 1.0f;
            break;
        case Waveform::Square:
            output = (currentPhase_ < 0.5f) ? 1.0f : -1.0f;
            break;
        case Waveform::Sawtooth:
            output = 2.0f * currentPhase_ - 1.0f;
            break;
        case Waveform::Random:
            output = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            break;
        case Waveform::Custom:
            if (!customWaveform_.empty()) {
                int index = static_cast<int>(currentPhase_ * customWaveform_.size());
                output = customWaveform_[index % customWaveform_.size()];
            }
            break;
    }
    
    currentPhase_ += rate_ / 44100.0f;  // Assume 44.1kHz
    if (currentPhase_ >= 1.0f) currentPhase_ -= 1.0f;
    
    return output * depth_ + offset_;
}

void AutomationLFO::reset() {
    currentPhase_ = phase_ / 360.0f;
}

void AutomationLFO::generateToLane(AutomationLane& lane, TickCount start,
                                    TickCount length, int ppqn) {
    reset();
    
    float samplesPerTick = 44100.0f / (tempo_ * ppqn / 60.0f);
    
    for (TickCount tick = start; tick < start + length; ++tick) {
        float value = process();
        AutomationPoint point{tick, (value + 1.0f) / 2.0f};  // Normalize to 0-1
        lane.addPoint(point);
    }
}

} // namespace maestro::studio
