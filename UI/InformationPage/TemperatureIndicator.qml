import QtQuick 2.15

Item {
    id: root
    property real temperature: 0.0
    property real scaleFactor: 1.0


    Image {
        id: temperatureIndicatorImage
        source: "../Assets/thermometer.png"
        height: Math.max(50, 70 * scaleFactor)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    Text {
        id: temperatureText
        text: root.temperature.toFixed(1) + " °C"
        color: "turquoise"
        font {
            bold: true
            family: "DS-Digital"
            pixelSize: Math.max(14, 22 * scaleFactor)
        }
        anchors {
            bottom: temperatureIndicatorImage.bottom
            horizontalCenter: temperatureIndicatorImage.horizontalCenter
            bottomMargin: -25 * scaleFactor
        }
    }

}
