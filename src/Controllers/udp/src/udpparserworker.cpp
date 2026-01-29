#include "../include/udpparserworker.h"
#include "../../can/include/candecoder.h"
#include "../../logging/include/asynclogger.h"
#include <QDebug>
#include <QStringList>
#include <QThread>

/*A worker class responsible for parsing raw datagram data. It is designed to run in a thread pool,
 *  using a thread-safe queue (internally) to receive datagrams, parse them into numeric values,
 *   and then emit a signal with the parsed values or an error if parsing fails.
 */

UdpParserWorker::UdpParserWorker(bool debugMode, QObject *parent)
    : QObject(parent),
    m_debugMode(debugMode),
    m_running(true),
    m_datagramsParsed(0)
{
    setAutoDelete(true);
}

UdpParserWorker::~UdpParserWorker()
{
    stop();

    // Clear the queue
    QMutexLocker locker(&m_queueMutex);
    m_queue.clear();
}

void UdpParserWorker::run()
{
    if (m_debugMode)
    {
        qDebug() << "Parser worker started in thread" << QThread::currentThreadId();
    }

    QByteArray datagram;

    while (m_running.load())
    { // The flag is accessed using load() to ensure that changes made to it in other threads are observed safely.
        // Get a datagram from the queue
        {
            QMutexLocker locker(&m_queueMutex); // Acquires m_queueMutex to protect concurrent access to the internal datagram queue (m_queue).

            // Wait for data if queue is empty
            while (m_queue.isEmpty() && m_running.load())
            {
                // checks if the queue is empty. If it is, it waits (up to 100 milliseconds) for new data to arrive.
                m_queueCondition.wait(&m_queueMutex, 100);

                /* If another thread enqueues data during the waiting period,
                the waiting thread will observe the new state after the wait and process the queued data accordingly.*/

                /* If Thread B is still working and holding the mutex when the timeout in Thread A expires,
                 *  Thread A will be blocked at the point of re-locking until Thread B unlocks the mutex.
                 *  Once Thread B finishes and unlocks, Thread A will acquire the lock.
                 */

                /*After reacquiring the lock, Thread A resumes execution and checks the condition (e.g., whether there is data in the queue).
                 *  Any modifications made by Thread B while Thread A was waiting will be visible
                 *  because the mutex ensures that shared memory access is serialized and consistent.
                 */
            }

            // Check if we should exit
            if (!m_running.load())
            {
                break;
            }

            // Get the next datagram
            if (!m_queue.isEmpty())
            {
                datagram = m_queue.dequeue();
            }
            else
            {
                continue;
            }
        }

        // Parse the datagram
        parseDatagram(datagram);
    }

    if (m_debugMode)
    {
        qDebug() << "Parser worker stopped in thread" << QThread::currentThreadId();
    }
}

void UdpParserWorker::queueDatagram(const QByteArray &data)
{
    QMutexLocker locker(&m_queueMutex);

    // Drop oldest datagrams if queue is too deep (prevents unbounded growth)
    static const int MAX_QUEUE_DEPTH = 50;
    while (m_queue.size() >= MAX_QUEUE_DEPTH) {
        m_queue.dequeue();
    }

    // Add datagram to queue
    m_queue.enqueue(data);

    // Wake up the worker thread
    m_queueCondition.wakeOne();
}

void UdpParserWorker::stop()
{
    m_running.store(false);

    // Wake up the worker thread
    QMutexLocker locker(&m_queueMutex);
    m_queueCondition.wakeAll(); // wake up any thread that might be blocked waiting on the condition variable
}

void UdpParserWorker::parseDatagram(const QByteArray &data)
{
    try
    {
        // Validate CAN packet size (20 bytes)
        if (data.size() != CANDecoder::PACKET_SIZE)
        {
            emit errorOccurred(QString("UDP: Invalid CAN packet size (expected %1 bytes, got %2)")
                .arg(CANDecoder::PACKET_SIZE).arg(data.size()));
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
        int tempFL = 0;
        int tempFR = 0;
        int tempBL = 0;
        int tempBR = 0;
        
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
            tempFL = static_cast<int>(temps.temp_fl);
            tempFR = static_cast<int>(temps.temp_fr);
            tempBL = static_cast<int>(temps.temp_rl);
            tempBR = static_cast<int>(temps.temp_rr);
            shouldEmit = true;
            break;
        }
        
        default:
            emit errorOccurred(QString("UDP: Unknown CAN ID: 0x%1").arg(canId, 0, 16));
            return;
        }
        
        // Emit parsed data if needed
        if (shouldEmit)
        {
            // Increment counter
            m_datagramsParsed++;
            
            emit datagramParsed(
                speed, rpm, accPedal, brakePedal,
                encoderAngle, temperature, batteryLevel,
                gpsLongitude, gpsLatitude,
                speedFL, speedFR, speedBL, speedBR,
                lateralG, longitudinalG,
                tempFL, tempFR, tempBL, tempBR);
            
            // Log debug info occasionally
            if (m_debugMode && m_datagramsParsed % 1000 == 0)
            {
                qDebug() << "Parser" << QThread::currentThreadId()
                         << "has processed" << m_datagramsParsed << "datagrams";
            }
        }
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("UDP: Exception during CAN decoding: %1").arg(e.what()));
    }
    catch (...)
    {
        emit errorOccurred("UDP: Unknown exception during CAN decoding");
    }
}

