/**
 * @file voice_test_standalone.ino
 * @brief Code test độc lập mô hình Edge Impulse Voice Recognition trên ESP32
 * Nhận diện: Bật/tắt đèn phòng khách (LED 1 - GPIO 26), Bật/tắt đèn phòng ngủ (LED 2 - GPIO 32)
 */

#include <Arduino.h>

// Sau khi cài thư viện Edge Impulse từ file zip, include header này vào:
// Thay bằng tên header project của bạn: ví dụ <ESP32-SmartHome-VoiceControl_inferencing.h>
// #include <ESP32-SmartHome-VoiceControl_inferencing.h>

// Định nghĩa chân 2 đèn LED
#define PIN_LED_KHACH   26  // Đèn phòng khách (LED Xanh 1)
#define PIN_LED_NGU     32  // Đèn phòng ngủ (LED Xanh 2)

// Ngưỡng độ tin cậy để kích hoạt lệnh (0.0 đến 1.0)
const float CONFIDENCE_THRESHOLD = 0.75;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== KHOI DONG MODULE VOICE CONTROL (EDGE IMPULSE) ===");

    // Cấu hình chân LED
    pinMode(PIN_LED_KHACH, OUTPUT);
    pinMode(PIN_LED_NGU, OUTPUT);
    digitalWrite(PIN_LED_KHACH, LOW);
    digitalWrite(PIN_LED_NGU, LOW);

    Serial.println("He thong da san sang nhan lenh giong noi...");
}

/**
 * Hàm mô phỏng xử lý kết quả phân loại từ Edge Impulse
 * @param label Tên nhãn dự đoán (bat_den_khach, tat_den_khach, bat_den_ngu, tat_den_ngu)
 * @param score Điểm tin cậy (0.0 - 1.0)
 */
void handleVoiceCommand(const char* label, float score) {
    if (score < CONFIDENCE_THRESHOLD) {
        return; // Bỏ qua nếu độ tin cậy thấp hoặc tiếng ồn
    }

    Serial.printf("[VOICE DETECTED] Lenh: %s | Do tin cay: %.2f%%\n", label, score * 100);

    if (strcmp(label, "bat_den_khach") == 0) {
        digitalWrite(PIN_LED_KHACH, HIGH);
        Serial.println("-> DA BAT DEN PHONG KHACH (GPIO 26)");
    } 
    else if (strcmp(label, "tat_den_khach") == 0) {
        digitalWrite(PIN_LED_KHACH, LOW);
        Serial.println("-> DA TAT DEN PHONG KHACH (GPIO 26)");
    } 
    else if (strcmp(label, "bat_den_ngu") == 0) {
        digitalWrite(PIN_LED_NGU, HIGH);
        Serial.println("-> DA BAT DEN PHONG NGU (GPIO 32)");
    } 
    else if (strcmp(label, "tat_den_ngu") == 0) {
        digitalWrite(PIN_LED_NGU, LOW);
        Serial.println("-> DA TAT DEN PHONG NGU (GPIO 32)");
    }
}

void loop() {
    // Trong code thực tế với microphone INMP441:
    // 1. Thu âm qua I2S buffer
    // 2. Chuyển đổi tín hiệu vào signal_t
    // 3. Gọi ei_impulse_result_t result; run_classifier(&signal, &result, false);
    // 4. Lặp qua result.classification[i] và gọi handleVoiceCommand(label, value);

    // Bạn có thể test nhanh bằng cách gửi lệnh giả lập qua Serial Monitor:
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd == "1") handleVoiceCommand("bat_den_khach", 0.95);
        else if (cmd == "2") handleVoiceCommand("tat_den_khach", 0.95);
        else if (cmd == "3") handleVoiceCommand("bat_den_ngu", 0.95);
        else if (cmd == "4") handleVoiceCommand("tat_den_ngu", 0.95);
    }
    delay(50);
}
