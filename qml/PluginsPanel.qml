import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 4
    leftPadding: 10
    rightPadding: 10
    topPadding: 8
    bottomPadding: 8

    Repeater {
        model: controller.pluginCount()

        delegate: ColumnLayout {
            spacing: 2
            Layout.fillWidth: true

            CheckBox {
                id: pluginCheck
                text: controller.pluginName(index)
                checked: controller.pluginStates[index]
                enabled: !controller.optimizing
                onToggled: controller.setPluginEnabled(index, checked)
            }

            // Precision row — only visible for Number Precision plugin (index 4)
            RowLayout {
                visible: controller.isNumberPrecisionPlugin(index)
                enabled: controller.pluginStates[4] && !controller.optimizing
                leftPadding: 20
                spacing: 6

                Label { text: qsTr("Precision:") }

                Slider {
                    id: precSlider
                    from: 1; to: 10; stepSize: 1
                    value: controller.precision
                    Layout.fillWidth: true
                    onMoved: controller.precision = value
                }

                Label {
                    text: precSlider.value.toFixed(0)
                    Layout.minimumWidth: 16
                }
            }
        }
    }
}
