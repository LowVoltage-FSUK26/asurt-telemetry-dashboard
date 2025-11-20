#include "../include/udpclient.h"
#include "../include/udpreceiverworker.h"
#include "../include/udpparserworker.h"
#include <QDebug>
#include <QThread>

/*UdpClient
 * The central class managing the overall UDP client.
 * It configures workers, maintains application-wide configuration (debug mode, thread count, performance counters, atomic property storage),
 * and exposes a public API (start/stop, property signals) for external use or QML integration.
 */

UdpClient::UdpClient(QObject *parent)
    : QObject(parent),
    m_nextParserIndex(0),
    m_parserThreadCount(QThread::idealThreadCount()),
    m_debugMode(true),
    m_datagramsProcessed(0),
    m_datagramsDropped(0),
    m_speed(0.0f),
    m_rpm(0),
    m_accPedal(0),
    m_brakePedal(0),
    m_encoderAngle(0.0),
    m_temperature(0.0f),
    m_batteryLevel(0),
    m_gpsLongitude(0.0),
    m_gpsLatitude(0.0),
    m_speedFL(0),
    m_speedFR(0),
    m_speedBL(0),
    m_speedBR(0),
    m_lateralG(0.0),
    m_longitudinalG(0.0)
{
    // Create and configure the receiver worker
    m_receiverWorker = new UdpReceiverWorker();
    m_receiverWorker->moveToThread(&m_receiverThread);

    // Connect signals and slots for receiver worker
    connect(this, &UdpClient::startReceiving, m_receiverWorker, &UdpReceiverWorker::startReceiving, Qt::QueuedConnection);
    connect(this, &UdpClient::stopReceiving, m_receiverWorker, &UdpReceiverWorker::stopReceiving, Qt::QueuedConnection);
    connect(m_receiverWorker, &UdpReceiverWorker::datagramReceived, this, &UdpClient::handleDatagramReceived, Qt::QueuedConnection);
    connect(m_receiverWorker, &UdpReceiverWorker::errorOccurred, this, &UdpClient::handleError, Qt::QueuedConnection);

    // Connect thread start/stop signals
    connect(&m_receiverThread, &QThread::started, m_receiverWorker, &UdpReceiverWorker::initialize);
    connect(&m_receiverThread, &QThread::finished, m_receiverWorker, &QObject::deleteLater);

    // Configure the parser thread pool
    m_parserPool.setMaxThreadCount(m_parserThreadCount);
}

UdpClient::~UdpClient()
{
    stop();

    // Wait for receiver thread to finish
    if (m_receiverThread.isRunning())
    {
        m_receiverThread.quit();
        m_receiverThread.wait();
    }

    // Clean up parsers
    cleanupParsers();
}

bool UdpClient::start(quint16 port)
{

    QThread::currentThread()->setObjectName("Main Thread");

    // Stop if already running
    stop();

    // Initialize parser threads
    initializeParsers();

    // Start the receiver thread
    m_receiverThread.start();
    m_receiverThread.setPriority(QThread::HighPriority);

    // Start receiving datagrams
    emit startReceiving(port);

    if (m_debugMode)
    {
        qDebug() << "UDP Client started on port" << port << "running on the " << QThread::currentThread()
        << "with" << m_parserThreadCount << "parser threads";
    }

    return true;
}

bool UdpClient::stop()
{
    // Stop receiving datagrams
    emit stopReceiving();

    // Clean up parser threads
    cleanupParsers();

    if (m_debugMode)
    {
        qDebug() << "UDP Client stopped";
    }

    return true;
}

void UdpClient::setParserThreadCount(int count)
{
    if (count > 0 && count <= QThread::idealThreadCount() * 2)
    {
        m_parserThreadCount = count;

        // Update thread pool configuration
        m_parserPool.setMaxThreadCount(m_parserThreadCount);

        if (m_debugMode)
        {
            qDebug() << "Parser thread count set to" << count;
        }
    }
}

void UdpClient::setDebugMode(bool enabled)
{
    m_debugMode = enabled;

    if (m_debugMode)
    {
        qDebug() << "Debug mode enabled";
    }
}

void UdpClient::handleDatagramReceived(const QByteArray &data)
{
    // Distribute datagrams among parsers in a round-robin fashion
    if (!m_parsers.isEmpty())
    {
        // Get the next parser
        UdpParserWorker *parser = m_parsers[m_nextParserIndex];

        // Queue the datagram for parsing
        parser->queueDatagram(data);

        // Update the next parser index
        m_nextParserIndex = (m_nextParserIndex + 1) % m_parsers.size();
    }
}

void UdpClient::handleParsedData(float speed, int rpm, int accPedal, int brakePedal,
                                 double encoderAngle, float temperature, int batteryLevel,
                                 double gpsLongitude, double gpsLatitude,
                                 int speedFL, int speedFR, int speedBL, int speedBR,
                                 double lateralG, double longitudinalG)
{
    // Increment processed count
    m_datagramsProcessed.fetch_add(1);

    // Update speed if changed
    float oldSpeed = m_speed.load(std::memory_order_relaxed);
    if (!qFuzzyCompare(oldSpeed, speed))
    {
        m_speed.store(speed, std::memory_order_relaxed);
        emit speedChanged(speed);
    }

    // Update rpm if changed
    int oldRpm = m_rpm.load(std::memory_order_relaxed);
    if (oldRpm != rpm)
    {
        m_rpm.store(rpm, std::memory_order_relaxed);
        emit rpmChanged(rpm);
    }

    // Update accPedal if changed
    int oldAccPedal = m_accPedal.load(std::memory_order_relaxed);
    if (oldAccPedal != accPedal)
    {
        m_accPedal.store(accPedal, std::memory_order_relaxed);
        emit accPedalChanged(accPedal);
    }

    // Update brakePedal if changed
    int oldBrakePedal = m_brakePedal.load(std::memory_order_relaxed);
    if (oldBrakePedal != brakePedal)
    {
        m_brakePedal.store(brakePedal, std::memory_order_relaxed);
        emit brakePedalChanged(brakePedal);
    }

    // Update encoderAngle if changed
    double oldEncoderAngle = m_encoderAngle.load(std::memory_order_relaxed);
    if (!qFuzzyCompare(oldEncoderAngle, encoderAngle))
    {
        m_encoderAngle.store(encoderAngle, std::memory_order_relaxed);
        emit encoderAngleChanged(encoderAngle);
    }

    // Update temperature if changed
    float oldTemperature = m_temperature.load(std::memory_order_relaxed);
    if (!qFuzzyCompare(oldTemperature, temperature))
    {
        m_temperature.store(temperature, std::memory_order_relaxed);
        emit temperatureChanged(temperature);
    }

    // Update batteryLevel if changed
    int oldBatteryLevel = m_batteryLevel.load(std::memory_order_relaxed);
    if (oldBatteryLevel != batteryLevel)
    {
        m_batteryLevel.store(batteryLevel, std::memory_order_relaxed);
        emit batteryLevelChanged(batteryLevel);
    }

    // Update gpsLongitude if changed
    double oldGpsLongitude = m_gpsLongitude.load(std::memory_order_relaxed);
    if (!qFuzzyCompare(oldGpsLongitude, gpsLongitude))
    {
        m_gpsLongitude.store(gpsLongitude, std::memory_order_relaxed);
        emit gpsLongitudeChanged(gpsLongitude);
    }

    // Update gpsLatitude if changed
    double oldGpsLatitude = m_gpsLatitude.load(std::memory_order_relaxed);
    if (!qFuzzyCompare(oldGpsLatitude, gpsLatitude))
    {
        m_gpsLatitude.store(gpsLatitude, std::memory_order_relaxed);
        emit gpsLatitudeChanged(gpsLatitude);
    }

    // Update speedFL if changed
    int oldSpeedFL = m_speedFL.load(std::memory_order_relaxed);
    if (oldSpeedFL != speedFL)
    {
        m_speedFL.store(speedFL, std::memory_order_relaxed);
        emit speedFLChanged(speedFL);
    }

    // Update speedFR if changed
    int oldSpeedFR = m_speedFR.load(std::memory_order_relaxed);
    if (oldSpeedFR != speedFR)
    {
        m_speedFR.store(speedFR, std::memory_order_relaxed);
        emit speedFRChanged(speedFR);
    }

    // Update speedBL if changed
    int oldSpeedBL = m_speedBL.load(std::memory_order_relaxed);
    if (oldSpeedBL != speedBL)
    {
        m_speedBL.store(speedBL, std::memory_order_relaxed);
        emit speedBLChanged(speedBL);
    }

    // Update speedBR if changed
    int oldSpeedBR = m_speedBR.load(std::memory_order_relaxed);
    if (oldSpeedBR != speedBR)
    {
        m_speedBR.store(speedBR, std::memory_order_relaxed);
        emit speedBRChanged(speedBR);
    }

    // Update lateralG if changed
    double oldLateralG = m_lateralG.load(std::memory_order_relaxed);
    if (!qFuzzyCompare(oldLateralG, lateralG))
    {
        m_lateralG.store(lateralG, std::memory_order_relaxed);
        emit lateralGChanged(lateralG);
    }

    // Update longitudinalG if changed
    double oldLongitudinalG = m_longitudinalG.load(std::memory_order_relaxed);
    if (!qFuzzyCompare(oldLongitudinalG, longitudinalG))
    {
        m_longitudinalG.store(longitudinalG, std::memory_order_relaxed);
        emit longitudinalGChanged(longitudinalG);
    }
}

void UdpClient::handleError(const QString &error)
{
    if (m_debugMode)
    {
        qDebug() << "UDP Client error:" << error;
    }

    emit errorOccurred(error);
}

void UdpClient::initializeParsers()
{
    // Create parser instances
    for (int i = 0; i < m_parserThreadCount; ++i)
    {
        UdpParserWorker *parser = new UdpParserWorker(m_debugMode);

        // Connect signals for results
        connect(parser, &UdpParserWorker::datagramParsed, this, &UdpClient::handleParsedData, Qt::QueuedConnection);
        connect(parser, &UdpParserWorker::errorOccurred, this, &UdpClient::handleError, Qt::QueuedConnection);

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

void UdpClient::cleanupParsers()
{
    // Stop all parsers
    for (UdpParserWorker *parser : m_parsers)
    {
        parser->stop();
    }

    // Wait for all tasks to complete
    m_parserPool.waitForDone();

    // Disconnect all signals
    for (UdpParserWorker *parser : m_parsers)
    {
        disconnect(parser, &UdpParserWorker::datagramParsed, this, &UdpClient::handleParsedData);
        disconnect(parser, &UdpParserWorker::errorOccurred, this, &UdpClient::handleError);
    }

    // Delete all parsers
    qDeleteAll(m_parsers);
    m_parsers.clear();
}

