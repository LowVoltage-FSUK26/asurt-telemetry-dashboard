import QtQuick 2.15
import QtQuick.Controls 2.15
import QtWebEngine

Rectangle {
    id: camera

    property real scaleFactor: 1.0

    width: parent ? parent.width - 30 * scaleFactor : 300
    height: parent ? parent.height - 50 * scaleFactor : 400

    WebEngineView {
        id: webView
        anchors.fill: parent
        url: "qrc:/qt/qml/GUI/Assets/CameraWebPage/index.html"
        settings.localContentCanAccessRemoteUrls: true
        settings.localContentCanAccessFileUrls: true
        settings.allowRunningInsecureContent: true
    }
}
