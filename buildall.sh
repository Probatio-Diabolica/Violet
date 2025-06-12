#!/bin/bash

set -e

RED_REPO_URL="https://github.com/Probatio-Diabolica/Red"
RED_DIR="Red"               
VIOLET_BUILD_DIR="build"    

echo "--- Initiating Comprehensive Build: Violet Server and Red Client ---"
echo "Performing explicit build steps to avoid external script permission issues. You're welcome."

echo "Building Violet server..."

mkdir -p "$VIOLET_BUILD_DIR"
pushd "$VIOLET_BUILD_DIR"

echo "Running CMake for Violet."
cmake ..

echo "Compiling Violet."
make

if [ ! -f "violet" ]; then
    echo "Error: 'violet' executable not found in '$VIOLET_BUILD_DIR/'. Violet server build failed."
    popd 
    exit 1
fi

popd 
echo "Violet server build completed."

echo "Cloning and building Red client..."

if [ -d "$RED_DIR" ]; then
    echo "Removing existing '$RED_DIR' directory for a clean build."
    rm -rf "$RED_DIR"
fi
echo "Cloning $RED_REPO_URL into $RED_DIR..."
git clone "$RED_REPO_URL"

echo "Entering '$RED_DIR' to build the Red client."
pushd "$RED_DIR"

mkdir -p build
cd build

echo "Running CMake for Red."
cmake ..

echo "Compiling Red."
make

if [ ! -f "red" ]; then
    echo "Error: 'red' executable not found in '$RED_DIR/build/'. Red client build failed."
    popd #
    exit 1
fi

popd
echo "Returned to Violet's root directory."

echo "Moving 'red' executable from '$RED_DIR/build/red' to '$VIOLET_BUILD_DIR/'."

mkdir -p "$VIOLET_BUILD_DIR"

mv "$RED_DIR/build/red" "$VIOLET_BUILD_DIR/"
echo "'red' executable successfully moved to '$VIOLET_BUILD_DIR'."

echo "--- All Builds Completed ---"
echo "Violet server and Red client are now built and ready."
echo "Executables 'violet' and 'red' can be found in the '$VIOLET_BUILD_DIR' directory."