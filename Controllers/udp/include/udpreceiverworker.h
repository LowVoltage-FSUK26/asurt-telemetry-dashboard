#ifndef UDPRECEIVERWORKER_H
#define UDPRECEIVERWORKER_H

#include <QObject>
#include <QUdpSocket>
#include <QElapsedTimer>
#include <atomic>

/**
 * @brief The UdpReceiverWorker class handles UDP datagram reception in a dedicated thread
 *
 * This class is designed to run in its own thread and efficiently receive UDP datagrams
 * without blocking the main thread or other processing threads.
 */
class UdpReceiverWorker : public QObject
{
    Q_OBJECT

public:
    explicit UdpReceiverWorker(QObject *parent = nullptr);
    ~UdpReceiverWorker();

public slots:
    /**
     * @brief Initialize the worker
     * Called when the thread starts
     */
    void initialize();

    /**
     * @brief Start receiving datagrams on the specified port
     * @param port The UDP port to listen on
     */
    void startReceiving(quint16 port);

    /**
     * @brief Stop receiving datagrams
     */
    void stopReceiving();

signals:
    /**
     * @brief Signal emitted when a datagram is received
     * @param data The datagram data
     */
    void datagramReceived(const QByteArray &data);

    /**
     * @brief Signal emitted when an error occurs
     * @param error The error message
     */
    void errorOccurred(const QString &error);

private slots:
    /**
     * @brief Process pending datagrams
     * Called when new datagrams are available
     */
    void processPendingDatagrams();

private:
    QUdpSocket *m_socket;
    std::atomic<bool> m_running;
    QElapsedTimer m_statsTimer;
    quint64 m_datagramsReceived;
    quint64 m_bytesReceived;
};

#endif // UDPRECEIVERWORKER_H

