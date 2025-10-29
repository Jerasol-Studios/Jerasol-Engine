@echo off
setlocal enabledelayedexpansion

echo ======================================
echo   Building and Running engine-ui
echo ======================================
echo.

:: --- Configuration ---
set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%build
set MINGW_DIR=%PROJECT_DIR%mingw64
set SDL_DIR=%PROJECT_DIR%thirdparty\SDL2
set CMAKE_EXE=%MINGW_DIR%\bin\cmake.exe
set MAKE_EXE=%MINGW_DIR%\bin\mingw32-make.exe

:: --- Build Type Switch ---
set MODE=%1
if "%MODE%"=="" set MODE=CONSOLE

echo Using build mode: %MODE%
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

:: --- Clean old build files ---
echo Cleaning old build files...
del /q * >nul 2>&1

:: --- Run CMake ---
echo Configuring project with CMake...
"%CMAKE_EXE%" -G "MinGW Makefiles" ^
    -DCMAKE_C_COMPILER="%MINGW_DIR%\bin\gcc.exe" ^
    -DCMAKE_CXX_COMPILER="%MINGW_DIR%\bin\g++.exe" ^
    -DCMAKE_MAKE_PROGRAM="%MAKE_EXE%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DAPP_MODE=%MODE% ^
    ..

if %errorlevel% neq 0 (
    echo [ERROR] Configuration failed.
    pause
    exit /b %errorlevel%
)

:: --- Build ---
echo Building project...
"%MAKE_EXE%"
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %errorlevel%
)

:: --- Copy dlls ---
echo ======================================
echo copying dlls
echo ======================================
copy /Y "%SDL_DIR%\lib\SDL2.dll" "%BUILD_DIR%\" >nul
echo --SDL2.dll Copyed!
copy /Y "%MINGW_DIR%\bin\libgcc_s_seh-1.dll" "%BUILD_DIR%\" >nul 
echo --libgcc_s_seh-1.dll Copyed!
copy /Y "%MINGW_DIR%\bin\libstdc++-6.dll" "%BUILD_DIR%\" >nul 
echo --ibstdc++-6.dll Copyed!
copy /Y "%MINGW_DIR%\bin\libwinpthread-1.dll" "%BUILD_DIR%\" >nul 
echo --libwinpthread-1.dll Copyed!
echo ======================================
echo dlls have succesfully been copyed
echo ======================================

:: --- Run the program ---
echo [SUCCESS] Build completed. Launching engine-ui.exe...
echo ======================================
echo.

if "%MODE%"=="CONSOLE" (
    "%BUILD_DIR%\engine-ui.exe"
) else (
    start "" "%BUILD_DIR%\engine-ui.exe"
)

echo.
pause
