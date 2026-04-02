import QtQuick
import QtQuick.Controls
import EsvgGui 1.0

Item {
    id: root

    SvgCompareItem {
        id: compareItem
        anchors.fill: parent
        svgA: controller.originalSvgBytes
        svgB: controller.optimizedSvgBytes
        splitRatio: sliderHandle.x / Math.max(1, root.width)
        zoom: compareItem.zoom
        panOffset: compareItem.panOffset
    }

    // Invisible drag target wider than the visual line for easy grabbing
    Item {
        id: sliderHandle
        x: root.width / 2
        y: 0
        width: 12
        height: root.height
        z: 20

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeHorCursor
            drag.target: sliderHandle
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: root.width
        }
    }

    // Pan + wheel-zoom area (lower z than slider)
    MouseArea {
        id: panArea
        anchors.fill: parent
        z: 5
        acceptedButtons: Qt.LeftButton

        // Disable pan when the cursor is near the slider handle
        property bool nearSlider: Math.abs(mouseX - sliderHandle.x) < 6
        enabled: !nearSlider

        property point lastPos

        onPressed: (mouse) => {
            lastPos = Qt.point(mouse.x, mouse.y)
        }

        onPositionChanged: (mouse) => {
            if (pressed) {
                let dx = mouse.x - lastPos.x
                let dy = mouse.y - lastPos.y
                compareItem.setPanOffset(Qt.point(
                    compareItem.panOffset.x + dx,
                    compareItem.panOffset.y + dy))
                lastPos = Qt.point(mouse.x, mouse.y)
            }
        }

        onWheel: (wheel) => {
            let factor = wheel.angleDelta.y > 0 ? 1.15 : (1.0 / 1.15)
            compareItem.zoomAroundPoint(wheel.x, wheel.y, factor)
        }

        cursorShape: containsPress ? Qt.ClosedHandCursor : Qt.ArrowCursor
    }

    // Cursor changes near slider (overlay on panArea)
    HoverHandler {
        id: sliderHover
        target: null
        onPointChanged: {
            if (Math.abs(point.position.x - sliderHandle.x) < 6)
                root.cursorShape = Qt.SizeHorCursor
            else
                root.cursorShape = Qt.ArrowCursor
        }
    }

    ZoomOverlay {
        id: zoomOverlay
        anchors { bottom: parent.bottom; right: parent.right; margins: 8 }
        z: 30
        zoomText: compareItem.zoomText
        onZoomIn:          compareItem.zoomIn()
        onZoomOut:         compareItem.zoomOut()
        onZoomTextEdited:  (pct) => compareItem.setZoomPercent(pct)
    }

    // Loading indicator while optimizing
    BusyIndicator {
        anchors.centerIn: parent
        running: controller.optimizing
        z: 40
    }
}
