// src/midi/midi_engine.cpp
#include "maestro/midi/midi_engine.hpp"
#include <RtMidi.h>
#include <algorithm>

namespace maestro {

class MidiEngine::Impl {
public:
    std::unique_ptr<RtMidiIn> midiIn;
    std::unique_ptr<RtMidiOut> midiOut;
    std::vector<MidiPort> inputPorts;
    std::vector<MidiPort> outputPorts;
    MidiInputCallback inputCallback;
    SysExCallback sysExCallback;
    std::vector<uint8_t> currentSysEx;
    bool learning = false;
    std::function<void(const LearnResult&)> learnCallback;
    uint64_t overflowCount = 0;

    static void midiCallback(double delTime, std::vector<unsigned char>* msg, void* userData) {
        auto* engine = static_cast<MidiEngine*>(userData);
        
        if (msg->size() >= 1) {
            midi::MidiMessage midiMsg;
            midiMsg.length = static_cast<uint8_t>(std::min(msg->size(), size_t(3)));
            for (size_t i = 0; i < midiMsg.length; ++i) {
                midiMsg.data[i] = (*msg)[i];
            }
            
            if (engine->impl_->inputCallback) {
                engine->impl_->inputCallback(midiMsg);
            }
            
            if (engine->impl_->learning && engine->impl_->learnCallback) {
                LearnResult result{midiMsg, ""};
                engine->impl_->learnCallback(result);
            }
        }
    }
};

MidiEngine::MidiEngine() : impl_(std::make_unique<Impl>()) {
    impl_->midiIn = std::make_unique<RtMidiIn>();
    impl_->midiOut = std::make_unique<RtMidiOut>();
}

MidiEngine::~MidiEngine() {
    stop();
}

Result<void> MidiEngine::initialize() {
    try {
        // Get input ports
        unsigned int portCount = impl_->midiIn->getPortCount();
        for (unsigned int i = 0; i < portCount; ++i) {
            MidiPort port;
            port.id = std::to_string(i);
            port.name = impl_->midiIn->getPortName(i);
            port.isInput = true;
            port.isOutput = false;
            port.isVirtual = false;
            impl_->inputPorts.push_back(port);
        }

        // Get output ports
        portCount = impl_->midiOut->getPortCount();
        for (unsigned int i = 0; i < portCount; ++i) {
            MidiPort port;
            port.id = std::to_string(i);
            port.name = impl_->midiOut->getPortName(i);
            port.isInput = false;
            port.isOutput = true;
            port.isVirtual = false;
            impl_->outputPorts.push_back(port);
        }

        return Result<void>();
    } catch (const RtMidiError& e) {
        return Result<void>(std::string("RtMidi error: ") + e.what());
    }
}

Result<void> MidiEngine::start() {
    try {
        impl_->midiIn->setCallback(&Impl::midiCallback, this);
        impl_->midiIn->ignoreTypes(false, false, false);
        return Result<void>();
    } catch (const RtMidiError& e) {
        return Result<void>(std::string("Failed to start MIDI: ") + e.what());
    }
}

Result<void> MidiEngine::stop() {
    try {
        if (impl_->midiIn->isCallbackActive()) {
            impl_->midiIn->cancelCallback();
        }
        return Result<void>();
    } catch (const RtMidiError& e) {
        return Result<void>(std::string("Failed to stop MIDI: ") + e.what());
    }
}

std::vector<MidiPort> MidiEngine::getInputPorts() const {
    return impl_->inputPorts;
}

std::vector<MidiPort> MidiEngine::getOutputPorts() const {
    return impl_->outputPorts;
}

Result<void> MidiEngine::openInput(const std::string& portId) {
    try {
        unsigned int portNum = std::stoul(portId);
        impl_->midiIn->openPort(portNum);
        return Result<void>();
    } catch (...) {
        return Result<void>("Invalid port ID");
    }
}

Result<void> MidiEngine::openOutput(const std::string& portId) {
    try {
        unsigned int portNum = std::stoul(portId);
        impl_->midiOut->openPort(portNum);
        return Result<void>();
    } catch (...) {
        return Result<void>("Invalid port ID");
    }
}

Result<void> MidiEngine::closeInput(const std::string& /*portId*/) {
    try {
        if (impl_->midiIn->isPortOpen()) {
            impl_->midiIn->closePort();
        }
        return Result<void>();
    } catch (const RtMidiError& e) {
        return Result<void>(std::string("Failed to close input: ") + e.what());
    }
}

Result<void> MidiEngine::closeOutput(const std::string& /*portId*/) {
    try {
        if (impl_->midiOut->isPortOpen()) {
            impl_->midiOut->closePort();
        }
        return Result<void>();
    } catch (const RtMidiError& e) {
        return Result<void>(std::string("Failed to close output: ") + e.what());
    }
}

Result<std::string> MidiEngine::createVirtualInput(const std::string& name) {
    try {
        impl_->midiIn->openVirtualPort(name);
        return Result<std::string>(name);
    } catch (const RtMidiError& e) {
        return Result<std::string>(std::string("Failed to create virtual input: ") + e.what());
    }
}

Result<std::string> MidiEngine::createVirtualOutput(const std::string& name) {
    try {
        impl_->midiOut->openVirtualPort(name);
        return Result<std::string>(name);
    } catch (const RtMidiError& e) {
        return Result<std::string>(std::string("Failed to create virtual output: ") + e.what());
    }
}

Result<void> MidiEngine::send(const midi::MidiMessage& msg, const std::string& /*portId*/) {
    try {
        std::vector<unsigned char> midiMsg(msg.data.begin(), msg.data.begin() + msg.length);
        impl_->midiOut->sendMessage(&midiMsg);
        return Result<void>();
    } catch (const RtMidiError& e) {
        return Result<void>(std::string("Failed to send MIDI: ") + e.what());
    }
}

void MidiEngine::sendToAll(const midi::MidiMessage& msg) {
    send(msg);
}

Result<void> MidiEngine::sendSysEx(const midi::SysExMessage& msg, const std::string& /*portId*/) {
    try {
        impl_->midiOut->sendMessage(&msg.data);
        return Result<void>();
    } catch (const RtMidiError& e) {
        return Result<void>(std::string("Failed to send SysEx: ") + e.what());
    }
}

void MidiEngine::setInputCallback(MidiInputCallback callback) {
    impl_->inputCallback = std::move(callback);
}

void MidiEngine::setSysExCallback(SysExCallback callback) {
    impl_->sysExCallback = std::move(callback);
}

void MidiEngine::startLearn(std::function<void(const LearnResult&)> callback) {
    impl_->learning = true;
    impl_->learnCallback = std::move(callback);
}

void MidiEngine::stopLearn() {
    impl_->learning = false;
    impl_->learnCallback = nullptr;
}

bool MidiEngine::isLearning() const {
    return impl_->learning;
}

void MidiEngine::sendClock() {
    std::vector<unsigned char> msg{0xF8};
    impl_->midiOut->sendMessage(&msg);
}

void MidiEngine::sendStart() {
    std::vector<unsigned char> msg{0xFA};
    impl_->midiOut->sendMessage(&msg);
}

void MidiEngine::sendStop() {
    std::vector<unsigned char> msg{0xFC};
    impl_->midiOut->sendMessage(&msg);
}

void MidiEngine::sendContinue() {
    std::vector<unsigned char> msg{0xFB};
    impl_->midiOut->sendMessage(&msg);
}

void MidiEngine::sendSongPosition(uint16_t position) {
    std::vector<unsigned char> msg{
        0xF2,
        static_cast<unsigned char>(position & 0x7F),
        static_cast<unsigned char>((position >> 7) & 0x7F)
    };
    impl_->midiOut->sendMessage(&msg);
}

double MidiEngine::getLatency() const {
    return 2.0; // Approximate MIDI latency in ms
}

uint64_t MidiEngine::getOverflowCount() const {
    return impl_->overflowCount;
}

// MidiRouter implementation
void MidiRouter::addRoute(const Route& route) {
    routes_.push_back(route);
}

void MidiRouter::removeRoute(size_t index) {
    if (index < routes_.size()) {
        routes_.erase(routes_.begin() + index);
    }
}

void MidiRouter::clearRoutes() {
    routes_.clear();
}

void MidiRouter::process(const midi::MidiMessage& msg, const std::string& sourcePort) {
    if (!enabled_) return;

    for (const auto& route : routes_) {
        if (route.sourcePort != sourcePort) continue;
        if (route.sourceChannel != 0 && route.sourceChannel != msg.channel()) continue;

        midi::MidiMessage outMsg = msg;
        
        // Apply transformations
        if (route.destChannel != 0) {
            outMsg.data[0] = (outMsg.data[0] & 0xF0) | (route.destChannel & 0x0F);
        }
        
        if (route.transpose != 0 && msg.isNote()) {
            outMsg.data[1] = std::clamp(static_cast<int>(msg.note()) + route.transpose, 0, 127);
        }
        
        if (route.velocityOffset != 0 && msg.isNote()) {
            outMsg.data[2] = std::clamp(static_cast<int>(msg.velocity()) + route.velocityOffset, 0, 127);
        }

        // Send to destination (would need MidiEngine reference in real impl)
    }
}

} // namespace maestro
