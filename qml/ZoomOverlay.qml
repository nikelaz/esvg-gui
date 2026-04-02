import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property string zoomText: "100%"

    signal zoomIn()
    signal zoomOut()
    signal zoomTextEdited(int pct)

    radius: 6
    color: palette.window
    implicitWidth: row.implicitWidth + 12
    implicitHeight: 32

    RowLayout {
        id: row
        anchors.centerIn: parent
        spacing: 2

        Button {
            text: "\u2212"
            implicitWidth: 28
            implicitHeight: 26
            flat: true
            onClicked: root.zoomOut()
        }

        TextField {
            id: zoomField
            text: root.zoomText
            horizontalAlignment: Text.AlignHCenter
            implicitWidth: 68
            selectByMouse: true
            onAccepted: {
                let pct = parseInt(text.replace('%', '').trim())
                if (!isNaN(pct) && pct > 0)
                    root.zoomTextEdited(pct)
            }
        }

        Button {
            text: "+"
            implicitWidth: 28
            implicitHeight: 26
            flat: true
            onClicked: root.zoomIn()
        }
    }

    // Keep field text in sync when zoomText changes externally (unless focused)
    Binding {
        target: zoomField
        property: "text"
        value: root.zoomText
        when: !zoomField.activeFocus
    }
}
