
#include "../include/serialparserworker.h"
#include "../../can/include/candecoder.h"
#include "../../logging/include/asynclogger.h"
#include <QDataStream>
#include <QDebug>
#include <QThread>

// Static member definitions for shared state across all parser instances
QMutex SerialParserWorker::s_stateMutex;
float SerialParserWorker::s_speed = 0.0f;
int SerialParserWorker::s_rpm = 0;
int SerialParserWorker::s_accPedal = 0;
int SerialParserWorker::s_brakePedal = 0;
double SerialParserWorker::s_encoderAngle = 0.0;
float SerialParserWorker::s_temperature = 0.0f;
int SerialParserWorker::s_batteryLevel = 0;
double SerialParserWorker::s_gpsLongitude = 0.0;
double SerialParserWorker::s_gpsLatitude = 0.0;
int SerialParserWorker::s_speedFL = 0;
int SerialParserWorker::s_speedFR = 0;
int SerialParserWorker::s_speedBL = 0;
int SerialParserWorker::s_speedBR = 0;
double SerialParserWorker::s_lateralG = 0.0;
double SerialParserWorker::s_longitudinalG = 0.0;
int SerialParserWorker::s_tempFL = 0;
int SerialParserWorker::s_tempFR = 0;
int SerialParserWorker::s_tempBL = 0;
int SerialParserWorker::s_tempBR = 0;

SerialParserWorker::SerialParserWorker(bool debugMode, QObject *parent)
    : QObject(parent), m_running(true), m_debugMode(debugMode) {
  setAutoDelete(true);
}

SerialParserWorker::~SerialParserWorker() { stop(); }

void SerialParserWorker::resetSharedState() {
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

void SerialParserWorker::queueData(const QByteArray &data) {
  QMutexLocker locker(&m_mutex);
  m_dataQueue.enqueue(data);
  m_waitCondition.wakeOne();
}

void SerialParserWorker::stop() {
  m_running.store(false);
  QMutexLocker locker(&m_mutex);
  m_waitCondition.wakeAll();
}

void SerialParserWorker::run() {
  if (m_debugMode) {
    qDebug() << "SerialParserWorker: Started in thread"
             << QThread::currentThreadId();
  }

  while (m_running.load()) {
    QByteArray data;
    {
      QMutexLocker locker(&m_mutex);
      while (m_dataQueue.isEmpty() && m_running.load()) {
        m_waitCondition.wait(&m_mutex, 100);
      }
      if (!m_running.load()) {
        break;
      }
      if (!m_dataQueue.isEmpty()) {
        data = m_dataQueue.dequeue();
      } else {
        continue;
      }
    }

    if (!data.isEmpty()) {
      parseData(data);
    }
  }

  if (m_debugMode) {
    qDebug() << "SerialParserWorker: Exiting run loop.";
  }
}

void SerialParserWorker::parseData(const QByteArray &data) {
  try {
    // Validate CAN packet size (20 bytes)
    if (data.size() != CANDecoder::PACKET_SIZE) {
      if (m_debugMode) {
        qDebug() << "SerialParserWorker: Invalid CAN packet size (expected"
                 << CANDecoder::PACKET_SIZE << "bytes, got" << data.size()
                 << ")";
      }
      emit errorOccurred("Serial: Invalid CAN packet size");
      return;
    }

    // Extract CAN ID
    uint32_t canId = CANDecoder::extractCANId(data);
    QByteArray payload = CANDecoder::extractPayload(data);

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
        qDebug() << "SerialParserWorker: Logged IMU data - X:" << imuAngle.ang_x
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
        qDebug() << "SerialParserWorker: Logged Suspension data - SUS:"
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
      if (m_debugMode) {
        qDebug() << "SerialParserWorker: Unknown CAN ID: 0x"
                 << QString::number(canId, 16);
      }
      emit errorOccurred(
          QString("Serial: Unknown CAN ID: 0x%1").arg(canId, 0, 16));
      return;
    }

    // Emit parsed data if needed (using current shared state values)
    if (shouldEmit) {
      // Read all values while mutex is locked to ensure thread-safe access
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

      emit dataParsed(speed, rpm, accPedal, brakePedal, encoderAngle,
                      temperature, batteryLevel, gpsLongitude, gpsLatitude,
                      speedFL, speedFR, speedBL, speedBR, lateralG,
                      longitudinalG, tempFL, tempFR, tempBL, tempBR);

      if (m_debugMode) {
        qDebug() << "SerialParserWorker: Decoded CAN ID 0x"
                 << QString::number(canId, 16) << "- Speed:" << speed
                 << "EncoderAngle:" << encoderAngle << "LatG:" << lateralG
                 << "AccPedal:" << accPedal << "BrakePedal:" << brakePedal;
      }
    }
  } catch (const std::exception &e) {
    emit errorOccurred(
        QString("Serial: Exception during CAN decoding: %1").arg(e.what()));
  } catch (...) {
    emit errorOccurred("Serial: Unknown exception during CAN decoding");
  }
}
