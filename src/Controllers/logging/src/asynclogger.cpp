#include "../include/asynclogger.h"
#include <QDebug>
#include <QDir>

AsyncLogger &AsyncLogger::instance() {
  static AsyncLogger instance;
  return instance;
}

AsyncLogger::AsyncLogger(QObject *parent)
    : QObject(parent), m_worker(nullptr), m_initialized(false) {}

AsyncLogger::~AsyncLogger() { shutdown(); }

void AsyncLogger::initialize(const QString &logDirectory) {
  if (m_initialized) {
    return;
  }

  // Register LogEntry metatype for cross-thread signal/slot communication
  // This must be called before any queued connections using LogEntry
  static bool metatypeRegistered = false;
  if (!metatypeRegistered) {
    qRegisterMetaType<LogEntry>("LogEntry");
    metatypeRegistered = true;
  }

  m_logDirectory = logDirectory;

  // Create log directory if it doesn't exist
  QDir dir;
  if (!dir.exists(m_logDirectory)) {
    if (!dir.mkpath(m_logDirectory)) {
      qWarning() << "AsyncLogger: Failed to create log directory:"
                 << m_logDirectory;
      return;
    }
  }

  // Create worker and move to thread
  m_worker = new LoggerWorker(m_logDirectory);
  m_worker->moveToThread(&m_workerThread);

  // Connect signals for log entries and shutdown
  connect(this, &AsyncLogger::logEntryReady, m_worker,
          &LoggerWorker::processEntry, Qt::QueuedConnection);
  connect(this, &AsyncLogger::shutdownWorker, m_worker, &LoggerWorker::shutdown,
          Qt::QueuedConnection);
  connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

  // Connect thread started signal to worker initialization
  // This ensures initialize() runs AFTER the event loop starts
  connect(&m_workerThread, &QThread::started, m_worker,
          &LoggerWorker::initialize);

  // Start worker thread
  m_workerThread.start();
  m_workerThread.setPriority(QThread::LowPriority); // Low priority for logging

  // Wait briefly for thread to fully start and initialize the worker
  // This ensures files are open before we try to log anything
  QThread::msleep(100);

  m_initialized = true;
  qDebug() << "AsyncLogger initialized with log directory:" << m_logDirectory;
}

void AsyncLogger::shutdown() {
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
      qWarning() << "AsyncLogger worker thread did not terminate gracefully, "
                    "forcing termination";
      m_workerThread.terminate();
      m_workerThread.wait(1000);
    }
  }

  m_initialized = false;
  qDebug() << "AsyncLogger shutdown complete";
}

void AsyncLogger::logIMU(int16_t ang_x, int16_t ang_y, int16_t ang_z) {
  if (!m_initialized) {
    qWarning()
        << "AsyncLogger: Attempted to log IMU data but logger not initialized";
    return;
  }

  LogEntry entry;
  entry.type = LogEntry::IMU;
  entry.timestamp = QDateTime::currentMSecsSinceEpoch();
  entry.data = QString("%1,%2,%3").arg(ang_x).arg(ang_y).arg(ang_z);

  emit logEntryReady(entry);
}

void AsyncLogger::logSuspension(uint16_t sus_1, uint16_t sus_2, uint16_t sus_3,
                                uint16_t sus_4) {
  if (!m_initialized) {
    qWarning() << "AsyncLogger: Attempted to log Suspension data but logger "
                  "not initialized";
    return;
  }

  LogEntry entry;
  entry.type = LogEntry::SUSPENSION;
  entry.timestamp = QDateTime::currentMSecsSinceEpoch();
  entry.data =
      QString("%1,%2,%3,%4").arg(sus_1).arg(sus_2).arg(sus_3).arg(sus_4);

  emit logEntryReady(entry);
}

// LoggerWorker implementation

LoggerWorker::LoggerWorker(const QString &logDir)
    : m_logDirectory(logDir), m_filesOpen(false) {}

LoggerWorker::~LoggerWorker() { closeFiles(); }

void LoggerWorker::initialize() {
  qDebug() << "LoggerWorker::initialize() called - log directory:"
           << m_logDirectory;

  // Verify directory exists or create it
  QDir dir(m_logDirectory);
  if (!dir.exists()) {
    qDebug() << "LoggerWorker: Directory doesn't exist, creating:"
             << m_logDirectory;
    if (!dir.mkpath(".")) {
      qWarning() << "LoggerWorker: Failed to create directory:"
                 << m_logDirectory;
      m_filesOpen = false;
      return;
    }
  }

  // Get absolute path for clarity
  QString absPath = dir.absolutePath();
  qDebug() << "LoggerWorker: Using absolute log directory:" << absPath;

  if (openFiles()) {
    m_filesOpen = true;
    qDebug() << "LoggerWorker: Log files opened successfully in:" << absPath;
  } else {
    qWarning() << "LoggerWorker: Failed to open log files in:" << absPath;
    m_filesOpen = false;
  }
}

void LoggerWorker::shutdown() { closeFiles(); }

bool LoggerWorker::openFiles() {
  // Open IMU log file
  m_imuFile.setFileName(m_logDirectory + "/IMU_logger.csv");
  if (!m_imuFile.open(QIODevice::WriteOnly | QIODevice::Append |
                      QIODevice::Text)) {
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
  if (!m_suspensionFile.open(QIODevice::WriteOnly | QIODevice::Append |
                             QIODevice::Text)) {
    qWarning() << "Failed to open suspension log file:"
               << m_suspensionFile.errorString();
    return false;
  }
  m_suspensionStream.setDevice(&m_suspensionFile);

  // Write header if file is empty
  if (m_suspensionFile.size() == 0) {
    writeHeader(m_suspensionStream, "timestamp,SUS_1,SUS_2,SUS_3,SUS_4");
  }

  return true;
}

void LoggerWorker::closeFiles() {
  if (m_imuFile.isOpen()) {
    m_imuStream.flush();
    m_imuFile.close();
  }

  if (m_suspensionFile.isOpen()) {
    m_suspensionStream.flush();
    m_suspensionFile.close();
  }

  m_filesOpen = false;
}

void LoggerWorker::writeHeader(QTextStream &stream, const QString &header) {
  stream << header << "\n";
  stream.flush();
}

void LoggerWorker::processEntry(const LogEntry &entry) {
  qDebug() << "LoggerWorker::processEntry() received entry type:" << entry.type
           << "data:" << entry.data;

  // Ensure files are open before processing
  if (!m_filesOpen) {
    qWarning() << "LoggerWorker: Attempted to log entry but files not open, "
                  "trying to initialize...";
    initialize();
    if (!m_filesOpen) {
      qWarning() << "LoggerWorker: Failed to open files, entry will be lost";
      return;
    }
  }

  QTextStream *stream = nullptr;

  switch (entry.type) {
  case LogEntry::IMU:
    stream = &m_imuStream;
    break;
  case LogEntry::SUSPENSION:
    stream = &m_suspensionStream;
    break;
  }

  if (stream) {
    *stream << entry.timestamp << "," << entry.data << "\n";
    stream->flush(); // Ensure data is written immediately
    qDebug() << "LoggerWorker: Written entry to file";
  } else {
    qWarning() << "LoggerWorker: Unknown log entry type:" << entry.type;
  }
}
