@echo off

//SET /P MODE= choose and type GUI or CONSOLE 
MODE = GUI

echo ======================================
echo Running %MODE% build 

build_and_run.bat %MODE%