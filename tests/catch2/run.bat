@echo off

:: copy ".\external\lib\*.dll"  ".\build\"

:: set enviroment vars and requred stuff for the msvc compiler
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: Set paths for include directories, libraries, and output directories
::set INCLUDE_DIRS=/I..\external\inc\ /I..\external\inc\IMGUI\ /I..\inc\
::set LIBRARY_DIRS=/LIBPATH:..\external\lib\
set LIBRARIES=user32.lib gdi32.lib shell32.lib kernel32.lib
set SRC_FILES=catch_amalgamated.cpp 
set C_FLAGS=/c /EHsc /O2 /W4 /MD /nologo /std:c++17 /Fo"catch2.obj"
set L_FLAGS=/SUBSYSTEM:WINDOWS
set I_FLAGS=/I.

cl %C_FLAGS% %I_FLAGS% %INCLUDE_DIRS% %SRC_FILES% /link %LIBRARY_DIRS% %LIBRARIES% %LFLAGS%
lib /OUT:catch2.lib catch2.obj