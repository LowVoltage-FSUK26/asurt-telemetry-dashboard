
#include "../include/serialmanager.h"
#include "../../logging/include/asynclogger.h"
#include "../include/serialparserworker.h"
#include "../include/serialreceiverworker.h"
#include <QDebug>
#include <QThread>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent), m_nextParserIndex(0),
      m_parserThreadCount(QThread::idealThreadCount()), m_debugMode(false),
      m_pendingUpdate(false),
      m_datagramsProcessed(0), m_datagramsDropped(0), m_speed(0.0f), m_rpm(0),
      m_accPedal(0), m_brakePedal(0), m_encoderAngle(0.0), m_temperature(0.0f),
      m_batteryLevel(0), m_gpsLongitude(0.0), m_gpsLatitude(0.0), m_speedFL(0),
      m_speedFR(0), m_speedBL(0), m_speedBR(0), m_lateralG(0.0),
      m_longitudinalG(0.0), m_tempFL(0), m_tempFR(0), m_tempBL(0), m_tempBR(0) {
  // Initialize async logger
  AsyncLogger::instance().initialize("./logs");

  // Initialize update throttling timer (60Hz = 16ms interval)
  m_updateTimer = new QTimer(this);
  m_updateTimer->setInterval(16);
  connect(m_updateTimer, &QTimer::timeout, this, &SerialManager::flushPendingUpdates);
  m_updateTimer->start();

  // Create and configure the receiver worker
  m_receiverWorker = new SerialReceiverWorker();
  m_receiverWorker->moveToThread(&m_receiverThread);

  // Connect signals and slots for receiver worker
  connect(this, &SerialManager::startReceiving, m_receiverWorker,
          &SerialReceiverWorker::startReceiving, Qt::QueuedConnection);
  connect(this, &SerialManager::stopReceiving, m_receiverWorker,
          &SerialReceiverWorker::stopReceiving, Qt::QueuedConnection);
  connect(m_receiverWorker, &SerialReceiverWorker::serialDataReceived, this,
          &SerialManager::handleSerialDataReceived, Qt::QueuedConnection);
  connect(m_receiverWorker, &SerialReceiverWorker::errorOccurred, this,
          &SerialManager::handleError, Qt::QueuedConnection);

  // Connect thread start/stop signals
  connect(&m_receiverThread, &QThread::started, m_receiverWorker,
          &SerialReceiverWorker::initialize);
  connect(&m_receiverThread, &QThread::finished, m_receiverWorker,
          &QObject::deleteLater);

  // Configure the parser thread pool
  m_parserPool.setMaxThreadCount(m_parserThreadCount);
}

SerialManager::~SerialManager() {
  // Stop the update timer first
  if (m_updateTimer) {
    m_updateTimer->stop();
  }

  stop();

  // Wait for receiver thread to finish
  if (m_receiverThread.isRunning()) {
    m_receiverThread.quit();
    if (!m_receiverThread.wait(3000)) {
      qWarning() << "Serial receiver thread did not terminate gracefully";
      m_receiverThread.terminate();
      m_receiverThread.wait(1000);
    }
  }

  // Clean up parsers
  cleanupParsers();
}

bool SerialManager::start(const QString &portName, qint32 baudRate) {
  QThread::currentThread()->setObjectName("Main Thread");

  // Stop if already running
  stop();

  // Initialize parser threads
  initializeParsers();

  // Start the receiver thread
  m_receiverThread.start();
  m_receiverThread.setPriority(QThread::HighPriority);

  // Start receiving serial data
  emit startReceiving(portName, baudRate);

  if (m_debugMode) {
    qDebug() << "Serial Manager started on port" << portName << "with baud rate"
             << baudRate << "running on the " << QThread::currentThread()
             << "with" << m_parserThreadCount << "parser threads";
  }

  return true;
}

bool SerialManager::stop() {
  // Stop receiving serial data
  emit stopReceiving();

  // Clean up parser threads
  cleanupParsers();

  if (m_debugMode) {
    qDebug() << "Serial Manager stopped";
  }

  return true;
}

void SerialManager::setParserThreadCount(int count) {
  if (count > 0 && count <= QThread::idealThreadCount() * 2) {
    m_parserThreadCount = count;

    // Update thread pool configuration
    m_parserPool.setMaxThreadCount(m_parserThreadCount);

    if (m_debugMode) {
      qDebug() << "Parser thread count set to" << count;
    }
  }
}

void SerialManager::setDebugMode(bool enabled) {
  m_debugMode = enabled;

  if (m_debugMode) {
    qDebug() << "Debug mode enabled";
  }
}

void SerialManager::handleSerialDataReceived(const QByteArray &data) {
  // Distribute data among parsers in a round-robin fashion
  if (!m_parsers.isEmpty()) {
    // Get the next parser
    SerialParserWorker *parser = m_parsers[m_nextParserIndex];

    // Queue the data for parsing
    parser->queueData(data);

    // Update the next parser index
    m_nextParserIndex = (m_nextParserIndex + 1) % m_parsers.size();
  }
}

void SerialManager::handleParsedData(float speed, int rpm, int accPedal,
                                     int brakePedal, double encoderAngle,
                                     float temperature, int batteryLevel,
                                     double gpsLongitude, double gpsLatitude,
                                     int speedFL, int speedFR, int speedBL,
                                     int speedBR, double lateralG,
                                     double longitudinalG, int tempFL,
                                     int tempFR, int tempBL, int tempBR) {
  // Increment processed count
  m_datagramsProcessed.fetch_add(1);

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

void SerialManager::flushPendingUpdates() {
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

void SerialManager::handleError(const QString &error) {
  if (m_debugMode) {
    qDebug() << "Serial Manager error:" << error;
  }

  emit errorOccurred(error);
}

void SerialManager::initializeParsers() {
  // Create parser instances
  for (int i = 0; i < m_parserThreadCount; ++i) {
    SerialParserWorker *parser = new SerialParserWorker(m_debugMode);

    // Connect signals for results
    connect(parser, &SerialParserWorker::dataParsed, this,
            &SerialManager::handleParsedData, Qt::QueuedConnection);
    connect(parser, &SerialParserWorker::errorOccurred, this,
            &SerialManager::handleError, Qt::QueuedConnection);

    // Add to list
    m_parsers.append(parser);

    // Start the parser in the thread pool
    m_parserPool.start(parser);

    // if (m_debugMode) {
    //     qDebug() << "Started parser" << i;
    // }
  }

  // Reset the next parser index
  m_nextParserIndex = 0;
}

void SerialManager::cleanupParsers() {
  // Stop all parsers
  for (SerialParserWorker *parser : m_parsers) {
    parser->stop();
  }

  // Wait for all tasks to complete with timeout
  if (!m_parserPool.waitForDone(3000)) {
    qWarning() << "Serial parser pool did not finish in time";
  }


  // Clear the list (autoDelete already handled deletion)
  m_parsers.clear();

  // Reset shared state to prevent stale data on reconnect
  SerialParserWorker::resetSharedState();
}
