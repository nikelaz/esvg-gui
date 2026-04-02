import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1100
    height: 700
    visible: true
    title: controller.fileLoaded
           ? controller.svgPath + " \u2014 esvg"
           : "esvg"

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action {
                text: qsTr("&Open\u2026")
                shortcut: "Ctrl+O"
                onTriggered: controller.openFileDialog()
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Export\u2026")
                shortcut: "Ctrl+Shift+E"
                enabled: controller.fileLoaded
                onTriggered: exportWindow.show()
            }
        }
        Menu {
            title: qsTr("&View")
            MenuItem {
                text: qsTr("Plugins")
                checkable: true
                checked: true
                onCheckedChanged: sidebar.pluginsVisible = checked
            }
            MenuItem {
                text: qsTr("Optimization")
                checkable: true
                checked: true
                onCheckedChanged: sidebar.optimizationVisible = checked
            }
            MenuItem {
                text: qsTr("Colors")
                checkable: true
                checked: true
                onCheckedChanged: sidebar.colorsVisible = checked
            }
            MenuItem {
                text: qsTr("Quick Export")
                checkable: true
                checked: true
                onCheckedChanged: sidebar.exportVisible = checked
            }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        Item {
            SplitView.fillWidth: true

            TabBar {
                id: tabBar
                anchors { top: parent.top; left: parent.left; right: parent.right }

                TabButton { text: qsTr("Visual") }
                TabButton { text: qsTr("Code") }
            }

            StackLayout {
                currentIndex: tabBar.currentIndex
                anchors {
                    top: tabBar.bottom
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }

                VisualTab { id: visualTab }
                CodeTab {}
            }
        }

        Sidebar {
            id: sidebar
            SplitView.preferredWidth: 270
            SplitView.minimumWidth: 220
        }
    }

    ExportDialog { id: exportWindow }
}
