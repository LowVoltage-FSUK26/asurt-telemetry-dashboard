import QtQuick
import QtQuick.Controls


Item {
    id: rpmMeter
    
    // Scale factor for responsive sizing
    property real scaleFactor: 1.0
    
    // Design-time dimensions
    readonly property real designWidth: 360
    readonly property real designHeight: 360
    
    // Scaled dimensions
    width: designWidth * scaleFactor
    height: designHeight * scaleFactor

    property int rpm: 0

    Behavior on rpm {
        NumberAnimation {
            duration: 25
            easing.type: Easing.InOutQuad
        }
    }

    onRpmChanged: {
        gaugeCanvas.requestPaint()
        needleCanvas.requestPaint()
    }
    
    onScaleFactorChanged: {
        gaugeCanvas.requestPaint()
        needleCanvas.requestPaint()
    }

    function gaugeValue() {
        return rpm / 1000.0;
    }

    function getRpmColor(value) {
        if (value <= 4) return "#4CAF50";
        if (value <= 6) return "#FFC107";
        return "#F44336";
    }

    property real maxGauge: 8;
    property real startAngle: 180;
    property real arcAngle: 180;

    Column {
        anchors.centerIn: parent
        spacing: 8 * scaleFactor

        Item {
            id: gaugeItem
            width: 270 * scaleFactor
            height: 250 * scaleFactor

            Canvas {
                id: gaugeCanvas
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d");
                    var centerX = width / 2;
                    var centerY = height / 2;
                    var radius = 108 * rpmMeter.scaleFactor;

                    ctx.clearRect(0, 0, width, height);
                    ctx.save();
                    ctx.translate(centerX, centerY);

                    var step = 0.2;
                    var numSteps = rpmMeter.maxGauge / step;
                    for (var i = 0; i <= numSteps; i++) {
                        var tickValue = i * step;
                        var intVal = Math.round(tickValue);
                        if (Math.abs(tickValue - intVal) < 0.001)
                            tickValue = intVal;
                        var t = tickValue / rpmMeter.maxGauge;
                        var angle = rpmMeter.startAngle + t * rpmMeter.arcAngle;
                        angle = angle * Math.PI / 180;

                        var isMajor = (Math.abs(tickValue - Math.round(tickValue)) < 0.001);
                        var lineWidth = (isMajor ? 4 : 2) * rpmMeter.scaleFactor;
                        var lineLength = (isMajor ? 18 : 9) * rpmMeter.scaleFactor;

                        var startX = radius * Math.cos(angle);
                        var startY = radius * Math.sin(angle);
                        var endX = (radius - lineLength) * Math.cos(angle);
                        var endY = (radius - lineLength) * Math.sin(angle);

                        var currentGauge = rpmMeter.gaugeValue();
                        var dashColor = (tickValue <= currentGauge) ? rpmMeter.getRpmColor(tickValue) : "#808080";

                        ctx.strokeStyle = dashColor;
                        ctx.lineWidth = lineWidth;
                        ctx.beginPath();
                        ctx.moveTo(startX, startY);
                        ctx.lineTo(endX, endY);
                        ctx.stroke();

                        if (isMajor) {
                            var labelRadius = radius - 31.5 * rpmMeter.scaleFactor;
                            var labelX = labelRadius * Math.cos(angle);
                            var labelY = labelRadius * Math.sin(angle) + 4.5 * rpmMeter.scaleFactor;
                            var fontSize = Math.max(9, 12.6 * rpmMeter.scaleFactor);
                            ctx.font = "bold " + fontSize + "px sans-serif";
                            ctx.fillStyle = rpmMeter.getRpmColor(tickValue);
                            ctx.textAlign = "center";
                            ctx.textBaseline = "middle";
                            ctx.fillText(tickValue.toString(), labelX, labelY);
                        }
                    }
                    ctx.restore();
                }
            }

            Canvas {
                id: needleCanvas
                anchors.fill: parent

                onPaint: {
                    var ctx = getContext("2d");
                    var centerX = width / 2;
                    var centerY = height / 2;
                    var radius = 90 * rpmMeter.scaleFactor;

                    ctx.clearRect(0, 0, width, height);
                    var currentGauge = rpmMeter.gaugeValue();
                    var t = currentGauge / rpmMeter.maxGauge;
                    var angle = rpmMeter.startAngle + t * rpmMeter.arcAngle;
                    angle = angle * Math.PI / 180;
                    var needleX = radius * Math.cos(angle);
                    var needleY = radius * Math.sin(angle);

                    ctx.shadowColor = 'rgba(0, 0, 0, 0.3)';
                    ctx.shadowBlur = 5 * rpmMeter.scaleFactor;
                    ctx.shadowOffsetX = 2 * rpmMeter.scaleFactor;
                    ctx.shadowOffsetY = 2 * rpmMeter.scaleFactor;

                    ctx.strokeStyle = rpmMeter.getRpmColor(currentGauge);
                    ctx.lineWidth = 4 * rpmMeter.scaleFactor;
                    ctx.beginPath();
                    ctx.moveTo(centerX, centerY);
                    ctx.lineTo(centerX + needleX, centerY + needleY);
                    ctx.stroke();

                    ctx.beginPath();
                    ctx.arc(centerX, centerY, 7.2 * rpmMeter.scaleFactor, 0, 2 * Math.PI);
                    ctx.fillStyle = rpmMeter.getRpmColor(currentGauge);
                    ctx.fill();
                }
            }

            Text {
                id: rpmText
                text: rpmMeter.rpm.toString() + " rpm"
                color: rpmMeter.getRpmColor(rpmMeter.gaugeValue())
                font.pixelSize: Math.max(14, 22 * rpmMeter.scaleFactor)
                font.bold: true
                anchors {
                    bottom: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                    bottomMargin: 2 * rpmMeter.scaleFactor
                }
            }

            Text {
                id: rangeText
                text: {
                    var val = rpmMeter.gaugeValue();
                    if (val <= 4) return "Safe";
                    if (val <= 6) return "Moderate";
                    return "Danger";
                }
                color: rpmMeter.getRpmColor(rpmMeter.gaugeValue())
                font.pixelSize: Math.max(9, 12 * rpmMeter.scaleFactor)
                font.bold: true
                anchors {
                    top: rpmText.bottom
                    horizontalCenter: parent.horizontalCenter
                    topMargin: 2 * rpmMeter.scaleFactor
                }
            }
        }
    }
}
