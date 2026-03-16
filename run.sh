#!/bin/bash
# MaestroStudio Run Script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Check if executable exists
if [ ! -f "$BUILD_DIR/MaestroStudio" ]; then
    echo "❌ Error: MaestroStudio executable not found!"
    echo "Please build the project first:"
    echo "  cd $BUILD_DIR && make"
    exit 1
fi

echo "🎹 Starting MaestroStudio..."
echo "Working directory: $BUILD_DIR"

# Run with offscreen platform for headless systems
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-offscreen}"

cd "$BUILD_DIR"
./MaestroStudio "$@"

EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ MaestroStudio exited normally"
else
    echo "❌ MaestroStudio exited with code: $EXIT_CODE"
fi

exit $EXIT_CODE
