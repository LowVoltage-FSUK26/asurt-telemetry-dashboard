#include "../include/udpclient.h"
#include "../include/udpreceiverworker.h"
#include "../include/udpparserworker.h"
#include "../../logging/include/asynclogger.h"
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
    m_debugMode(false),
    m_pendingUpdate(false),
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
    m_longitudinalG(0.0),
    m_tempFL(0),
    m_tempFR(0),
    m_tempBL(0),
    m_tempBR(0)
{
    // Initialize async logger
    AsyncLogger::instance().initialize("./logs");

    // Initialize update throttling timer (60Hz = 16ms interval)
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16);
    connect(m_updateTimer, &QTimer::timeout, this, &UdpClient::flushPendingUpdates);
    m_updateTimer->start();
    
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
    // Stop the update timer first
    if (m_updateTimer) {
        m_updateTimer->stop();
    }

    stop();

    // Wait for receiver thread to finish
    if (m_receiverThread.isRunning())
    {
        m_receiverThread.quit();
        if (!m_receiverThread.wait(3000)) {
            qWarning() << "UDP receiver thread did not terminate gracefully";
            m_receiverThread.terminate();
            m_receiverThread.wait(1000);
        }
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
                                 double lateralG, double longitudinalG,
                                 int tempFL, int tempFR, int tempBL, int tempBR)
{
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

void UdpClient::flushPendingUpdates()
{
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

    // Wait for all tasks to complete with timeout
    if (!m_parserPool.waitForDone(3000)) {
        qWarning() << "UDP parser pool did not finish in time";
    }
    
    m_parsers.clear();
}

