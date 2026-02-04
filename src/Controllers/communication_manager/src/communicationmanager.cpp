#include "../include/communicationmanager.h"
#include "../../serial/include/serialmanager.h"
#include "../../udp/include/udpclient.h"
#include "../../mqtt/include/mqttclient.h"
#include <QDebug>

CommunicationManager::CommunicationManager(QObject *parent)
    : QObject(parent),
    m_udpClient(new UdpClient(this)),
    m_serialManager(new SerialManager(this)),
    m_mqttClient(new MqttClient(this)),
    m_currentSource(SourceType::None),
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
    m_tempBR(0),
    m_isSerialSource(false)
{
    // Connect signals from UdpClient to CommunicationManager's slots
    connect(m_udpClient, &UdpClient::speedChanged, this, [this](float newSpeed){ handleSpeedChanged(newSpeed, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::rpmChanged, this, [this](int newRPM){ handleRpmChanged(newRPM, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::accPedalChanged, this, [this](int newAccPedal){ handleAccPedalChanged(newAccPedal, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::brakePedalChanged, this, [this](int newBrakePedal){ handleBrakePedalChanged(newBrakePedal, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::encoderAngleChanged, this, [this](double newAngle){ handleEncoderAngleChanged(newAngle, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::temperatureChanged, this, [this](float newTemp){ handleTemperatureChanged(newTemp, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::batteryLevelChanged, this, [this](int newBatteryLevel){ handleBatteryLevelChanged(newBatteryLevel, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::gpsLongitudeChanged, this, [this](double newLongtitude){ handleGpsLongitudeChanged(newLongtitude, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::gpsLatitudeChanged, this, [this](double newLatitude){ handleGpsLatitudeChanged(newLatitude, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::speedFLChanged, this, [this](double newSpeedFL){ handleSpeedFLChanged(newSpeedFL, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::speedFRChanged, this, [this](double newSpeedFR){ handleSpeedFRChanged(newSpeedFR, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::speedBLChanged, this, [this](double newSpeedBL){ handleSpeedBLChanged(newSpeedBL, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::speedBRChanged, this, [this](double newSpeedBR){ handleSpeedBRChanged(newSpeedBR, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::lateralGChanged, this, [this](double newLateralG) { handleLateralGChanged(newLateralG, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::longitudinalGChanged, this, [this](double newLogitudinalG){ handleLongitudinalGChanged(newLogitudinalG, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::tempFLChanged, this, [this](int newTempFL) { handleTempFLChanged(newTempFL, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::tempFRChanged, this, [this](int newTempFR) { handleTempFRChanged(newTempFR, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::tempBLChanged, this, [this](int newTempBL) { handleTempBLChanged(newTempBL, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::tempBRChanged, this, [this](int newTempBR) { handleTempBRChanged(newTempBR, SourceType::Udp); });
    connect(m_udpClient, &UdpClient::errorOccurred, this, &CommunicationManager::handleError);

    // Connect signals from SerialManager to CommunicationManager's slots
    connect(m_serialManager, &SerialManager::speedChanged, this, [this](float newSpeed){ handleSpeedChanged(newSpeed, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::rpmChanged, this, [this](int newRPM){ handleRpmChanged(newRPM, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::accPedalChanged, this, [this](int newAccPedal){ handleAccPedalChanged(newAccPedal, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::brakePedalChanged, this, [this](int newBrakePedal){ handleBrakePedalChanged(newBrakePedal, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::encoderAngleChanged, this, [this](double newAngle){ handleEncoderAngleChanged(newAngle, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::temperatureChanged, this, [this](float newTemp){ handleTemperatureChanged(newTemp, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::batteryLevelChanged, this, [this](int newBatteryLevel){ handleBatteryLevelChanged(newBatteryLevel, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::gpsLongitudeChanged, this, [this](double newLongtitude){ handleGpsLongitudeChanged(newLongtitude, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::gpsLatitudeChanged, this, [this](double newLatitude){ handleGpsLatitudeChanged(newLatitude, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::speedFLChanged, this, [this](double newSpeedFL){ handleSpeedFLChanged(newSpeedFL, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::speedFRChanged, this, [this](double newSpeedFR){ handleSpeedFRChanged(newSpeedFR, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::speedBLChanged, this, [this](double newSpeedBL){ handleSpeedBLChanged(newSpeedBL, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::speedBRChanged, this, [this](double newSpeedBR){ handleSpeedBRChanged(newSpeedBR, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::lateralGChanged, this, [this](double newLateralG) { handleLateralGChanged(newLateralG, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::longitudinalGChanged, this, [this](double newLogitudinalG){ handleLongitudinalGChanged(newLogitudinalG, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::tempFLChanged, this, [this](int newTempFL) { handleTempFLChanged(newTempFL, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::tempFRChanged, this, [this](int newTempFR) { handleTempFRChanged(newTempFR, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::tempBLChanged, this, [this](int newTempBL) { handleTempBLChanged(newTempBL, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::tempBRChanged, this, [this](int newTempBR) { handleTempBRChanged(newTempBR, SourceType::Serial); });
    connect(m_serialManager, &SerialManager::errorOccurred, this, &CommunicationManager::handleError);

    connect(m_mqttClient, &MqttClient::speedChanged, this, [this](float newSpeed){ handleSpeedChanged(newSpeed, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::rpmChanged, this, [this](int newRPM){ handleRpmChanged(newRPM, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::accPedalChanged, this, [this](int newAccPedal){ handleAccPedalChanged(newAccPedal, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::brakePedalChanged, this, [this](int newBrakePedal){ handleBrakePedalChanged(newBrakePedal, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::encoderAngleChanged, this, [this](double newAngle){ handleEncoderAngleChanged(newAngle, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::temperatureChanged, this, [this](float newTemp){ handleTemperatureChanged(newTemp, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::batteryLevelChanged, this, [this](int newBatteryLevel){ handleBatteryLevelChanged(newBatteryLevel, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::gpsLongitudeChanged, this, [this](double newLongtitude){ handleGpsLongitudeChanged(newLongtitude, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::gpsLatitudeChanged, this, [this](double newLatitude){ handleGpsLatitudeChanged(newLatitude, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::speedFLChanged, this, [this](double newSpeedFL){ handleSpeedFLChanged(newSpeedFL, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::speedFRChanged, this, [this](double newSpeedFR){ handleSpeedFRChanged(newSpeedFR, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::speedBLChanged, this, [this](double newSpeedBL){ handleSpeedBLChanged(newSpeedBL, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::speedBRChanged, this, [this](double newSpeedBR){ handleSpeedBRChanged(newSpeedBR, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::lateralGChanged, this, [this](double newLateralG) { handleLateralGChanged(newLateralG, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::longitudinalGChanged, this, [this](double newLogitudinalG){ handleLongitudinalGChanged(newLogitudinalG, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::tempFLChanged, this, [this](int newTempFL) { handleTempFLChanged(newTempFL, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::tempFRChanged, this, [this](int newTempFR) { handleTempFRChanged(newTempFR, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::tempBLChanged, this, [this](int newTempBL) { handleTempBLChanged(newTempBL, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::tempBRChanged, this, [this](int newTempBR) { handleTempBRChanged(newTempBR, SourceType::Mqtt); });
    connect(m_mqttClient, &MqttClient::errorOccurred, this, &CommunicationManager::handleError);
}

CommunicationManager::~CommunicationManager()
{
    stop();
}

bool CommunicationManager::startSerial(const QString &portName, qint32 baudRate)
{
    stop(); // Stop any active communication first
    bool success = m_serialManager->start(portName, baudRate);
    if (success)
    {
        m_currentSource = SourceType::Serial;
        setIsSerialSource(true);
        qDebug() << "CommunicationManager: Serial started.";
    }
    else
    {
        qDebug() << "CommunicationManager: Failed to start Serial.";
    }
    return success;
}

bool CommunicationManager::startUdp(quint16 port)
{
    stop(); // Stop any active communication first
    bool success = m_udpClient->start(port);
    if (success)
    {
        m_currentSource = SourceType::Udp;
        setIsSerialSource(false);
        qDebug() << "CommunicationManager: UDP started.";
    }
    else
    {
        qDebug() << "CommunicationManager: Failed to start UDP.";
    }
    return success;
}

bool CommunicationManager::startMqtt(const QString &brokerAddress, quint16 port, bool useTls, const QString &clientId, const QString &username, const QString &password, const QString &topic)
{
    stop(); // Stop any active communication first
    bool success = m_mqttClient->start(brokerAddress, port, useTls, clientId, username, password, topic);
    if (success)
    {
        m_currentSource = SourceType::Mqtt;
        setIsSerialSource(false);
        qDebug() << "CommunicationManager: MQTT started.";
    }
    else
    {
        qDebug() << "CommunicationManager: Failed to start MQTT.";
    }
    return success;
}

bool CommunicationManager::stop()
{
    bool success = false;
    if (m_currentSource == SourceType::Serial)
    {
        success = m_serialManager->stop();
        qDebug() << "CommunicationManager: Serial stopped.";
    }
    else if (m_currentSource == SourceType::Udp)
    {
        success = m_udpClient->stop();
        qDebug() << "CommunicationManager: UDP stopped.";
    }
    else if (m_currentSource == SourceType::Mqtt)
    {
        success = m_mqttClient->stop();
        qDebug() << "CommunicationManager: MQTT stopped.";
    }
    m_currentSource = SourceType::None;
    return success;
}

void CommunicationManager::setIsSerialSource(bool isSerialSource)
{
    if (m_isSerialSource != isSerialSource)
    {
        m_isSerialSource = isSerialSource;
        emit isSerialSourceChanged(m_isSerialSource);
    }
}

void CommunicationManager::handleError(const QString &error)
{
    emit errorOccurred(error);
}

void CommunicationManager::handleSpeedChanged(float newSpeed, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_speed != newSpeed)
        {
            m_speed = newSpeed;
            emit speedChanged(m_speed);
        }
    }
}

void CommunicationManager::handleRpmChanged(int newRpm, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_rpm != newRpm)
        {
            m_rpm = newRpm;
            emit rpmChanged(m_rpm);
        }
    }
}

void CommunicationManager::handleAccPedalChanged(int newAccPedal, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_accPedal != newAccPedal)
        {
            m_accPedal = newAccPedal;
            emit accPedalChanged(m_accPedal);
        }
    }
}

void CommunicationManager::handleBrakePedalChanged(int newBrakePedal, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_brakePedal != newBrakePedal)
        {
            m_brakePedal = newBrakePedal;
            emit brakePedalChanged(m_brakePedal);
        }
    }
}

void CommunicationManager::handleEncoderAngleChanged(double newAngle, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_encoderAngle != newAngle)
        {
            m_encoderAngle = newAngle;
            emit encoderAngleChanged(m_encoderAngle);
        }
    }
}

void CommunicationManager::handleTemperatureChanged(float newTemperature, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_temperature != newTemperature)
        {
            m_temperature = newTemperature;
            emit temperatureChanged(m_temperature);
        }
    }
}

void CommunicationManager::handleBatteryLevelChanged(int newBatteryLevel, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_batteryLevel != newBatteryLevel)
        {
            m_batteryLevel = newBatteryLevel;
            emit batteryLevelChanged(m_batteryLevel);
        }
    }
}

void CommunicationManager::handleGpsLongitudeChanged(double newLongitude, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_gpsLongitude != newLongitude)
        {
            m_gpsLongitude = newLongitude;
            emit gpsLongitudeChanged(m_gpsLongitude);
        }
    }
}

void CommunicationManager::handleGpsLatitudeChanged(double newGpsLatitude, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_gpsLatitude != newGpsLatitude)
        {
            m_gpsLatitude = newGpsLatitude;
            emit gpsLatitudeChanged(m_gpsLatitude);
        }
    }
}

void CommunicationManager::handleSpeedFLChanged(int newSpeedFL, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_speedFL != newSpeedFL)
        {
            m_speedFL = newSpeedFL;
            emit speedFLChanged(m_speedFL);
        }
    }
}

void CommunicationManager::handleSpeedFRChanged(int newSpeedFR, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_speedFR != newSpeedFR)
        {
            m_speedFR = newSpeedFR;
            emit speedFRChanged(m_speedFR);
        }
    }
}

void CommunicationManager::handleSpeedBLChanged(int newSpeedBL, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_speedBL != newSpeedBL)
        {
            m_speedBL = newSpeedBL;
            emit speedBLChanged(m_speedBL);
        }
    }
}

void CommunicationManager::handleSpeedBRChanged(int newSpeedBR, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_speedBR != newSpeedBR)
        {
            m_speedBR = newSpeedBR;
            emit speedBRChanged(m_speedBR);
        }
    }
}

void CommunicationManager::handleLateralGChanged(double newLateralG, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_lateralG != newLateralG)
        {
            m_lateralG = newLateralG;
            emit lateralGChanged(m_lateralG);
        }
    }
}

void CommunicationManager::handleLongitudinalGChanged(double newLongitudinalG, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_longitudinalG != newLongitudinalG)
        {
            m_longitudinalG = newLongitudinalG;
            emit longitudinalGChanged(m_longitudinalG);
        }
    }
}

void CommunicationManager::handleTempFLChanged(int newTempFL, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_tempFL != newTempFL)
        {
            m_tempFL = newTempFL;
            emit tempFLChanged(m_tempFL);
        }
    }
}

void CommunicationManager::handleTempFRChanged(int newTempFR, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_tempFR != newTempFR)
        {
            m_tempFR = newTempFR;
            emit tempFRChanged(m_tempFR);
        }
    }
}

void CommunicationManager::handleTempBLChanged(int newTempBL, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_tempBL != newTempBL)
        {
            m_tempBL = newTempBL;
            emit tempBLChanged(m_tempBL);
        }
    }
}

void CommunicationManager::handleTempBRChanged(int newTempBR, SourceType source)
{
    if (m_currentSource == source)
    {
        if (m_tempBR != newTempBR)
        {
            m_tempBR = newTempBR;
            emit tempBRChanged(m_tempBR);
        }
    }
}
