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
        RESOURCES Assets/30.gif Assets/AI_car_transparent.png Assets/back-button.png Assets/batteryIcon.png Assets/batteryIcon_blue.png Assets/car3_white.png Assets/Car1.png Assets/Car2.png Assets/CAR-215-ASURT.png Assets/formulalogo.jpeg Assets/GG_Diagram.png Assets/marker.png Assets/point.png Assets/power.png Assets/powerButton.png Assets/racinglogo.png Assets/road2.png Assets/Steering_wheel.png Assets/thermometer.png Assets/Trial1.jpg
        QML_FILES src/UI/WelcomePage/MyButton.qml src/UI/WelcomePage/WaitingScreen.qml src/UI/WelcomePage/WelcomeScreen.qml
        QML_FILES src/UI/InformationPage/AcceleratorPedal.qml src/UI/InformationPage/BatteryLevelIndicator.qml src/UI/InformationPage/BrakePadel.qml src/UI/InformationPage/EulerGauges.qml src/UI/InformationPage/EulerVisual.qml src/UI/InformationPage/GpsPlotter.qml src/UI/InformationPage/Information.qml src/UI/InformationPage/RpmMeter.qml src/UI/InformationPage/Speedometer.qml src/UI/InformationPage/SteeringWheel.qml src/UI/InformationPage/TemperatureIndicator.qml src/UI/InformationPage/WheelSpeed.qml
        SOURCES src/Controllers/communication_manager/src/communicationmanager.cpp src/Controllers/communication_manager/include/communicationmanager.h src/Controllers/mqtt/src/mqttclient.cpp src/Controllers/mqtt/include/mqttclient.h src/Controllers/mqtt/src/mqttparserworker.cpp src/Controllers/mqtt/include/mqttparserworker.h src/Controllers/mqtt/src/mqttreceiverworker.cpp src/Controllers/mqtt/include/mqttreceiverworker.h src/Controllers/serial/src/serialmanager.cpp src/Controllers/serial/include/serialmanager.h src/Controllers/serial/src/serialparserworker.cpp src/Controllers/serial/include/serialparserworker.h src/Controllers/serial/src/serialreceiverworker.cpp src/Controllers/serial/include/serialreceiverworker.h src/Controllers/udp/src/udpclient.cpp src/Controllers/udp/include/udpclient.h src/Controllers/udp/src/udpparserworker.cpp src/Controllers/udp/include/udpparserworker.h src/Controllers/udp/src/udpreceiverworker.cpp src/Controllers/udp/include/udpreceiverworker.h src/Controllers/can/src/candecoder.cpp src/Controllers/can/include/candecoder.h src/Controllers/logging/src/asynclogger.cpp src/Controllers/logging/include/asynclogger.h
        QML_FILES src/UI/StatusBar/StatusBar.qml
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
