#include "../include/asynclogger.h"
#include <QDebug>
#include <QDir>

AsyncLogger& AsyncLogger::instance()
{
    static AsyncLogger instance;
    return instance;
}

AsyncLogger::AsyncLogger(QObject *parent)
    : QObject(parent),
    m_worker(nullptr),
    m_initialized(false)
{
}

AsyncLogger::~AsyncLogger()
{
    shutdown();
}

void AsyncLogger::initialize(const QString &logDirectory)
{
    if (m_initialized) {
        return;
    }
    
    m_logDirectory = logDirectory;
    
    // Create log directory if it doesn't exist
    QDir dir;
    if (!dir.exists(m_logDirectory)) {
        dir.mkpath(m_logDirectory);
    }
    
    // Create worker and move to thread
    m_worker = new LoggerWorker(m_logDirectory);
    m_worker->moveToThread(&m_workerThread);
    
    // Connect signals
    connect(this, &AsyncLogger::logEntryReady, m_worker, &LoggerWorker::processEntry, Qt::QueuedConnection);
    connect(this, &AsyncLogger::initializeWorker, m_worker, &LoggerWorker::initialize, Qt::QueuedConnection);
    connect(this, &AsyncLogger::shutdownWorker, m_worker, &LoggerWorker::shutdown, Qt::QueuedConnection);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    
    // Start worker thread
    m_workerThread.start();
    m_workerThread.setPriority(QThread::LowPriority); // Low priority for logging
    
    emit initializeWorker();
    
    m_initialized = true;
    qDebug() << "AsyncLogger initialized with log directory:" << m_logDirectory;
}

void AsyncLogger::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    // Signal worker to close files
    emit shutdownWorker();
    
    // Give worker time to process signal
    QThread::msleep(50);
    
    // Stop the thread
    if (m_workerThread.isRunning()) {
        m_workerThread.quit();
        if (!m_workerThread.wait(3000)) { // Wait up to 3 seconds
            qWarning() << "AsyncLogger worker thread did not terminate gracefully, forcing termination";
            m_workerThread.terminate();
            m_workerThread.wait(1000);
        }
    }
    
    m_initialized = false;
    qDebug() << "AsyncLogger shutdown complete";
}

void AsyncLogger::logIMU(int16_t ang_x, int16_t ang_y, int16_t ang_z)
{
    if (!m_initialized) {
        return;
    }
    
    LogEntry entry;
    entry.type = LogEntry::IMU;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    entry.data = QString("%1,%2,%3").arg(ang_x).arg(ang_y).arg(ang_z);
    
    emit logEntryReady(entry);
}

void AsyncLogger::logSuspension(uint16_t sus_1, uint16_t sus_2, uint16_t sus_3, uint16_t sus_4)
{
    if (!m_initialized) {
        return;
    }
    
    LogEntry entry;
    entry.type = LogEntry::SUSPENSION;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    entry.data = QString("%1,%2,%3,%4").arg(sus_1).arg(sus_2).arg(sus_3).arg(sus_4);
    
    emit logEntryReady(entry);
}

void AsyncLogger::logTemperature(int16_t temp_fl, int16_t temp_fr, int16_t temp_rl, int16_t temp_rr)
{
    if (!m_initialized) {
        return;
    }
    
    LogEntry entry;
    entry.type = LogEntry::TEMPERATURE;
    entry.timestamp = QDateTime::currentMSecsSinceEpoch();
    entry.data = QString("%1,%2,%3,%4").arg(temp_fl).arg(temp_fr).arg(temp_rl).arg(temp_rr);
    
    emit logEntryReady(entry);
}

// LoggerWorker implementation

LoggerWorker::LoggerWorker(const QString &logDir)
    : m_logDirectory(logDir)
{
}

LoggerWorker::~LoggerWorker()
{
    closeFiles();
}

void LoggerWorker::initialize()
{
    if (!openFiles()) {
        qWarning() << "Failed to open log files";
    }
}

void LoggerWorker::shutdown()
{
    closeFiles();
}

bool LoggerWorker::openFiles()
{
    // Open IMU log file
    m_imuFile.setFileName(m_logDirectory + "/IMU_logger.csv");
    if (!m_imuFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open IMU log file:" << m_imuFile.errorString();
        return false;
    }
    m_imuStream.setDevice(&m_imuFile);
    
    // Write header if file is empty
    if (m_imuFile.size() == 0) {
        writeHeader(m_imuStream, "timestamp,IMU_Ang_X,IMU_Ang_Y,IMU_Ang_Z");
    }
    
    // Open Suspension log file
    m_suspensionFile.setFileName(m_logDirectory + "/suspension_logger.csv");
    if (!m_suspensionFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open suspension log file:" << m_suspensionFile.errorString();
        return false;
    }
    m_suspensionStream.setDevice(&m_suspensionFile);
    
    // Write header if file is empty
    if (m_suspensionFile.size() == 0) {
        writeHeader(m_suspensionStream, "timestamp,SUS_1,SUS_2,SUS_3,SUS_4");
    }
    
    // Open Temperature log file
    m_temperatureFile.setFileName(m_logDirectory + "/Temp.csv");
    if (!m_temperatureFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open temperature log file:" << m_temperatureFile.errorString();
        return false;
    }
    m_temperatureStream.setDevice(&m_temperatureFile);
    
    // Write header if file is empty
    if (m_temperatureFile.size() == 0) {
        writeHeader(m_temperatureStream, "timestamp,Temp_FL,Temp_FR,Temp_RL,Temp_RR");
    }
    
    return true;
}

void LoggerWorker::closeFiles()
{
    if (m_imuFile.isOpen()) {
        m_imuStream.flush();
        m_imuFile.close();
    }
    
    if (m_suspensionFile.isOpen()) {
        m_suspensionStream.flush();
        m_suspensionFile.close();
    }
    
    if (m_temperatureFile.isOpen()) {
        m_temperatureStream.flush();
        m_temperatureFile.close();
    }
}

void LoggerWorker::writeHeader(QTextStream &stream, const QString &header)
{
    stream << header << "\n";
    stream.flush();
}

void LoggerWorker::processEntry(const LogEntry &entry)
{
    QTextStream *stream = nullptr;
    
    switch (entry.type) {
    case LogEntry::IMU:
        stream = &m_imuStream;
        break;
    case LogEntry::SUSPENSION:
        stream = &m_suspensionStream;
        break;
    case LogEntry::TEMPERATURE:
        stream = &m_temperatureStream;
        break;
    }
    
    if (stream) {
        *stream << entry.timestamp << "," << entry.data << "\n";
        stream->flush(); // Ensure data is written immediately
    }
}

