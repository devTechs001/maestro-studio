// src/midi/midi_file.cpp
#include "maestro/midi/midi_engine.hpp"
#include <fstream>
#include <cstring>

namespace maestro {

// Helper functions for MIDI file reading/writing
namespace {

uint16_t readBE16(const uint8_t* data) {
    return (static_cast<uint16_t>(data[0]) << 8) | data[1];
}

uint32_t readBE32(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 24) |
           (static_cast<uint32_t>(data[1]) << 16) |
           (static_cast<uint32_t>(data[2]) << 8) | data[3];
}

void writeBE16(uint8_t* data, uint16_t value) {
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;
}

void writeBE32(uint8_t* data, uint32_t value) {
    data[0] = (value >> 24) & 0xFF;
    data[1] = (value >> 16) & 0xFF;
    data[2] = (value >> 8) & 0xFF;
    data[3] = value & 0xFF;
}

uint32_t readVarLen(const std::vector<uint8_t>& data, size_t& offset) {
    uint32_t value = 0;
    while (offset < data.size()) {
        uint8_t byte = data[offset++];
        value = (value << 7) | (byte & 0x7F);
        if (!(byte & 0x80)) break;
    }
    return value;
}

void writeVarLen(std::vector<uint8_t>& data, uint32_t value) {
    uint8_t buffer[4];
    int len = 0;
    
    buffer[len++] = value & 0x7F;
    while ((value >>= 7) > 0) {
        buffer[len++] = (value & 0x7F) | 0x80;
    }
    
    for (int i = len - 1; i >= 0; --i) {
        data.push_back(buffer[i]);
    }
}

} // anonymous namespace

TickCount MidiFile::duration() const {
    TickCount maxTick = 0;
    for (const auto& track : tracks_) {
        for (const auto& event : track.events) {
            maxTick = std::max(maxTick, event.tick);
        }
    }
    return maxTick;
}

Result<MidiFile> MidiFile::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return Result<MidiFile>("Cannot open file: " + path);
    }
    
    // Read entire file
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    return loadFromMemory(data);
}

Result<MidiFile> MidiFile::loadFromMemory(const std::vector<uint8_t>& data) {
    if (data.size() < 14) {
        return Result<MidiFile>("File too small to be a valid MIDI file");
    }
    
    // Check header
    if (std::memcmp(data.data(), "MThd", 4) != 0) {
        return Result<MidiFile>("Invalid MIDI header");
    }
    
    uint32_t headerSize = readBE32(&data[4]);
    if (headerSize < 6) {
        return Result<MidiFile>("Invalid header size");
    }
    
    MidiFile midiFile;
    midiFile.format_ = readBE16(&data[8]);
    midiFile.tracks_.resize(readBE16(&data[10]));
    midiFile.ppqn_ = readBE16(&data[12]);
    
    // Parse tracks
    size_t offset = 8 + headerSize;
    for (size_t trackIdx = 0; trackIdx < midiFile.tracks_.size() && offset < data.size(); ++trackIdx) {
        if (offset + 8 > data.size()) break;
        
        if (std::memcmp(&data[offset], "MTrk", 4) != 0) {
            return Result<MidiFile>("Invalid track header");
        }
        
        uint32_t trackSize = readBE32(&data[offset + 4]);
        offset += 8;
        
        if (offset + trackSize > data.size()) break;
        
        auto& track = midiFile.tracks_[trackIdx];
        TickCount currentTick = 0;
        uint8_t runningStatus = 0;
        size_t trackEnd = offset + trackSize;
        
        while (offset < trackEnd) {
            // Read delta time
            uint32_t deltaTime = readVarLen(data, offset);
            currentTick += deltaTime;
            
            if (offset >= trackEnd) break;
            
            uint8_t status = data[offset];
            
            // Handle running status
            if (status < 0x80) {
                if (runningStatus == 0) continue;
                status = runningStatus;
            } else {
                runningStatus = status;
                offset++;
            }
            
            midi::MidiMessage msg;
            
            if (status == 0xFF) {
                // Meta event
                if (offset >= trackEnd) break;
                uint8_t type = data[offset++];
                uint32_t length = readVarLen(data, offset);
                
                if (type == 0x51 && length >= 3) {
                    // Tempo
                    uint32_t tempo = (data[offset] << 16) | (data[offset + 1] << 8) | data[offset + 2];
                    midiFile.initialTempo_ = 60000000.0 / tempo;
                }
                
                offset += length;
                continue;
            } else if (status == 0xF0 || status == 0xF7) {
                // SysEx
                uint32_t length = readVarLen(data, offset);
                offset += length;
                continue;
            } else {
                // Regular MIDI message
                msg.data[0] = status;
                
                int msgLen = 0;
                switch (status & 0xF0) {
                    case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0:
                        msgLen = 3;
                        break;
                    case 0xC0: case 0xD0:
                        msgLen = 2;
                        break;
                    default:
                        continue;
                }
                
                for (int i = 1; i < msgLen && offset < trackEnd; ++i) {
                    msg.data[i] = data[offset++];
                }
                msg.length = msgLen;
            }
            
            midi::MidiEvent event;
            event.message = msg;
            event.tick = currentTick;
            event.track = static_cast<int>(trackIdx);
            track.events.push_back(event);
        }
        
        offset = trackEnd;
    }
    
    return Result<MidiFile>(std::move(midiFile));
}

Result<void> MidiFile::save(const std::string& path) const {
    auto data = toMemory();
    
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return Result<void>("Cannot create file: " + path);
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return Result<void>();
}

std::vector<uint8_t> MidiFile::toMemory() const {
    std::vector<uint8_t> data;
    
    // Write header chunk
    data.insert(data.end(), {'M', 'T', 'h', 'd'});
    
    uint8_t headerData[6];
    writeBE16(headerData, static_cast<uint16_t>(format_));
    writeBE16(headerData + 2, static_cast<uint16_t>(tracks_.size()));
    writeBE16(headerData + 4, static_cast<uint16_t>(ppqn_));
    
    writeBE32(&data.emplace_back(0), 6);
    data.pop_back();
    data.insert(data.end(), headerData, headerData + 6);
    
    // Write track chunks
    for (const auto& track : tracks_) {
        std::vector<uint8_t> trackData;
        
        // Sort events by tick
        auto sortedEvents = track.events;
        std::sort(sortedEvents.begin(), sortedEvents.end());
        
        TickCount lastTick = 0;
        
        for (const auto& event : sortedEvents) {
            // Write delta time
            writeVarLen(trackData, static_cast<uint32_t>(event.tick - lastTick));
            lastTick = event.tick;
            
            // Write message
            const auto& msg = event.message;
            for (int i = 0; i < msg.length; ++i) {
                trackData.push_back(msg.data[i]);
            }
        }
        
        // Write track chunk
        data.insert(data.end(), {'M', 'T', 'r', 'k'});
        
        uint8_t sizeData[4];
        writeBE32(sizeData, static_cast<uint32_t>(trackData.size()));
        data.insert(data.end(), sizeData, sizeData + 4);
        data.insert(data.end(), trackData.begin(), trackData.end());
    }
    
    return data;
}

void MidiFile::addTrack(const Track& track) {
    tracks_.push_back(track);
}

void MidiFile::removeTrack(size_t index) {
    if (index < tracks_.size()) {
        tracks_.erase(tracks_.begin() + index);
    }
}

void MidiFile::mergeTrack(size_t sourceIndex, size_t destIndex) {
    if (sourceIndex < tracks_.size() && destIndex < tracks_.size() && sourceIndex != destIndex) {
        auto& dest = tracks_[destIndex];
        const auto& src = tracks_[sourceIndex];
        dest.events.insert(dest.events.end(), src.events.begin(), src.events.end());
        std::sort(dest.events.begin(), dest.events.end());
        tracks_.erase(tracks_.begin() + sourceIndex);
    }
}

void MidiFile::transpose(int semitones) {
    for (auto& track : tracks_) {
        for (auto& event : track.events) {
            if (event.message.isNote()) {
                int note = static_cast<int>(event.message.note()) + semitones;
                event.message.data[1] = std::clamp(note, 0, 127);
            }
        }
    }
}

void MidiFile::quantize(int gridTicks) {
    for (auto& track : tracks_) {
        for (auto& event : track.events) {
            event.tick = ((event.tick + gridTicks / 2) / gridTicks) * gridTicks;
        }
        std::sort(track.events.begin(), track.events.end());
    }
}

void MidiFile::setTempo(double bpm) {
    initialTempo_ = bpm;
}

} // namespace maestro
