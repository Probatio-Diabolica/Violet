#!/bin/bash

# Exit immediately if any command fails. Because we don't have all day.
set -e

# --- Configuration ---
RED_REPO_URL="https://github.com/Probatio-Diabolica/Red"
RED_DIR="Red" # Directory where Red will be cloned
# Assuming this script is run from the Violet project's root,
# and Violet's build directory is simply 'build/'.
VIOLET_BUILD_DIR="build"

echo "--- Building Red Client for Violet ---"
echo "Starting the complex task of cloning and compiling your client, BTW."

# Step 1: Clone the Red repository
if [ -d "$RED_DIR" ]; then
    echo "Directory '$RED_DIR' already exists. Removing it to ensure a clean clone. Consistency is key, even for you."
    rm -rf "$RED_DIR"
fi
echo "Cloning $RED_REPO_URL into $RED_DIR..."
git clone "$RED_REPO_URL"

# Step 2: Build the Red package
echo "Navigating into '$RED_DIR' to initiate the build process..."
pushd "$RED_DIR" # Save current directory, then change into Red's directory

# Create and enter the build directory within Red.
# Because even simple projects need a proper build setup.
mkdir -p build
cd build

echo "Running CMake for Red. Don't worry, it's almost automatic."
cmake ..

echo "Compiling Red. This is where the magic (or errors) happens."
make

# Basic sanity check: did the executable actually get created?
if [ ! -f "red" ]; then
    echo "Error: 'red' executable not found in '$RED_DIR/build/'. Your build likely failed. Shocking."
    exit 1
fi

popd # Return to the original directory (Violet's root)

# Step 3: Move the "red" executable to Violet's build directory
echo "Moving the glorious 'red' executable to Violet's build directory ($VIOLET_BUILD_DIR)..."

# Ensure Violet's build directory exists. Don't assume anything.
mkdir -p "$VIOLET_BUILD_DIR"

mv "$RED_DIR/build/red" "$VIOLET_BUILD_DIR/"

echo "--- Build Complete ---"
echo "Successfully cloned, built Red, and moved 'red' executable to '$VIOLET_BUILD_DIR'."
echo "You can now run '$VIOLET_BUILD_DIR/red' from this directory."
echo "You're welcome, Arch user."