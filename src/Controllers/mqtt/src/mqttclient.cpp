
#include "../include/mqttclient.h"
#include "../../logging/include/asynclogger.h"
#include "../include/mqttparserworker.h"
#include "../include/mqttreceiverworker.h"
#include <QDebug>
#include <QThread>

MqttClient::MqttClient(QObject *parent)
    : QObject(parent), m_nextParserIndex(0),
      m_parserThreadCount(QThread::idealThreadCount()), m_debugMode(false),
      m_pendingUpdate(false),
      m_messagesProcessed(0), m_messagesDropped(0), m_speed(0.0f), m_rpm(0),
      m_accPedal(0), m_brakePedal(0), m_encoderAngle(0.0), m_temperature(0.0f),
      m_batteryLevel(0), m_gpsLongitude(0.0), m_gpsLatitude(0.0), m_speedFL(0),
      m_speedFR(0), m_speedBL(0), m_speedBR(0), m_lateralG(0.0),
      m_longitudinalG(0.0), m_tempFL(0), m_tempFR(0), m_tempBL(0), m_tempBR(0) {
  // Initialize async logger
  AsyncLogger::instance().initialize("./logs");

  // Initialize update throttling timer (60Hz = 16ms interval)
  m_updateTimer = new QTimer(this);
  m_updateTimer->setInterval(16);
  connect(m_updateTimer, &QTimer::timeout, this, &MqttClient::flushPendingUpdates);
  m_updateTimer->start();

  m_receiverWorker = new MqttReceiverWorker();
  m_receiverWorker->moveToThread(&m_receiverThread);

  connect(this, &MqttClient::startReceiving, m_receiverWorker,
          &MqttReceiverWorker::startReceiving, Qt::QueuedConnection);
  connect(this, &MqttClient::stopReceiving, m_receiverWorker,
          &MqttReceiverWorker::stopReceiving, Qt::QueuedConnection);
  connect(m_receiverWorker, &MqttReceiverWorker::messageReceived, this,
          &MqttClient::handleMqttMessageReceived, Qt::QueuedConnection);
  connect(m_receiverWorker, &MqttReceiverWorker::errorOccurred, this,
          &MqttClient::handleError, Qt::QueuedConnection);

  connect(&m_receiverThread, &QThread::started, m_receiverWorker,
          &MqttReceiverWorker::initialize);
  connect(&m_receiverThread, &QThread::finished, m_receiverWorker,
          &QObject::deleteLater);

  m_parserPool.setMaxThreadCount(m_parserThreadCount);
}

MqttClient::~MqttClient() {
  // Stop the update timer first
  if (m_updateTimer) {
    m_updateTimer->stop();
  }

  stop();

  if (m_receiverThread.isRunning()) {
    m_receiverThread.quit();
    if (!m_receiverThread.wait(3000)) {
      qWarning() << "MQTT receiver thread did not terminate gracefully";
      m_receiverThread.terminate();
      m_receiverThread.wait(1000);
    }
  }

  cleanupParsers();
}

bool MqttClient::start(const QString &brokerAddress, quint16 port, bool useTls,
                       const QString &clientId, const QString &username,
                       const QString &password, const QString &topic) {
  QThread::currentThread()->setObjectName("Main Thread");

  stop();
  initializeParsers();

  m_receiverThread.start();
  m_receiverThread.setPriority(QThread::HighPriority);

  emit startReceiving(brokerAddress, port, useTls, clientId, username, password,
                      topic);

  if (m_debugMode) {
    qDebug() << "MQTT Client started on broker" << brokerAddress << ":" << port
             << "running on the " << QThread::currentThread() << "with"
             << m_parserThreadCount << "parser threads";
  }

  return true;
}

bool MqttClient::stop() {
  emit stopReceiving();
  cleanupParsers();

  if (m_debugMode) {
    qDebug() << "MQTT Client stopped";
  }

  return true;
}

void MqttClient::setParserThreadCount(int count) {
  if (count > 0 && count <= QThread::idealThreadCount() * 2) {
    m_parserThreadCount = count;
    m_parserPool.setMaxThreadCount(m_parserThreadCount);

    if (m_debugMode) {
      qDebug() << "Parser thread count set to" << count;
    }
  }
}

void MqttClient::setDebugMode(bool enabled) {
  m_debugMode = enabled;

  if (m_debugMode) {
    qDebug() << "Debug mode enabled";
  }
}

void MqttClient::handleMqttMessageReceived(const QByteArray &message) {
  if (!m_parsers.isEmpty()) {
    MqttParserWorker *parser = m_parsers[m_nextParserIndex];
    parser->queueMessage(message);
    m_nextParserIndex = (m_nextParserIndex + 1) % m_parsers.size();
  }
}

void MqttClient::handleParsedData(float speed, int rpm, int accPedal,
                                  int brakePedal, double encoderAngle,
                                  float temperature, int batteryLevel,
                                  double gpsLongitude, double gpsLatitude,
                                  int speedFL, int speedFR, int speedBL,
                                  int speedBR, double lateralG,
                                  double longitudinalG, int tempFL, int tempFR,
                                  int tempBL, int tempBR) {
  m_messagesProcessed.fetch_add(1);

  // Store all values atomically without emitting signals
  // Signals will be emitted by flushPendingUpdates() at 60Hz
  m_speed.store(speed, std::memory_order_relaxed);
  m_rpm.store(rpm, std::memory_order_relaxed);
  m_accPedal.store(accPedal, std::memory_order_relaxed);
  m_brakePedal.store(brakePedal, std::memory_order_relaxed);
  m_encoderAngle.store(encoderAngle, std::memory_order_relaxed);
  m_temperature.store(temperature, std::memory_order_relaxed);
  m_batteryLevel.store(batteryLevel, std::memory_order_relaxed);
  m_gpsLongitude.store(gpsLongitude, std::memory_order_relaxed);
  m_gpsLatitude.store(gpsLatitude, std::memory_order_relaxed);
  m_speedFL.store(speedFL, std::memory_order_relaxed);
  m_speedFR.store(speedFR, std::memory_order_relaxed);
  m_speedBL.store(speedBL, std::memory_order_relaxed);
  m_speedBR.store(speedBR, std::memory_order_relaxed);
  m_lateralG.store(lateralG, std::memory_order_relaxed);
  m_longitudinalG.store(longitudinalG, std::memory_order_relaxed);
  m_tempFL.store(tempFL, std::memory_order_relaxed);
  m_tempFR.store(tempFR, std::memory_order_relaxed);
  m_tempBL.store(tempBL, std::memory_order_relaxed);
  m_tempBR.store(tempBR, std::memory_order_relaxed);

  // Mark that we have pending updates
  m_pendingUpdate.store(true, std::memory_order_release);
}

void MqttClient::flushPendingUpdates() {
  // Check if we have any pending updates
  if (!m_pendingUpdate.exchange(false, std::memory_order_acquire)) {
    return; // No updates pending
  }

  // Read all current values and emit signals
  // This batches all updates to a maximum of 60Hz
  emit speedChanged(m_speed.load(std::memory_order_relaxed));
  emit rpmChanged(m_rpm.load(std::memory_order_relaxed));
  emit accPedalChanged(m_accPedal.load(std::memory_order_relaxed));
  emit brakePedalChanged(m_brakePedal.load(std::memory_order_relaxed));
  emit encoderAngleChanged(m_encoderAngle.load(std::memory_order_relaxed));
  emit temperatureChanged(m_temperature.load(std::memory_order_relaxed));
  emit batteryLevelChanged(m_batteryLevel.load(std::memory_order_relaxed));
  emit gpsLongitudeChanged(m_gpsLongitude.load(std::memory_order_relaxed));
  emit gpsLatitudeChanged(m_gpsLatitude.load(std::memory_order_relaxed));
  emit speedFLChanged(m_speedFL.load(std::memory_order_relaxed));
  emit speedFRChanged(m_speedFR.load(std::memory_order_relaxed));
  emit speedBLChanged(m_speedBL.load(std::memory_order_relaxed));
  emit speedBRChanged(m_speedBR.load(std::memory_order_relaxed));
  emit lateralGChanged(m_lateralG.load(std::memory_order_relaxed));
  emit longitudinalGChanged(m_longitudinalG.load(std::memory_order_relaxed));
  emit tempFLChanged(m_tempFL.load(std::memory_order_relaxed));
  emit tempFRChanged(m_tempFR.load(std::memory_order_relaxed));
  emit tempBLChanged(m_tempBL.load(std::memory_order_relaxed));
  emit tempBRChanged(m_tempBR.load(std::memory_order_relaxed));
}

void MqttClient::handleError(const QString &error) {
  if (m_debugMode) {
    qDebug() << "MQTT Client error:" << error;
  }

  emit errorOccurred(error);
}

void MqttClient::initializeParsers() {
  for (int i = 0; i < m_parserThreadCount; ++i) {
    MqttParserWorker *parser = new MqttParserWorker(m_debugMode);

    connect(parser, &MqttParserWorker::messageParsed, this,
            &MqttClient::handleParsedData, Qt::QueuedConnection);
    connect(parser, &MqttParserWorker::errorOccurred, this,
            &MqttClient::handleError, Qt::QueuedConnection);

    m_parsers.append(parser);
    m_parserPool.start(parser);
  }

  m_nextParserIndex = 0;
}

void MqttClient::cleanupParsers() {
  // Stop all parsers
  for (MqttParserWorker *parser : m_parsers) {
    parser->stop();
  }

  // Wait for all tasks to complete with timeout
  if (!m_parserPool.waitForDone(3000)) {
    qWarning() << "MQTT parser pool did not finish in time";
  }

  // Clear the list (autoDelete already handled deletion)
  m_parsers.clear();

  // Reset shared state to prevent stale data on reconnect
  MqttParserWorker::resetSharedState();
}
