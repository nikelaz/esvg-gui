import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root

    property bool pluginsVisible:      true
    property bool optimizationVisible: true
    property bool colorsVisible:       true
    property bool exportVisible:       true

    contentWidth: availableWidth
    clip: true

    ColumnLayout {
        width: root.availableWidth
        spacing: 0

        SidebarPanel {
            id: pluginsPanel
            title: qsTr("Plugins")
            visible: root.pluginsVisible
            contentItem: PluginsPanel {}
        }

        SidebarPanel {
            id: optimizePanel
            title: qsTr("Optimization")
            visible: root.optimizationVisible
            contentItem: OptimizePanel {}
        }

        SidebarPanel {
            id: colorsPanel
            title: qsTr("Colors")
            visible: root.colorsVisible
            contentItem: ColorsPanel {}
        }

        SidebarPanel {
            id: exportPanel
            title: qsTr("Quick Export")
            visible: root.exportVisible
            contentItem: ExportPanel {}
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: 8
        }
    }
}
