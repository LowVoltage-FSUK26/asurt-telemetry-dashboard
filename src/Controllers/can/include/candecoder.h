#ifndef CANDECODER_H
#define CANDECODER_H

#include <QByteArray>
#include <cstdint>

/**
 * @brief CAN message decoder for 20-byte fixed packets
 * 
 * Packet structure:
 * - Bytes 0-3: Timestamp (skipped)
 * - Bytes 4-7: CAN ID (uint32_t, little endian)
 * - Byte 8: DLC (skipped)
 * - Bytes 9-16: Payload (8 bytes)
 * - Bytes 17-19: Padding (ignored)
 */
class CANDecoder
{
public:
    // Constants
    static constexpr double WHEEL_CIRCUMFERENCE = 0.0254 * 3.15 * 18 * 2; // meters
    static constexpr double GRAVITY_ACCEL = 9.81; // m/s²
    static constexpr int PACKET_SIZE = 20;
    
    // CAN IDs
    static constexpr uint32_t CAN_ID_IMU_ANGLE = 0x071;
    static constexpr uint32_t CAN_ID_IMU_ACCEL = 0x072;
    static constexpr uint32_t CAN_ID_ADC = 0x073;
    static constexpr uint32_t CAN_ID_PROXIMITY_ENCODER = 0x074;
    static constexpr uint32_t CAN_ID_GPS = 0x075;
    static constexpr uint32_t CAN_ID_TEMPERATURES = 0x076;
    
    /**
     * @brief Extract CAN ID from 20-byte packet
     * @param packet The 20-byte CAN packet
     * @return CAN ID (little endian)
     */
    static uint32_t extractCANId(const QByteArray &packet);
    
    /**
     * @brief Extract 8-byte payload from packet
     * @param packet The 20-byte CAN packet
     * @return 8-byte payload
     */
    static QByteArray extractPayload(const QByteArray &packet);
    
    // Decoder structures for each CAN ID
    
    struct IMUAngle {
        int16_t ang_x;
        int16_t ang_y;
        int16_t ang_z;
    };
    
    struct IMUAccel {
        double lateral_g;      // Y-axis converted to G-force
        double longitudinal_g; // X-axis converted to G-force
        int16_t accel_z;       // Z-axis (raw, not used)
    };
    
    struct ADCData {
        uint16_t sus_1;
        uint16_t sus_2;
        uint16_t sus_3;
        uint16_t sus_4;
        uint16_t brake_pedal;  // PRESSURE_1
        uint16_t acc_pedal;    // PRESSURE_2
    };
    
    struct ProximityAndEncoder {
        double speed_fl;       // km/h
        double speed_fr;       // km/h
        double speed_bl;       // km/h
        double speed_br;       // km/h
        uint16_t encoder_angle;
        uint8_t speed_kmh;
    };
    
    struct GPS {
        float longitude;
        float latitude;
    };
    
    struct Temperatures {
        int16_t temp_fl;
        int16_t temp_fr;
        int16_t temp_rl;
        int16_t temp_rr;
    };
    
    /**
     * @brief Decode IMU Angle (CAN ID 0x071)
     * @param payload 8-byte payload
     * @return Decoded IMU angle data
     */
    static IMUAngle decodeIMUAngle(const QByteArray &payload);
    
    /**
     * @brief Decode IMU Acceleration (CAN ID 0x072)
     * @param payload 8-byte payload
     * @return Decoded IMU acceleration with G-force conversion
     */
    static IMUAccel decodeIMUAccel(const QByteArray &payload);
    
    /**
     * @brief Decode ADC Data (CAN ID 0x073)
     * @param payload 8-byte payload
     * @return Decoded ADC data with bit-packed values
     */
    static ADCData decodeADC(const QByteArray &payload);
    
    /**
     * @brief Decode Proximity and Encoder Data (CAN ID 0x074)
     * @param payload 8-byte payload
     * @return Decoded proximity/encoder data with RPM to km/h conversion
     */
    static ProximityAndEncoder decodeProximityAndEncoder(const QByteArray &payload);
    
    /**
     * @brief Decode GPS Data (CAN ID 0x075)
     * @param payload 8-byte payload
     * @return Decoded GPS coordinates
     */
    static GPS decodeGPS(const QByteArray &payload);
    
    /**
     * @brief Decode Temperature Data (CAN ID 0x076)
     * @param payload 8-byte payload
     * @return Decoded temperature data
     */
    static Temperatures decodeTemperatures(const QByteArray &payload);

private:
    /**
     * @brief Read int16_t from byte array (little endian)
     */
    static int16_t readInt16LE(const QByteArray &data, int offset);
    
    /**
     * @brief Read uint32_t from byte array (little endian)
     */
    static uint32_t readUInt32LE(const QByteArray &data, int offset);
    
    /**
     * @brief Read uint64_t from byte array (little endian)
     */
    static uint64_t readUInt64LE(const QByteArray &data, int offset);
    
    /**
     * @brief Read float from byte array (little endian)
     */
    static float readFloatLE(const QByteArray &data, int offset);
    
    /**
     * @brief Convert RPM to km/h
     */
    static double rpmToKmh(uint16_t rpm);
};

#endif // CANDECODER_H

