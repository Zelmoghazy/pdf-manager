cmake_minimum_required(VERSION 3.10)
project(StaticQtApp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.0/6.8.2_Static_msvc") 
add_definitions(-DQT_STATIC) 

# use the static runtime library
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)

add_executable(StaticQtApp main.cpp)

target_link_libraries(StaticQtApp PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)

if(WIN32)
    find_library(QT_QWINDOWS_PLUGIN qwindows PATHS "${CMAKE_PREFIX_PATH}/plugins/platforms")
    target_link_libraries(StaticQtApp PRIVATE ${QT_QWINDOWS_PLUGIN})
endif()