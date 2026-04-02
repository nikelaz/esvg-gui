import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

SplitView {
    orientation: Qt.Horizontal

    Item {
        SplitView.fillWidth: true

        Label {
            id: lblBefore
            text: qsTr("Before")
            leftPadding: 6
            topPadding: 4
            bottomPadding: 4
        }

        ScrollView {
            anchors {
                top: lblBefore.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            clip: true

            TextArea {
                text: controller.originalSvgText
                readOnly: true
                font.family: "Courier New"
                font.pointSize: 9
                wrapMode: TextArea.NoWrap
                selectByMouse: true
            }
        }
    }

    Item {
        SplitView.fillWidth: true

        Label {
            id: lblAfter
            text: qsTr("After")
            leftPadding: 6
            topPadding: 4
            bottomPadding: 4
        }

        ScrollView {
            anchors {
                top: lblAfter.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            clip: true

            TextArea {
                text: controller.optimizedSvgText
                readOnly: true
                font.family: "Courier New"
                font.pointSize: 9
                wrapMode: TextArea.NoWrap
                selectByMouse: true
            }
        }
    }
}
