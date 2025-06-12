#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
RED_REPO_URL="https://github.com/Probatio-Diabolica/Red"
RED_DIR="Red"                # Directory where Red will be cloned, relative to Violet's root
VIOLET_BUILD_DIR="build"     # Violet's main build directory, relative to Violet's root

echo "--- Initiating Comprehensive Build: Violet Server and Red Client ---"
echo "Performing explicit build steps to avoid external script permission issues. You're welcome."

# --- Step 1: Build Violet Server ---
echo "Building Violet server..."

# Create and navigate into Violet's build directory
mkdir -p "$VIOLET_BUILD_DIR"
pushd "$VIOLET_BUILD_DIR"

echo "Running CMake for Violet."
cmake ..

echo "Compiling Violet."
make

# Verify Violet executable was created.
if [ ! -f "violet" ]; then
    echo "Error: 'violet' executable not found in '$VIOLET_BUILD_DIR/'. Violet server build failed."
    popd # Return to Violet's root before exiting
    exit 1
fi

popd # Return to Violet's root directory
echo "Violet server build completed."

# --- Step 2: Clone and Build Red Client ---
echo "Cloning and building Red client..."

# Clean up existing Red directory to ensure a clean clone.
if [ -d "$RED_DIR" ]; then
    echo "Removing existing '$RED_DIR' directory for a clean build."
    rm -rf "$RED_DIR"
fi
echo "Cloning $RED_REPO_URL into $RED_DIR..."
git clone "$RED_REPO_URL"

# Navigate into the cloned Red repository to build it.
echo "Entering '$RED_DIR' to build the Red client."
pushd "$RED_DIR"

# Create Red's internal build directory and navigate into it.
mkdir -p build
cd build

echo "Running CMake for Red."
cmake ..

echo "Compiling Red."
make

# Verify Red executable was created.
if [ ! -f "red" ]; then
    echo "Error: 'red' executable not found in '$RED_DIR/build/'. Red client build failed."
    popd # Return to Violet's root before exiting
    exit 1
fi

# Return to Violet's root directory.
popd
echo "Returned to Violet's root directory."

# --- Step 3: Move Red executable to Violet's build directory ---
echo "Moving 'red' executable from '$RED_DIR/build/red' to '$VIOLET_BUILD_DIR/'."

# Ensure Violet's main build directory exists (it should now).
mkdir -p "$VIOLET_BUILD_DIR"

mv "$RED_DIR/build/red" "$VIOLET_BUILD_DIR/"
echo "'red' executable successfully moved to '$VIOLET_BUILD_DIR'."

echo "--- All Builds Completed ---"
echo "Violet server and Red client are now built and ready."
echo "Executables 'violet' and 'red' can be found in the '$VIOLET_BUILD_DIR' directory."