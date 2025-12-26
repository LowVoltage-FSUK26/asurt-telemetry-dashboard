#ifndef ASYNCLOGGER_H
#define ASYNCLOGGER_H

#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTextStream>
#include <QThread>
#include <QWaitCondition>
// Forward declaration
class LoggerWorker;

/**
 * @brief Log entry structure for async logging
 *
 * This struct is passed across threads via signal/slot,
 * requiring Qt metatype registration.
 */
struct LogEntry {
  enum Type { IMU, SUSPENSION, TEMPERATURE };

  // Default constructor required for Qt metatype system
  LogEntry() : type(IMU), timestamp(0) {}

  Type type;
  qint64 timestamp;
  QString data;
};

// Required for cross-thread signal/slot communication with Qt::QueuedConnection
Q_DECLARE_METATYPE(LogEntry)

/**
 * @brief Worker class for async file logging
 */
class LoggerWorker : public QObject {
  Q_OBJECT
public:
  explicit LoggerWorker(const QString &logDir);
  ~LoggerWorker();

public slots:
  void processEntry(const LogEntry &entry);
  void initialize();
  void shutdown();

private:
  QString m_logDirectory;
  QFile m_imuFile;
  QFile m_suspensionFile;
  QFile m_temperatureFile;
  QTextStream m_imuStream;
  QTextStream m_suspensionStream;
  QTextStream m_temperatureStream;
  bool m_filesOpen;

  bool openFiles();
  void closeFiles();
  void writeHeader(QTextStream &stream, const QString &header);
};

/**
 * @brief Async logger for high-frequency CAN data logging
 *
 * Provides thread-safe, non-blocking logging to CSV files with
 * dedicated worker thread and message queue.
 */
class AsyncLogger : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Get singleton instance
   */
  static AsyncLogger &instance();

  /**
   * @brief Initialize the async logger
   * @param logDirectory Directory path for log files
   */
  void initialize(const QString &logDirectory = ".");

  /**
   * @brief Shutdown the async logger
   */
  void shutdown();

  /**
   * @brief Log IMU angle data
   */
  void logIMU(int16_t ang_x, int16_t ang_y, int16_t ang_z);

  /**
   * @brief Log suspension data
   */
  void logSuspension(uint16_t sus_1, uint16_t sus_2, uint16_t sus_3,
                     uint16_t sus_4);

  /**
   * @brief Log temperature data
   */
  void logTemperature(int16_t temp_fl, int16_t temp_fr, int16_t temp_rl,
                      int16_t temp_rr);

private:
  explicit AsyncLogger(QObject *parent = nullptr);
  ~AsyncLogger();

  // Delete copy constructor and assignment operator
  AsyncLogger(const AsyncLogger &) = delete;
  AsyncLogger &operator=(const AsyncLogger &) = delete;

  QThread m_workerThread;
  LoggerWorker *m_worker;
  QString m_logDirectory;
  bool m_initialized;

signals:
  void logEntryReady(const LogEntry &entry);
  void shutdownWorker();
};

#endif // ASYNCLOGGER_H
