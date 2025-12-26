import QtQuick
import QtQuick.Controls 2.15

Image {
    id: root

    signal clicked
    property string hoverText
    opacity: mouseArea.containsMouse ? 0.4 : 1

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent
        onClicked: {
            root.clicked()
        }
        onEntered: toolTip.visible = true
        onExited: toolTip.visible = false
    }

    ToolTip {
        id: toolTip
        text: root.hoverText
        visible: false
    }
}
