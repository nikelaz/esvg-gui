import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 4
    leftPadding: 8
    rightPadding: 8
    topPadding: 6
    bottomPadding: 6

    // Placeholder colors until SVG color parsing is wired up
    Repeater {
        model: ["#C0392B", "#2980B9", "#27AE60", "#F39C12"]

        delegate: RowLayout {
            id: colorRow
            spacing: 6

            property color currentColor: modelData

            // Sync color from controller signal
            Connections {
                target: controller
                function onColorChanged(idx, color) {
                    if (idx === index) colorRow.currentColor = color
                }
            }

            Rectangle {
                width: 24
                height: 24
                color: colorRow.currentColor
                border.color: palette.mid
                radius: 2

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: controller.pickColor(index, colorRow.currentColor)
                }
            }

            TextField {
                id: hexField
                text: colorRow.currentColor.toString().toUpperCase()
                implicitWidth: 80
                maximumLength: 7
                validator: RegularExpressionValidator {
                    regularExpression: /^#[0-9A-Fa-f]{0,6}$/
                }
                onEditingFinished: {
                    if (text.length === 7) {
                        let c = Qt.color(text)
                        if (c !== Qt.color("transparent"))
                            colorRow.currentColor = c
                    }
                }
            }

            // Update hex field when color changes via swatch/controller
            Binding {
                target: hexField
                property: "text"
                value: colorRow.currentColor.toString().toUpperCase()
                when: !hexField.activeFocus
            }
        }
    }
}
