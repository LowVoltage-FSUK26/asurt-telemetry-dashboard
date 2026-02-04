#ifndef COMMUNICATIONMANAGER_H
#define COMMUNICATIONMANAGER_H

#include <QObject>
#include <QVariant>
#include <QDebug>

// Forward declarations
class UdpClient;
class SerialManager;
class MqttClient;


class CommunicationManager : public QObject
{
    Q_OBJECT

    // Expose all properties that were previously in UdpClient and SerialManager
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
    Q_PROPERTY(bool isSerialSource READ isSerialSource WRITE setIsSerialSource NOTIFY isSerialSourceChanged)

public:
    explicit CommunicationManager(QObject *parent = nullptr);
    ~CommunicationManager();

    // Property getters
    float speed() const { return m_speed; }
    int rpm() const { return m_rpm; }
    int accPedal() const { return m_accPedal; }
    int brakePedal() const { return m_brakePedal; }
    double encoderAngle() const { return m_encoderAngle; }
    float temperature() const { return m_temperature; }
    int batteryLevel() const { return m_batteryLevel; }
    double gpsLongitude() const { return m_gpsLongitude; }
    double gpsLatitude() const { return m_gpsLatitude; }
    int speedFL() const { return m_speedFL; }
    int speedFR() const { return m_speedFR; }
    int speedBL() const { return m_speedBL; }
    int speedBR() const { return m_speedBR; }
    double lateralG() const { return m_lateralG; }
    double longitudinalG() const { return m_longitudinalG; }
    int tempFL() const { return m_tempFL; }
    int tempFR() const { return m_tempFR; }
    int tempBL() const { return m_tempBL; }
    int tempBR() const { return m_tempBR; }

    Q_INVOKABLE bool startSerial(const QString &portName, qint32 baudRate);
    Q_INVOKABLE bool startUdp(quint16 port);
    Q_INVOKABLE bool startMqtt(const QString &brokerAddress, quint16 port, bool useTls, const QString &clientId, const QString &username, const QString &password, const QString &topic);
    Q_INVOKABLE bool stop();

    bool isSerialSource() const { return m_isSerialSource; }
    void setIsSerialSource(bool isSerialSource);

    enum class SourceType { None, Serial, Udp, Mqtt };

signals:
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
    void isSerialSourceChanged(bool isSerialSource);
    void errorOccurred(const QString &error);

private slots:

    void handleSpeedChanged(float newSpeed, CommunicationManager::SourceType source);
    void handleRpmChanged(int newRpm, CommunicationManager::SourceType source);
    void handleAccPedalChanged(int newAccPedal, CommunicationManager::SourceType source);
    void handleBrakePedalChanged(int newBrakePedal, CommunicationManager::SourceType source);
    void handleEncoderAngleChanged(double newAngle, CommunicationManager::SourceType source);
    void handleTemperatureChanged(float newTemperature, CommunicationManager::SourceType source);
    void handleBatteryLevelChanged(int newBatteryLevel, CommunicationManager::SourceType source);
    void handleGpsLongitudeChanged(double newLongitude, CommunicationManager::SourceType source);
    void handleGpsLatitudeChanged(double newGpsLatitude, CommunicationManager::SourceType source);
    void handleSpeedFLChanged(int newSpeedFL, CommunicationManager::SourceType source);
    void handleSpeedFRChanged(int newSpeedFR, CommunicationManager::SourceType source);
    void handleSpeedBLChanged(int newSpeedBL, CommunicationManager::SourceType source);
    void handleSpeedBRChanged(int newSpeedBR, CommunicationManager::SourceType source);
    void handleLateralGChanged(double newLateralG, CommunicationManager::SourceType source);
    void handleLongitudinalGChanged(double newLongitudinalG, CommunicationManager::SourceType source);
    void handleTempFLChanged(int newTempFL, CommunicationManager::SourceType source);
    void handleTempFRChanged(int newTempFR, CommunicationManager::SourceType source);
    void handleTempBLChanged(int newTempBL, CommunicationManager::SourceType source);
    void handleTempBRChanged(int newTempBR, CommunicationManager::SourceType source);
    
    // Error Handler
    void handleError(const QString &error);

private:


    UdpClient *m_udpClient;
    SerialManager *m_serialManager;
    MqttClient *m_mqttClient;

    SourceType m_currentSource;

    // Internal storage for properties
    float m_speed;
    int m_rpm;
    int m_accPedal;
    int m_brakePedal;
    double m_encoderAngle;
    float m_temperature;
    int m_batteryLevel;
    double m_gpsLongitude;
    double m_gpsLatitude;
    int m_speedFL;
    int m_speedFR;
    int m_speedBL;
    int m_speedBR;
    double m_lateralG;
    double m_longitudinalG;
    int m_tempFL;
    int m_tempFR;
    int m_tempBL;
    int m_tempBR;
    bool m_isSerialSource;

};

#endif // COMMUNICATIONMANAGER_H

