cmake_minimum_required(VERSION 3.20)
project(MaestroStudio VERSION 1.0.0 LANGUAGES CXX)

# C++20 required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(MAESTRO_BUILD_TESTS "Build tests" ON)
option(MAESTRO_BUILD_SHARED "Build shared library" ON)
option(MAESTRO_ENABLE_PLUGINS "Enable plugin hosting" ON)
option(MAESTRO_ENABLE_MOBILE "Build for mobile platforms" OFF)
option(MAESTRO_ENABLE_SIMD "Enable SIMD optimizations" ON)

# Package manager
find_package(PkgConfig REQUIRED)

# Find dependencies
find_package(Qt6 COMPONENTS Widgets Core Gui Multimedia REQUIRED)
find_package(RtAudio REQUIRED)
find_package(RtMidi REQUIRED)

# Optional dependencies
if(MAESTRO_ENABLE_PLUGINS)
    # JUCE for plugin hosting
    # add_subdirectory(libs/JUCE)
endif()

# SIMD optimizations
if(MAESTRO_ENABLE_SIMD)
    if(MSVC)
        add_compile_options(/O2 /arch:AVX2)
    else()
        add_compile_options(-O3 -march=native)
    endif()
endif()

# Core library sources
set(MAESTRO_CORE_SOURCES
    src/core/engine.cpp
    src/core/settings.cpp
    src/audio/audio_engine.cpp
    src/audio/effects/reverb.cpp
    src/audio/effects/delay.cpp
    src/audio/effects/eq.cpp
    src/audio/effects/dynamics.cpp
    src/midi/midi_engine.cpp
    src/midi/midi_message.cpp
    src/midi/midi_file.cpp
    src/midi/midi_sequencer.cpp
    src/style/style_engine.cpp
    src/style/style_parser.cpp
    src/voice/voice_engine.cpp
    src/pad/pad_engine.cpp
    src/sync/sync_engine.cpp
    src/instruments/instrument_factory.cpp
    src/instruments/yamaha/yamaha_genos.cpp
    src/instruments/yamaha/yamaha_motif.cpp
    src/instruments/yamaha/sff_parser.cpp
    src/instruments/roland/roland_fantom.cpp
    src/instruments/korg/korg_pa.cpp
    src/studio/project.cpp
    src/studio/plugin_host.cpp
    src/studio/recording.cpp
    src/studio/automation.cpp
    src/scripting/script_engine.cpp
)

# Build core library
if(MAESTRO_BUILD_SHARED)
    add_library(maestro_core SHARED ${MAESTRO_CORE_SOURCES})
    target_compile_definitions(maestro_core PRIVATE MAESTRO_BUILD_SHARED)
else()
    add_library(maestro_core STATIC ${MAESTRO_CORE_SOURCES})
endif()

target_include_directories(maestro_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(maestro_core PUBLIC
    RtAudio::RtAudio
    RtMidi::RtMidi
    Qt6::Core
)

# GUI application
set(MAESTRO_GUI_SOURCES
    src/gui/qt/main.cpp
    src/gui/qt/mainwindow.cpp
    src/gui/qt/widgets/pad_grid.cpp
    src/gui/qt/widgets/piano_roll.cpp
    src/gui/qt/widgets/mixer.cpp
    src/gui/qt/widgets/style_control.cpp
    src/gui/qt/widgets/voice_select.cpp
    src/gui/qt/widgets/registration.cpp
    src/gui/qt/widgets/transport.cpp
    src/gui/qt/widgets/midi_monitor.cpp
    src/gui/qt/widgets/splash_screen.cpp
    src/gui/qt/widgets/onboarding_wizard.cpp
    src/gui/qt/widgets/settings_dialog.cpp
)

qt_add_executable(MaestroStudio
    ${MAESTRO_GUI_SOURCES}
)

target_link_libraries(MaestroStudio PRIVATE
    maestro_core
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
)

# Resources
qt_add_resources(MaestroStudio "resources"
    PREFIX "/"
    FILES
        resources/icons/app.png
        resources/icons/play.png
        resources/icons/stop.png
        resources/icons/record.png
        resources/styles/dark.qss
)

# Install targets
install(TARGETS maestro_core MaestroStudio
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

# Tests
if(MAESTRO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "MaestroStudio")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "MaestroStudio")
include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "MaestroStudio Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests: ${MAESTRO_BUILD_TESTS}")
message(STATUS "  Build Shared: ${MAESTRO_BUILD_SHARED}")
message(STATUS "  Enable Plugins: ${MAESTRO_ENABLE_PLUGINS}")
message(STATUS "  Enable SIMD: ${MAESTRO_ENABLE_SIMD}")
message(STATUS "")
