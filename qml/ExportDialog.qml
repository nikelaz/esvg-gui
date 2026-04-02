import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import EsvgGui 1.0

Window {
    id: exportWin
    title: qsTr("Export")
    width: 900
    height: 600
    modality: Qt.ApplicationModal

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left: SVG preview (show optimized SVG full-width, no split)
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            SvgCompareItem {
                id: previewItem
                anchors.fill: parent
                svgA: controller.optimizedSvgBytes
                svgB: controller.optimizedSvgBytes
                splitRatio: 1.0
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton

                property point lastPos

                onPressed: (mouse) => {
                    lastPos = Qt.point(mouse.x, mouse.y)
                }
                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let dx = mouse.x - lastPos.x
                        let dy = mouse.y - lastPos.y
                        previewItem.setPanOffset(Qt.point(
                            previewItem.panOffset.x + dx,
                            previewItem.panOffset.y + dy))
                        lastPos = Qt.point(mouse.x, mouse.y)
                    }
                }
                onWheel: (wheel) => {
                    let factor = wheel.angleDelta.y > 0 ? 1.15 : (1.0 / 1.15)
                    previewItem.zoomAroundPoint(wheel.x, wheel.y, factor)
                }
                cursorShape: containsPress ? Qt.ClosedHandCursor : Qt.ArrowCursor
            }
        }

        // Right: export sidebar (240px)
        ColumnLayout {
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            spacing: 6
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 12
            Layout.bottomMargin: 12

            Label { text: qsTr("Format") }

            ButtonGroup { id: fmtGroup }

            Repeater {
                model: exportCtrl.formatNames()
                RadioButton {
                    text: modelData
                    ButtonGroup.group: fmtGroup
                    checked: index === 0
                    onCheckedChanged: if (checked) exportCtrl.formatIndex = index
                }
            }

            Item { height: 8 }

            CheckBox {
                text: qsTr("Include viewBox")
                checked: exportCtrl.includeViewBox
                onToggled: exportCtrl.includeViewBox = checked
            }

            CheckBox {
                id: customSizeCheck
                text: qsTr("Custom size")
                checked: exportCtrl.customSize
                onToggled: exportCtrl.customSize = checked
            }

            GridLayout {
                columns: 2
                visible: customSizeCheck.checked

                Label { text: "W:" }
                SpinBox {
                    from: 1; to: 99999
                    value: exportCtrl.exportWidth
                    editable: true
                    onValueModified: exportCtrl.exportWidth = value
                }

                Label { text: "H:" }
                SpinBox {
                    from: 1; to: 99999
                    value: exportCtrl.exportHeight
                    editable: true
                    onValueModified: exportCtrl.exportHeight = value
                }
            }

            Item { Layout.fillHeight: true }

            Button {
                text: qsTr("Export")
                Layout.fillWidth: true
                onClicked: {
                    exportCtrl.doExport()
                    exportWin.close()
                }
            }
        }
    }
}
