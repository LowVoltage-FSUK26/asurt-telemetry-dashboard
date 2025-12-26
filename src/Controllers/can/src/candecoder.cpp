#include "../include/candecoder.h"
#include <cstring>

uint32_t CANDecoder::extractCANId(const QByteArray &packet)
{
    if (packet.size() < 8) {
        return 0;
    }
    return readUInt32LE(packet, 4);
}

QByteArray CANDecoder::extractPayload(const QByteArray &packet)
{
    if (packet.size() < 17) {
        return QByteArray();
    }
    return packet.mid(9, 8);
}

CANDecoder::IMUAngle CANDecoder::decodeIMUAngle(const QByteArray &payload)
{
    IMUAngle result;
    result.ang_x = readInt16LE(payload, 0);
    result.ang_y = readInt16LE(payload, 2);
    result.ang_z = readInt16LE(payload, 4);
    return result;
}

CANDecoder::IMUAccel CANDecoder::decodeIMUAccel(const QByteArray &payload)
{
    IMUAccel result;
    int16_t accel_x = readInt16LE(payload, 0);
    int16_t accel_y = readInt16LE(payload, 2);
    result.accel_z = readInt16LE(payload, 4);
    
    // Convert to G-force
    result.longitudinal_g = static_cast<double>(accel_x) / GRAVITY_ACCEL;
    result.lateral_g = static_cast<double>(accel_y) / GRAVITY_ACCEL;
    
    return result;
}

CANDecoder::ADCData CANDecoder::decodeADC(const QByteArray &payload)
{
    ADCData result;
    uint64_t raw_data = readUInt64LE(payload, 0);
    
    // Extract bit-packed values
    result.sus_1 = (raw_data >> 0) & 0x3FF;   // 10 bits
    result.sus_2 = (raw_data >> 10) & 0x3FF;  // 10 bits
    result.sus_3 = (raw_data >> 20) & 0x3FF;  // 10 bits
    result.sus_4 = (raw_data >> 30) & 0x3FF;  // 10 bits
    result.brake_pedal = (raw_data >> 40) & 0x3FF;  // 10 bits (PRESSURE_1)
    result.acc_pedal = (raw_data >> 50) & 0x3FF;    // 10 bits (PRESSURE_2)
    // Bits 60-63 are ignored
    
    return result;
}

CANDecoder::ProximityAndEncoder CANDecoder::decodeProximityAndEncoder(const QByteArray &payload)
{
    ProximityAndEncoder result;
    uint64_t raw_data = readUInt64LE(payload, 0);
    
    // Extract bit-packed values
    uint16_t rpm_fl = (raw_data >> 0) & 0x7FF;   // 11 bits
    uint16_t rpm_fr = (raw_data >> 11) & 0x7FF;  // 11 bits
    uint16_t rpm_rl = (raw_data >> 22) & 0x7FF;  // 11 bits
    uint16_t rpm_rr = (raw_data >> 33) & 0x7FF;  // 11 bits
    result.encoder_angle = (raw_data >> 44) & 0x3FF;  // 10 bits
    result.speed_kmh = (raw_data >> 54) & 0xFF;       // 8 bits
    
    // Convert RPM to km/h
    result.speed_fl = rpmToKmh(rpm_fl);
    result.speed_fr = rpmToKmh(rpm_fr);
    result.speed_bl = rpmToKmh(rpm_rl);
    result.speed_br = rpmToKmh(rpm_rr);
    
    return result;
}

CANDecoder::GPS CANDecoder::decodeGPS(const QByteArray &payload)
{
    GPS result;
    result.longitude = readFloatLE(payload, 0);
    result.latitude = readFloatLE(payload, 4);
    return result;
}

CANDecoder::Temperatures CANDecoder::decodeTemperatures(const QByteArray &payload)
{
    Temperatures result;
    result.temp_fl = readInt16LE(payload, 0);
    result.temp_fr = readInt16LE(payload, 2);
    result.temp_rl = readInt16LE(payload, 4);
    result.temp_rr = readInt16LE(payload, 6);
    return result;
}

// Private helper methods

int16_t CANDecoder::readInt16LE(const QByteArray &data, int offset)
{
    if (offset + 2 > data.size()) {
        return 0;
    }
    int16_t value;
    std::memcpy(&value, data.constData() + offset, sizeof(int16_t));
    return value;  // Assumes little-endian system
}

uint32_t CANDecoder::readUInt32LE(const QByteArray &data, int offset)
{
    if (offset + 4 > data.size()) {
        return 0;
    }
    uint32_t value;
    std::memcpy(&value, data.constData() + offset, sizeof(uint32_t));
    return value;  // Assumes little-endian system
}

uint64_t CANDecoder::readUInt64LE(const QByteArray &data, int offset)
{
    if (offset + 8 > data.size()) {
        return 0;
    }
    uint64_t value;
    std::memcpy(&value, data.constData() + offset, sizeof(uint64_t));
    return value;  // Assumes little-endian system
}

float CANDecoder::readFloatLE(const QByteArray &data, int offset)
{
    if (offset + 4 > data.size()) {
        return 0.0f;
    }
    float value;
    std::memcpy(&value, data.constData() + offset, sizeof(float));
    return value;  // Assumes little-endian system
}

double CANDecoder::rpmToKmh(uint16_t rpm)
{
    // Convert RPM to km/h: (RPM * circumference * 60) / 1000
    return (static_cast<double>(rpm) * WHEEL_CIRCUMFERENCE * 60.0) / 1000.0;
}

