import QtQuick

Rectangle {
    id: root
    property string nameofsession : " "
    property string nameOfport : " "
    property real scaleFactor: 1.0

    // Responsive width based on parent, with minimum height
    width: parent ? parent.width - 24 : 1000
    height: Math.max(30, 35 * scaleFactor)

    color: "#09122C"
    radius: 9
    border.color: "#A6F1E0"
    border.width: Math.max(3, 5 * scaleFactor)
    anchors {
        top : parent.top
        horizontalCenter : parent.horizontalCenter
    }



    Text {
        id: timeText
        text: Qt.formatDateTime(new Date(), "hh:mm A") // Formats time dynamically
        color: "white"
        font.pixelSize: Math.max(16, 25 * scaleFactor)
        font.family : "DS-Digital"
        font.bold : true
        anchors.centerIn : parent
    }

    Text {
        id : sessionNameText
        text : "Session Name : " + root.nameofsession
        color : "white"
        font {
            family : "DS-Digital"
            pixelSize : Math.max(12, 18 * scaleFactor)
        }
        anchors {
            verticalCenter : parent.verticalCenter
            left : parent.left
            leftMargin : 10 * scaleFactor
        }
    }

    Text {
        id : portNumberText
        text : "Port Number : " + root.nameOfport
        color : "white"
        font {
            family : "DS-Digital"
            pixelSize : Math.max(12, 18 * scaleFactor)
        }
        anchors {
            verticalCenter : parent.verticalCenter
            right : parent.right
            rightMargin : 10 * scaleFactor
        }
    }

    Timer {
        interval: 1000 // Updates every second
        running: true
        repeat: true
        onTriggered: timeText.text = Qt.formatDateTime(new Date(), "hh:mm A")
    }




}
