#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QVideoSink>
#include <QThread>
#include <QtQml>
#include <src/Controllers/communication_manager/include/communicationmanager.h>
#include <src/Controllers/logging/include/asynclogger.h>
#include <src/Controllers/mqtt/include/mqttclient.h>
#include <src/Controllers/serial/include/serialmanager.h>
#include <src/Controllers/udp/include/udpclient.h>
#include <src/Controllers/webrtc/include/webrtcclient.h>

int main(int argc, char *argv[]) {

  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  QGuiApplication app(argc, argv);

  QCoreApplication::setOrganizationName("ASURT");
  QCoreApplication::setOrganizationDomain("asurt.eu");
  QCoreApplication::setApplicationName("Car_Dashboard");

  qmlRegisterUncreatableType<QVideoSink>("QtMultimedia", 6, 0, "VideoSink", "Not creatable");

  QQmlApplicationEngine engine;
  UdpClient udpClient;
  SerialManager serialManager;
  MqttClient mqttClient;
  webrtcclient *rtc = new webrtcclient(&app);
  CommunicationManager communicationManager;

  engine.rootContext()->setContextProperty("communicationManager",
                                           &communicationManager);

  engine.rootContext()->setContextProperty("udpClient", &udpClient);
  engine.rootContext()->setContextProperty("serialManager", &serialManager);
  engine.rootContext()->setContextProperty("mqttClient", &mqttClient);
  engine.rootContext()->setContextProperty("webrtcClient", rtc);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("GUI", "Main");

  int result = app.exec();

  // Ensure proper cleanup before exit
  AsyncLogger::instance().shutdown();

  return result;
}
