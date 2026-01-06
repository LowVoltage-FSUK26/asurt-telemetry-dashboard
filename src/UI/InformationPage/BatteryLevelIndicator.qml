import QtQuick 2.15

Item {
    id: root
    property int batteryLevel: 0
    property real scaleFactor: 1.0
    
    // Explicit dimensions for proper anchoring
    width: indicator.width
    height: indicator.height

    onScaleFactorChanged: batteryCanvas.requestPaint()

    Column {
        id: indicator
        spacing: 2 * root.scaleFactor
        
        Image {
            id: batteryIcon
            source: (root.batteryLevel > 30) ? "../../../Assets/batteryIcon_blue.png" : "../../../Assets/batteryIcon.png"
            width: Math.max(16, 24 * root.scaleFactor)
            height: Math.max(16, 24 * root.scaleFactor)
            fillMode: Image.PreserveAspectFit
            smooth: true
        }
        Text {
            id: batteryText
            text: root.batteryLevel + "%"
            color: batteryIndicator.getColorForBattery(root.batteryLevel)
            font {
                pixelSize: Math.max(12, 18 * root.scaleFactor)
                bold: true
                family: "DS-Digital"
            }
        }
        Item {
            id: batteryIndicator
            width: Math.max(30, 45 * root.scaleFactor)
            height: Math.max(50, 80 * root.scaleFactor)
            
            function getColorForBattery(level) {
                var r = Math.floor(255 * (1 - level / 100));
                var g = Math.floor(255 * (level / 100));
                var b = 0;
                return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1).toUpperCase();
            }
            
            Canvas {
                id: batteryCanvas
                anchors.fill: parent
                
                onPaint: {
                    var ctx = getContext("2d");
                    var barWidth = batteryCanvas.width;
                    var barHeight = batteryCanvas.height / 10;
                    var numberOfBars = 10;
                    ctx.clearRect(0, 0, batteryCanvas.width, batteryCanvas.height);
                    var barColor = batteryIndicator.getColorForBattery(root.batteryLevel);
                    for (var i = 0; i < numberOfBars; i++) {
                        var yPosition = i * barHeight;
                        ctx.fillStyle = (i < Math.ceil((100 - root.batteryLevel) / 10)) ? "black" : barColor;
                        ctx.fillRect(0, yPosition, barWidth, barHeight - 2 * root.scaleFactor);
                    }
                }
            }
        }
        
        Connections {
            target: communicationManager
            function onBatteryLevelChanged() {
                root.batteryLevel = communicationManager ? communicationManager.batteryLevel : 0;
                batteryCanvas.requestPaint();
            }
        }
    }
}
