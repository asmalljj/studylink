@echo off
chcp 65001 >nul
cls

cd /d "%~dp0"

echo ==============================
echo   C++ Team System Runner
echo ==============================
echo Current dir: %cd%
echo.

REM ---- 自动关闭正在运行的 main.exe ----
taskkill /f /im main.exe >nul 2>nul

where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERR] g++ not found. Please install MinGW or gcc.
    pause
    exit /b 1
)

if not exist data mkdir data
if not exist data\users.jsonl type nul > data\users.jsonl
if not exist data\applications.jsonl type nul > data\applications.jsonl
if not exist data\teams.jsonl type nul > data\teams.jsonl
if not exist data\messages.jsonl type nul > data\messages.jsonl
if not exist data\reviews.jsonl type nul > data\reviews.jsonl

echo ===== [1/2] Building (Compiling) =====
echo.

g++ -std=c++17 ^
main.cpp ^
auth/auth.cpp ^
storage/storage.cpp ^
storage/meta.cpp ^
storage/migrate.cpp ^
matching/matcher.cpp ^
matching/matching_config.cpp ^
team/application.cpp ^
team/message.cpp ^
team/review.cpp ^
team/team.cpp ^
analytics/analytics.cpp ^
-o main.exe 2>&1

if %errorlevel% neq 0 (
    echo.
    echo [ERR] Build failed. Please check error above.
    pause
    exit /b %errorlevel%
)

echo.
echo [OK] Build success!
echo.
echo ===== [2/2] Running =====
echo.
main.exe
pause
