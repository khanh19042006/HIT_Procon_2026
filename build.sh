#!/bin/bash
CXX=g++
CXXFLAGS="-std=c++17 -Iinclude -Ithird_party"
CORE_SRC="src/io/JsonReader.cpp src/io/JsonWriter.cpp src/map/Map.cpp src/solver/ActionValidator.cpp src/solver/PathFinder.cpp src/solver/Solver.cpp"

if [ "$1" == "test" ]; then
    echo "[BUILDING & RUNNING UNIT TESTS]..."
    $CXX $CXXFLAGS tests/test_all.cpp $CORE_SRC -o test_all
    if [ $? -eq 0 ]; then
        ./test_all
    fi
elif [ "$1" == "clean" ]; then
    echo "[CLEANING EXECUTABLES]..."
    rm -f HexaUdon test_all HexaUdon.exe test_all.exe
    echo "Done!"
else
    echo "[BUILDING MAIN EXECUTABLE] HexaUdon..."
    $CXX $CXXFLAGS src/main.cpp $CORE_SRC -o HexaUdon
    if [ $? -eq 0 ]; then
        echo "[SUCCESS] HexaUdon created successfully!"
    fi
fi
