// src/midi/midi_message.cpp
#include "maestro/midi/midi_types.hpp"
#include <sstream>
#include <iomanip>

namespace maestro::midi {

MidiMessage MidiMessage::noteOn(MidiChannel ch, MidiNote note, MidiVelocity vel) {
    MidiMessage msg;
    msg.data[0] = static_cast<uint8_t>(MessageType::NoteOn) | (ch & 0x0F);
    msg.data[1] = note & 0x7F;
    msg.data[2] = vel & 0x7F;
    msg.length = 3;
    return msg;
}

MidiMessage MidiMessage::noteOff(MidiChannel ch, MidiNote note, MidiVelocity vel) {
    MidiMessage msg;
    msg.data[0] = static_cast<uint8_t>(MessageType::NoteOff) | (ch & 0x0F);
    msg.data[1] = note & 0x7F;
    msg.data[2] = vel & 0x7F;
    msg.length = 3;
    return msg;
}

MidiMessage MidiMessage::controlChange(MidiChannel ch, MidiCC cc, uint8_t value) {
    MidiMessage msg;
    msg.data[0] = static_cast<uint8_t>(MessageType::ControlChange) | (ch & 0x0F);
    msg.data[1] = cc & 0x7F;
    msg.data[2] = value & 0x7F;
    msg.length = 3;
    return msg;
}

MidiMessage MidiMessage::programChange(MidiChannel ch, uint8_t program) {
    MidiMessage msg;
    msg.data[0] = static_cast<uint8_t>(MessageType::ProgramChange) | (ch & 0x0F);
    msg.data[1] = program & 0x7F;
    msg.length = 2;
    return msg;
}

MidiMessage MidiMessage::pitchBend(MidiChannel ch, int16_t value) {
    MidiMessage msg;
    uint16_t bendValue = static_cast<uint16_t>(value + 8192);
    msg.data[0] = static_cast<uint8_t>(MessageType::PitchBend) | (ch & 0x0F);
    msg.data[1] = bendValue & 0x7F;
    msg.data[2] = (bendValue >> 7) & 0x7F;
    msg.length = 3;
    return msg;
}

MidiMessage MidiMessage::channelPressure(MidiChannel ch, uint8_t pressure) {
    MidiMessage msg;
    msg.data[0] = static_cast<uint8_t>(MessageType::ChannelPressure) | (ch & 0x0F);
    msg.data[1] = pressure & 0x7F;
    msg.length = 2;
    return msg;
}

MidiMessage MidiMessage::polyPressure(MidiChannel ch, MidiNote note, uint8_t pressure) {
    MidiMessage msg;
    msg.data[0] = static_cast<uint8_t>(MessageType::PolyPressure) | (ch & 0x0F);
    msg.data[1] = note & 0x7F;
    msg.data[2] = pressure & 0x7F;
    msg.length = 3;
    return msg;
}

MessageType MidiMessage::type() const {
    if (data[0] >= 0xF0) {
        return MessageType::System;
    }
    return static_cast<MessageType>(data[0] & 0xF0);
}

MidiChannel MidiMessage::channel() const {
    return data[0] & 0x0F;
}

MidiNote MidiMessage::note() const {
    return data[1];
}

MidiVelocity MidiMessage::velocity() const {
    return data[2];
}

MidiCC MidiMessage::ccNumber() const {
    return data[1];
}

uint8_t MidiMessage::ccValue() const {
    return data[2];
}

uint8_t MidiMessage::program() const {
    return data[1];
}

int16_t MidiMessage::pitchBendValue() const {
    uint16_t value = (static_cast<uint16_t>(data[2]) << 7) | data[1];
    return static_cast<int16_t>(value) - 8192;
}

bool MidiMessage::isNoteOn() const {
    return type() == MessageType::NoteOn && velocity() > 0;
}

bool MidiMessage::isNoteOff() const {
    return type() == MessageType::NoteOff ||
           (type() == MessageType::NoteOn && velocity() == 0);
}

bool MidiMessage::isNote() const {
    return isNoteOn() || isNoteOff();
}

bool MidiMessage::isCC() const {
    return type() == MessageType::ControlChange;
}

bool MidiMessage::isChannelMessage() const {
    return data[0] < 0xF0;
}

bool MidiMessage::isSystemMessage() const {
    return data[0] >= 0xF0;
}

std::string MidiMessage::toString() const {
    std::stringstream ss;

    switch (type()) {
        case MessageType::NoteOn:
            ss << "NoteOn ch=" << static_cast<int>(channel())
               << " note=" << static_cast<int>(note())
               << " vel=" << static_cast<int>(velocity());
            break;
        case MessageType::NoteOff:
            ss << "NoteOff ch=" << static_cast<int>(channel())
               << " note=" << static_cast<int>(note());
            break;
        case MessageType::ControlChange:
            ss << "CC ch=" << static_cast<int>(channel())
               << " cc=" << static_cast<int>(ccNumber())
               << " val=" << static_cast<int>(ccValue());
            break;
        case MessageType::ProgramChange:
            ss << "PC ch=" << static_cast<int>(channel())
               << " prog=" << static_cast<int>(program());
            break;
        case MessageType::PitchBend:
            ss << "PitchBend ch=" << static_cast<int>(channel())
               << " val=" << pitchBendValue();
            break;
        default:
            ss << "Unknown: ";
            for (int i = 0; i < length; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(data[i]) << " ";
            }
    }

    return ss.str();
}

uint32_t SysExMessage::manufacturerId() const {
    if (data.size() >= 4) {
        return (data[1] << 16) | (data[2] << 8) | data[3];
    }
    return 0;
}

uint8_t SysExMessage::modelId() const {
    if (data.size() >= 6) {
        return data[5];
    }
    return 0;
}

std::string SysExMessage::toHexString() const {
    std::stringstream ss;
    for (size_t i = 0; i < data.size(); i++) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(data[i]);
        if (i < data.size() - 1) ss << " ";
    }
    return ss.str();
}

} // namespace maestro::midi
