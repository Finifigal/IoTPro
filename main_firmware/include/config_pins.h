#ifndef CONFIG_PINS_H
#define CONFIG_PINS_H

#include <Arduino.h>

// ==========================================
// 1. MODULE DIEU KHIEN DEN (VOICE & MANUAL)
// Tong cong: 4 den dieu khien phong + 1 den trang thai + 1 den bao dong
// ==========================================
#define PIN_LED_KHACH       26  // LED 1: Đèn phòng khách
#define PIN_LED_NGU         32  // LED 2: Đèn phòng ngủ
#define PIN_LED_BEP         1   // LED 3 (MỚI): Đèn phòng bếp (chân TX0)
#define PIN_LED_BANCONG     3   // LED 4 (MỚI): Đèn ban công / hành lang (chân RX0)
#define PIN_LED_STATUS      33  // LED 5: Đèn trạng thái hệ thống / WiFi
#define PIN_LED_ALARM       16  // LED 6 (Đỏ): Báo động khẩn cấp hỏa hoạn / đột nhập

// ==========================================
// 2. KHU VỰC CỬA & XÁC THỰC (RFID & SERVO)
// ==========================================
#define PIN_RFID_SS         5   // SDA / CS (VSPI)
#define PIN_RFID_RST        4   // Reset RFID
#define PIN_RFID_SCK        18  // Clock SPI
#define PIN_RFID_MISO       19  // MISO SPI
#define PIN_RFID_MOSI       23  // MOSI SPI
#define PIN_SERVO           13  // Động cơ mở chốt khóa cửa SG90
#define PIN_BUZZER_SMALL    2   // Còi beep nhỏ phản hồi quẹt thẻ

// ==========================================
// 3. AN TOÀN & KHÍ GAS & CHÁY NỔ
// ==========================================
#define PIN_SENSOR_MQ       34  // Cảm biến khí gas / khói (ADC1 - Input Only)
#define PIN_SENSOR_DHT22    27  // Cảm biến nhiệt độ, độ ẩm môi trường
#define PIN_ALARM_SIREN     25  // Còi hú lớn (Kích qua Transistor NPN)

// ==========================================
// 4. AN NINH & ĐỘT NHẬP & ÁNH SÁNG
// ==========================================
#define PIN_PIR_MOTION      14  // Cảm biến chuyển động hồng ngoại
#define PIN_SENSOR_LDR      35  // Cảm biến ánh sáng quang trở (ADC1)

// ==========================================
// 5. GIAO TIẾP I2C (OLED SSD1306 & MPU-6050)
// ==========================================
#define PIN_I2C_SDA         21  // Bus I2C Data
#define PIN_I2C_SCL         22  // Bus I2C Clock

// ==========================================
// 6. MICROPHONE I2S (EDGE IMPULSE VOICE AI)
// ==========================================
#define PIN_I2S_SD          12  // Serial Data từ mic INMP441
#define PIN_I2S_WS          15  // Word Select / L/R Clock
#define PIN_I2S_SCK         17  // Continuous Serial Clock

#endif // CONFIG_PINS_H
