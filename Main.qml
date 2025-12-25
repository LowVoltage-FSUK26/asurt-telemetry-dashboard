import QtQuick
import QtQuick.Window
import QtQuick.Controls



Window {
    id: root
    width: 1400
    height: 780
    minimumWidth: 1000
    minimumHeight: 560
    visible: true
    title: qsTr("CarDashboard_AsuRacingTeam")
    color: "black"

    // Global scale factor based on window size vs design size (1400x780)
    readonly property real globalScale: Math.min(width / 1400, height / 780)

    StackView {
        id : stackView
        anchors.fill : parent
        initialItem : "UI/WelcomePage/WelcomeScreen.qml"
    }

}
