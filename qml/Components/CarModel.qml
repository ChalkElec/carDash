import QtQuick 2.4

import "../Hardware"


Item {
    id: root

    property alias paintedWidth: carImage.paintedWidth

    Image {
        id: sideLights
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: "../../resources/side_lights.svg"
        sourceSize.width: root.width
        visible: Hardware.sideLightsOn
    }

    Image {
        id: carImage

        anchors.left: parent.left
        width: implicitWidth
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        source: "../../resources/car_dashboard_car_black.svg"
        sourceSize.height: root.height

        // Doors
        Image {
            id: leftFrontDoor
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/left_front_door_open.svg"
            sourceSize.height: root.height
            visible: Hardware.leftFrontDoorOpen
        }

        Image {
            id: rightFrontDoor
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/right_front_door_open.svg"
            sourceSize.height: root.height
            visible: Hardware.rightFrontDoorOpen
        }

        Image {
            id: leftRearDoor
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/left_rear_door_open.svg"
            sourceSize.height: root.height
            visible: Hardware.leftRearDoorOpen
        }

        Image {
            id: rightRearDoor
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/right_rear_door_open.svg"
            sourceSize.height: root.height
            visible: Hardware.rightRearDoorOpen
        }

        Image {
            id: engine
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/engine_big.svg"
            sourceSize.height: root.height
            visible: Hardware.engineProblem
        }

        Image {
            id: battery
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/battery_big.svg"
            sourceSize.height: root.height
            visible: Hardware.batteryProblem
        }

        Image {
            id: handbrake
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/handbrake_big.svg"
            sourceSize.height: root.height
            visible: Hardware.handbrakeOn
        }

        Image {
            id: seatlbelt
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/seatbelt_big.svg"
            sourceSize.height: root.height
            visible: Hardware.seatbeltOn
        }

        Image {
            id: frontLight
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/front_lights.png"
            visible: Hardware.frontLightsOn
        }

        Image {
            id: frontFogLight
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/front_wave_lights.png"
            visible: Hardware.frontFogLightsOn
        }

        Image {
            id: frontLongRangeLight
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/front_long_range_lights.svg"
            sourceSize.height: root.height
            visible: Hardware.frontLongRangeLightsOn
        }

        Image {
            id: rearFogLight
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/rear_wave_red_lights.png"
            visible: Hardware.rearFogLightsOn
        }

        Image {
            id: leftTurnLight
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/left_turn_light_big.svg"
            sourceSize.height: root.height
            visible: Hardware.leftTurnLightsOn
        }

        Image {
            id: rightTurnLight
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "../../resources/right_turn_light_big.svg"
            sourceSize.height: root.height
            visible: Hardware.rightTurnLightsOn
        }
    }
}
