import QtQuick
import QtMultimedia

Item {
    id: camera
    property real scaleFactor: 1.0
    width:  parent ? parent.width  - 30 * scaleFactor : 300
    height: parent ? parent.height - 50 * scaleFactor : 400

    VideoOutput {
        id: cameraFeed
        anchors.fill: parent
    }

    Component.onCompleted: {
        // console.log("Wiring video sink...")
        webrtcClient.setVideoSink(cameraFeed.videoSink)
        // console.log("Video sink wired")
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
        visible: !webrtcClient.isStreaming
        opacity: 0.8

        Text {
            anchors.centerIn: parent
            text: "No Feed"
            color: "red"
            font.pixelSize: 32
        }
    }

        Rectangle {
            id: pttButton
            width: 80
            height: 80
            radius: 40
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 20

            color: mouseArea.pressed ? "#ff3333" : "#333333"
            border.color: mouseArea.pressed ? "#ff8888" : "#666666"
            border.width: 3

            Text {
                anchors.centerIn: parent
                text: "PTT"
                color: "white"
                font.bold: true
                font.pixelSize: 20
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                onPressed: {
                    console.log("PTT Button Held - Opening Valve")
                    webrtcClient.setPushToTalk(true)
                }
                onReleased: {
                    console.log("PTT Button Released - Closing Valve")
                    webrtcClient.setPushToTalk(false)
                }
                onCanceled: webrtcClient.setPushToTalk(false)
            }
        }

    Connections {
        target: webrtcClient
        function onStreamConnected() { console.log("Stream is live") }
        function onStreamLost()      { console.log("Stream lost")    }
    }
}
