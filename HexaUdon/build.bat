@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

set CXX=g++
set CXXFLAGS=-std=c++17 -O2 -Iinclude -Ithird_party

set CORE_SRC=src/io/JsonReader.cpp src/io/JsonWriter.cpp src/map/Map.cpp src/solver/ActionValidator.cpp src/solver/PathFinder.cpp src/solver/Solver.cpp src/solver/SpotAssigner.cpp

if "%1"=="runner" (
    echo [DANG BIEN DICH VA CHAY AGENT DEBUGGER RUNNER]...
    %CXX% %CXXFLAGS% tests/test_runner.cpp %CORE_SRC% -o test_runner.exe
    if !errorlevel! equ 0 (
        type demo_input.json | .\test_runner.exe
    ) else (
        echo [LOI] Bien dich that bai!
    )
    goto end
)

if "%1"=="test" (
    echo [DANG BIEN DICH VA CHAY UNIT TESTS]...
    %CXX% %CXXFLAGS% tests/test_all.cpp %CORE_SRC% -o test_all.exe
    if !errorlevel! equ 0 (
        echo [DANG CHAY CAC BAI TEST]:
        .\test_all.exe
    ) else (
        echo [LOI] Bien dich that bai!
    )
    goto end
)

if "%1"=="clean" (
    echo [DANG DON DEP CAC FILE EXECUTABLE]...
    if exist HexaUdon.exe del HexaUdon.exe
    if exist test_all.exe del test_all.exe
    if exist test_runner.exe del test_runner.exe
    echo Hoan tat!
    goto end
)

echo [DANG BIEN DICH CHUONG TRINH CHINH] HexaUdon.exe...
%CXX% %CXXFLAGS% src/main.cpp %CORE_SRC% -o HexaUdon.exe
if !errorlevel! equ 0 (
    echo [THANH CONG] Da tao file HexaUdon.exe!
) else (
    echo [LOI] Bien dich that bai!
)

:end
