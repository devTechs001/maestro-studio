// include/maestro/scripting/script_engine.hpp
#pragma once

#include "maestro/core/types.hpp"
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <string>
#include <variant>

namespace maestro::scripting {

using ScriptValue = std::variant<
    std::nullptr_t,
    bool,
    int64_t,
    double,
    std::string,
    std::vector<struct ScriptValue>,
    std::map<std::string, struct ScriptValue>
>;

/**
 * @brief Script execution context
 */
class MAESTRO_API ScriptContext {
public:
    ScriptContext();
    ~ScriptContext();

    void setVariable(const std::string& name, const ScriptValue& value);
    ScriptValue getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;

    using NativeFunction = std::function<ScriptValue(
        const std::vector<ScriptValue>&)>;
    void registerFunction(const std::string& name, NativeFunction func);

    void registerObject(const std::string& name, void* object,
                        const std::string& typeName);

    using OutputCallback = std::function<void(const std::string&)>;
    void setOutputCallback(OutputCallback callback);
    void print(const std::string& message);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Main script engine (Lua-based)
 */
class MAESTRO_API ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    Result<void> initialize();
    void shutdown();

    Result<ScriptValue> execute(const std::string& code);
    Result<ScriptValue> executeFile(const std::string& path);

    void executeAsync(const std::string& code,
                      std::function<void(Result<ScriptValue>)> callback);

    ScriptContext& getContext() { return *context_; }

    void registerMaestroAPI();
    void registerMidiAPI();
    void registerAudioAPI();
    void registerProjectAPI();

    std::string getLastError() const;

    void setDebugMode(bool enabled);
    void setBreakpoint(const std::string& file, int line);
    void removeBreakpoint(const std::string& file, int line);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::unique_ptr<ScriptContext> context_;
};

/**
 * @brief Macro recorder/player
 */
class MAESTRO_API MacroEngine {
public:
    struct MacroAction {
        enum Type {
            MidiSend,
            ParameterChange,
            MenuCommand,
            KeyPress,
            Wait,
            Script,
            Loop,
            Condition
        };

        Type type;
        std::map<std::string, ScriptValue> parameters;
        double timestamp = 0.0;
    };

    struct Macro {
        std::string name;
        std::string description;
        std::string category;
        std::vector<MacroAction> actions;
        std::string keyBinding;
        bool recordedTiming = false;
    };

    MacroEngine();
    ~MacroEngine();

    void startRecording();
    void stopRecording();
    bool isRecording() const;
    void recordAction(const MacroAction& action);
    Macro getRecordedMacro() const;

    void play(const Macro& macro);
    void playByName(const std::string& name);
    void stop();
    bool isPlaying() const;
    void setPlaybackSpeed(float speed);

    void saveMacro(const Macro& macro);
    void deleteMacro(const std::string& name);
    Macro* getMacro(const std::string& name);
    std::vector<std::string> getMacroNames() const;
    std::vector<Macro*> getMacrosByCategory(const std::string& category) const;

    Result<void> exportMacro(const std::string& name, const std::string& path);
    Result<void> importMacro(const std::string& path);

    void setKeyBinding(const std::string& macroName, const std::string& keySequence);
    void processKeyEvent(const std::string& keySequence);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Control surface scripting
 */
class MAESTRO_API ControlSurfaceScript {
public:
    ControlSurfaceScript();
    ~ControlSurfaceScript();

    void setDeviceName(const std::string& name);
    void setDeviceManufacturer(const std::string& manufacturer);

    struct MidiMapping {
        uint8_t channel;
        uint8_t noteOrCC;
        bool isNote;
        std::string action;
        std::string parameter;
        bool feedback = true;
    };

    void addMapping(const MidiMapping& mapping);
    void removeMapping(int index);

    void setLEDState(int ledIndex, int state);
    void setDisplayText(const std::string& text, int line = 0);
    void setMotorFaderPosition(int faderIndex, float position);

    void processMidi(const midi::MidiMessage& msg);

    Result<void> loadScript(const std::string& path);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace maestro::scripting
