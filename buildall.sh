#!/bin/bash

set -e

# --- Configuration ---
RED_REPO_URL="https://github.com/Probatio-Diabolica/Red"
RED_DIR="Red"                # Directory where Red will be cloned, relative to Violet's root
VIOLET_BUILD_DIR="build"     # Violet's main build directory, relative to Violet's root

echo "--- Initiating Comprehensive Build: Violet Server and Red Client ---"

echo "Building Violet server using its dedicated build script..."

if [ ! -f "./build.sh" ]; then
    echo "Error: Violet's 'build.sh' script not found. Ensure you are in the Violet project root."
    exit 1
fi
./build.sh
echo "Violet server build completed."

echo "Cloning and building Red client..."

if [ -d "$RED_DIR" ]; then
    echo "Removing existing '$RED_DIR' directory for a clean build."
    rm -rf "$RED_DIR"
fi
echo "Cloning $RED_REPO_URL into $RED_DIR..."
git clone "$RED_REPO_URL"

echo "Entering '$RED_DIR' to build the Red client using its own build script."
pushd "$RED_DIR"

if [ ! -f "./build.sh" ]; then
    echo "Error: Red's 'build.sh' script not found inside '$RED_DIR'. Red build cannot proceed."
    popd # Return to Violet's root before exiting
    exit 1
fi
./build.sh

RED_EXECUTABLE_PATH="build/red"

if [ ! -f "$RED_EXECUTABLE_PATH" ]; then
    echo "Error: 'red' executable not found in '$RED_DIR/$RED_EXECUTABLE_PATH'. Red client build failed."
    popd # Return to Violet's root before exiting
    exit 1
fi

popd
echo "Returned to Violet's root directory."

# --- Step 3: Move Red executable to Violet's build directory ---
echo "Moving 'red' executable from '$RED_DIR/$RED_EXECUTABLE_PATH' to '$VIOLET_BUILD_DIR/'."

mkdir -p "$VIOLET_BUILD_DIR"

mv "$RED_DIR/$RED_EXECUTABLE_PATH" "$VIOLET_BUILD_DIR/"
echo "'red' executable successfully moved to '$VIOLET_BUILD_DIR'."

echo "--- All Builds Completed ---"
echo "Violet server and Red client are now built and ready."
echo "Executables 'violet' and 'red' can be found in the '$VIOLET_BUILD_DIR' directory."