@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

set CXX=g++
set CXXFLAGS=-std=c++17 -IHexaUdon/include -IHexaUdon/third_party
set CORE_SRC=HexaUdon/src/io/JsonReader.cpp HexaUdon/src/io/JsonWriter.cpp HexaUdon/src/map/Map.cpp HexaUdon/src/solver/ActionValidator.cpp HexaUdon/src/solver/PathFinder.cpp HexaUdon/src/solver/Solver.cpp

if "%1"=="runner" (
    echo [DANG BIEN DICH VA CHAY AGENT DEBUGGER RUNNER]...
    %CXX% %CXXFLAGS% HexaUdon/tests/test_runner.cpp %CORE_SRC% -o test_runner.exe
    if !errorlevel! equ 0 (
        type HexaUdon\demo_input.json | .\test_runner.exe
    ) else (
        echo [LOI] Bien dich that bai!
    )
    goto end
)

if "%1"=="test" (
    echo [DANG BIEN DICH VA CHAY UNIT TESTS]...
    %CXX% %CXXFLAGS% HexaUdon/tests/test_all.cpp %CORE_SRC% -o test_all.exe
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
    if exist HexaUdon\HexaUdon.exe del HexaUdon\HexaUdon.exe
    if exist HexaUdon\test_all.exe del HexaUdon\test_all.exe
    if exist HexaUdon\test_runner.exe del HexaUdon\test_runner.exe
    echo Hoan tat!
    goto end
)

echo [DANG BIEN DICH CHUONG TRINH CHINH] HexaUdon.exe...
%CXX% %CXXFLAGS% HexaUdon/src/main.cpp %CORE_SRC% -o HexaUdon.exe
if !errorlevel! equ 0 (
    echo [THANH CONG] Da tao file HexaUdon.exe!
) else (
    echo [LOI] Bien dich that bai!
)

:end
