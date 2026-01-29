import QtQuick
import QtQuick.Controls
import "../StatusBar"
import "../../.."

Rectangle {
    id: root

    // Dynamic scale factor based on dimensions vs design size (1400x780)
    property real scaleFactor: Math.min(width / 1400, height / 780)
    property string sessionName: ""
    property string portNumber: ""
    property string portName: ""
    property int baudRate: 0
    property bool isSerialSource: false

    color: "#1A3438"
    radius: Math.max(25, 40 * scaleFactor)
    border.color: "#A6F1E0"
    border.width: Math.max(3, 5 * scaleFactor)

    // Add Timer component for auto-navigation
    Timer {
        id: navigationTimer
        interval: 2000  // 2 seconds
        running: true    // Start timer automatically when the page loads
        repeat: false    // Run only once
        onTriggered: {
            stackView.push("../InformationPage/Information.qml", {
                "sessionName": root.sessionName,
                "portNumber": root.portNumber,
                "portName": root.portName,
                "baudRate": root.baudRate,
                "isSerialSource": root.isSerialSource
            })
        }
    }

    Text {
        id: waitingText
        text: "Loading..."
        font {
            family: "Amiri"
            bold: true
            pixelSize: Math.max(20, 30 * root.scaleFactor)
        }
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: busyIndicator.top
            bottomMargin: 10 * scaleFactor
        }
        color: "turquoise"
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        width: Math.max(50, 70 * scaleFactor)
        height: Math.max(40, 60 * scaleFactor)
    }

    StatusBar {
        id: statusBar
        scaleFactor: root.scaleFactor
        nameofsession : root.sessionName
        nameOfport : root.isSerialSource ? root.portName + " (" + root.baudRate + ")" : root.portNumber

    }

    MyButton {
        id: backButton
        source: "../../../Assets/back-button.png"
        hoverText: "Back"
        smooth: true
        width: Math.max(30, 48 * scaleFactor)
        height: Math.max(30, 48 * scaleFactor)
        fillMode: Image.PreserveAspectFit
        anchors {
            left: parent.left
            leftMargin: 12 * scaleFactor
            bottom: parent.bottom
            bottomMargin: 40 * scaleFactor
        }
        onClicked: {
                   navigationTimer.stop();
                   communicationManager.stop();
                   stackView.pop();
        }
    }

    Text {
        id: backText
        text: "Back"
        font {
            family: "DS-Digital"
            bold: true
            pixelSize: Math.max(12, 18 * scaleFactor)
        }
        color: "turquoise"
        anchors {
            horizontalCenter: backButton.horizontalCenter
            top: backButton.bottom
            topMargin: 5 * scaleFactor
        }
    }
}
