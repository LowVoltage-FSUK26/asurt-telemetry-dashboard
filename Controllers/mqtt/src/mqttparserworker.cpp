#include "../include/mqttparserworker.h"
#include "../../can/include/candecoder.h"
#include "../../logging/include/asynclogger.h"
#include <QDebug>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

MqttParserWorker::MqttParserWorker(bool debugMode, QObject *parent)
    : QObject(parent),
    m_debugMode(debugMode),
    m_running(true),
    m_messagesParsed(0)
{
    setAutoDelete(true);
}

MqttParserWorker::~MqttParserWorker()
{
    stop();

    // Clear the queue
    QMutexLocker locker(&m_queueMutex);
    m_queue.clear();
}

void MqttParserWorker::run()
{
    if (m_debugMode)
    {
        qDebug() << "MQTT Parser worker started in thread" << QThread::currentThreadId();
    }

    QByteArray message;

    while (m_running.load())
    {
        // Get a message from the queue
        {
            QMutexLocker locker(&m_queueMutex);

            // Wait for data if queue is empty
            while (m_queue.isEmpty() && m_running.load())
            {
                m_queueCondition.wait(&m_queueMutex, 100);
            }

            // Check if we should exit
            if (!m_running.load())
            {
                break;
            }

            // Get the next message
            if (!m_queue.isEmpty())
            {
                message = m_queue.dequeue();
            }
            else
            {
                continue;
            }
        }

        // Parse the message
        parseMessage(message);
    }

    if (m_debugMode)
    {
        qDebug() << "MQTT Parser worker stopped in thread" << QThread::currentThreadId();
    }
}

void MqttParserWorker::queueMessage(const QByteArray &data)
{
    QMutexLocker locker(&m_queueMutex);

    // Add message to queue
    m_queue.enqueue(data);

    // Wake up the worker thread
    m_queueCondition.wakeOne();
}

void MqttParserWorker::stop()
{
    m_running.store(false);

    // Wake up the worker thread
    QMutexLocker locker(&m_queueMutex);
    m_queueCondition.wakeAll();
}

void MqttParserWorker::parseMessage(const QByteArray &message)
{
    try
    {
        // Validate CAN packet size (20 bytes)
        if (message.size() != CANDecoder::PACKET_SIZE)
        {
            emit errorOccurred(QString("MQTT: Invalid CAN packet size (expected %1 bytes, got %2)")
                .arg(CANDecoder::PACKET_SIZE).arg(message.size()));
            return;
        }

        // Extract CAN ID
        uint32_t canId = CANDecoder::extractCANId(message);
        QByteArray payload = CANDecoder::extractPayload(message);
        
        // Initialize default values
        float speed = 0.0f;
        int rpm = 0;
        int accPedal = 0;
        int brakePedal = 0;
        double encoderAngle = 0.0;
        float temperature = 0.0f;
        int batteryLevel = 0;
        double gpsLongitude = 0.0;
        double gpsLatitude = 0.0;
        int speedFL = 0;
        int speedFR = 0;
        int speedBL = 0;
        int speedBR = 0;
        double lateralG = 0.0;
        double longitudinalG = 0.0;
        
        bool shouldEmit = false;
        
        // Decode based on CAN ID
        switch (canId)
        {
        case CANDecoder::CAN_ID_IMU_ANGLE: // 0x071
        {
            auto imuAngle = CANDecoder::decodeIMUAngle(payload);
            AsyncLogger::instance().logIMU(imuAngle.ang_x, imuAngle.ang_y, imuAngle.ang_z);
            // Log only, no GUI update
            break;
        }
        
        case CANDecoder::CAN_ID_IMU_ACCEL: // 0x072
        {
            auto imuAccel = CANDecoder::decodeIMUAccel(payload);
            lateralG = imuAccel.lateral_g;
            longitudinalG = imuAccel.longitudinal_g;
            shouldEmit = true;
            break;
        }
        
        case CANDecoder::CAN_ID_ADC: // 0x073
        {
            auto adc = CANDecoder::decodeADC(payload);
            accPedal = adc.acc_pedal;
            brakePedal = adc.brake_pedal;
            AsyncLogger::instance().logSuspension(adc.sus_1, adc.sus_2, adc.sus_3, adc.sus_4);
            shouldEmit = true;
            break;
        }
        
        case CANDecoder::CAN_ID_PROXIMITY_ENCODER: // 0x074
        {
            auto prox = CANDecoder::decodeProximityAndEncoder(payload);
            speed = prox.speed_kmh;
            speedFL = static_cast<int>(prox.speed_fl);
            speedFR = static_cast<int>(prox.speed_fr);
            speedBL = static_cast<int>(prox.speed_bl);
            speedBR = static_cast<int>(prox.speed_br);
            encoderAngle = prox.encoder_angle;
            shouldEmit = true;
            break;
        }
        
        case CANDecoder::CAN_ID_GPS: // 0x075
        {
            auto gps = CANDecoder::decodeGPS(payload);
            gpsLongitude = gps.longitude;
            gpsLatitude = gps.latitude;
            shouldEmit = true;
            break;
        }
        
        case CANDecoder::CAN_ID_TEMPERATURES: // 0x076
        {
            auto temps = CANDecoder::decodeTemperatures(payload);
            AsyncLogger::instance().logTemperature(temps.temp_fl, temps.temp_fr, temps.temp_rl, temps.temp_rr);
            // Log only, no GUI update
            break;
        }
        
        default:
            emit errorOccurred(QString("MQTT: Unknown CAN ID: 0x%1").arg(canId, 0, 16));
            return;
        }
        
        // Emit parsed data if needed
        if (shouldEmit)
        {
            emit messageParsed(
                speed, rpm, accPedal, brakePedal,
                encoderAngle, temperature, batteryLevel,
                gpsLongitude, gpsLatitude,
                speedFL, speedFR, speedBL, speedBR,
                lateralG, longitudinalG);
            
            if (m_debugMode)
            {
                qDebug() << "MqttParserWorker: Decoded CAN ID 0x" << QString::number(canId, 16)
                         << "- Speed:" << speed << "LatG:" << lateralG;
            }
        }
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("MQTT: Exception during CAN decoding: %1").arg(e.what()));
    }
    catch (...)
    {
        emit errorOccurred("MQTT: Unknown exception during CAN decoding");
    }
}



