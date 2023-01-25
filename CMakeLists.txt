cmake_minimum_required(VERSION 3.5)

project(RTTY_App VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS SerialPort)

#include_directories("C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT\\Include")
add_library(nivisa SHARED IMPORTED)
#target_link_libraries(nivisa INTERFACE "C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT\\Lib_x64\\msc\\visa64.lib")
set_target_properties(nivisa PROPERTIES
IMPORTED_IMPLIB "C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT\\Lib_x64\\msc\\visa64.lib"
#IMPORTED_LOCATION "C:\\Windows\\System32\\visa64.dll"
INTERFACE_INCLUDE_DIRECTORIES "C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT\\Include"
)
#"C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT\\Bin\\NiViEnet.dll"
#include_directories("C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT\\Include")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        rttyboard.cpp
        rttyboard.h
        rtty.h
        rtty.cpp
        siglentspecan.h
        siglentspecan.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RTTY_App
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET RTTY_App APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(RTTY_App SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(RTTY_App
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(RTTY_App PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(RTTY_App PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(RTTY_App PRIVATE nivisa)

set_target_properties(RTTY_App PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS RTTY_App
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RTTY_App)
endif()
