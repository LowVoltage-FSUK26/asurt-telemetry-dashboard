#ifndef SERIALPARSERWORKER_H
#define SERIALPARSERWORKER_H

#include <QObject>
#include <QByteArray>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QRunnable>
#include <atomic>

/**
 * @brief The SerialParserWorker class parses raw serial data in a separate thread.
 *
 * This class uses shared static state so all parser workers share the same
 * accumulated values, matching the MQTT parser pattern.
 */
class SerialParserWorker : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit SerialParserWorker(bool debugMode = false, QObject *parent = nullptr);
    ~SerialParserWorker();

    /**
     * @brief Reset all shared static state values to defaults
     * Call this when stopping the Serial manager to prevent stale data on reconnect
     */
    static void resetSharedState();

    void queueData(const QByteArray &data);
    void stop();

protected:
    void run() override;

signals:
    void dataParsed(float speed, int rpm, int accPedal, int brakePedal,
                    double encoderAngle, float temperature, int batteryLevel,
                    double gpsLongitude, double gpsLatitude,
                    int speedFL, int speedFR, int speedBL, int speedBR,
                    double lateralG, double longitudinalG);
    void errorOccurred(const QString &error);

private:
    QQueue<QByteArray> m_dataQueue;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    std::atomic<bool> m_running;
    bool m_debugMode;

    void parseData(const QByteArray &data);

    // Shared state across all parser instances
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
};

#endif // SERIALPARSERWORKER_H

