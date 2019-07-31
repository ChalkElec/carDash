import QtQuick 2.4
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.3

Image {
    id: root

    property alias scaleSource: scaleImage.source
    property real value: 0
    property real maxValue: 0

    property bool valueSmoothing: false

    property real smoothedValue: {
        if (valueSmoothing)
            return ema(value)
        else
            return value
    }

    property real lastSmoothedValue: 0

    function ema(newValue) {
        var alpha = 0.7;
        lastSmoothedValue = (alpha * newValue) + (1.0 - alpha) * lastSmoothedValue;
        return lastSmoothedValue;
    }

    // Background of arrow trace, which limits outer edge of trace by clipping it via OpacityMask
    Rectangle {
        id: gradientMask

        anchors.centerIn: parent
        width: parent.width * 0.8
        height: width
        radius: width / 2
        color: "black"
        visible: false // OpacityMask will draw it with ConicalGradient

        // Conical gradient draws trace of arrow. It's not visible, because without OpacityMask
        // it will draw a rectangle, not a circle, though bounded to circle
        ConicalGradient {
            id: grad

            visible: false
            anchors.fill: parent
            angle: arrow.startAngle * 180 / Math.PI - 180

            gradient: Gradient {
                // This gradient stop changes transparent to black immediately
                GradientStop {
                    position: 0.0
                    color: "black"
                }
                GradientStop {
                    position: (arrow.valueAngle * 180 / Math.PI) / 360
                    color: "#0078a5"
                }
                GradientStop {
                    position: (arrow.valueAngle * 180 / Math.PI) / 360 + 0.001
                    color: "transparent"
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
                }
            }

            Rectangle {
                color: "black"
                width: parent.width * 0.76
                height: width
                anchors.centerIn: parent
                radius: width / 2
            }
        }
    }

    // OpacityMask clips gradient to bounding circle
    OpacityMask {
        anchors.centerIn: parent
        width: parent.width * 0.895
        height: width
        source: grad
        maskSource: gradientMask
    }

    Image {
        id: scaleImage

        anchors.fill: parent
        anchors.margins: 2 // To prevent scale stick out of round circle border
        // source: root.scaleSource
    }

    // Arrow movement consists of its rotation around its bottom point and its movement around speedometer center
    Image {
        id: arrow

        property real radius: parent.width / 6.8 // Magic number which is radius from speedometer center to image bottom point
        readonly property real startAngle: Math.PI / 4.02 // to match 0 on scale
        readonly property real endAngle: Math.PI * 1.75 // to match 240 on scale
        property real valueAngle: (root.smoothedValue / root.maxValue) * (endAngle - startAngle)
        property real valueAbsAngle: startAngle + valueAngle
        property real limitedAngle: valueAbsAngle < startAngle ? startAngle : (valueAbsAngle > endAngle ? endAngle : valueAbsAngle)

        readonly property real xOffset: parent.width / 2 - arrow.width / 2
        readonly property real yOffset: parent.height / 2
        x: -radius * Math.cos(limitedAngle - Math.PI / 2) + xOffset
        y: -radius * Math.sin(limitedAngle - Math.PI / 2) + yOffset

        source: "../../resources/dashboard_arrow_down.png"
        property real scaleFactor: sourceSize.width / sourceSize.height
        width: height * sourceSize.width / sourceSize.height
        height: root.width * 0.3
        transform: Rotation {
            origin.x: arrow.width / 2
            angle: arrow.limitedAngle * 180 / Math.PI
        }
    }

    // Inner and outer circles
    Image {
        id: circles

        anchors.fill: parent
        source: "../../resources/dashboard_circles.png"
    }
}
