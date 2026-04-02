import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root

    property string title: ""
    property alias contentItem: loader.sourceComponent

    spacing: 0
    Layout.fillWidth: true

    // Header bar
    Rectangle {
        Layout.fillWidth: true
        height: 26
        color: palette.button

        Label {
            text: root.title
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: 8
            }
            font.weight: Font.Medium
        }
    }

    // Content
    Loader {
        id: loader
        Layout.fillWidth: true
    }
}
