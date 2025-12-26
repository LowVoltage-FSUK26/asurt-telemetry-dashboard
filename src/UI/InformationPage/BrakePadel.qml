import QtQuick 2.15

Rectangle {
    id: root
    
    // Scale factor for responsive sizing
    property real scaleFactor: 1.0
    
    // Scaled dimensions
    width: Math.max(80, 120 * scaleFactor)
    height: Math.max(18, 25 * scaleFactor)
    color: "black"
    border.color: "transparent"
    border.width: 2

    property int pedalPosition: 0
    property real smoothedPosition: pedalPosition
    property int totalBars: 20

    Behavior on smoothedPosition {
        NumberAnimation {
            duration: 25
            easing.type: Easing.InOutQuad
        }
    }


    Repeater {
        model: root.totalBars
        delegate: Rectangle {
            id: bar
            width: Math.max(3, 5 * scaleFactor)
            height: parent.height
            property bool shouldBeActive: index <= Math.floor(root.smoothedPosition / (100 / root.totalBars))
            color: shouldBeActive ? "green" : "black"
            anchors.verticalCenter: parent.verticalCenter
            x: index * (parent.width / root.totalBars)
            transformOrigin: Item.Center
        }
    }

    Text {
        id: brakePedalText
        color: "white"
        anchors {
            bottom: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        text: "Brake " + Math.round(root.smoothedPosition) + " %"
        font.pixelSize: Math.max(10, 14 * scaleFactor)
        font.family: "DS-Digital"
        font.bold: true
        opacity: root.smoothedPosition > 0 ? 1 : 0.7

        Behavior on opacity {
            NumberAnimation {
                duration: 25
                easing.type: Easing.InOutQuad
            }
        }
    }
}
