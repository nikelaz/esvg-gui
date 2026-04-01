cmake_minimum_required(VERSION 3.16)

project(esvg-gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGLWidgets Svg SvgWidgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets Svg SvgWidgets Concurrent)

# ---- esvg-rs static library (Rust) ----
find_program(CARGO_EXECUTABLE cargo
    HINTS "$ENV{USERPROFILE}/.cargo/bin" "$ENV{HOME}/.cargo/bin"
    REQUIRED
)

set(ESVG_RS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/esvg-rs")
set(ESVG_RS_TARGET "x86_64-pc-windows-gnu")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(ESVG_RS_BUILD_DIR "${ESVG_RS_DIR}/target/${ESVG_RS_TARGET}/release")
    set(ESVG_RS_CARGO_FLAG "--release")
else()
    set(ESVG_RS_BUILD_DIR "${ESVG_RS_DIR}/target/${ESVG_RS_TARGET}/debug")
    set(ESVG_RS_CARGO_FLAG "")
endif()

set(ESVG_RS_LIB "${ESVG_RS_BUILD_DIR}/libesvg_rs.a")

file(GLOB_RECURSE ESVG_RS_SOURCES "${ESVG_RS_DIR}/src/*.rs" "${ESVG_RS_DIR}/Cargo.toml")

add_custom_command(
    OUTPUT  "${ESVG_RS_LIB}"
    COMMAND "${CARGO_EXECUTABLE}" build --target ${ESVG_RS_TARGET} --features ffi ${ESVG_RS_CARGO_FLAG}
    WORKING_DIRECTORY "${ESVG_RS_DIR}"
    DEPENDS ${ESVG_RS_SOURCES}
    COMMENT "Building esvg-rs (${ESVG_RS_TARGET})"
    VERBATIM
)
add_custom_target(esvg_rs_build DEPENDS "${ESVG_RS_LIB}")

add_library(esvg_rs STATIC IMPORTED GLOBAL)
set_target_properties(esvg_rs PROPERTIES IMPORTED_LOCATION "${ESVG_RS_LIB}")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        sidebarpanel.cpp
        sidebarpanel.h
        exportdialog.h
        exportdialog.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(esvg-gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ESVGGui APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(esvg-gui SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(esvg-gui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

qt_add_resources(esvg-gui "svgs"
    PREFIX "/"
    FILES test-svg.svg
)

target_include_directories(esvg-gui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/esvg-rs/include"
)

target_link_libraries(esvg-gui PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    esvg_rs
    ws2_32 ntdll bcrypt userenv
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.esvg-gui)
endif()
set_target_properties(esvg-gui PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS esvg-gui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(esvg-gui)
endif()

add_dependencies(esvg-gui esvg_rs_build)
