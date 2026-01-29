import QtQuick

Rectangle {
    id: root

    // Scale factor for responsive sizing
    property real scaleFactor: 1.0
    
    // Scaled dimensions
    width: Math.max(10, 15 * scaleFactor)
    height: Math.max(35, 50 * scaleFactor)
    color: "#E0E0E0"
    border.color: "black"
    border.width: Math.max(1, 2 * scaleFactor)
    
    // Temperature properties
    property int minTemp: 0
    property int maxTemp: 120
    property int currentTemp: 0
    
    // Position label (FL, FR, BL, BR)
    required property string wheelPos
    
    Rectangle {
        id: progressBar
        width: parent.width - 4 * scaleFactor
        height: {
            var normalizedTemp = Math.max(0, Math.min(root.currentTemp - root.minTemp, root.maxTemp - root.minTemp));
            return parent.height * (normalizedTemp / (root.maxTemp - root.minTemp));
        }
        color: root.getColorForTemperature(root.currentTemp)
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 2 * scaleFactor
    }
    
    Text {
        id: posText
        text: root.wheelPos + ":"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: tempText.top
        font.pixelSize: Math.max(8, 12 * root.scaleFactor)  // Reduced for visual balance
        color: "turquoise"
    }

    Text {
        id: tempText
        text: root.currentTemp + "°C"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.top
        font.pixelSize: Math.max(9, 13 * root.scaleFactor)  // Reduced for visual balance
        font.family: "DS-Digital"
        color: root.getColorForTemperature(root.currentTemp)
    }

    // Function to interpolate color based on temperature
    // Blue (cold: 0-40°C) -> Green (optimal: 40-80°C) -> Red (hot: 80-120°C)
    function getColorForTemperature(temp) {
        var r, g, b;
        
        if (temp <= 40) {
            // Cold: Blue to Cyan
            // Interpolating from blue (0, 100, 255) to cyan (0, 255, 200)
            var ratio = temp / 40;
            r = 0;
            g = Math.floor(100 + 155 * ratio);  // 100 to 255
            b = Math.floor(255 - 55 * ratio);   // 255 to 200
        } else if (temp <= 80) {
            // Optimal: Cyan/Green
            // Interpolating from green (0, 255, 100) to yellow (255, 255, 0)
            var ratio = (temp - 40) / 40;
            r = Math.floor(255 * ratio);  // 0 to 255
            g = 255;
            b = Math.floor(100 * (1 - ratio));  // 100 to 0
        } else if (temp <= 120) {
            // Hot: Yellow to Red
            // Interpolating from yellow (255, 255, 0) to red (255, 0, 0)
            var ratio = (temp - 80) / 40;
            r = 255;
            g = Math.floor(255 * (1 - ratio));  // 255 to 0
            b = 0;
        } else {
            // Very hot: Deep red
            r = 200;
            g = 0;
            b = 0;
        }
        
        return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1).toUpperCase();
    }
}
