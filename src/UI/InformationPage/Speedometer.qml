import QtQuick 2.15

Item {
    id: speedometer
    
    // Scale factor for responsive sizing
    property real scaleFactor: 1.0
    
    // Design-time dimensions
    readonly property real designWidth: 360
    readonly property real designHeight: 450
    
    // Scaled dimensions
    width: designWidth * scaleFactor
    height: designHeight * scaleFactor

    // Speed (float) property
    property real speed: 0

    Behavior on speed {
        NumberAnimation {
            duration: 25
            easing.type: Easing.InOutQuad
        }
    }

    onSpeedChanged: {
        gaugeCanvas.requestPaint()
        needleCanvas.requestPaint()
    }
    
    onScaleFactorChanged: {
        gaugeCanvas.requestPaint()
        needleCanvas.requestPaint()
    }

    function getSpeedColor(value) {
        if (value <= 60) return "#4CAF50"
        if (value <= 120) return "#FFC107"
        if (value <= 180) return "#FF9800"
        return "#F44336"
    }

    Column {
        anchors.centerIn: parent
        spacing: 18 * scaleFactor

        Item {
            id: gaugeItem
            width: 270 * scaleFactor
            height: 270 * scaleFactor

            Canvas {
                id: gaugeCanvas
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d")
                    var centerX = width / 2
                    var centerY = height / 2
                    var radius = 126 * speedometer.scaleFactor

                    ctx.clearRect(0, 0, width, height)
                    ctx.save()
                    ctx.translate(centerX, centerY)

                    for (var i = 0; i <= 240; i += 2) {
                        var angle = (i / 240 * 240 - 210) * Math.PI / 180
                        var isMajor = (i % 10 === 0)
                        var lineWidth = (isMajor ? 4 : 3) * speedometer.scaleFactor
                        var lineLength = (isMajor ? 18 : 9) * speedometer.scaleFactor
                        var startX = radius * Math.cos(angle)
                        var startY = radius * Math.sin(angle)
                        var endX = (radius - lineLength) * Math.cos(angle)
                        var endY = (radius - lineLength) * Math.sin(angle)
                        var dashColor = (i <= speedometer.speed) ? speedometer.getSpeedColor(i) : "#808080"

                        ctx.strokeStyle = dashColor
                        ctx.lineWidth = lineWidth
                        ctx.beginPath()
                        ctx.moveTo(startX, startY)
                        ctx.lineTo(endX, endY)
                        ctx.stroke()

                        if (i === 0 || i === 60 || i === 120 || i === 180 || i === 240) {
                            var labelRadius = radius - 31.5 * speedometer.scaleFactor
                            var labelX = labelRadius * Math.cos(angle)
                            var labelY = labelRadius * Math.sin(angle) + 4.5 * speedometer.scaleFactor
                            var fontSize = Math.max(9, 12.6 * speedometer.scaleFactor)
                            ctx.font = "bold " + fontSize + "px sans-serif"
                            ctx.fillStyle = speedometer.getSpeedColor(i)
                            ctx.textAlign = "center"
                            ctx.textBaseline = "middle"
                            ctx.fillText(i.toString(), labelX, labelY)
                        }
                    }
                    ctx.restore()
                }
            }

            Canvas {
                id: needleCanvas
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d")
                    var centerX = width / 2
                    var centerY = height / 2
                    var radius = 108 * speedometer.scaleFactor

                    ctx.clearRect(0, 0, width, height)
                    var angle = (speedometer.speed / 240 * 240 - 210) * Math.PI / 180
                    var needleX = radius * Math.cos(angle)
                    var needleY = radius * Math.sin(angle)

                    ctx.shadowColor = 'rgba(0, 0, 0, 0.3)'
                    ctx.shadowBlur = 5 * speedometer.scaleFactor
                    ctx.shadowOffsetX = 2 * speedometer.scaleFactor
                    ctx.shadowOffsetY = 2 * speedometer.scaleFactor

                    ctx.strokeStyle = speedometer.getSpeedColor(speedometer.speed)
                    ctx.lineWidth = 4 * speedometer.scaleFactor
                    ctx.beginPath()
                    ctx.moveTo(centerX, centerY)
                    ctx.lineTo(centerX + needleX, centerY + needleY)
                    ctx.stroke()

                    ctx.beginPath()
                    ctx.arc(centerX, centerY, 7.2 * speedometer.scaleFactor, 0, 2 * Math.PI)
                    ctx.fillStyle = speedometer.getSpeedColor(speedometer.speed)
                    ctx.fill()
                }
            }

            Text {
                id: speedText
                text: speedometer.speed.toFixed(1) + " km/h"
                color: speedometer.getSpeedColor(speedometer.speed)
                font.pixelSize: Math.max(14, 20 * speedometer.scaleFactor)
                font.bold: true
                anchors {
                    bottom: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                }
            }

            Text {
                id: rangeText
                text: {
                    if (speedometer.speed <= 60) return "Safe"
                    if (speedometer.speed <= 120) return "Moderate"
                    if (speedometer.speed <= 180) return "High"
                    return "Danger"
                }
                color: speedometer.getSpeedColor(speedometer.speed)
                font.pixelSize: Math.max(9, 12 * speedometer.scaleFactor)
                font.bold: true
                anchors {
                    top: speedText.bottom
                    horizontalCenter: parent.horizontalCenter
                    topMargin: 2 * speedometer.scaleFactor
                }
            }
        }
    }
}
