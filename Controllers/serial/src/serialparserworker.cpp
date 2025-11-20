
#include "../include/serialparserworker.h"
#include "../../can/include/candecoder.h"
#include "../../logging/include/asynclogger.h"
#include <QDebug>
#include <QDataStream>

SerialParserWorker::SerialParserWorker(bool debugMode, QObject *parent)
    : QObject(parent),
    m_running(true),
    m_debugMode(debugMode)
{
    setAutoDelete(true);

}

SerialParserWorker::~SerialParserWorker()
{
    stop();
}

void SerialParserWorker::queueData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_dataQueue.enqueue(data);
    m_waitCondition.wakeOne();
}

void SerialParserWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_running = false;
    m_waitCondition.wakeOne(); // Wake up the thread to let it exit the run loop
}

void SerialParserWorker::run()
{
    while (m_running)
    {
        QByteArray data;
        {
            QMutexLocker locker(&m_mutex);
            if (m_dataQueue.isEmpty())
            {
                m_waitCondition.wait(locker.mutex());
            }
            if (!m_running)
            {
                break;
            }
            if (!m_dataQueue.isEmpty())
            {
                data = m_dataQueue.dequeue();
            }
        }

        if (!data.isEmpty())
        {
            parseData(data);
        }
    }
    if (m_debugMode)
    {
        qDebug() << "SerialParserWorker: Exiting run loop.";
    }
}

void SerialParserWorker::parseData(const QByteArray &data)
{
    try
    {
        // Validate CAN packet size (20 bytes)
        if (data.size() != CANDecoder::PACKET_SIZE)
        {
            if (m_debugMode)
            {
                qDebug() << "SerialParserWorker: Invalid CAN packet size (expected" 
                         << CANDecoder::PACKET_SIZE << "bytes, got" << data.size() << ")";
            }
            emit errorOccurred("Serial: Invalid CAN packet size");
            return;
        }

        // Extract CAN ID
        uint32_t canId = CANDecoder::extractCANId(data);
        QByteArray payload = CANDecoder::extractPayload(data);
        
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
            if (m_debugMode)
            {
                qDebug() << "SerialParserWorker: Unknown CAN ID: 0x" << QString::number(canId, 16);
            }
            emit errorOccurred(QString("Serial: Unknown CAN ID: 0x%1").arg(canId, 0, 16));
            return;
        }
        
        // Emit parsed data if needed
        if (shouldEmit)
        {
            emit dataParsed(speed, rpm, accPedal, brakePedal, encoderAngle, temperature, batteryLevel,
                            gpsLongitude, gpsLatitude, speedFL, speedFR, speedBL, speedBR,
                            lateralG, longitudinalG);
        }
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("Serial: Exception during CAN decoding: %1").arg(e.what()));
    }
    catch (...)
    {
        emit errorOccurred("Serial: Unknown exception during CAN decoding");
    }
}




