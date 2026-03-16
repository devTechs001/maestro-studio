// src/style/style_parser.cpp
#include "maestro/style/style_engine.hpp"
#include <fstream>
#include <cstring>

namespace maestro {

// Style Parser Implementation
class StyleParser {
public:
    static Result<StyleEngine::ParsedStyle> parseSFF1(const std::vector<uint8_t>& data) {
        StyleEngine::ParsedStyle style;
        
        if (data.size() < 14) {
            return Result<StyleEngine::ParsedStyle>("File too small for SFF1");
        }
        
        // Check header "FORM"
        if (std::memcmp(data.data(), "FORM", 4) != 0) {
            return Result<StyleEngine::ParsedStyle>("Invalid SFF1 header");
        }
        
        // Parse chunks
        size_t pos = 12;  // Skip FORM header and size
        while (pos + 8 < data.size()) {
            char chunkId[5] = {0};
            std::memcpy(chunkId, &data[pos], 4);
            uint32_t chunkSize = *reinterpret_cast<const uint32_t*>(&data[pos + 4]);
            
            if (pos + 8 + chunkSize > data.size()) {
                break;
            }
            
            const uint8_t* chunkData = &data[pos + 8];
            
            if (std::strcmp(chunkId, "MThd") == 0) {
                parseMThd(chunkData, chunkSize, style);
            } else if (std::strcmp(chunkId, "CASM") == 0) {
                parseCASM(chunkData, chunkSize, style);
            } else if (std::strcmp(chunkId, "OTSC") == 0) {
                parseOTSC(chunkData, chunkSize, style);
            }
            
            pos += 8 + chunkSize;
        }
        
        return Result<StyleEngine::ParsedStyle>(style);
    }
    
    static Result<StyleEngine::ParsedStyle> parseSFF2(const std::vector<uint8_t>& data) {
        StyleEngine::ParsedStyle style;
        
        if (data.size() < 16) {
            return Result<StyleEngine::ParsedStyle>("File too small for SFF2");
        }
        
        // Check header "SFF2"
        if (std::memcmp(data.data(), "SFF2", 4) != 0) {
            return Result<StyleEngine::ParsedStyle>("Invalid SFF2 header");
        }
        
        // Parse chunks
        size_t pos = 8;  // Skip SFF2 header and version
        while (pos + 8 < data.size()) {
            char chunkId[5] = {0};
            std::memcpy(chunkId, &data[pos], 4);
            uint32_t chunkSize = *reinterpret_cast<const uint32_t*>(&data[pos + 4]);
            
            if (pos + 8 + chunkSize > data.size()) {
                break;
            }
            
            const uint8_t* chunkData = &data[pos + 8];
            
            if (std::strcmp(chunkId, "MThd") == 0) {
                parseMThd(chunkData, chunkSize, style);
            } else if (std::strcmp(chunkId, "CASM") == 0) {
                parseCASM(chunkData, chunkSize, style);
            } else if (std::strcmp(chunkId, "CTAB") == 0) {
                parseCTAB(chunkData, chunkSize, style);
            } else if (std::strcmp(chunkId, "OTSC") == 0) {
                parseOTSC(chunkData, chunkSize, style);
            } else if (std::strcmp(chunkId, "NAME") == 0) {
                style.name = std::string(reinterpret_cast<const char*>(chunkData), chunkSize);
            } else if (std::strcmp(chunkId, "MDB") == 0) {
                parseMDB(chunkData, chunkSize, style);
            }
            
            pos += 8 + chunkSize;
        }
        
        return Result<StyleEngine::ParsedStyle>(style);
    }
    
    static Result<StyleEngine::ParsedStyle> parse(const std::vector<uint8_t>& data) {
        if (data.size() < 4) {
            return Result<StyleEngine::ParsedStyle>("File too small");
        }
        
        // Auto-detect format
        if (std::memcmp(data.data(), "SFF2", 4) == 0) {
            return parseSFF2(data);
        } else if (std::memcmp(data.data(), "FORM", 4) == 0) {
            return parseSFF1(data);
        }
        
        return Result<StyleEngine::ParsedStyle>("Unknown style format");
    }
    
    static Result<StyleEngine::ParsedStyle> parseFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return Result<StyleEngine::ParsedStyle>("Cannot open file: " + path);
        }
        
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
        file.close();
        
        return parse(data);
    }
    
private:
    static Result<void> parseMThd(const uint8_t* data, size_t size,
                                   StyleEngine::ParsedStyle& style) {
        if (size < 6) {
            return Result<void>("Invalid MThd chunk");
        }
        
        // MIDI file header
        // Format type (2 bytes), num tracks (2 bytes), division (2 bytes)
        style.timeSignature.numerator = 4;
        style.timeSignature.denominator = 4;
        style.tempo = 120;
        
        return Result<void>();
    }
    
    static Result<void> parseCASM(const uint8_t* data, size_t size,
                                   StyleEngine::ParsedStyle& style) {
        if (size < 8) {
            return Result<void>("Invalid CASM chunk");
        }
        
        // Channel Assignment data
        // Contains NTR (Note Transposition Range) and NTT (Note Transposition Table)
        uint8_t numChannels = data[0];
        
        for (uint8_t i = 0; i < numChannels && i < 16; ++i) {
            StyleEngine::ParsedStyle::ChannelAssignment assignment;
            assignment.sourceChannel = i;
            assignment.destChannel = data[1 + i];
            assignment.ntc = (data[17 + i] & 0x80) != 0;
            style.channelAssignments.push_back(assignment);
        }
        
        return Result<void>();
    }
    
    static Result<void> parseCTAB(const uint8_t* data, size_t size,
                                   StyleEngine::ParsedStyle& style) {
        if (size < 4) {
            return Result<void>("Invalid CTAB chunk");
        }
        
        // Chord Table data - maps source chords to target notes
        uint8_t numEntries = data[0];
        
        for (uint8_t i = 0; i < numEntries; ++i) {
            StyleEngine::ParsedStyle::ChordTable entry;
            entry.sourceChord = static_cast<ChordInfo::Type>(data[1 + i * 16]);
            
            for (int j = 0; j < 12; ++j) {
                entry.noteMapping.push_back(data[2 + i * 16 + j]);
            }
            
            style.chordTables.push_back(entry);
        }
        
        return Result<void>();
    }
    
    static Result<void> parseOTSC(const uint8_t* data, size_t size,
                                   StyleEngine::ParsedStyle& style) {
        if (size < 4) {
            return Result<void>("Invalid OTSC chunk");
        }
        
        // One Touch Setting data
        for (int i = 0; i < 4; ++i) {
            StyleEngine::OTSSetting ots;
            size_t offset = 1 + i * 32;
            
            if (offset + 32 <= size) {
                ots.enabled[i] = (data[offset] & 0x01) != 0;
                ots.volumes[i] = data[offset + 1];
                // Voice data would be parsed here
            }
            
            style.otsSettings[i] = ots;
        }
        
        return Result<void>();
    }
    
    static Result<void> parseMDB(const uint8_t* data, size_t size,
                                  StyleEngine::ParsedStyle& style) {
        // Music Database metadata
        if (size < 8) {
            return Result<void>();
        }
        
        style.mdbInfo.energy = data[0];
        style.mdbInfo.complexity = data[1];
        
        // Genre and keywords would be parsed from remaining data
        
        return Result<void>();
    }
};

// Exported functions
Result<StyleEngine::ParsedStyle> StyleEngine::parseStyle(const std::string& path) {
    return StyleParser::parseFile(path);
}

Result<StyleEngine::ParsedStyle> StyleEngine::parseStyleFromMemory(
    const std::vector<uint8_t>& data) {
    return StyleParser::parse(data);
}

} // namespace maestro
