# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**esvg-gui** is a Qt6 C++ desktop application for viewing and comparing SVG files. It features a side-by-side slider comparison view, pan/zoom, and OpenGL-accelerated rendering.

## Build

This project uses CMake with Qt6. Open in Qt Creator and build via the IDE, or from the command line:

```bash
cmake -B build/my-build -DCMAKE_PREFIX_PATH=/path/to/Qt6
cmake --build build/my-build
```

Required Qt modules: `Widgets`, `OpenGLWidgets`, `Svg`, `SvgWidgets`.

There are no tests in this project currently.

## Architecture

The app is a single-window Qt application. All logic lives in `MainWindow` (`mainwindow.h` / `mainwindow.cpp`); the UI layout is defined in `mainwindow.ui`.

### Key design decisions in `MainWindow`

- **Dual-SVG comparison**: Two `QGraphicsSvgItem` instances (`m_svgItem` = A, `m_svgItemB` = B) are loaded into the same `QGraphicsScene`. Each item is parented to a `QGraphicsRectItem` clip container (`m_clipContainerA`, `m_clipContainer`) that uses `ItemClipsChildrenToShape`. The clip rects are updated in `updateComparisonClip()` whenever the slider moves.

- **Slider**: `m_sliderViewX` is the slider position in **viewport coordinates**. It is converted to scene coordinates inside `updateComparisonClip()` via `mapToScene`. The visible `m_sliderHandle` widget is a thin 2px `QWidget` overlaid on `ui->svgView`; it is repositioned by `repositionSliderHandle()`.

- **Zoom**: Zoom is expressed as a percentage relative to `m_fitScale` (the scale at which the SVG fits the view). `applyZoom()` and the wheel handler both zoom around the cursor/center point by adjusting scroll bars after scaling to keep the focal point stationary.

- **Pan**: Middle-mouse-button-style left-click-drag panning is implemented in the `eventFilter` on `ui->svgView->viewport()`. The same event filter handles wheel zoom, slider dragging, and cursor changes.

- **OpenGL viewport**: `QOpenGLWidget` with 8x MSAA is set as the viewport for `ui->svgView` to get smooth SVG rendering.

- **Embedded SVG resource**: `test-svg.svg` is compiled into the binary via Qt's resource system (`qt_add_resources` in CMakeLists.txt) and loaded as `":/test-svg.svg"` on startup.

### UI layout (`mainwindow.ui`)

- `QSplitter` (horizontal) splits the window into `svgView` (`QGraphicsView`) on the left and `sidebarWidget` on the right (min width 260px).
- The sidebar contains a "Plugins" label and placeholder checkboxes (`checkBox_2` through `checkBox_6`) — these are not yet wired up.
- `menuFile` → `actionOpen` (Ctrl+O) triggers `MainWindow::openFile()`.

### Qt AUTOUIC / AUTOMOC

CMake uses `CMAKE_AUTOUIC`, `CMAKE_AUTOMOC`, and `CMAKE_AUTORCC`. The generated header `ui_mainwindow.h` is included as `"./ui_mainwindow.h"` in `mainwindow.cpp`.
