#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>
#include <src/Controllers/communication_manager/include/communicationmanager.h>
#include <src/Controllers/logging/include/asynclogger.h>
#include <src/Controllers/mqtt/include/mqttclient.h>
#include <src/Controllers/serial/include/serialmanager.h>
#include <src/Controllers/udp/include/udpclient.h>

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  // Set application information for QSettings
  QCoreApplication::setOrganizationName("ASURT");
  QCoreApplication::setOrganizationDomain("asurt.eu");
  QCoreApplication::setApplicationName("Car_Dashboard");

  QQmlApplicationEngine engine;
  UdpClient udpClient;
  SerialManager serialManager;
  MqttClient mqttClient;
  CommunicationManager communicationManager;

  engine.rootContext()->setContextProperty("communicationManager",
                                           &communicationManager);

  engine.rootContext()->setContextProperty("udpClient", &udpClient);
  engine.rootContext()->setContextProperty("serialManager", &serialManager);
  engine.rootContext()->setContextProperty("mqttClient", &mqttClient);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("GUI", "Main");

  int result = app.exec();

  // Ensure proper cleanup before exit
  AsyncLogger::instance().shutdown();

  return result;
}
