import QtQuick 2.15
import QtQuick.Shapes
import "../StatusBar"

Rectangle {
    id: root

    // Dynamic scale factor based on actual dimensions vs design size (1400x780)
    property real scaleFactor: Math.min(width / 1400, height / 780)
    property string sessionName: ""
    property string portNumber: ""
    property string portName: ""
    property int baudRate: 0
    property bool isSerialSource: false
    property real maxLateralG: 3.5  // Maximum lateral G-force (cornering)
    property real maxLongitudinalG: 2.0  // Maximum longitudinal G-force (acceleration)
    property real maxBrakingG: 3.5  // Maximum braking G-force
    property real xDiagram: (communicationManager.lateralG / maxLateralG) * (ggImage.width / 2 - 20 * scaleFactor)
    property real yDiagram: (communicationManager.longitudinalG / maxLongitudinalG) * (ggImage.height / 2 - 20 * scaleFactor)

    color: "#1A3438"
    radius: 40 * scaleFactor
    border.color: "#A6F1E0"
    border.width: Math.max(3, 5 * scaleFactor)

        /******* Status Bar *********/

    StatusBar {
        id: statusBar
        scaleFactor: root.scaleFactor
        nameofsession : root.sessionName
        nameOfport : communicationManager.isSerialSource ? root.portName + " (" + root.baudRate + ")" : root.portNumber
   }




    /************  Steering Wheel and Proximity Sensor ************/

    Rectangle {
        id : leftRect
        width : parent.width / 4
        color : "#09122C"
        radius : Math.max(20, 30 * scaleFactor)
        border.width : 2
        border.color : "#D84040"

        anchors {
            top : statusBar.bottom
            left: parent.left
            bottom : parent.bottom
            leftMargin : 12 * scaleFactor
            topMargin : 5 * scaleFactor
            bottomMargin : 12 * scaleFactor
        }

        Rectangle {
            id: steeringWheelRect
            width: parent.width - 40 * scaleFactor
            height: parent.height / 3
            color: "#636363"
            border.color: "turquoise"
            border.width: 2
            radius: Math.max(12, 20 * scaleFactor)
            anchors {
                horizontalCenter : parent.horizontalCenter
                top : parent.top
                topMargin : 10 * scaleFactor

            }
            SteeringWheel {
                id: steeringWheel
                scaleFactor: root.scaleFactor
                encoderAngle : communicationManager ? communicationManager.encoderAngle : 0
                anchors.centerIn: parent
            }
        }


        Rectangle {
            id : proximityRect
            width : parent.width - 30 * scaleFactor
            color : "#636363"
            border.color: "turquoise"
            border.width: 2
            radius: Math.max(12, 20 * scaleFactor)

            anchors {
                horizontalCenter : parent.horizontalCenter
                top : steeringWheelRect.bottom
                bottom : parent.bottom
                topMargin : 10 * scaleFactor
                bottomMargin : 10 * scaleFactor
            }

            Image {
                id : car
                source : "../../../Assets/AI_car_transparent.png"
                anchors.centerIn : parent
                height : parent.height - 30 * scaleFactor
                fillMode : Image.PreserveAspectFit
                smooth : true
            }
            // Front Left - Temperature then Wheel Speed
            TireTemperature {
                id: tempFL
                wheelPos: "T"
                scaleFactor: root.scaleFactor
                currentTemp: communicationManager ? communicationManager.tempFL : 0
                anchors {
                    top: car.top
                    left: proximityRect.left
                    topMargin: 70 * scaleFactor
                    leftMargin: 5 * scaleFactor
                }
            }
            WheelSpeed {
                id: fl
                wheelPos: "FL"
                scaleFactor: root.scaleFactor
                currentSpeed: communicationManager ? communicationManager.speedFL : 0
                anchors {
                    top: car.top
                    left: tempFL.right
                    topMargin: 70 * scaleFactor
                    leftMargin: 5 * scaleFactor
                }
            }
            
            // Front Right - Wheel Speed then Temperature
            WheelSpeed {
                id: fr
                wheelPos: "FR"
                scaleFactor: root.scaleFactor
                currentSpeed: communicationManager ? communicationManager.speedFR : 0
                anchors {
                    top: car.top
                    right: tempFR.left
                    topMargin: 70 * scaleFactor
                    rightMargin: 5 * scaleFactor
                }
            }
            TireTemperature {
                id: tempFR
                wheelPos: "T"
                scaleFactor: root.scaleFactor
                currentTemp: communicationManager ? communicationManager.tempFR : 0
                anchors {
                    top: car.top
                    right: proximityRect.right
                    topMargin: 70 * scaleFactor
                    rightMargin: 5 * scaleFactor
                }
            }

            // Back Left - Temperature then Wheel Speed
            TireTemperature {
                id: tempBL
                wheelPos: "T"
                scaleFactor: root.scaleFactor
                currentTemp: communicationManager ? communicationManager.tempBL : 0
                anchors {
                    bottom: car.bottom
                    left: proximityRect.left
                    bottomMargin: 70 * scaleFactor
                    leftMargin: 5 * scaleFactor
                }
            }
            WheelSpeed {
                id: bl
                wheelPos: "BL"
                scaleFactor: root.scaleFactor
                currentSpeed: communicationManager ? communicationManager.speedBL : 0
                anchors {
                    bottom: car.bottom
                    left: tempBL.right
                    bottomMargin: 70 * scaleFactor
                    leftMargin: 5 * scaleFactor
                }
            }

            // Back Right - Wheel Speed then Temperature
            WheelSpeed {
                id: br
                wheelPos: "BR"
                scaleFactor: root.scaleFactor
                currentSpeed: communicationManager ? communicationManager.speedBR : 0
                anchors {
                    bottom: car.bottom
                    right: tempBR.left
                    bottomMargin: 70 * scaleFactor
                    rightMargin: 5 * scaleFactor
                }
            }
            TireTemperature {
                id: tempBR
                wheelPos: "T"
                scaleFactor: root.scaleFactor
                currentTemp: communicationManager ? communicationManager.tempBR : 0
                anchors {
                    bottom: car.bottom
                    right: proximityRect.right
                    bottomMargin: 70 * scaleFactor
                    rightMargin: 5 * scaleFactor
                }
            }
        }
    }

    /********************************************************/




    /******** Meters Screen *******/

    Rectangle {
        id: metersScreen
        // Proportional width and height based on design size (675x335 at scale 1.0)
        width: Math.max(500, 675 * scaleFactor)
        height: Math.max(280, 335 * scaleFactor)
        color: "#09122C"
        border.color: "#D84040"
        border.width: 2

        anchors {
            top : statusBar.bottom
            topMargin: 8 * scaleFactor
            // Center horizontally between leftRect and rightRect
            horizontalCenter: parent.horizontalCenter
        }

        radius: Math.max(12, 20 * scaleFactor)

        Speedometer {
            id: speedometer
            scaleFactor: root.scaleFactor
            speed: communicationManager ? communicationManager.speed : 0
            anchors {
                left: parent.left
                leftMargin: -20 * scaleFactor
                verticalCenter: parent.verticalCenter
            }
        }

        RpmMeter {
            id: rpmMeter
            scaleFactor: root.scaleFactor
            rpm: communicationManager ? communicationManager.rpm : 0
            anchors {
                left: speedometer.right
                verticalCenter: parent.verticalCenter
            }
        }
    }


    /******** Battery , Accelator , Braker Pedal Readings *************/

    Rectangle {
        id : pedalTempRect
        width : parent.width / 5.1
        height : parent.height / 3.5
        clip: true  // Prevent content from overflowing

        color: "#09122C"
        border.color: "#D84040"
        border.width: 2
        radius : Math.max(12, 20 * scaleFactor)

        anchors {
            top : metersScreen.bottom
            topMargin : 65 * scaleFactor
            left : leftRect.right
            leftMargin : 30 * scaleFactor
        }

        AcceleratorPedal {
            id: acceleratorPedal
            scaleFactor: root.scaleFactor
            pedalPosition:  communicationManager ? communicationManager.accPedal : 0
            anchors {
                bottom: parent.bottom
                left: parent.left
                margins: 10 * scaleFactor
            }
        }

        BrakePadel {
            id: brakePedal
            scaleFactor: root.scaleFactor
            pedalPosition: communicationManager ? communicationManager.brakePedal : 0
            anchors {
                bottom: parent.bottom
                left: acceleratorPedal.right
                margins: 10 * scaleFactor
            }
        }

        TemperatureIndicator {
            id: temperatureIndicator
            scaleFactor: root.scaleFactor
            temperature: communicationManager ? communicationManager.temperature : 0
            anchors {
                top: parent.top
                left: parent.left
                leftMargin: 10 * scaleFactor
                topMargin: 20 * scaleFactor
            }
        }

        BatteryLevelIndicator {
            id: batteryLevelIndicator
            scaleFactor: root.scaleFactor
            batteryLevel: communicationManager ? communicationManager.batteryLevel : 0
            anchors {
                right: parent.right
                top: parent.top
                rightMargin: 10 * scaleFactor
                topMargin: 10 * scaleFactor
            }
        }
    }
    /********************************************************/


    /****** IMU *****/

    // Replacement for the bottomRect section only in the main QML file

    Rectangle {
        id: bottomRect

        width: parent.width / 3
        color: "#09122C"
        border.color: "#D84040"
        border.width: 2
        radius: Math.max(20, 30 * scaleFactor)

        anchors {
            top: metersScreen.bottom
            left: pedalTempRect.right
            right: rightRect.left
            bottom: parent.bottom
            margins: 10 * scaleFactor
        }

        // Calculate diagram size based on available space
        property real diagramSize: Math.min(width * 0.85, height * 0.75, 294 * scaleFactor)
        
        // Track if we've received actual G-force data
        property bool hasReceivedData: communicationManager.lateralG !== 0 || communicationManager.longitudinalG !== 0
        
        // Clear path when scale factor changes (on resize)
        Connections {
            target: root
            function onScaleFactorChanged() {
                pathTracker.pathPoints = []
                polyline.path = []
            }
        }

        Image {
            id: ggImage
            source: "../../../Assets/GG_Diagram.png"
            width: bottomRect.diagramSize
            height: bottomRect.diagramSize
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -20 * scaleFactor
            rotation: -90
            fillMode: Image.PreserveAspectFit
            smooth: true

            // Calculate center point dynamically
            property real centerX: width / 2
            property real centerY: height / 2



            Shape {
                id: pathShape
                anchors.fill: parent
                smooth: true
                antialiasing: true

                ShapePath {
                    id: movementPath
                    strokeWidth: 2 * root.scaleFactor
                    strokeColor: "white"
                    strokeStyle: ShapePath.SolidLine
                    fillColor: "transparent"

                    // Start at the center point
                    startX: ggImage.centerX
                    startY: ggImage.centerY

                    // Create path segments using a dynamic PathPolyline
                    PathPolyline {
                        id: polyline
                    }
                }
            }

            // Add the point marker
            Image {
                id: pointImage
                source: "../../../Assets/point.png"
                width: 20 * root.scaleFactor
                height: 20 * root.scaleFactor

                // Position at the center of the marker with limits
                x: Math.max(0, Math.min(ggImage.width - width, root.yDiagram + ggImage.centerX - width/2))
                y: Math.max(0, Math.min(ggImage.height - height, root.xDiagram + ggImage.centerY - height/2))

                fillMode: Image.PreserveAspectFit
                smooth: true
                z: 2


                // Add smooth animations for x and y movements
                Behavior on x {
                    SmoothedAnimation {
                        easing.type: Easing.InOutQuad
                        velocity: 2000
                    }
                }
                Behavior on y {
                    SmoothedAnimation {
                        easing.type: Easing.InOutQuad
                        velocity: 2000
                    }
                }

            }
        }

        // Point tracking for line drawing
        Timer {
            id: pathTracker
            interval: 100  // Update every 100ms
            running: bottomRect.hasReceivedData  // Only run when we have actual G-force data
            repeat: true

            property var pathPoints: []

            onTriggered: {
                // Calculate the center point of the marker with limits
                const pointX = Math.max(0, Math.min(ggImage.width, pointImage.x + pointImage.width/2));
                const pointY = Math.max(0, Math.min(ggImage.height, pointImage.y + pointImage.height/2));

                // Add point to the path
                pathPoints.push(Qt.point(pointX, pointY));

                // Update the polyline path
                polyline.path = pathPoints;
            }
        }

        // Add acceleration text displays
        Row {
            anchors {
                top: ggImage.bottom
                horizontalCenter: ggImage.horizontalCenter
                topMargin: 10 * root.scaleFactor
            }
            spacing: 10 * root.scaleFactor

            // Add clear button
            Rectangle {
                id: clearButton
                width: Math.max(40, 50 * root.scaleFactor)
                height: Math.max(16, 20 * root.scaleFactor)
                color: "#636363"
                radius: 15 * root.scaleFactor
                border.color: "white"
                border.width: 1
                z: 3

                Text {
                    text: "Clear"
                    color: "white"
                    anchors.centerIn: parent
                    font {
                        family: "Arial"
                        pixelSize: Math.max(10, 14 * root.scaleFactor)
                        bold: true
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.color = "#808080"
                    onExited: parent.color = "#636363"
                    onClicked: {
                        pathTracker.pathPoints = []
                        polyline.path = []
                    }
                }
            }

            Text {
                text: "Lateral G: " + communicationManager.lateralG.toFixed(2) + " G"
                color: "white"
                font {
                    family: "Arial"
                    pixelSize: Math.max(10, 14 * root.scaleFactor)
                    bold: true
                }
            }

            Text {
                text: "Longitudinal G: " + communicationManager.longitudinalG.toFixed(2) + " G"
                color: "white"
                font {
                    family: "Arial"
                    pixelSize: Math.max(10, 14 * root.scaleFactor)
                    bold: true
                }
            }
        }
    }



    /*********** Right Rect (GPS) **********/

    Rectangle {
        id : rightRect
        width : parent.width / 4
        color : "#09122C"
        radius : Math.max(20, 30 * scaleFactor)
        border.width : 2
        border.color : "#D84040"

        anchors {
            top : statusBar.bottom
            right: parent.right
            bottom : parent.bottom
            rightMargin : 12 * scaleFactor
            topMargin : 5 * scaleFactor
            bottomMargin : 12 * scaleFactor
        }

        Text {
            id : gpsText
            text : "GPS"
            color : "turquoise"
            font {
                pixelSize: Math.max(14, 20 * scaleFactor)
                family: "DS-Digital"
            }

            anchors {
                top : parent.top
                horizontalCenter : parent.horizontalCenter
                topMargin : 5 * scaleFactor
            }
        }

        GpsPlotter {
            id : gps
            scaleFactor: root.scaleFactor
            anchors {
                horizontalCenter : parent.horizontalCenter
                top : parent.top
                bottom : parent.bottom
                topMargin : 30 * scaleFactor
                bottomMargin : 10 * scaleFactor

            }
        }

    }
    /********************************************************/




}
