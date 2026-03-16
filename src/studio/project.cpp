// src/studio/project.cpp
#include "maestro/studio/project.hpp"
#include <fstream>
#include <cstring>

namespace maestro::studio {

// Track implementation
Track::Track(const std::string& name, Type type)
    : name_(name), type_(type) {
}

void Track::addClip(const Clip& clip) {
    clips_.push_back(clip);
}

void Track::removeClip(size_t index) {
    if (index < clips_.size()) {
        clips_.erase(clips_.begin() + index);
    }
}

Clip& Track::clip(size_t index) {
    return clips_[index];
}

size_t Track::clipCount() const {
    return clips_.size();
}

void Track::setOutput(const std::string& busName) {
    output_ = busName;
}

void Track::setMidiOutput(const std::string& portId, MidiChannel channel) {
    midiPort_ = portId;
    midiChannel_ = channel;
}

void Track::addEffect(std::shared_ptr<AudioEngine::AudioProcessor> effect) {
    effects_.push_back(effect);
}

void Track::removeEffect(size_t index) {
    if (index < effects_.size()) {
        effects_.erase(effects_.begin() + index);
    }
}

void Track::addAutomationLane(const std::string& parameter) {
    // Check if lane already exists
    for (auto& lane : automationLanes_) {
        if (lane.parameter == parameter) return;
    }
    automationLanes_.push_back({parameter, {}});
}

Track::AutomationLane& Track::automationLane(const std::string& parameter) {
    for (auto& lane : automationLanes_) {
        if (lane.parameter == parameter) return lane;
    }
    // Create new lane if not found
    addAutomationLane(parameter);
    return automationLanes_.back();
}

// Project implementation
Project::Project() {
    timeSig_.numerator = 4;
    timeSig_.denominator = 4;
}

Result<Project> Project::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return Result<Project>("Cannot open file: " + path);
    }

    // Read file header
    char header[16];
    file.read(header, 16);
    if (std::memcmp(header, "MSPROJ", 6) != 0) {
        return Result<Project>("Invalid project file format");
    }

    Project project;
    project.path_ = path;

    // Read project data (simplified implementation)
    // In a full implementation, this would parse the complete project structure

    return Result<Project>(std::move(project));
}

Result<void> Project::save(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return Result<void>("Cannot create file: " + path);
    }

    // Write file header
    file.write("MSPROJ", 6);
    uint8_t version = 1;
    file.write(reinterpret_cast<char*>(&version), 1);
    uint8_t flags = 0;
    file.write(reinterpret_cast<char*>(&flags), 1);

    // Write project name
    uint16_t nameLen = static_cast<uint16_t>(name_.length());
    file.write(reinterpret_cast<char*>(&nameLen), 2);
    file.write(name_.c_str(), nameLen);

    // Write tempo
    file.write(reinterpret_cast<char*>(&tempo_), sizeof(tempo_));

    // Write time signature
    file.write(reinterpret_cast<char*>(&timeSig_), sizeof(timeSig_));

    // Write sample rate
    file.write(reinterpret_cast<char*>(&sampleRate_), sizeof(sampleRate_));

    // Write track count
    uint32_t trackCount = static_cast<uint32_t>(tracks_.size());
    file.write(reinterpret_cast<char*>(&trackCount), 4);

    // Write tracks
    for (const auto& track : tracks_) {
        // Write track name
        uint16_t trackNameLen = static_cast<uint16_t>(track->name().length());
        file.write(reinterpret_cast<char*>(&trackNameLen), 2);
        file.write(track->name().c_str(), trackNameLen);

        // Write track type
        uint8_t trackType = static_cast<uint8_t>(track->type());
        file.write(reinterpret_cast<char*>(&trackType), 1);

        // Write track settings
        float volume = track->volume();
        float pan = track->pan();
        file.write(reinterpret_cast<const char*>(&volume), sizeof(float));
        file.write(reinterpret_cast<const char*>(&pan), sizeof(float));

        uint8_t trackFlags = 0;
        if (track->isMuted()) trackFlags |= 0x01;
        if (track->isSoloed()) trackFlags |= 0x02;
        if (track->isArmed()) trackFlags |= 0x04;
        file.write(reinterpret_cast<char*>(&trackFlags), 1);

        // Write clip count
        uint32_t clipCount = static_cast<uint32_t>(track->clipCount());
        file.write(reinterpret_cast<char*>(&clipCount), 4);

        // Write clips
        for (size_t i = 0; i < track->clipCount(); ++i) {
            const auto& clip = track->clip(i);
            
            // Write clip name
            uint16_t clipNameLen = static_cast<uint16_t>(clip.name.length());
            file.write(reinterpret_cast<char*>(&clipNameLen), 2);
            file.write(clip.name.c_str(), clipNameLen);

            // Write clip type
            uint8_t clipType = static_cast<uint8_t>(clip.type);
            file.write(reinterpret_cast<char*>(&clipType), 1);

            // Write clip timing
            file.write(reinterpret_cast<const char*>(&clip.startTick), sizeof(TickCount));
            file.write(reinterpret_cast<const char*>(&clip.length), sizeof(TickCount));
            file.write(reinterpret_cast<const char*>(&clip.offset), sizeof(TickCount));
            file.write(reinterpret_cast<const char*>(&clip.gain), sizeof(double));

            uint8_t clipFlags = 0;
            if (clip.muted) clipFlags |= 0x01;
            file.write(reinterpret_cast<char*>(&clipFlags), 1);

            // Write clip color
            file.write(reinterpret_cast<const char*>(&clip.color), sizeof(int));
        }
    }

    // Write markers
    uint32_t markerCount = static_cast<uint32_t>(markers_.size());
    file.write(reinterpret_cast<char*>(&markerCount), 4);
    for (const auto& marker : markers_) {
        uint16_t markerNameLen = static_cast<uint16_t>(marker.name.length());
        file.write(reinterpret_cast<char*>(&markerNameLen), 2);
        file.write(marker.name.c_str(), markerNameLen);
        file.write(reinterpret_cast<const char*>(&marker.tick), sizeof(TickCount));
        file.write(reinterpret_cast<const char*>(&marker.color), sizeof(int));
    }

    path_ = path;
    return Result<void>();
}

Track& Project::addTrack(const std::string& name, Track::Type type) {
    tracks_.push_back(std::make_unique<Track>(name, type));
    return *tracks_.back();
}

void Project::removeTrack(size_t index) {
    if (index < tracks_.size()) {
        tracks_.erase(tracks_.begin() + index);
    }
}

Track& Project::track(size_t index) {
    return *tracks_[index];
}

const Track& Project::track(size_t index) const {
    return *tracks_[index];
}

size_t Project::trackCount() const {
    return tracks_.size();
}

void Project::addMarker(const Marker& marker) {
    markers_.push_back(marker);
}

void Project::setLoopRegion(TickCount start, TickCount end) {
    loopStart_ = start;
    loopEnd_ = end;
}

std::pair<TickCount, TickCount> Project::loopRegion() const {
    return {loopStart_, loopEnd_};
}

} // namespace maestro::studio
