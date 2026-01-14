#ifndef MQTTPARSERWORKER_H
#define MQTTPARSERWORKER_H

#include <QByteArray>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QRunnable>
#include <QWaitCondition>
#include <atomic>

/**
 * @brief The MqttParserWorker class parses MQTT messages in a thread pool
 *
 * This class is designed to run in a thread pool and efficiently parse MQTT
 * messages without blocking the main thread or the receiver thread.
 */
class MqttParserWorker : public QObject, public QRunnable {
  Q_OBJECT

public:
  explicit MqttParserWorker(bool debugMode = false, QObject *parent = nullptr);
  ~MqttParserWorker();

  /**
   * @brief Reset all shared static state values to defaults
   * Call this when stopping the MQTT client to prevent stale data on reconnect
   */
  static void resetSharedState();

  /**
   * @brief Implement QRunnable interface
   * This method will be executed in a thread pool thread
   */
  void run() override;

public slots:
  /**
   * @brief Queue a message for parsing
   * @param data The message data to parse
   */
  void queueMessage(const QByteArray &data);

  /**
   * @brief Stop the parser worker
   */
  void stop();

signals:
  /**
   * @brief Signal emitted when a message is successfully parsed
   */
  void messageParsed(float speed, int rpm, int accPedal, int brakePedal,
                     double encoderAngle, float temperature, int batteryLevel,
                     double gpsLongitude, double gpsLatitude, int speedFL,
                     int speedFR, int speedBL, int speedBR, double lateralG,
                     double longitudinalG, int tempFL, int tempFR, int tempBL,
                     int tempBR);

  /**
   * @brief Signal emitted when an error occurs during parsing
   * @param error The error message
   */
  void errorOccurred(const QString &error);

private:
  /**
   * @brief Parse a single message
   * @param data The message data to parse
   */
  void parseMessage(const QByteArray &data);

  bool m_debugMode;
  std::atomic<bool> m_running;
  std::atomic<quint64> m_messagesParsed;

  // Thread-safe queue for messages
  QQueue<QByteArray> m_queue;
  QMutex m_queueMutex;
  QWaitCondition m_queueCondition;

  // Shared state across all parser instances
  // All values are static so all parser workers share the same state
  static QMutex s_stateMutex;
  static float s_speed;
  static int s_rpm;
  static int s_accPedal;
  static int s_brakePedal;
  static double s_encoderAngle;
  static float s_temperature;
  static int s_batteryLevel;
  static double s_gpsLongitude;
  static double s_gpsLatitude;
  static int s_speedFL;
  static int s_speedFR;
  static int s_speedBL;
  static int s_speedBR;
  static double s_lateralG;
  static double s_longitudinalG;
  static int s_tempFL;
  static int s_tempFR;
  static int s_tempBL;
  static int s_tempBR;
};

#endif // MQTTPARSERWORKER_H
