// src/instruments/instrument_factory.cpp
#include "maestro/instruments/instrument.hpp"

namespace maestro::instruments {

std::unique_ptr<Instrument> InstrumentFactory::create(const std::string& manufacturer,
                                                       const std::string& model) {
    // Factory implementation for creating instrument instances
    // In a full implementation, this would create specific instrument classes
    
    if (manufacturer == "Yamaha") {
        if (model == "Genos") {
            // return std::make_unique<YamahaGenos>();
        } else if (model.find("Motif") != std::string::npos ||
                   model == "MODX" || model == "Montage") {
            // return std::make_unique<YamahaMotif>();
        }
    } else if (manufacturer == "Roland") {
        if (model == "Fantom") {
            // return std::make_unique<RolandFantom>();
        }
    } else if (manufacturer == "Korg") {
        if (model.find("PA") != std::string::npos) {
            // return std::make_unique<KorgPA>();
        }
    }
    
    return nullptr;
}

std::unique_ptr<Instrument> InstrumentFactory::detect(const std::string& midiInPort,
                                                       const std::string& midiOutPort) {
    // Auto-detection implementation
    // Would send identity request SysEx and parse response
    return nullptr;
}

std::vector<std::pair<std::string, std::string>> InstrumentFactory::supportedInstruments() {
    return {
        {"Yamaha", "Genos"},
        {"Yamaha", "Tyros5"},
        {"Yamaha", "PSR-SX900"},
        {"Yamaha", "PSR-SX700"},
        {"Yamaha", "Motif XF"},
        {"Yamaha", "MODX"},
        {"Yamaha", "Montage"},
        {"Roland", "Fantom"},
        {"Roland", "Jupiter X"},
        {"Roland", "RD-2000"},
        {"Korg", "PA5X"},
        {"Korg", "PA4X"},
        {"Korg", "PA1000"},
        {"Korg", "Kronos"},
        {"Korg", "Nautilus"},
        {"Nord", "Stage 3"},
        {"Nord", "Piano 4"},
        {"Kurzweil", "PC4"},
        {"Kurzweil", " Forte"}
    };
}

} // namespace maestro::instruments
