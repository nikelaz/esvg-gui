import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GridLayout {
    columns: 2
    columnSpacing: 8
    rowSpacing: 4
    leftPadding: 10
    rightPadding: 10
    topPadding: 8
    bottomPadding: 8

    Label { text: qsTr("Original size:") }
    Label { text: controller.origSize.length > 0 ? controller.origSize : "\u2014" }

    Label { text: qsTr("Optimized size:") }
    Label { text: controller.optSize.length > 0 ? controller.optSize : "\u2014" }

    Label { text: qsTr("Original gzip:") }
    Label { text: controller.origGzip.length > 0 ? controller.origGzip : "\u2014" }

    Label { text: qsTr("Optimized gzip:") }
    Label { text: controller.optGzip.length > 0 ? controller.optGzip : "\u2014" }
}
