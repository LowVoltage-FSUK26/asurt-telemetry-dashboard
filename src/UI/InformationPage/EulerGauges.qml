import QtQuick 2.15

Item {
    id : root
    property double eulerAngle
    // Angle property to represent Euler angle
    required property string textVal
    property real scaleFactor

    Column {
        id: gauge
        anchors.centerIn: parent
        spacing: 10 * root.scaleFactor
        // Euler Angle Gauge using Dial
        Item {
            id: eulerGauge
            width: 90 * root.scaleFactor
            height: 90 * root.scaleFactor

            // Draw gauge dashes and labels
            Canvas {
                id: gaugeCanvas
                anchors.centerIn: parent
                width: 90 * root.scaleFactor
                height: 90 * root.scaleFactor

                onPaint: {
                    var ctx = getContext("2d")
                    var centerX = width / 2
                    var centerY = height
                    var radius = 40 * root.scaleFactor

                    // Clear canvas
                    ctx.clearRect(0, 0, width, height)

                    // Draw gauge lines
                    ctx.save()
                    ctx.translate(centerX, centerY)
                    ctx.rotate(2 * Math.PI) // Rotate to make it a half-circle

                    // From -90° (left) to 90° (right)
                    for (var i = -90; i <= 90; i += 10) {
                        //var angle = (i * Math.PI / 180) // Convert to radians
                        var angle = (i / 360 * 360 - 90) * Math.PI / 180 // Adjusted for full circle
                        var isMajor = i % 30 === 0
                        var lineLength = (isMajor ? 15 : 10) * root.scaleFactor
                        var startX = radius * Math.cos(angle)
                        var startY = radius * Math.sin(angle)
                        var endX = (radius - lineLength) * Math.cos(angle)
                        var endY = (radius - lineLength) * Math.sin(angle)

                        // Check if dash should be colored turquoise (if needle has passed)
                        var dashColor = (i < 0) ? ((i > root.eulerAngle) ? "turquoise" : "black") : ((i <= root.eulerAngle) ? "turquoise" : "black")

                        // Draw the dash
                        ctx.strokeStyle = dashColor
                        ctx.lineWidth = 2 * root.scaleFactor
                        ctx.beginPath()
                        ctx.moveTo(startX, startY)
                        ctx.lineTo(endX, endY)
                        ctx.stroke()

                        // Draw label for 0 position
                        if (i === 0) {
                            var labelRadius = radius - 15 * root.scaleFactor
                            var labelX = labelRadius * Math.cos(angle)
                            var labelY = labelRadius * Math.sin(angle) + 3
                            ctx.font = "bold " + 8 * root.scaleFactor + "px " + "DS-Digital"
                            ctx.fillStyle = "turquoise"
                            ctx.textAlign = "center"
                            ctx.textBaseline = "middle"
                            ctx.fillText(i.toString(), labelX, labelY)
                        }
                    }
                    ctx.restore()
                }
            }

            // Euler angle label (display the Euler angle in the center)
            Text {
                id: angleText
                text: root.eulerAngle + "°"
                color: Math.abs(root.eulerAngle) > 45 ? "red" : "turquoise"
                font {
                    pixelSize: 15 * root.scaleFactor
                    bold: true
                    family: "DS-Digital"
                }

                anchors {
                    bottom: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
    // angle label
    Text {
        id: label
        text: root.textVal
        color: "turquoise"
        font {
            bold: true
            pixelSize: 10 * root.scaleFactor
        }

        anchors {
            top: gauge.bottom
            horizontalCenter: parent.horizontalCenter
            topMargin: 10 * root.scaleFactor
        }
    }
    function rePaint() {
        gaugeCanvas.requestPaint()
    }
}
