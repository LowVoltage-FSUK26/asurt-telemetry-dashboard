import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphs

Rectangle {
    id: lineGraph
    color: "transparent"

    property real scaleFactor: 1.0
    property real graphVariable: 0.0
    property int pointCount: 0

    GraphsView {
        id: graph
        anchors.fill: parent
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            theme: GraphsTheme.Theme.QtGreen
            backgroundVisible: false
            plotAreaBackgroundVisible: false
        }
        axisX: ValueAxis {
            id: xAxis; min: 0; max: 100
            Behavior on max { NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }
        }
        axisY: ValueAxis {
            id: yAxis; min: 0; max: 100
            Behavior on max { NumberAnimation { duration: 500; easing.type: Easing.OutCubic } }
        }
        LineSeries {
            id: graphData
            name: "Line Graph"
        }
    }

    function updateHistory(newValue) {
        graphData.append(pointCount, newValue);
        if (pointCount > xAxis.max) xAxis.max *= 2;
        if (newValue > yAxis.max) yAxis.max *= 2;
        pointCount++;
    }

    onGraphVariableChanged: {
        updateHistory(graphVariable)
    }
}
