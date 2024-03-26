@echo off
if not defined vcvars call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set vcvars=true
if not exist bin mkdir bin
cl src\main.cpp user32.lib gdi32.lib /Fobin\main /Febin\main
bin\main.exe