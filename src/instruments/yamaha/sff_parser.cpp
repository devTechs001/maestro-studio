// src/instruments/yamaha/sff_parser.cpp
#include "maestro/instruments/yamaha/yamaha_instrument.hpp"
#include <fstream>
#include <cstring>

namespace maestro::instruments::yamaha {

namespace {

uint16_t readLE16(const uint8_t* data) {
    return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
}

uint32_t readLE32(const uint8_t* data) {
    return static_cast<uint32_t>(data[0]) |
           (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) |
           (static_cast<uint32_t>(data[3]) << 24);
}

} // anonymous namespace

YamahaSFFParser::Format YamahaSFFParser::detectFormat(const std::vector<uint8_t>& data) {
    if (data.size() < 16) return Format::SFF1;
    
    // Check for SFF2 signature
    if (std::memcmp(data.data(), "SFF2", 4) == 0) {
        return Format::SFF2;
    }
    
    // Check for SFF GE signature
    if (std::memcmp(data.data(), "SFFGE", 5) == 0) {
        return Format::SFFGE;
    }
    
    return Format::SFF1;
}

Result<Style> YamahaSFFParser::parse(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return Result<Style>("Cannot open file: " + path);
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    return parseFromMemory(data);
}

Result<Style> YamahaSFFParser::parseFromMemory(const std::vector<uint8_t>& data) {
    Format format = detectFormat(data);
    
    switch (format) {
        case Format::SFF1:
            return parseSFF1(data);
        case Format::SFF2:
            return parseSFF2(data);
        case Format::SFFGE:
            // SFFGE uses same parser as SFF2 with extensions
            return parseSFF2(data);
    }
    
    return Result<Style>("Unknown style format");
}

Result<Style> YamahaSFFParser::parseSFF1(const std::vector<uint8_t>& data) {
    Style style;
    style.name = "SFF1 Style";
    style.tempo = 120;
    style.timeSignature.numerator = 4;
    style.timeSignature.denominator = 4;
    
    // Parse SFF1 header
    if (data.size() < 512) {
        return Result<Style>("Invalid SFF1 file");
    }
    
    // Read style name from header
    size_t nameOffset = 0x1C;
    for (size_t i = 0; i < 20 && nameOffset + i < data.size(); ++i) {
        if (data[nameOffset + i] == 0) break;
        style.name += static_cast<char>(data[nameOffset + i]);
    }
    
    // Read tempo
    if (data.size() > 0x28) {
        style.tempo = readLE16(&data[0x28]);
    }
    
    // Create default parts
    style.parts.resize(16);
    style.parts[0].type = StylePart::MainA;
    style.parts[1].type = StylePart::MainB;
    style.parts[2].type = StylePart::MainC;
    style.parts[3].type = StylePart::MainD;
    
    // Create default tracks
    for (int i = 0; i < 16; ++i) {
        Style::Track track;
        track.name = "Track " + std::to_string(i + 1);
        track.channel = static_cast<MidiChannel>(i);
        track.volume = 100;
        track.pan = 64;
        style.tracks.push_back(track);
    }
    
    return Result<Style>(std::move(style));
}

Result<Style> YamahaSFFParser::parseSFF2(const std::vector<uint8_t>& data) {
    Style style;
    style.name = "SFF2 Style";
    style.tempo = 120;
    style.timeSignature.numerator = 4;
    style.timeSignature.denominator = 4;
    
    if (data.size() < 1024) {
        return Result<Style>("Invalid SFF2 file");
    }
    
    // SFF2 header starts after "SFF2" signature
    size_t headerOffset = 4;
    
    // Read style name
    size_t nameOffset = headerOffset + 0x18;
    for (size_t i = 0; i < 40 && nameOffset + i < data.size(); ++i) {
        if (data[nameOffset + i] == 0) break;
        style.name += static_cast<char>(data[nameOffset + i]);
    }
    
    // Read tempo
    if (data.size() > headerOffset + 0x48) {
        style.tempo = readLE16(&data[headerOffset + 0x48]);
    }
    
    // Read time signature
    if (data.size() > headerOffset + 0x4A) {
        style.timeSignature.numerator = data[headerOffset + 0x4A];
        style.timeSignature.denominator = 4;
    }
    
    // Create parts for all variations
    style.parts = {
        {StylePart::Intro1}, {StylePart::Intro2}, {StylePart::Intro3},
        {StylePart::MainA}, {StylePart::MainB}, {StylePart::MainC}, {StylePart::MainD},
        {StylePart::FillA}, {StylePart::FillB}, {StylePart::FillC}, {StylePart::FillD},
        {StylePart::BreakA}, {StylePart::BreakB},
        {StylePart::Ending1}, {StylePart::Ending2}, {StylePart::Ending3}
    };
    
    // Create default tracks
    for (int i = 0; i < 16; ++i) {
        Style::Track track;
        track.name = "Track " + std::to_string(i + 1);
        track.channel = static_cast<MidiChannel>(i);
        track.volume = 100;
        track.pan = 64;
        track.reverb = 40;
        style.tracks.push_back(track);
    }
    
    style.otsEnabled = true;
    
    return Result<Style>(std::move(style));
}

Result<void> YamahaSFFParser::save(const Style& style, const std::string& path, Format format) {
    std::vector<uint8_t> data;
    
    // Build style file based on format
    switch (format) {
        case Format::SFF1:
            // SFF1 saving implementation
            break;
        case Format::SFF2:
            // SFF2 saving implementation
            break;
        case Format::SFFGE:
            // SFFGE saving implementation
            break;
    }
    
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return Result<void>("Cannot create file: " + path);
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return Result<void>();
}

} // namespace maestro::instruments::yamaha
