import QtQuick 2.4

import "../Hardware"

Item {
    id: root

    RoundWidget {
        id: background

        anchors.fill: parent
        source: "../../resources/dashboard_bg.png"
        scaleSource: "../../resources/speed_scale.png"
        value: Hardware.speed
        maxValue: Hardware.maxSpeed

        Column {
            anchors.centerIn: parent
            Text {
                id: speedText
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
                font { pixelSize: root.height / 11 }
                text: Math.round(Hardware.speed)
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#9d9d9d"
                font { pixelSize: root.height / 30 }
                text: "km/h"
            }
        }

        // Fuel bars
        Row {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height / 30
            anchors.horizontalCenter: parent.horizontalCenter
            Repeater {
                id: repeater

                model: 12
                property int fuelNormIndex: Math.round((repeater.count) * (Hardware.fuelValue / (Hardware.maxFuelValue - Hardware.minFuelValue)))

                Image {
                    property real relRealCenterIndex: index - (repeater.count + 1) / 2 + 1
                    property int relCenterIndex: relRealCenterIndex < 0 ? relRealCenterIndex - 0.5 : relRealCenterIndex + 0.5
                    property real angle: -relCenterIndex * 0.026 // 0.026 is the calculated angle of rotation of one item
                    property real scaleFactor: root.width / background.sourceSize.width
                    y: -Math.abs(relCenterIndex) * (width * Math.sin(Math.abs(angle)))
                    width: sourceSize.width * scaleFactor
                    height: sourceSize.height * scaleFactor
                    source: repeater.fuelNormIndex > index ? "../../resources/dashboard_fuel_full.png" : "../../resources/dashboard_fuel_empty.png"
                    rotation: angle * 180 / Math.PI * 1.8 // 1.8 is magic number, without it the items aren't rotated enough. There must be an error in formulae
                    antialiasing: true
                }
            }
        }
    }
}
