#include "../include/mqttparserworker.h"
#include "../../can/include/candecoder.h"
#include "../../logging/include/asynclogger.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QThread>

// Static member definitions for shared state across all parser instances
QMutex MqttParserWorker::s_stateMutex;
float MqttParserWorker::s_speed = 0.0f;
int MqttParserWorker::s_rpm = 0;
int MqttParserWorker::s_accPedal = 0;
int MqttParserWorker::s_brakePedal = 0;
double MqttParserWorker::s_encoderAngle = 0.0;
float MqttParserWorker::s_temperature = 0.0f;
int MqttParserWorker::s_batteryLevel = 0;
double MqttParserWorker::s_gpsLongitude = 0.0;
double MqttParserWorker::s_gpsLatitude = 0.0;
int MqttParserWorker::s_speedFL = 0;
int MqttParserWorker::s_speedFR = 0;
int MqttParserWorker::s_speedBL = 0;
int MqttParserWorker::s_speedBR = 0;
double MqttParserWorker::s_lateralG = 0.0;
double MqttParserWorker::s_longitudinalG = 0.0;
int MqttParserWorker::s_tempFL = 0;
int MqttParserWorker::s_tempFR = 0;
int MqttParserWorker::s_tempBL = 0;
int MqttParserWorker::s_tempBR = 0;

MqttParserWorker::MqttParserWorker(bool debugMode, QObject *parent)
    : QObject(parent), m_debugMode(debugMode), m_running(true),
      m_messagesParsed(0) {
  setAutoDelete(true);
}

MqttParserWorker::~MqttParserWorker() {
  stop();

  // Clear the queue
  QMutexLocker locker(&m_queueMutex);
  m_queue.clear();
}

void MqttParserWorker::resetSharedState() {
  QMutexLocker locker(&s_stateMutex);
  s_speed = 0.0f;
  s_rpm = 0;
  s_accPedal = 0;
  s_brakePedal = 0;
  s_encoderAngle = 0.0;
  s_temperature = 0.0f;
  s_batteryLevel = 0;
  s_gpsLongitude = 0.0;
  s_gpsLatitude = 0.0;
  s_speedFL = 0;
  s_speedFR = 0;
  s_speedBL = 0;
  s_speedBR = 0;
  s_lateralG = 0.0;
  s_longitudinalG = 0.0;
  s_tempFL = 0;
  s_tempFR = 0;
  s_tempBL = 0;
  s_tempBR = 0;
}

void MqttParserWorker::run() {
  if (m_debugMode) {
    qDebug() << "MQTT Parser worker started in thread"
             << QThread::currentThreadId();
  }

  QByteArray message;

  while (m_running.load()) {
    // Get a message from the queue
    {
      QMutexLocker locker(&m_queueMutex);

      // Wait for data if queue is empty
      while (m_queue.isEmpty() && m_running.load()) {
        m_queueCondition.wait(&m_queueMutex, 100);
      }

      // Check if we should exit
      if (!m_running.load()) {
        break;
      }

      // Get the next message
      if (!m_queue.isEmpty()) {
        message = m_queue.dequeue();
      } else {
        continue;
      }
    }

    // Parse the message
    parseMessage(message);
  }

  if (m_debugMode) {
    qDebug() << "MQTT Parser worker stopped in thread"
             << QThread::currentThreadId();
  }
}

void MqttParserWorker::queueMessage(const QByteArray &data) {
  QMutexLocker locker(&m_queueMutex);

  // Add message to queue
  m_queue.enqueue(data);

  // Wake up the worker thread
  m_queueCondition.wakeOne();
}

void MqttParserWorker::stop() {
  m_running.store(false);

  // Wake up the worker thread
  QMutexLocker locker(&m_queueMutex);
  m_queueCondition.wakeAll();
}

void MqttParserWorker::parseMessage(const QByteArray &message) {
  try {
    // Validate CAN packet size (20 bytes)
    if (message.size() != CANDecoder::PACKET_SIZE) {
      emit errorOccurred(
          QString("MQTT: Invalid CAN packet size (expected %1 bytes, got %2)")
              .arg(CANDecoder::PACKET_SIZE)
              .arg(message.size()));
      return;
    }

    // Extract CAN ID
    uint32_t canId = CANDecoder::extractCANId(message);
    QByteArray payload = CANDecoder::extractPayload(message);

    bool shouldEmit = false;

    // Decode based on CAN ID and update only the relevant values
    // Use shared static state so all parser instances share the same values
    QMutexLocker locker(&s_stateMutex);

    switch (canId) {
    case CANDecoder::CAN_ID_IMU_ANGLE: // 0x071
    {
      auto imuAngle = CANDecoder::decodeIMUAngle(payload);
      AsyncLogger::instance().logIMU(imuAngle.ang_x, imuAngle.ang_y,
                                     imuAngle.ang_z);
      if (m_debugMode) {
        qDebug() << "MqttParserWorker: Logged IMU data - X:" << imuAngle.ang_x
                 << "Y:" << imuAngle.ang_y << "Z:" << imuAngle.ang_z;
      }
      // Log only, no GUI update
      break;
    }

    case CANDecoder::CAN_ID_IMU_ACCEL: // 0x072
    {
      auto imuAccel = CANDecoder::decodeIMUAccel(payload);
      s_lateralG = imuAccel.lateral_g;
      s_longitudinalG = imuAccel.longitudinal_g;
      shouldEmit = true;
      break;
    }

    case CANDecoder::CAN_ID_ADC: // 0x073
    {
      auto adc = CANDecoder::decodeADC(payload);
      s_accPedal = adc.acc_pedal;
      s_brakePedal = adc.brake_pedal;
      AsyncLogger::instance().logSuspension(adc.sus_1, adc.sus_2, adc.sus_3,
                                            adc.sus_4);
      if (m_debugMode) {
        qDebug() << "MqttParserWorker: Logged Suspension data - SUS:"
                 << adc.sus_1 << adc.sus_2 << adc.sus_3 << adc.sus_4;
      }
      shouldEmit = true;
      break;
    }

    case CANDecoder::CAN_ID_PROXIMITY_ENCODER: // 0x074
    {
      auto prox = CANDecoder::decodeProximityAndEncoder(payload);
      s_speed = static_cast<float>(prox.speed_kmh);
      s_speedFL = static_cast<int>(prox.speed_fl);
      s_speedFR = static_cast<int>(prox.speed_fr);
      s_speedBL = static_cast<int>(prox.speed_bl);
      s_speedBR = static_cast<int>(prox.speed_br);
      s_encoderAngle = static_cast<double>(prox.encoder_angle);
      shouldEmit = true;
      break;
    }

    case CANDecoder::CAN_ID_GPS: // 0x075
    {
      auto gps = CANDecoder::decodeGPS(payload);
      s_gpsLongitude = static_cast<double>(gps.longitude);
      s_gpsLatitude = static_cast<double>(gps.latitude);
      shouldEmit = true;
      break;
    }

    case CANDecoder::CAN_ID_TEMPERATURES: // 0x076
    {
      auto temps = CANDecoder::decodeTemperatures(payload);
      s_tempFL = static_cast<int>(temps.temp_fl);
      s_tempFR = static_cast<int>(temps.temp_fr);
      s_tempBL = static_cast<int>(temps.temp_rl);
      s_tempBR = static_cast<int>(temps.temp_rr);
      shouldEmit = true;
      break;
    }

    default:
      emit errorOccurred(
          QString("MQTT: Unknown CAN ID: 0x%1").arg(canId, 0, 16));
      return;
    }

    // Emit parsed data if needed (using current shared state values)
    if (shouldEmit) {
      // Read all values while mutex is locked to ensure thread-safe access
      // All values are shared static members, so all parser instances use the
      // same state
      float speed = s_speed;
      int rpm = s_rpm;
      int accPedal = s_accPedal;
      int brakePedal = s_brakePedal;
      double encoderAngle = s_encoderAngle;
      float temperature = s_temperature;
      int batteryLevel = s_batteryLevel;
      double gpsLongitude = s_gpsLongitude;
      double gpsLatitude = s_gpsLatitude;
      int speedFL = s_speedFL;
      int speedFR = s_speedFR;
      int speedBL = s_speedBL;
      int speedBR = s_speedBR;
      double lateralG = s_lateralG;
      double longitudinalG = s_longitudinalG;
      int tempFL = s_tempFL;
      int tempFR = s_tempFR;
      int tempBL = s_tempBL;
      int tempBR = s_tempBR;

      // Unlock mutex before emitting to avoid blocking during signal emission
      locker.unlock();

      emit messageParsed(speed, rpm, accPedal, brakePedal, encoderAngle,
                         temperature, batteryLevel, gpsLongitude, gpsLatitude,
                         speedFL, speedFR, speedBL, speedBR, lateralG,
                         longitudinalG, tempFL, tempFR, tempBL, tempBR);

      if (m_debugMode) {
        qDebug() << "MqttParserWorker: Decoded CAN ID 0x"
                 << QString::number(canId, 16) << "- Speed:" << speed
                 << "EncoderAngle:" << encoderAngle << "LatG:" << lateralG
                 << "AccPedal:" << accPedal << "BrakePedal:" << brakePedal;
      }
    }
  } catch (const std::exception &e) {
    emit errorOccurred(
        QString("MQTT: Exception during CAN decoding: %1").arg(e.what()));
  } catch (...) {
    emit errorOccurred("MQTT: Unknown exception during CAN decoding");
  }
}
