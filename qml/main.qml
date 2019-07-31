import QtQuick 2.4
import QtQuick.Window 2.0
import QtQuick.Controls 1.3

import "Components"
import "Hardware"

ApplicationWindow {
    id: root

    title: qsTr("Car dashboard")
    visible: true
    color: "black"

    width: Screen.width // Despite visibility FullScreen, we need exact sizes, othervise there is infinite loop of constant resizing
    height: Screen.height / 2
    //visibility: Window.FullScreen

    Row {
        id: row

        property int count: 3
        property int widgetSize: Math.min((root.width - (count + 1) * spacing) / count, root.height - 2 * spacing)
        anchors.centerIn: parent
        spacing: root.height / 15

        Speedometer {
            id: speedometer

            width: row.widgetSize
            height: width

            // Time must be centered between speedometer and carModel, but must not be a row component
            // to not occupy extra space in row width
            Text {
                id: timeText

                anchors.bottom: parent.bottom
                anchors.left: parent.right
                anchors.leftMargin: (row.spacing - contentWidth) / 2
                color: "#c8c8c8"
                font { pixelSize: parent.height / 13; bold: true }
                horizontalAlignment: Text.AlignHCenter
                text: new Date().toLocaleTimeString(Qt.locale(), Locale.ShortFormat) // Date doesn't send signals

                Timer {
                    interval: 1000; running: true; repeat: true;
                    onTriggered: timeText.text = new Date().toLocaleTimeString(Qt.locale(), Locale.ShortFormat)
                }
            }
        }

        CarModel {
            id: carModel

            width: paintedWidth // Assign width to the painted width calculated using aspect ratio
            height: row.widgetSize

            // Temperature must be centered between carModel and tachometer, but must not be a row component
            // to not occupy extra space in row width
            Text {
                id: temperatureText

                anchors.bottom: parent.bottom
                anchors.left: parent.right
                anchors.leftMargin: (row.spacing - contentWidth) / 2
                color: "#c8c8c8"
                font { pixelSize: parent.height / 13; bold: true }
                horizontalAlignment: Text.AlignHCenter
                text: Hardware.outsideTemperature + "°" + Hardware.temperatureUnit
            }
        }

        Tachometer {
            width: row.widgetSize
            height: width
        }
    }

    Rectangle {
        id: splashScreenBackground

        anchors.fill: parent
        color: "#151922"

        visible: !HardwareClass.isInitialized

        AnimatedImage {
            id: splashScreenLogo

            anchors.centerIn: parent
            height: parent.height / 2
            source: "../resources/logo.gif"
            fillMode: Image.PreserveAspectFit
        }
    }
}
