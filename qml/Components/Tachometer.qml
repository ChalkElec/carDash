import QtQuick 2.4
import QtGraphicalEffects 1.0

import "../Hardware"

Item {
    id: root

    RoundWidget {
        id: background

        anchors.fill: parent
        source: "../../resources/dashboard_bg.png"
        scaleSource: "../../resources/rpm_scale.png"
        value: Hardware.rpm / 1000.0
        maxValue: Hardware.maxRpm / 1000.0

        Column {
            anchors.centerIn: parent

            Text {
                id: transmissionText

                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                font { pixelSize: root.height / 11 }
                text: Hardware.gear
            }
        }

        Column {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: parent.height / 5

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                color: "#9d9d9d"
                font { pixelSize: root.height / 30 }
                text: "rpm\nx1000"
            }
        }

        // Coolant temperature bars
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height / 30
            anchors.horizontalCenter: parent.horizontalCenter

            Repeater {
                id: repeater

                model: 12

                property int coolantTempNormIndex: Math.round((repeater.count) * (Hardware.coolantTempValue / (Hardware.maxCoolantTempValue - Hardware.minCoolantTempValue)))

                Image {
                    property real relRealCenterIndex: index - (repeater.count + 1) / 2 + 1
                    property int relCenterIndex: relRealCenterIndex < 0 ? relRealCenterIndex - 0.5 : relRealCenterIndex + 0.5
                    property real angle: -relCenterIndex * 0.026 // 0.026 is the calculated angle of rotation of one item
                    property real scaleFactor: root.width / background.sourceSize.width
                    y: -Math.abs(relCenterIndex) * (width * Math.sin(Math.abs(angle)))
                    width: sourceSize.width * scaleFactor
                    height: sourceSize.height * scaleFactor
                    rotation: angle * 180 / Math.PI * 1.8 // 1.8 is magic number, without it the items aren't rotated enough. There must be an error in formulae
                    source: { // I tried to use ColorOverlay instead of textures of another color, but antialiasing didn't work for it
                        var color = repeater.coolantTempNormIndex >= 9 ? "red" : "green"
                        return repeater.coolantTempNormIndex > index ? "../../resources/fuel_full_" + color + ".png" : "../../resources/fuel_empty_" + color + ".png"
                    }
                    antialiasing: true
                }
            }
        }
    }
}
