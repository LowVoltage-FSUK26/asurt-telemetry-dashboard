cmake_minimum_required(VERSION 3.16)

project(GUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort Mqtt)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appGUI
    main.cpp
)

qt_add_qml_module(appGUI
    URI GUI
    VERSION 1.0
    QML_FILES
        Main.qml
        RESOURCES UI/Assets/30.gif UI/Assets/AI_car_transparent.png UI/Assets/back-button.png UI/Assets/batteryIcon.png UI/Assets/batteryIcon_blue.png UI/Assets/car3_white.png UI/Assets/Car1.png UI/Assets/Car2.png UI/Assets/CAR-215-ASURT.png UI/Assets/formulalogo.jpeg UI/Assets/GG_Diagram.png UI/Assets/marker.png UI/Assets/point.png UI/Assets/power.png UI/Assets/powerButton.png UI/Assets/racinglogo.png UI/Assets/road2.png UI/Assets/Steering_wheel.png UI/Assets/thermometer.png UI/Assets/Trial1.jpg
        QML_FILES UI/WelcomePage/MyButton.qml UI/WelcomePage/WaitingScreen.qml UI/WelcomePage/WelcomeScreen.qml
        QML_FILES UI/InformationPage/AcceleratorPedal.qml UI/InformationPage/BatteryLevelIndicator.qml UI/InformationPage/BrakePadel.qml UI/InformationPage/EulerGauges.qml UI/InformationPage/EulerVisual.qml UI/InformationPage/GpsPlotter.qml UI/InformationPage/Information.qml UI/InformationPage/RpmMeter.qml UI/InformationPage/Speedometer.qml UI/InformationPage/SteeringWheel.qml UI/InformationPage/TemperatureIndicator.qml UI/InformationPage/WheelSpeed.qml
        SOURCES Controllers/communication_manager/src/communicationmanager.cpp Controllers/communication_manager/include/communicationmanager.h Controllers/mqtt/src/mqttclient.cpp Controllers/mqtt/include/mqttclient.h Controllers/mqtt/src/mqttparserworker.cpp Controllers/mqtt/include/mqttparserworker.h Controllers/mqtt/src/mqttreceiverworker.cpp Controllers/mqtt/include/mqttreceiverworker.h Controllers/serial/src/serialmanager.cpp Controllers/serial/include/serialmanager.h Controllers/serial/src/serialparserworker.cpp Controllers/serial/include/serialparserworker.h Controllers/serial/src/serialreceiverworker.cpp Controllers/serial/include/serialreceiverworker.h Controllers/udp/src/udpclient.cpp Controllers/udp/include/udpclient.h Controllers/udp/src/udpparserworker.cpp Controllers/udp/include/udpparserworker.h Controllers/udp/src/udpreceiverworker.cpp Controllers/udp/include/udpreceiverworker.h
        QML_FILES UI/StatusBar/StatusBar.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appGUI PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appGUI
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appGUI
    PRIVATE Qt6::Quick Qt6::SerialPort Qt6::Mqtt
)

include(GNUInstallDirs)
install(TARGETS appGUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
