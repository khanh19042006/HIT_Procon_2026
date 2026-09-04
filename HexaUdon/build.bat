@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

set CXX=g++
set CXXFLAGS=-std=c++17 -Iinclude -Ithird_party

set CORE_SRC=src/io/ApiClient.cpp src/io/JsonReader.cpp src/io/JsonWriter.cpp src/map/Map.cpp src/solver/ActionValidator.cpp src/solver/PathFinder.cpp src/solver/Solver.cpp

if "%1"=="api" (
    echo [BUILDING SOLVER API]...
    %CXX% %CXXFLAGS% src/solver_api_main.cpp src/solver/SolverApi.cpp %CORE_SRC% -o solver_api.exe
    goto end
)

if "%1"=="runner" (
    echo [ĐANG BIÊN DỊCH VÀ CHẠY AGENT DEBUGGER RUNNER]...
    %CXX% %CXXFLAGS% tests/test_runner.cpp %CORE_SRC% -o test_runner.exe
    if !errorlevel! equ 0 (
        type demo_input.json | .\test_runner.exe
    ) else (
        echo [LỖI] Biên dịch thất bại!
    )
    goto end
)

if "%1"=="test" (
    echo [ĐANG BIÊN DỊCH VÀ CHẠY UNIT TESTS]...
    %CXX% %CXXFLAGS% tests/test_all.cpp %CORE_SRC% -o test_all.exe
    if !errorlevel! equ 0 (
        echo [ĐANG CHẠY CÁC BÀI TEST]:
        .\test_all.exe
    ) else (
        echo [LỖI] Biên dịch thất bại!
    )
    goto end
)

if "%1"=="clean" (
    echo [ĐANG DỌN DẸP CÁC FILE EXECUTABLE]...
    if exist HexaUdon.exe del HexaUdon.exe
    if exist test_all.exe del test_all.exe
    if exist test_runner.exe del test_runner.exe
    echo Hoàn tất!
    goto end
)

echo [ĐANG BIÊN DỊCH CHƯƠNG TRÌNH CHÍNH] HexaUdon.exe...
%CXX% %CXXFLAGS% src/main.cpp %CORE_SRC% -o HexaUdon.exe
if !errorlevel! equ 0 (
    echo [THÀNH CÔNG] Đã tạo file HexaUdon.exe!
) else (
    echo [LỖI] Biên dịch thất bại!
)

:end
