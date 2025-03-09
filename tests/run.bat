@echo off

mkdir build
:: copy ".\external\lib\*.dll"  ".\build\"

:: set enviroment vars and requred stuff for the msvc compiler
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: Set paths for include directories, libraries, and output directories
set INCLUDE_DIRS=/I..\catch2\
set LIBRARY_DIRS=/LIBPATH:..\catch2
set LIBRARIES=user32.lib gdi32.lib shell32.lib kernel32.lib catch2.lib
set SRC_FILES=..\test.cpp 
set C_FLAGS=/Zi /EHsc /W4 /MD /nologo /std:c++17 
set L_FLAGS=/SUBSYSTEM:WINDOWS

pushd .\build
cl  %C_FLAGS% %INCLUDE_DIRS% %SRC_FILES% /link %LIBRARY_DIRS% %LIBRARIES% %LFLAGS%
.\test.exe
popd