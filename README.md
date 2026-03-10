# 🎹 MaestroStudio - Enterprise Music Production Suite

## Overview

MaestroStudio is a professional-grade, cross-platform music production application that seamlessly integrates with hardware instruments (Yamaha, Roland, Korg, etc.), provides real-time MIDI processing, multi-pad performance capabilities, and full DAW functionality.

## Features

- **Real-time MIDI I/O** with hardware instruments
- **Multi-pad performance interface**
- **Style playback and editing** (Yamaha SFF, Roland STL)
- **Voice/Sound management**
- **Registration memory system**
- **Full DAW/Studio capabilities**
- **VST/AU plugin hosting**
- **Cross-platform** (Windows, macOS, Linux)

## Technology Stack

- **Core Engine**: C++20
- **Audio**: JUCE, PortAudio, RtAudio
- **MIDI**: RtMidi, JUCE MIDI
- **GUI Desktop**: Qt 6
- **Build System**: CMake

## Building

### Prerequisites

- CMake 3.20+
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Qt 6.5+
- RtAudio
- RtMidi

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get install -y \
    build-essential cmake ninja-build \
    libasound2-dev libjack-jackd2-dev libpulse-dev \
    qt6-base-dev qt6-multimedia-dev \
    librtaudio-dev librtmidi-dev

# Build
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Run
./MaestroStudio
```

### macOS

```bash
# Install dependencies
brew install cmake ninja qt rtaudio rtmidi

# Build
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Run
open MaestroStudio.app
```

### Windows

```powershell
# Install dependencies using vcpkg
vcpkg install qt6-base qt6-multimedia rtaudio rtmidi

# Build
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release

# Run
.\Release\MaestroStudio.exe
```

## Project Structure

```
maestro-studio/
├── include/maestro/       # Public headers
│   ├── core/             # Core types and engine
│   ├── audio/            # Audio engine and DSP
│   ├── midi/             # MIDI handling
│   ├── style/            # Style engine
│   ├── voice/            # Voice engine
│   ├── pad/              # Pad engine
│   ├── sync/             # Sync engine
│   ├── instruments/      # Instrument integration
│   └── studio/           # DAW features
├── src/                   # Implementation
│   ├── core/
│   ├── audio/
│   ├── midi/
│   ├── style/
│   ├── voice/
│   ├── pad/
│   ├── sync/
│   ├── instruments/
│   ├── studio/
│   └── gui/qt/           # Qt GUI
├── tests/                 # Unit and integration tests
├── resources/             # Assets and styles
└── docs/                  # Documentation
```

## Testing

```bash
cd build
ctest --output-on-failure
```

## License

MIT License - See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests
5. Submit a pull request

## Support

For issues and feature requests, please use the GitHub Issues page.
