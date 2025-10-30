@echo off
setlocal

REM Choose GUI or CONSOLE mode (default GUI)
if "%~1"=="" (
  set APP_MODE=GUI
) else (
  set APP_MODE=%~1
)

echo ============================================
echo   Building and Running engine-ui (Mode: %APP_MODE%)
echo ============================================

REM Ensure we use the provided mingw64 tools
set MINGW_ROOT=%~dp0mingw64
set CMAKE_EXE=%MINGW_ROOT%\bin\cmake.exe
set MAKE_EXE=%MINGW_ROOT%\bin\mingw32-make.exe

if not exist "%CMAKE_EXE%" (
  echo ERROR: cmake.exe not found at %CMAKE_EXE%
  echo Please place cmake.exe in mingw64\bin or install system cmake.
  pause
  exit /b 1
)

if not exist "%MAKE_EXE%" (
  echo ERROR: mingw32-make.exe not found at %MAKE_EXE%
  pause
  exit /b 1
)

REM Clean build folder
cd /d "%~dp0build" || (echo Cannot cd to build folder & pause & exit /b 1)
del * /Q >nul 2>nul

REM Configure
"%CMAKE_EXE%" -G "MinGW Makefiles" ^
  -DCMAKE_C_COMPILER="%MINGW_ROOT%/bin/gcc.exe" ^
  -DCMAKE_CXX_COMPILER="%MINGW_ROOT%/bin/g++.exe" ^
  -DCMAKE_MAKE_PROGRAM="%MAKE_EXE%" ^
  -DAPP_MODE=%APP_MODE% ^
  ..

if errorlevel 1 (
  echo [ERROR] CMake configuration failed.
  pause
  exit /b 1
)

REM Build
"%MAKE_EXE%" -j 4
if errorlevel 1 (
  echo [ERROR] Build failed.
  pause
  exit /b 1
)

REM Copy SDL2 DLL (if exists) to output
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


REM Run the built application
echo Launching...
cd "%cd%\output"
if /I "%APP_MODE%"=="GUI" (
  REM Start without console inherit (WIN32 exe)
  start "" "engine-ui.exe"
) else (
  engine-ui.exe
)

endlocal