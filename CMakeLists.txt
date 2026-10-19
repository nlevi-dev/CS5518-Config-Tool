cmake_minimum_required(VERSION 3.16)
project(CS5518ConfigTool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/maintab.cpp
    src/lvdstab.cpp
    src/mipitab.cpp
    src/regbuilder.cpp
)

set(HEADERS
    src/mainwindow.h
    src/maintab.h
    src/lvdstab.h
    src/mipitab.h
    src/regbuilder.h
)

if(WIN32)
    add_executable(CS5518ConfigTool WIN32 ${SOURCES} ${HEADERS})
elseif(APPLE)
    add_executable(CS5518ConfigTool MACOSX_BUNDLE ${SOURCES} ${HEADERS})
else()
    add_executable(CS5518ConfigTool ${SOURCES} ${HEADERS})
endif()

target_link_libraries(CS5518ConfigTool PRIVATE Qt5::Widgets)
target_include_directories(CS5518ConfigTool PRIVATE src)
