# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(windowlayout LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

set(POPPLER_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/inc/poppler/qt6")
set(POPPLER_LIB_DIR "${CMAKE_SOURCE_DIR}/external/lib")
set(POPPLER_LIBRARY "${POPPLER_LIB_DIR}/poppler-qt6.lib")  

include_directories(
    ${CMAKE_SOURCE_DIR}/inc
    ${POPPLER_INCLUDE_DIR}
)

file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB INC_FILES "${CMAKE_SOURCE_DIR}/inc/*.h" "${CMAKE_SOURCE_DIR}/inc/*.hpp")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent)

qt_standard_project_setup()

qt_add_executable(windowlayout
    ${SRC_FILES}
    ${INC_FILES}
)

set_target_properties(windowlayout PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_include_directories(windowlayout PRIVATE 
    ${CMAKE_SOURCE_DIR}/inc
    ${VCPKG_ROOT}/include
)

link_directories(${VCPKG_ROOT}/lib)

target_link_libraries(windowlayout PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    ${POPPLER_LIBRARY}
)

install(TARGETS windowlayout
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET windowlayout
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

if(WIN32)
    # Gather all DLLs in the Poppler library directory
    file(GLOB POPPLER_DLLS "${POPPLER_LIB_DIR}/*.dll")

    # Loop through each DLL and copy it to the target directory
    foreach(DLL ${POPPLER_DLLS})
        add_custom_command(TARGET windowlayout POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL}
                $<TARGET_FILE_DIR:windowlayout>
            COMMENT "Copying ${DLL} to the executable directory..."
        )
    endforeach()
endif()