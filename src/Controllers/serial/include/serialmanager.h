#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QThreadPool>
#include <QAtomicInt>
#include <QTimer>
#include <atomic>

// Forward declarations
class SerialReceiverWorker;
class SerialParserWorker;

/**
 * @brief The SerialManager class provides a high-performance serial client for receiving and parsing data
 */
class SerialManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(int rpm READ rpm NOTIFY rpmChanged)
    Q_PROPERTY(int accPedal READ accPedal NOTIFY accPedalChanged)
    Q_PROPERTY(int brakePedal READ brakePedal NOTIFY brakePedalChanged)
    Q_PROPERTY(double encoderAngle READ encoderAngle NOTIFY encoderAngleChanged)
    Q_PROPERTY(float temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(double gpsLongitude READ gpsLongitude NOTIFY gpsLongitudeChanged)
    Q_PROPERTY(double gpsLatitude READ gpsLatitude NOTIFY gpsLatitudeChanged)
    Q_PROPERTY(int speedFL READ speedFL NOTIFY speedFLChanged)
    Q_PROPERTY(int speedFR READ speedFR NOTIFY speedFRChanged)
    Q_PROPERTY(int speedBL READ speedBL NOTIFY speedBLChanged)
    Q_PROPERTY(int speedBR READ speedBR NOTIFY speedBRChanged)
    Q_PROPERTY(double lateralG READ lateralG NOTIFY lateralGChanged)
    Q_PROPERTY(double longitudinalG READ longitudinalG NOTIFY longitudinalGChanged)
    Q_PROPERTY(int tempFL READ tempFL NOTIFY tempFLChanged)
    Q_PROPERTY(int tempFR READ tempFR NOTIFY tempFRChanged)
    Q_PROPERTY(int tempBL READ tempBL NOTIFY tempBLChanged)
    Q_PROPERTY(int tempBR READ tempBR NOTIFY tempBRChanged)

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    Q_INVOKABLE bool start(const QString &portName, qint32 baudRate);
    Q_INVOKABLE bool stop();
    Q_INVOKABLE void setParserThreadCount(int count);
    Q_INVOKABLE void setDebugMode(bool enabled);

    // Property getters
    float speed() const { return m_speed.load(); }
    int rpm() const { return m_rpm.load(); }
    int accPedal() const { return m_accPedal.load(); }
    int brakePedal() const { return m_brakePedal.load(); }
    double encoderAngle() const { return m_encoderAngle.load(); }
    float temperature() const { return m_temperature.load(); }
    int batteryLevel() const { return m_batteryLevel.load(); }
    double gpsLongitude() const { return m_gpsLongitude.load(); }
    double gpsLatitude() const { return m_gpsLatitude.load(); }
    int speedFL() const { return m_speedFL.load(); }
    int speedFR() const { return m_speedFR.load(); }
    int speedBL() const { return m_speedBL.load(); }
    int speedBR() const { return m_speedBR.load(); }
    double lateralG() const { return m_lateralG.load(); }
    double longitudinalG() const { return m_longitudinalG.load(); }
    int tempFL() const { return m_tempFL.load(); }
    int tempFR() const { return m_tempFR.load(); }
    int tempBL() const { return m_tempBL.load(); }
    int tempBR() const { return m_tempBR.load(); }

signals:
    // Property change signals
    void speedChanged(float newSpeed);
    void rpmChanged(int newRpm);
    void accPedalChanged(int newAccPedal);
    void brakePedalChanged(int newBrakePedal);
    void encoderAngleChanged(double newAngle);
    void temperatureChanged(float newTemperature);
    void batteryLevelChanged(int newBatteryLevel);
    void gpsLongitudeChanged(double newLongitude);
    void gpsLatitudeChanged(double newLatitude);
    void speedFLChanged(int newSpeedFL);
    void speedFRChanged(int newSpeedFR);
    void speedBLChanged(int newSpeedBL);
    void speedBRChanged(int newSpeedBR);
    void lateralGChanged(double newLateralG);
    void longitudinalGChanged(double newLongitudinalG);
    void tempFLChanged(int newTempFL);
    void tempFRChanged(int newTempFR);
    void tempBLChanged(int newTempBL);
    void tempBRChanged(int newTempBR);

    // Error signal
    void errorOccurred(const QString &error);

    // Internal signals for worker communication
    void startReceiving(const QString &portName, qint32 baudRate);
    void stopReceiving();

private slots:
    void handleParsedData(float speed, int rpm, int accPedal, int brakePedal,
                          double encoderAngle, float temperature, int batteryLevel,
                          double gpsLongitude, double gpsLatitude,
                          int speedFL, int speedFR, int speedBL, int speedBR,
                          double lateralG, double longitudinalG,
                          int tempFL, int tempFR, int tempBL, int tempBR);

    void handleError(const QString &error);
    void handleSerialDataReceived(const QByteArray &data);

    /**
     * @brief Flush pending updates to QML at 60Hz rate
     */
    void flushPendingUpdates();

private:
    QThread m_receiverThread;
    SerialReceiverWorker *m_receiverWorker;

    QThreadPool m_parserPool;
    QList<SerialParserWorker *> m_parsers;
    int m_nextParserIndex;

    int m_parserThreadCount;
    bool m_debugMode;

    // Update throttling (60Hz)
    QTimer *m_updateTimer;
    std::atomic<bool> m_pendingUpdate;

    std::atomic<qint64> m_datagramsProcessed;
    std::atomic<qint64> m_datagramsDropped;

    // Data storage with atomic access
    std::atomic<float> m_speed;
    std::atomic<int> m_rpm;
    std::atomic<int> m_accPedal;
    std::atomic<int> m_brakePedal;
    std::atomic<double> m_encoderAngle;
    std::atomic<float> m_temperature;
    std::atomic<int> m_batteryLevel;
    std::atomic<double> m_gpsLongitude;
    std::atomic<double> m_gpsLatitude;
    std::atomic<int> m_speedFL;
    std::atomic<int> m_speedFR;
    std::atomic<int> m_speedBL;
    std::atomic<int> m_speedBR;
    std::atomic<double> m_lateralG;
    std::atomic<double> m_longitudinalG;
    std::atomic<int> m_tempFL;
    std::atomic<int> m_tempFR;
    std::atomic<int> m_tempBL;
    std::atomic<int> m_tempBR;

    void initializeParsers();
    void cleanupParsers();
};

#endif // SERIALMANAGER_H

