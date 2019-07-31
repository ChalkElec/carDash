pragma Singleton

import QtQuick 2.4

Item {
    property bool leftFrontDoorOpen: false
    property bool rightFrontDoorOpen: false
    property bool leftRearDoorOpen: false
    property bool rightRearDoorOpen: false
    property bool engineProblem: false
    property bool batteryProblem: false
    property bool handbrakeOn: false
    property bool seatbeltOn: false
    property bool frontLightsOn: true
    property bool frontFogLightsOn: false
    property bool frontLongRangeLightsOn: false
    property bool rearFogLightsOn: false
    property bool leftTurnLightsOn: false
    property bool rightTurnLightsOn: false
    property bool sideLightsOn: false

    property real speed: 0
    // onSpeedChanged:console.log("speed:", speed)
    property real maxSpeed: 240
    property real fuelValue: 90
    property real maxFuelValue: 100
    property real minFuelValue: 0

    property real rpm: 0
    // onRpmChanged:console.log("rpm:", rpm)
    property real maxRpm: 8000
    property string gear: "D"
    property real coolantTempValue: 0
    property real maxCoolantTempValue: 100
    property real minCoolantTempValue: 0

    property int outsideTemperature: +27
    property string temperatureUnit: "C"

    Behavior on rpm {
        NumberAnimation { duration: 100 }
    }

    Behavior on speed {
        NumberAnimation { duration: 100 }
    }

    Connections {
        target: HardwareClass
        onDataReceived: { // (targetId, value)
            console.log("targetId:", targetId, "value:", value)

            switch (targetId) {
                case "81": speed = value; break;
                case "37": rpm = value; break;
                case "170": fuelValue = value; break;
                case "14": coolantTempValue = value; break;
                case "173": outsideTemperature = value; break;
                case "139": gear = value; break;
                default: break;
            }
        }
    }
}
