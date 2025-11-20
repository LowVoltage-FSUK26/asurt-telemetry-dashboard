#ifndef UDPPARSERWORKER_H
#define UDPPARSERWORKER_H

#include <QObject>
#include <QRunnable>
#include <QByteArray>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <atomic>

/**
 * @brief The UdpParserWorker class parses UDP datagrams in a thread pool
 *
 * This class is designed to run in a thread pool and efficiently parse UDP datagrams
 * without blocking the main thread or the receiver thread.
 */
class UdpParserWorker : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit UdpParserWorker(bool debugMode = false, QObject *parent = nullptr);
    ~UdpParserWorker();

    /**
     * @brief Implement QRunnable interface
     * This method will be executed in a thread pool thread
     */
    void run() override;

public slots:
    /**
     * @brief Queue a datagram for parsing
     * @param data The datagram data to parse
     */
    void queueDatagram(const QByteArray &data);

    /**
     * @brief Stop the parser worker
     */
    void stop();

signals:
    /**
     * @brief Signal emitted when a datagram is successfully parsed
     */
    void datagramParsed(float speed, int rpm, int accPedal, int brakePedal,
                        double encoderAngle, float temperature, int batteryLevel,
                        double gpsLongitude, double gpsLatitude,
                        int speedFL, int speedFR, int speedBL, int speedBR,
                        double lateralG, double longitudinalG);

    /**
     * @brief Signal emitted when an error occurs during parsing
     * @param error The error message
     */
    void errorOccurred(const QString &error);

private:
    /**
     * @brief Parse a single datagram
     * @param data The datagram data to parse
     */
    void parseDatagram(const QByteArray &data);

    bool m_debugMode;
    std::atomic<bool> m_running;
    std::atomic<quint64> m_datagramsParsed;

    // Thread-safe queue for datagrams
    QQueue<QByteArray> m_queue;
    QMutex m_queueMutex;
    QWaitCondition m_queueCondition;
};

#endif // UDPPARSERWORKER_H

