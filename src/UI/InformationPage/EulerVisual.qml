import QtQuick
import QtQuick3D

View3D {
    id: view3D
    width: 150
    height: 150
    environment: SceneEnvironment {
        clearColor: "transparent"
        backgroundMode: SceneEnvironment.Color
    }

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(50, 350, 400)
        eulerRotation.x: -30
        eulerRotation.y: 0 // Look straight down the Y-axis
        eulerRotation.z: 0 // No roll
    }

    DirectionalLight {
        id: light
        eulerRotation.x: -30
        eulerRotation.y: -70
    }

    Model {
        id: cubeModel
        position: Qt.vector3d(0, 150, 0)
        source: "#Cube"
        materials: [
            DefaultMaterial {
                diffuseColor: "blue"
            }
        ]
        // Scale to adjust size (width, height, depth)
        scale: Qt.vector3d(1, 0.5, 2)
        // Custom properties to hold Euler angles
        property real yaw: 0
        property real pitch: 0
        property real roll: 0
    }

    Connections {
        target: udpClient  // udpClient is the source of Euler angle data
        function onYawAngleChanged() {
            cubeModel.yaw = udpClient.yawAngle
            cubeModel.eulerRotation.y = cubeModel.yaw  // Update Yaw (Y-axis rotation)
        }
        function onPitchAngleChanged() {
            cubeModel.pitch = udpClient.pitchAngle
            cubeModel.eulerRotation.x = cubeModel.pitch  // Update Pitch (X-axis rotation)
        }
        function onRollAngleChanged() {
            cubeModel.roll = udpClient.rollAngle
            cubeModel.eulerRotation.z = cubeModel.roll  // Update Roll (Z-axis rotation)
        }
    }
}
