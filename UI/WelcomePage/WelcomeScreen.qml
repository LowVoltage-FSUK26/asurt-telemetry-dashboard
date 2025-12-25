import QtQuick 2.15
import QtQuick.Controls
import Qt.labs.settings 1.0
import "../.."

Rectangle {
    id: root

    // Dynamic scale factor based on dimensions vs design size (1400x780)
    property real scaleFactor: Math.min(width / 1400, height / 780)
    
    color: "#1A3438"
    radius: Math.max(25, 40 * scaleFactor)
    border.color: "#A6F1E0"
    border.width: Math.max(3, 5 * scaleFactor)

    // Settings component for persistent storage
    Settings {
        id: appSettings
        category: "WelcomePageInputs"
        
        // TextField values
        property alias sessionName: sessionNameField.text
        property alias mqttBrokerAddress: mqttbrokerAddressField.text
        property alias serverClientPort: serverClientPortField.text
        property alias mqttClientId: mqttClientIdField.text
        property alias mqttUsername: mqttUsernameField.text
        property alias mqttPassword: mqttPasswordField.text
        property alias mqttTopic: mqttTopicField.text
        property alias driverName: driverNameField.text
        property alias sessionGoals: goalsField.text
        
        // ComboBox selections (storing currentIndex)
        property alias mqttProtocolIndex: mqttProtocolField.currentIndex
        property alias serialPortIndex: serialPortField.currentIndex
        property alias serialBaudRateIndex: serialBaudRate.currentIndex
    }

    /**** Layout for the Two Rectangles ***/

    Row {
        id: row
        anchors.fill: parent
        anchors {
            rightMargin: 10 * scaleFactor
            leftMargin: Math.max(40, 70 * scaleFactor)
            topMargin: 3 * scaleFactor
            bottomMargin: 3 * scaleFactor
        }

        spacing: 10 * scaleFactor

        /**** Left Rectangle ****/

        Rectangle {
            id: leftRect
            width: parent.width / 3
            height: parent.height - 8 * root.scaleFactor
            color: "#09122C"
            radius: Math.max(20, 30 * scaleFactor)
            border.width: 1.5
            border.color: "#D84040"

            /**** Image of the Formula Student Car ****/

            Image {
                id: carImg
                source: "../Assets/CAR-215-ASURT.png"
                width: Math.max(180, 300 * root.scaleFactor)
                height: Math.max(120, 200 * root.scaleFactor)
                fillMode: Image.PreserveAspectFit
                smooth: true

                anchors {
                    top: formulalogo.top
                    horizontalCenter: parent.horizontalCenter
                    topMargin: 40 * scaleFactor
                }
            }

            /**** Formula logo ****/

            Image {
                id: formulalogo
                source: "../Assets/formulalogo.jpeg"
                width: Math.max(120, 200 * scaleFactor)
                height: Math.max(36, 60 * scaleFactor)
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: 10 * scaleFactor
                    topMargin : 3 * scaleFactor
                }
            }

            /**** Layout for the Components of this Rectangle ****/

            Column {
                spacing: Math.max(3, 5 * scaleFactor)

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: carImg.bottom
                    topMargin: 10 * scaleFactor
                }

                /**** Field to Get Session Name ****/

                Text {
                    id: sessionInfo
                    text: "Please Enter Session details"
                    font {
                        family: "DS-Digital"
                        pixelSize: Math.max(12, 18 * root.scaleFactor)
                        bold: true
                    }

                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "turquoise"
                }

                Text {
                    id: sessionName
                    text: "Enter Session Name : "
                    font {
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                        bold: true
                    }

                    anchors.left: parent.left
                    color: "white"
                }

                TextField {
                    id: sessionNameField
                    placeholderText: startButton.inValid_Name ? "Don't Leave This Empty!" : "Enter Session Name (e.g, Test)"
                    placeholderTextColor: startButton.inValid_Name ? "darkRed" : "turquoise"

                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 11 * root.scaleFactor)
                    verticalAlignment: TextInput.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"

                    background: Rectangle {
                        color: "#636363"
                        radius: 50 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : startButton.inValid_Name ? "darkRed" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }

                /**** Field to Get the Borker Address ****/

                Text {
                    visible: mqttradio.checked
                    text: "Enter Borker Address"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                TextField {
                    id: mqttbrokerAddressField
                    visible: mqttradio.checked
                    placeholderText: startButton.inValid_Broker ? "Don't Leave This Empty!" : "Enter Broker Address (e.g., localhost)"
                    placeholderTextColor: startButton.inValid_Broker ? "darkRed" : "turquoise"

                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    verticalAlignment: TextInput.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"

                    background: Rectangle {
                        color: "#636363"
                        radius: 50 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : startButton.inValid_Broker ? "darkRed" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }

                /**** Field to Get Port Number ****/

                Text {
                    text: "Choose Port:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                TextField {
                    id: serverClientPortField
                    visible: server_clientradio.checked || mqttradio.checked
                    placeholderText: startButton.inValid_Port ? "Don't Leave This Empty!" : "Enter Port (e.g. , 8080)"
                    placeholderTextColor: startButton.inValid_Port ? "darkRed" : "turquoise"

                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    verticalAlignment: TextInput.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"

                    background: Rectangle {
                        color: "#636363"
                        radius: 50 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : startButton.inValid_Port ? "darkRed" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }



                /**** Field to choose the mqttProtocol ****/
                Text {
                    visible : mqttradio.checked
                    text : "Select the Protocol"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                ComboBox {
                    id: mqttProtocolField
                    visible: mqttradio.checked
                    model: ["MQTT over TCP", "MQTT over TLS"]
                    currentIndex: 0
                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    anchors.horizontalCenter: parent.horizontalCenter

                    contentItem: Text {
                        text: mqttProtocolField.displayText
                        color: "turquoise"
                        font.pointSize: Math.max(8, 12 * root.scaleFactor)
                        verticalAlignment: Text.AlignVCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 20 * scaleFactor
                    }

                    background: Rectangle {
                        color: "#636363"
                        radius: 100 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }

                    delegate: ItemDelegate {
                        background: Rectangle {
                            id: mqttItemBackground
                            width: Math.max(200, 300 * root.scaleFactor)
                            color: "#636363"
                        }

                        contentItem: Text {
                            text: modelData
                            color: "turquoise"
                            font.pointSize: Math.max(8, 11 * root.scaleFactor)
                        }

                        MouseArea {
                            anchors.fill: mqttItemBackground
                            hoverEnabled: true
                            onEntered: {
                                mqttItemBackground.color = "#4a4848"; // Color when hovered
                            }

                            onPressed: {
                                mqttProtocolField.currentIndex = index;
                                mqttProtocolField.popup.close();
                            }
                            onExited: {
                                mqttItemBackground.color = "#636363"; // Default color when not hovered
                            }
                        }
                    }
                }

                /** Field to get mqtt Client ID*/

                Text {
                    visible: mqttradio.checked
                    text: "Enter Client ID:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                TextField {
                    id: mqttClientIdField
                    visible: mqttradio.checked
                    placeholderText: "Enter Client ID"
                    placeholderTextColor: "turquoise"
                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    verticalAlignment: TextInput.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter

                    color: "white"

                    background: Rectangle {
                        color: "#636363"
                        radius: 50 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }


                /*** Field to get the mqtt username ****/


                Text {
                    visible: mqttradio.checked
                    text: "Enter Username:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                TextField {
                    id: mqttUsernameField
                    visible: mqttradio.checked
                    placeholderText: "Enter Username (e.g, John)"
                    placeholderTextColor: "turquoise"

                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    verticalAlignment: TextInput.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"

                    background: Rectangle {
                        color: "#636363"
                        radius: 50 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }



                /**** Field to get the mqtt password ****/

                Text {
                    visible: mqttradio.checked
                    text: "Enter Password:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                Rectangle {
                    visible: mqttradio.checked
                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)
                    color: "#636363"
                    radius: 50 * scaleFactor
                    border.color: mqttPasswordField.activeFocus ? "turquoise" : "transparent"
                    border.width: Math.max(2, 4 * scaleFactor)
                    anchors.horizontalCenter: parent.horizontalCenter

                    TextField {
                        id: mqttPasswordField
                        anchors {
                            left: parent.left
                            right: showPasswordCheck.left
                            verticalCenter: parent.verticalCenter
                            rightMargin: 10 * scaleFactor
                        }
                        height: parent.height

                        placeholderText: "Enter Password"
                        placeholderTextColor: "turquoise"
                        font.pointSize: Math.max(8, 11 * root.scaleFactor)
                        verticalAlignment: TextInput.AlignVCenter
                        echoMode: showPasswordCheck.checked ? TextInput.Normal : TextInput.Password
                        color: "white"

                        background: Rectangle {
                            color: "transparent"
                        }
                    }

                    CheckBox {
                        id: showPasswordCheck
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            rightMargin: 5 * scaleFactor
                        }
                        width: Math.max(16, 20 * scaleFactor)
                        height: Math.max(16, 20 * scaleFactor)

                        indicator: Rectangle {
                            width: Math.max(12, 16 * scaleFactor)
                            height: Math.max(12, 16 * scaleFactor)
                            radius: 3 * scaleFactor
                            border.color: "turquoise"
                            border.width: 1
                            color: showPasswordCheck.checked ? "turquoise" : "transparent"

                            Text {
                                text: "👁"
                                font.pixelSize: Math.max(8, 10 * scaleFactor)
                                color: "white"
                                anchors.centerIn: parent
                                visible: showPasswordCheck.checked
                            }
                        }
                    }
                }


                /**** Field to get the mqtt topic ****/

                Text {
                    visible: mqttradio.checked
                    text: "Enter Topic:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                TextField {
                    id: mqttTopicField
                    visible: mqttradio.checked
                    placeholderText: startButton.inValid_Topic ? "Don't Leave This Empty!" : "Enter Topic (e.g., ASURT/Sensors)"
                    placeholderTextColor: startButton.inValid_Topic ? "darkRed" : "turquoise"
                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    verticalAlignment: TextInput.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"

                    background: Rectangle {
                        color: "#636363"
                        radius: 50 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : startButton.inValid_Topic ? "darkRed" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }




                /**** Field to choose the Serial Port ****/

                ComboBox {
                    id: serialPortField
                    visible: serialradio.checked
                    model: ["COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "COM10", "COM11", "COM12"]
                    currentIndex: 2
                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    anchors.horizontalCenter: parent.horizontalCenter

                    contentItem: Text {
                        text: serialPortField.displayText
                        color: "turquoise"
                        font.pointSize: Math.max(9, 13 * root.scaleFactor)
                        verticalAlignment: Text.AlignVCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 20 * scaleFactor
                    }

                    background: Rectangle {
                        color: "#636363"
                        radius: 100 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }

                    delegate: ItemDelegate {
                        background: Rectangle {
                            id: itemBackground
                            width: Math.max(200, 300 * root.scaleFactor)
                            color: "#636363"
                        }

                        contentItem: Text {
                            text: modelData
                            color: "turquoise"
                            font.pointSize: Math.max(8, 11 * root.scaleFactor)
                        }

                        MouseArea {
                            id: control
                            anchors.fill: itemBackground
                            hoverEnabled: true
                            onEntered: {
                                itemBackground.color = "#4a4848"; // Color when hovered
                            }

                            onPressed: {
                                serialPortField.currentIndex = index;
                                serialPortField.popup.close();
                            }
                            onExited: {
                                itemBackground.color = "#636363"; // Default color when not hovered
                            }
                        }
                    }
                }

                /**** Field to Get Baud Rate ****/

                Text {
                    visible: serialradio.checked
                    text: "Select Baud Rate:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }

                ComboBox {
                    id: serialBaudRate
                    visible: serialradio.checked
                    model: ["9600", "57600", "115200"]
                    currentIndex: 2
                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(20, 25 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    anchors.horizontalCenter: parent.horizontalCenter

                    contentItem: Text {
                        text: serialBaudRate.displayText
                        color: "turquoise"
                        font.pointSize: Math.max(9, 13 * root.scaleFactor)
                        verticalAlignment: Text.AlignVCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 20 * scaleFactor
                    }

                    background: Rectangle {
                        color: "#636363"
                        radius: 100 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }

                    delegate: ItemDelegate {
                        background: Rectangle {
                            id: itemBackground1
                            width: Math.max(200, 300 * root.scaleFactor)
                            color: "#636363"
                        }

                        contentItem: Text {
                            text: modelData
                            color: "turquoise"
                            font.pointSize: Math.max(8, 11 * root.scaleFactor)
                        }

                        MouseArea {
                            anchors.fill: itemBackground1
                            hoverEnabled: true
                            onEntered: {
                                itemBackground1.color = "#4a4848"; // Color when hovered
                            }

                            onPressed: {
                                serialBaudRate.currentIndex = index;
                                serialBaudRate.popup.close();
                            }
                            onExited: {
                                itemBackground1.color = "#636363"; // Default color when not hovered
                            }
                        }
                    }
                }
            }

            /**** Start Button ****/

            MyButton {
                id: startButton
                property bool inValid_Name: false
                property bool inValid_Port: false
                property bool inValid_Broker : false
                property bool inValid_Topic: false


                source: "../Assets/powerButton.png"
                hoverText: "Start Session"
                width: Math.max(25, 35 * root.scaleFactor)
                height: Math.max(25, 35 * root.scaleFactor)
                fillMode: Image.PreserveAspectFit
                smooth: true

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: Math.max(25, 35 * scaleFactor)
                }

                onClicked: {
                    inValid_Name = (sessionNameField.text === "");

                    if (server_clientradio.checked) {
                        inValid_Port = (serverClientPortField.text === "");
                        if (!inValid_Name && !inValid_Port) {
                            var portNumber = parseInt(serverClientPortField.text.trim());
                            if (isNaN(portNumber) || portNumber < 1 || portNumber > 65535) {
                                inValid_Port = true;
                                return;
                            }
                            var success = communicationManager.startUdp(portNumber);
                            if (success) {
                                console.log("UDP client started successfully on port: " + portNumber);
                                stackView.push("WaitingScreen.qml", {
                                    "sessionName": sessionNameField.text,
                                    "portNumber": portNumber,
                                    "isSerialSource": false
                                });
                            } else {
                                console.error("Failed to start UDP client on port: " + portNumber);
                                inValid_Port = true;
                            }
                        }
                    }
                    else if(mqttradio.checked){
                        inValid_Port = (serverClientPortField.text === "");
                        inValid_Broker = (mqttbrokerAddressField.text === "");
                        inValid_Topic = (mqttTopicField.text === "");

                        if (!inValid_Name && !inValid_Port && !inValid_Broker && !inValid_Topic) {
                            var portNumber = parseInt(serverClientPortField.text.trim());
                            if (isNaN(portNumber) || portNumber < 1 || portNumber > 65535) {
                                inValid_Port = true;
                                return;
                            }

                            var useTls = (mqttProtocolField.currentIndex === 1);
                            var success = communicationManager.startMqtt(mqttbrokerAddressField.text, portNumber, useTls, mqttClientIdField.text, mqttUsernameField.text, mqttPasswordField.text, mqttTopicField.text);

                            if (success) {
                                console.log("MQTT client started successfully");
                                stackView.push("WaitingScreen.qml", {
                                    "sessionName": sessionNameField.text,
                                    "portNumber": portNumber,
                                    "isSerialSource": false
                                });
                            } else {
                                console.error("Failed to start MQTT client");
                            }
                        }
                    }


                    else if (serialradio.checked) {
                        inValid_Port = (serialPortField.currentIndex === -1 || serialBaudRate.currentIndex === -1);
                        if (!inValid_Name && !inValid_Port) {
                            var portName = serialPortField.model[serialPortField.currentIndex];
                            var baudRate = parseInt(serialBaudRate.model[serialBaudRate.currentIndex]);
                            var success = communicationManager.startSerial(portName, baudRate);
                            if (success) {
                                console.log("Serial manager started successfully on port: " + portName + " with baud rate: " + baudRate);
                                stackView.push("WaitingScreen.qml", {
                                    "sessionName": sessionNameField.text,
                                    "portName": portName,
                                    "baudRate": baudRate,
                                    "isSerialSource": true
                                });
                            } else {
                                console.error("Failed to start Serial manager on port: " + portName);
                                inValid_Port = true;
                            }
                        }
                    }
                }
            }

            Text {
                id: startText
                text: "Start"

                font {
                    family: "DS-Digital"
                    bold: true
                    pixelSize: Math.max(12, 18 * root.scaleFactor)
                }
                color: "turquoise"
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: startButton.bottom
                    topMargin: 5 * scaleFactor
                }
            }
        }

        /**** Right Rectangle (Extra Information Rectangle)  ****/

        Rectangle {
            id: extraInfromationRect
            width: (2 * parent.width / 3) - 50 * scaleFactor
            height: parent.height - 8 * root.scaleFactor
            color: "#09122C"
            radius: Math.max(20, 30 * scaleFactor)
            border.width: 1.5
            border.color: "#D84040"

            /**** Road Image ****/

            Image {
                id: roadImg
                source: "../Assets/road2.png"
                width: Math.max(240, 400 * root.scaleFactor)
                height: Math.max(240, 400 * root.scaleFactor)
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }

            /**** Car on The Road ****/

            Image {
                id: roadCarImg
                source: "../Assets/car3_white.png"
                width: Math.max(90, 150 * root.scaleFactor)
                height: Math.max(90, 150 * root.scaleFactor)
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.centerIn: roadImg
            }

            /**** Team Logo ****/

            Image {
                id: racingTeamLogo
                source: "../Assets/racinglogo.png"
                width: Math.max(60, 100 * root.scaleFactor)
                height: Math.max(60, 100 * root.scaleFactor)
                fillMode: Image.PreserveAspectFit
                smooth: true

                anchors {
                    right: parent.right
                    top: parent.top

                    topMargin: 10 * scaleFactor
                    rightMargin: 10 * scaleFactor
                }
            }

            /**** Layout for the Fields ****/

            Column {
                id: col
                anchors {
                    left: parent.left
                    top: parent.top
                    margins: 20 * scaleFactor
                }
                spacing: Math.max(10, 15 * scaleFactor)

                /**** Field to Get Driver Name ****/

                Text {
                    id: extraText
                    text: "Extra Information"
                    font {
                        bold: true
                        pixelSize: Math.max(12, 16 * root.scaleFactor)
                        family: "Amiri"
                    }
                    color: "turquoise"
                }
                Text {
                    text: "Enter Driver Name:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                        family: "Amiri"
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }
                TextField {
                    id: driverNameField
                    placeholderText: "Enter Driver Name"
                    placeholderTextColor: "turquoise"
                    width: Math.max(200, 300 * root.scaleFactor)
                    height: Math.max(24, 30 * root.scaleFactor)

                    font.pointSize: Math.max(8, 12 * root.scaleFactor)
                    verticalAlignment: TextInput.AlignVCenter
                    anchors.left: parent.left
                    color: "white"
                    background: Rectangle {
                        color: "#636363"
                        radius: 100 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }

                /**** Area to Write the Session Goals ****/

                Text {
                    text: "Enter Session Goals:"
                    font {
                        bold: true
                        pixelSize: Math.max(10, 13 * root.scaleFactor)
                        family: "Amiri"
                    }
                    anchors {
                        left: parent.left
                    }
                    color: "white"
                }
                TextArea {
                    id: goalsField
                    width: Math.max(220, 350 * root.scaleFactor)
                    height: Math.max(150, 250 * root.scaleFactor)
                    wrapMode: Text.Wrap
                    placeholderText: "Write your goals here..."
                    placeholderTextColor: "turquoise"
                    font.pointSize: Math.max(9, 13 * root.scaleFactor)
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"
                    background: Rectangle {
                        color: "#636363"
                        radius: 10 * scaleFactor
                        border.color: parent.activeFocus ? "turquoise" : "transparent"
                        border.width: Math.max(2, 4 * scaleFactor)
                    }
                }

                Text {
                    text: "Choose type of Communication : "
                    font {
                        bold: true
                        pixelSize: Math.max(11, 15 * root.scaleFactor)
                        family: "Amiri"
                    }

                    anchors.left: parent.left
                    color: "turquoise"
                }

                RadioButton {
                    id: server_clientradio
                    Text {
                        id: radio1text
                        text: "Server Client Communication (UDP)"
                        font {
                            bold: true
                            family: "DS-Digital"
                            pixelSize: Math.max(12, 18 * root.scaleFactor)
                        }
                        color: "white"

                        anchors.left: parent.right
                        anchors.leftMargin: 5 * scaleFactor
                    }
                }

                RadioButton {
                    id: mqttradio
                    checked: true
                    Text {
                        id: radio2text
                        text: "Server Client Communication (Mqtt)"
                        font {
                            bold: true
                            family: "DS-Digital"
                            pixelSize: Math.max(12, 18 * root.scaleFactor)
                        }
                        color: "white"

                        anchors.left: parent.right
                        anchors.leftMargin: 5 * scaleFactor
                    }
                }

                RadioButton {
                    id: serialradio
                    Text {
                        id: radio3text
                        text: "Serial Communication"
                        font {
                            bold: true
                            family: "DS-Digital"
                            pixelSize: Math.max(12, 18 * root.scaleFactor)
                        }
                        color: "white"

                        anchors.left: parent.right
                        anchors.leftMargin: 5 * scaleFactor
                    }
                }
            }
        }
    }
}
