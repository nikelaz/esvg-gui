cmake_minimum_required(VERSION 3.16)

project(esvg-gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Quick QuickControls2 Svg SvgWidgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Quick QuickControls2 Svg SvgWidgets Concurrent)

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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(esvg-gui
        MANUAL_FINALIZATION
        main.cpp
        optimizationcontroller.h optimizationcontroller.cpp
        svgcompareitem.h svgcompareitem.cpp
        exportcontroller.h exportcontroller.cpp
    )
else()
    if(ANDROID)
        add_library(esvg-gui SHARED
            main.cpp
            optimizationcontroller.h optimizationcontroller.cpp
            svgcompareitem.h svgcompareitem.cpp
            exportcontroller.h exportcontroller.cpp
        )
    else()
        add_executable(esvg-gui
            main.cpp
            optimizationcontroller.h optimizationcontroller.cpp
            svgcompareitem.h svgcompareitem.cpp
            exportcontroller.h exportcontroller.cpp
        )
    endif()
endif()

qt_add_resources(esvg-gui "svgs"
    PREFIX "/"
    FILES test-svg.svg
)

qt_add_qml_module(esvg-gui
    URI EsvgGui
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/VisualTab.qml
        qml/CodeTab.qml
        qml/Sidebar.qml
        qml/SidebarPanel.qml
        qml/PluginsPanel.qml
        qml/OptimizePanel.qml
        qml/ColorsPanel.qml
        qml/ExportPanel.qml
        qml/ExportDialog.qml
        qml/ZoomOverlay.qml
)

target_include_directories(esvg-gui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/esvg-rs/include"
)

target_link_libraries(esvg-gui PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    esvg_rs
    ws2_32 ntdll bcrypt userenv
)

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
