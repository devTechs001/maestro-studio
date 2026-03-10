// tests/unit/test_midi_messages.cpp
#include <gtest/gtest.h>
#include "maestro/midi/midi_types.hpp"

using namespace maestro;
using namespace maestro::midi;

TEST(MidiMessageTest, NoteOn) {
    auto msg = MidiMessage::noteOn(0, 60, 100);
    
    EXPECT_EQ(msg.type(), MessageType::NoteOn);
    EXPECT_EQ(msg.channel(), 0);
    EXPECT_EQ(msg.note(), 60);
    EXPECT_EQ(msg.velocity(), 100);
    EXPECT_TRUE(msg.isNoteOn());
}

TEST(MidiMessageTest, NoteOff) {
    auto msg = MidiMessage::noteOff(5, 72, 64);
    
    EXPECT_EQ(msg.type(), MessageType::NoteOff);
    EXPECT_EQ(msg.channel(), 5);
    EXPECT_EQ(msg.note(), 72);
    EXPECT_TRUE(msg.isNoteOff());
}

TEST(MidiMessageTest, ControlChange) {
    auto msg = MidiMessage::controlChange(2, CC::Volume, 100);
    
    EXPECT_EQ(msg.type(), MessageType::ControlChange);
    EXPECT_EQ(msg.channel(), 2);
    EXPECT_EQ(msg.ccNumber(), CC::Volume);
    EXPECT_EQ(msg.ccValue(), 100);
    EXPECT_TRUE(msg.isCC());
}

TEST(MidiMessageTest, ProgramChange) {
    auto msg = MidiMessage::programChange(10, 25);
    
    EXPECT_EQ(msg.type(), MessageType::ProgramChange);
    EXPECT_EQ(msg.channel(), 10);
    EXPECT_EQ(msg.program(), 25);
    EXPECT_EQ(msg.length, 2);
}

TEST(MidiMessageTest, PitchBend) {
    auto msg = MidiMessage::pitchBend(0, 0);  // Center
    
    EXPECT_EQ(msg.type(), MessageType::PitchBend);
    EXPECT_EQ(msg.pitchBendValue(), 0);
    
    msg = MidiMessage::pitchBend(0, 8191);  // Max up
    EXPECT_GT(msg.pitchBendValue(), 8000);
    
    msg = MidiMessage::pitchBend(0, -8192);  // Max down
    EXPECT_LT(msg.pitchBendValue(), -8000);
}

TEST(MidiMessageTest, ToString) {
    auto msg = MidiMessage::noteOn(0, 60, 100);
    std::string str = msg.toString();
    
    EXPECT_NE(str.find("NoteOn"), std::string::npos);
    EXPECT_NE(str.find("60"), std::string::npos);
}

TEST(SysExMessageTest, ManufacturerId) {
    SysExMessage msg;
    msg.data = {0xF0, 0x43, 0x10, 0x00, 0x7F, 0xF7};  // Yamaha
    
    EXPECT_TRUE(msg.isYamaha());
    EXPECT_FALSE(msg.isRoland());
    EXPECT_FALSE(msg.isKorg());
    EXPECT_EQ(msg.manufacturerId(), 0x43);
}
