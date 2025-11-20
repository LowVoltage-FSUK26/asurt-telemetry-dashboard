#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <Controllers/udp/include/udpclient.h>
#include <Controllers/serial/include/serialmanager.h>
#include <Controllers/communication_manager/include/communicationmanager.h>
#include <Controllers/mqtt/include/mqttclient.h>
#include <QQmlContext>
#include <QThread>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    UdpClient udpClient;
    SerialManager serialManager;
    MqttClient mqttClient;
    CommunicationManager communicationManager;

    engine.rootContext()->setContextProperty("communicationManager", &communicationManager);

    engine.rootContext()->setContextProperty("udpClient", &udpClient);
    engine.rootContext()->setContextProperty("serialManager", &serialManager);
    engine.rootContext()->setContextProperty("mqttClient", &mqttClient);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("GUI", "Main");

    return app.exec();
}


