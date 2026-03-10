// tests/unit/test_style_engine.cpp
#include <gtest/gtest.h>
#include "maestro/style/style_engine.hpp"

using namespace maestro;

TEST(ChordInfoTest, MajorChord) {
    ChordInfo chord;
    chord.root = 60;  // C
    chord.type = ChordInfo::Type::Major;
    
    auto notes = chord.getNotes();
    ASSERT_EQ(notes.size(), 3);
    EXPECT_EQ(notes[0], 60);  // C
    EXPECT_EQ(notes[1], 64);  // E
    EXPECT_EQ(notes[2], 67);  // G
}

TEST(ChordInfoTest, MinorChord) {
    ChordInfo chord;
    chord.root = 60;  // C
    chord.type = ChordInfo::Type::Minor;
    
    auto notes = chord.getNotes();
    ASSERT_EQ(notes.size(), 3);
    EXPECT_EQ(notes[0], 60);  // C
    EXPECT_EQ(notes[1], 63);  // Eb
    EXPECT_EQ(notes[2], 67);  // G
}

TEST(ChordInfoTest, SeventhChord) {
    ChordInfo chord;
    chord.root = 60;  // C
    chord.type = ChordInfo::Type::Seventh;
    
    auto notes = chord.getNotes();
    ASSERT_EQ(notes.size(), 4);
    EXPECT_EQ(notes[0], 60);  // C
    EXPECT_EQ(notes[1], 64);  // E
    EXPECT_EQ(notes[2], 67);  // G
    EXPECT_EQ(notes[3], 70);  // Bb
}

TEST(ChordInfoTest, Detection) {
    std::vector<MidiNote> notes = {60, 64, 67};  // C major
    auto detected = ChordInfo::detect(notes);
    
    EXPECT_EQ(detected.root, 60);
    EXPECT_EQ(detected.type, ChordInfo::Type::Major);
}

TEST(ChordInfoTest, DetectionMinor) {
    std::vector<MidiNote> notes = {57, 60, 64};  // A minor
    auto detected = ChordInfo::detect(notes);
    
    EXPECT_EQ(detected.root, 57);
    EXPECT_EQ(detected.type, ChordInfo::Type::Minor);
}

TEST(ChordInfoTest, ToString) {
    ChordInfo chord;
    chord.root = 60;
    chord.type = ChordInfo::Type::Major;
    
    std::string str = chord.toString();
    EXPECT_EQ(str, "C");
    
    chord.type = ChordInfo::Type::Minor;
    str = chord.toString();
    EXPECT_EQ(str, "Cm");
    
    chord.type = ChordInfo::Type::Seventh;
    str = chord.toString();
    EXPECT_EQ(str, "C7");
}
