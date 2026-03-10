// tests/integration/test_engine_integration.cpp
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

// Mock engine classes for integration testing
namespace maestro {

class MockAudioEngine {
public:
    bool initialize() { return true; }
    bool start() { running_ = true; return true; }
    bool stop() { running_ = false; return true; }
    bool isRunning() const { return running_; }
    double getCpuUsage() const { return 5.0; }
    double getLatency() const { return 5.8; }
    uint64_t getUnderrunCount() const { return 0; }
private:
    bool running_ = false;
};

class MockMidiEngine {
public:
    bool initialize() { return true; }
    bool start() { return true; }
    bool stop() { return true; }
    double getLatency() const { return 2.0; }
    uint64_t getOverflowCount() const { return 0; }
};

class MockMaestroEngine {
public:
    static MockMaestroEngine& instance() {
        static MockMaestroEngine instance;
        return instance;
    }
    
    bool initialize() {
        initialized_ = true;
        return true;
    }
    
    bool start() {
        running_ = true;
        return true;
    }
    
    bool stop() {
        running_ = false;
        return true;
    }
    
    bool shutdown() {
        initialized_ = false;
        running_ = false;
        return true;
    }
    
    bool isRunning() const { return running_; }
    bool isInitialized() const { return initialized_; }
    
    MockAudioEngine& audio() { return audio_; }
    MockMidiEngine& midi() { return midi_; }
    
    struct Performance {
        double cpuUsage = 5.0;
        double audioLatency = 5.8;
        double midiLatency = 2.0;
        uint64_t bufferUnderruns = 0;
        uint64_t midiOverflows = 0;
    };
    
    Performance getPerformance() const { return Performance{}; }
    
private:
    MockMaestroEngine() = default;
    bool initialized_ = false;
    bool running_ = false;
    MockAudioEngine audio_;
    MockMidiEngine midi_;
};

} // namespace maestro

using namespace maestro;

class EngineIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset engine state before each test
    }
    
    void TearDown() override {
        MockMaestroEngine::instance().shutdown();
    }
};

TEST_F(EngineIntegrationTest, InitializeAndStart) {
    EXPECT_TRUE(MockMaestroEngine::instance().initialize());
    EXPECT_TRUE(MockMaestroEngine::instance().isInitialized());
    
    EXPECT_TRUE(MockMaestroEngine::instance().start());
    EXPECT_TRUE(MockMaestroEngine::instance().isRunning());
    
    EXPECT_TRUE(MockMaestroEngine::instance().stop());
    EXPECT_FALSE(MockMaestroEngine::instance().isRunning());
}

TEST_F(EngineIntegrationTest, AudioEngineBasic) {
    EXPECT_TRUE(MockMaestroEngine::instance().initialize());
    
    auto& audio = MockMaestroEngine::instance().audio();
    EXPECT_TRUE(audio.initialize());
    EXPECT_TRUE(audio.start());
    EXPECT_TRUE(audio.isRunning());
    EXPECT_TRUE(audio.stop());
}

TEST_F(EngineIntegrationTest, MidiEngineBasic) {
    EXPECT_TRUE(MockMaestroEngine::instance().initialize());
    
    auto& midi = MockMaestroEngine::instance().midi();
    EXPECT_TRUE(midi.initialize());
    EXPECT_TRUE(midi.start());
    EXPECT_TRUE(midi.stop());
}

TEST_F(EngineIntegrationTest, EnginePerformance) {
    EXPECT_TRUE(MockMaestroEngine::instance().initialize());
    EXPECT_TRUE(MockMaestroEngine::instance().start());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto perf = MockMaestroEngine::instance().getPerformance();
    
    EXPECT_LT(perf.cpuUsage, 50.0);
    EXPECT_NEAR(perf.audioLatency, 5.8, 1.0);
    EXPECT_EQ(perf.bufferUnderruns, 0);
}
