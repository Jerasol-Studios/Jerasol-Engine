@echo off

SET /P MODE= choose and type GUI or CONSOLE 

echo ======================================
echo Running %MODE% build 

build_and_run.bat %MODE%