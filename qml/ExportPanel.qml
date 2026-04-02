import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 8
    leftPadding: 10
    rightPadding: 10
    topPadding: 8
    bottomPadding: 8

    Label { text: qsTr("Format / Preset:") }

    ComboBox {
        model: ["SVG", "PNG", "PDF"]
        Layout.fillWidth: true
    }

    Button {
        text: qsTr("Export")
        Layout.fillWidth: true
        enabled: controller.fileLoaded
        onClicked: exportWindow.show()
    }
}
