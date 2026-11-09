#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <MFRC522.h>

// 1. INCLUDE THƯ VIỆN EDGE IMPULSE & AUDIO
#include <IoT_Pro_inferencing.h>
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include <driver/i2s.h>

// 2. INCLUDE ASSETS GIAO DIỆN WEB & GIỌNG NÓI PHẢN HỒI
#include "web_assets.h"
#include "audio_prompts.h"

// ==========================================
// 3. SƠ ĐỒ CHÂN PHẦN CỨNG TOÀN DIỆN (ESP32-S3)
// ==========================================
// --- Thiết bị đầu ra (LED & Motor Cửa) ---
#define PIN_LED_KHACH   5   // LED Xanh - Phòng khách
#define PIN_LED_BEP     18  // LED Vàng - Phòng bếp
#define PIN_SERVO_DOOR  4   // Motor Servo SG90 mở/đóng cửa

// --- Micro I2S INMP441 (Kênh I2S_NUM_0 - Thu âm) ---
#define PIN_I2S_SD      12
#define PIN_I2S_WS      15
#define PIN_I2S_SCK     17

// --- Loa I2S MAX98357A (Kênh I2S_NUM_1 - Phát tiếng) ---
#define PIN_SPK_BCLK    2   // Bit Clock
#define PIN_SPK_LRC     1   // Word Select (LRC)
#define PIN_SPK_DIN     42  // Data In

// --- Màn hình OLED I2C SSD1306 (4 chân) ---
#define PIN_OLED_SDA    8
#define PIN_OLED_SCL    9
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

// --- Cảm biến Môi trường & An ninh ---
#define PIN_DHT         16  // Cảm biến Nhiệt độ & Độ ẩm DHT22
#define DHTTYPE         DHT22
#define PIN_PIR         7   // Cảm biến chuyển động PIR
#define PIN_GAS         6   // Cảm biến khí Gas MQ-2 (ADC1_CH5)
#define PIN_VIB         14  // Cảm biến Rung SW-420

// --- Cảm biến Thẻ từ RFID RC522 (SPI) ---
#define PIN_RC522_SS    10  // SDA (SS / CS)
#define PIN_RC522_RST   21  // RST
#define PIN_RC522_SCK   13  // SCK
#define PIN_RC522_MOSI  11  // MOSI
#define PIN_RC522_MISO  47  // MISO

// Khởi tạo các đối tượng phần cứng
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(PIN_DHT, DHTTYPE);
MFRC522 rfid(PIN_RC522_SS, PIN_RC522_RST);

// ==========================================
// 4. THÔNG TIN WIFI & WEB SERVER
// ==========================================
const char* ssid     = "97 LVT TANG 4";
const char* password = "0965882295";

WebServer server(80);

// ==========================================
// 5. BIẾN TRẠNG THÁI HỆ THỐNG
// ==========================================
bool stateLedKhach   = false;
bool stateLedBep     = false;
bool stateDoor       = false; // true = Mở cửa (90 độ), false = Đóng cửa (0 độ)
bool micEnabled      = true;  // Quản lý bật/tắt micro
bool speakerEnabled  = true;  // Quản lý bật/tắt tiếng loa (Mute button)

// Dữ liệu cảm biến thời gian thực
float currentTemp       = 0.0;
float currentHumi       = 0.0;
bool statePIR           = false;
int gasRawValue         = 0;
bool gasAlert           = false;
bool stateVibration     = false;
String lastRfidCard     = "Chua quet the";
unsigned long doorAutoCloseTime = 0;

// Biến điều khiển màn hình & bus âm thanh
volatile bool oled_needs_update = true;
volatile bool is_speaker_playing = false;

// Thuật toán chống loạn lệnh Voice AI
#define CONFIDENCE_THRESHOLD    0.78f
#define VOICE_ENERGY_THRESHOLD  110
#define COOLDOWN_TIME_MS        2200

static unsigned long last_trigger_time = 0;

// Khai báo trước các hàm
void updateOLED();
void setDoor(bool open);

// ==========================================
// QUẢN LÝ MOTOR SERVO MỞ/ĐÓNG CỬA
// ==========================================
void initServo() {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(PIN_SERVO_DOOR, 50, 16);
#else
    ledcSetup(4, 50, 16);
    ledcAttachPin(PIN_SERVO_DOOR, 4);
#endif
    setDoor(false); // Mặc định đóng cửa
}

void setDoor(bool open) {
    stateDoor = open;
    // 0 độ (1.0ms) = 3276 duty | 90 độ (2.0ms) = 6553 duty (độ phân giải 16-bit ở 50Hz)
    int duty = open ? 6553 : 3276;
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(PIN_SERVO_DOOR, duty);
#else
    ledcWrite(4, duty);
#endif
    oled_needs_update = true;
    Serial.printf("-> [CUA] %s (Servo: %s)\n", open ? "DA MO CUA (90 do)" : "DA DONG CUA (0 do)", open ? "OPEN" : "CLOSED");
}

// ==========================================
// QUẢN LÝ PHÁT ÂM THANH GIỌNG NÓI (TASK RIÊNG)
// ==========================================
typedef enum {
    VOICE_NONE = 0,
    VOICE_BAT_KHACH,
    VOICE_TAT_KHACH,
    VOICE_BAT_BEP,
    VOICE_TAT_BEP
} voice_prompt_t;

static volatile voice_prompt_t pending_voice_prompt = VOICE_NONE;

static int i2s_speaker_init(uint32_t sampling_rate) {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = sampling_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = true
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_SPK_BCLK,
        .ws_io_num = PIN_SPK_LRC,
        .data_out_num = PIN_SPK_DIN,
        .data_in_num = -1
    };

    esp_err_t ret = i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
    if (ret != ESP_OK) return int(ret);

    ret = i2s_set_pin(I2S_NUM_1, &pin_config);
    if (ret != ESP_OK) return int(ret);

    ret = i2s_zero_dma_buffer(I2S_NUM_1);
    return int(ret);
}

static void speaker_playback_task(void* arg) {
    static int16_t stereo_chunk[512 * 2];

    while (true) {
        if (pending_voice_prompt != VOICE_NONE) {
            voice_prompt_t prompt = pending_voice_prompt;
            pending_voice_prompt = VOICE_NONE;

            // Nếu người dùng chọn TẮT LOA thì bỏ qua không phát tiếng
            if (!speakerEnabled) {
                continue;
            }

            const int16_t* audio_ptr = nullptr;
            size_t num_samples = 0;

            if (prompt == VOICE_BAT_KHACH) {
                audio_ptr = sound_bat_khach;
                num_samples = SOUND_BAT_KHACH_LEN;
            } else if (prompt == VOICE_TAT_KHACH) {
                audio_ptr = sound_tat_khach;
                num_samples = SOUND_TAT_KHACH_LEN;
            } else if (prompt == VOICE_BAT_BEP) {
                audio_ptr = sound_bat_bep;
                num_samples = SOUND_BAT_BEP_LEN;
            } else if (prompt == VOICE_TAT_BEP) {
                audio_ptr = sound_tat_bep;
                num_samples = SOUND_TAT_BEP_LEN;
            }

            if (audio_ptr != nullptr && num_samples > 0) {
                is_speaker_playing = true;
                i2s_zero_dma_buffer(I2S_NUM_1);

                size_t bytes_written = 0;
                const size_t chunk_samples = 256;
                size_t sent = 0;

                while (sent < num_samples) {
                    size_t to_send = (num_samples - sent > chunk_samples) ? chunk_samples : (num_samples - sent);
                    
                    for (size_t i = 0; i < to_send; i++) {
                        int16_t val = audio_ptr[sent + i];
                        stereo_chunk[i * 2]     = val;
                        stereo_chunk[i * 2 + 1] = val;
                    }

                    i2s_write(I2S_NUM_1, (const char*)stereo_chunk, to_send * 2 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
                    sent += to_send;
                }
                vTaskDelay(pdMS_TO_TICKS(350));
                i2s_zero_dma_buffer(I2S_NUM_1);
                is_speaker_playing = false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ==========================================
// QUẢN LÝ MÀN HÌNH OLED 4 CHÂN (SSD1306)
// ==========================================
void updateOLED() {
    if (!oled_needs_update) return;
    if (is_speaker_playing) return;
    oled_needs_update = false;

    display.clearDisplay();
    display.setTextWrap(false);

    // --- VÙNG 1: DẢI MÀU VÀNG PHÍA TRÊN (Y: 0 -> 15) ---
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 4);
    display.print("DO AN IOT - NHOM 8");
    display.drawFastHLine(0, 15, 128, SSD1306_WHITE);

    // --- VÙNG 2: THÔNG TIN MÔI TRƯỜNG & AN NINH (Y: 18 -> 42) ---
    display.setCursor(2, 20);
    display.printf("T:%.1fC  H:%.0f%%  Cua:%s", currentTemp, currentHumi, stateDoor ? "MO" : "DONG");

    display.setCursor(2, 31);
    display.printf("Gas:%d %s | Vib:%s", gasRawValue, gasAlert ? "NGUY HIEM" : "OK", stateVibration ? "RUNG!" : "OK");
    display.drawFastHLine(0, 42, 128, SSD1306_WHITE);

    // --- VÙNG 3: TRẠNG THÁI 2 ĐÈN DẠNG HỘP THỜI GIAN THỰC (Y: 45 -> 62) ---
    // Hộp Đèn Khách
    if (stateLedKhach) {
        display.fillRoundRect(2, 45, 60, 17, 3, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(6, 50);
        display.print("KHACH: ON");
    } else {
        display.drawRoundRect(2, 45, 60, 17, 3, SSD1306_WHITE);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(6, 50);
        display.print("KHACH: OFF");
    }

    // Hộp Đèn Bếp
    if (stateLedBep) {
        display.fillRoundRect(66, 45, 60, 17, 3, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(74, 50);
        display.print("BEP: ON");
    } else {
        display.drawRoundRect(66, 45, 60, 17, 3, SSD1306_WHITE);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(74, 50);
        display.print("BEP: OFF");
    }

    display.display();
}

// ==========================================
// HÀM ĐIỀU KHIỂN ĐỒNG BỘ THIẾT BỊ
// ==========================================
void triggerDevice(const String& device, bool state) {
    if (device == "khach") {
        if (stateLedKhach == state) return;
        stateLedKhach = state;
        digitalWrite(PIN_LED_KHACH, stateLedKhach ? HIGH : LOW);
        oled_needs_update = true;
        updateOLED();

        if (speakerEnabled) {
            pending_voice_prompt = stateLedKhach ? VOICE_BAT_KHACH : VOICE_TAT_KHACH;
        }
        Serial.printf("-> [ACTION] DA %s DEN PHONG KHACH\n", stateLedKhach ? "BAT" : "TAT");
    } 
    else if (device == "bep") {
        if (stateLedBep == state) return;
        stateLedBep = state;
        digitalWrite(PIN_LED_BEP, stateLedBep ? HIGH : LOW);
        oled_needs_update = true;
        updateOLED();

        if (speakerEnabled) {
            pending_voice_prompt = stateLedBep ? VOICE_BAT_BEP : VOICE_TAT_BEP;
        }
        Serial.printf("-> [ACTION] DA %s DEN PHONG BEP\n", stateLedBep ? "BAT" : "TAT");
    }
    else if (device == "door") {
        setDoor(state);
    }
}

// ==========================================
// CẤU TRÚC BUFFER ÂM THANH
// ==========================================
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

static inference_t inference;
static const uint32_t sample_buffer_size = 2048;
static int32_t raw_i2s_buffer[sample_buffer_size];
static signed short sampleBuffer[sample_buffer_size];
static bool record_status = true;

// ==========================================
// HÀM XỬ LÝ LỆNH TỪ AI GIỌNG NÓI
// ==========================================
void handleVoiceCommand(const char* label, float score) {
    if (!micEnabled) return;

    if (strcmp(label, "bat_den_khach") == 0) {
        triggerDevice("khach", true);
    } 
    else if (strcmp(label, "tat_den_khach") == 0) {
        triggerDevice("khach", false);
    }
    else if (strcmp(label, "bat_den_bep") == 0) {
        triggerDevice("bep", true);
    } 
    else if (strcmp(label, "tat_den_bep") == 0) {
        triggerDevice("bep", false);
    }
    else if (strcmp(label, "tat_micro") == 0) {
        micEnabled = false;
        oled_needs_update = true;
        updateOLED();
        Serial.println("-> [VOICE] DA TAT MICROPHONE! (Mo lai tren Web)");
    }
}

// ==========================================
// ROUTE WEB SERVER & REST API NÂNG CẤP
// ==========================================
void handleRoot() {
    server.send_P(200, "text/html", PAGE_INDEX);
}

void handleStyle() {
    server.send_P(200, "text/css", PAGE_STYLE);
}

void handleApiStatus() {
    String json = "{";
    json += "\"khach\":" + String(stateLedKhach ? "true" : "false") + ",";
    json += "\"bep\":" + String(stateLedBep ? "true" : "false") + ",";
    json += "\"door\":" + String(stateDoor ? "true" : "false") + ",";
    json += "\"spk\":" + String(speakerEnabled ? "true" : "false") + ",";
    json += "\"mic\":" + String(micEnabled ? "true" : "false") + ",";
    json += "\"temp\":" + String(currentTemp, 1) + ",";
    json += "\"humi\":" + String(currentHumi, 1) + ",";
    json += "\"pir\":" + String(statePIR ? "true" : "false") + ",";
    json += "\"gas\":" + String(gasRawValue) + ",";
    json += "\"gas_alert\":" + String(gasAlert ? "true" : "false") + ",";
    json += "\"vib\":" + String(stateVibration ? "true" : "false") + ",";
    json += "\"rfid\":\"" + lastRfidCard + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handleApiControl() {
    if (server.hasArg("device") && server.hasArg("state")) {
        String device = server.arg("device");
        bool state = (server.arg("state") == "1");

        if (device == "khach" || device == "bep" || device == "door") {
            triggerDevice(device, state);
        } 
        else if (device == "mic") {
            if (micEnabled != state) {
                micEnabled = state;
                oled_needs_update = true;
            }
        }
        else if (device == "spk") {
            speakerEnabled = state;
            Serial.printf("-> [WEB] LOA GIONG NOI: %s\n", speakerEnabled ? "BAT TIENG" : "TAT TIENG (MUTE)");
        }
    }
    handleApiStatus();
}

// ==========================================
// CÁC HÀM XỬ LÝ ÂM THANH I2S
// ==========================================
static void audio_inference_callback(uint32_t n_samples) {
    for (int i = 0; i < n_samples; i++) {
        inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];
        if (inference.buf_count >= inference.n_samples) {
            inference.buf_select ^= 1;
            inference.buf_count = 0;
            inference.buf_ready = 1;
        }
    }
}

static void capture_samples(void* arg) {
    const int32_t i2s_bytes_to_read = (uint32_t)arg;
    size_t bytes_read = 0;

    while (record_status) {
        if (!micEnabled || is_speaker_playing) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        i2s_read(I2S_NUM_0, (void*)raw_i2s_buffer, i2s_bytes_to_read, &bytes_read, 100);

        if (bytes_read > 0) {
            int samples_read = bytes_read / sizeof(int32_t);
            for (int x = 0; x < samples_read; x++) {
                int32_t val = (raw_i2s_buffer[x] >> 14) * 2;
                if (val > 32767) val = 32767;
                else if (val < -32768) val = -32768;
                sampleBuffer[x] = (int16_t)val;
            }
            if (record_status && !is_speaker_playing) {
                audio_inference_callback(samples_read);
            }
        }
    }
    vTaskDelete(NULL);
}

static int i2s_init(uint32_t sampling_rate) {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampling_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = -1
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_SCK,
        .ws_io_num = PIN_I2S_WS,
        .data_out_num = -1,
        .data_in_num = PIN_I2S_SD
    };

    esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (ret != ESP_OK) return int(ret);

    ret = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (ret != ESP_OK) return int(ret);

    ret = i2s_zero_dma_buffer(I2S_NUM_0);
    return int(ret);
}

static bool microphone_inference_start(uint32_t n_samples) {
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[0] == NULL) return false;

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[1] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    if (i2s_init(EI_CLASSIFIER_FREQUENCY)) {
        Serial.println("ERR: Loi khoi dong I2S Micro!");
    }

    delay(100);
    record_status = true;

    xTaskCreatePinnedToCore(capture_samples, "CaptureSamples", 1024 * 8, (void*)sample_buffer_size, 10, NULL, 1);
    return true;
}

static bool microphone_inference_record(void) {
    while (inference.buf_ready == 0) {
        server.handleClient();
        delay(5);
    }
    inference.buf_ready = 0;
    return true;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
    return 0;
}

// ==========================================
// HÀM KHỞI ĐỘNG HỆ THỐNG SETUP
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=======================================================");
    Serial.println(">>> KHOI DONG ESP32-S3 SMART HOME PRO (NHOM 8) <<<");
    Serial.println("=======================================================");

    // 1. Cấu hình GPIO đèn
    pinMode(PIN_LED_KHACH, OUTPUT);
    pinMode(PIN_LED_BEP, OUTPUT);
    digitalWrite(PIN_LED_KHACH, LOW);
    digitalWrite(PIN_LED_BEP, LOW);

    // 2. Cấu hình Motor Servo Cửa
    initServo();

    // 3. Cấu hình các cảm biến an ninh
    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_GAS, INPUT);
    pinMode(PIN_VIB, INPUT);
    dht.begin();
    Serial.println("-> Da khoi dong DHT22, PIR, Gas MQ-2 va Rung SW-420!");

    // 4. Khởi động Thẻ từ RFID RC522 (SPI)
    SPI.begin(PIN_RC522_SCK, PIN_RC522_MISO, PIN_RC522_MOSI, PIN_RC522_SS);
    rfid.PCD_Init();
    delay(50);
    Serial.println("-> Da khoi dong RFID RC522 tren bus SPI!");

    // 5. Khởi động OLED I2C SSD1306
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    Wire.setClock(100000);
    bool oled_ok = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    if (!oled_ok) {
        oled_ok = display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    }

    if (oled_ok) {
        display.clearDisplay();
        display.setTextWrap(false);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 16);
        display.println("SMART HOME PRO NHOM 8");
        display.setCursor(12, 34);
        display.println("DANG KET NOI WIFI...");
        display.display();
        Serial.println("-> OLED SSD1306 da bat sang!");
    } else {
        Serial.println("WARN: OLED chua phan hoi (kiem tra day SDA=8, SCL=9)!");
    }

    // 6. Khởi động Loa I2S MAX98357A
    if (i2s_speaker_init(AUDIO_SAMPLE_RATE) == 0) {
        xTaskCreatePinnedToCore(speaker_playback_task, "SpeakerTask", 1024 * 4, NULL, 5, NULL, 0);
        Serial.println("-> Loa MAX98357A da san sang phan hoi giong noi!");
    } else {
        Serial.println("ERR: Loi khoi dong I2S Loa!");
    }

    // 7. Kết nối WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.printf("Dang ket noi WiFi: %s ...\n", ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n------------------------------------------------");
    Serial.println("-> KET NOI WIFI THANH CONG!");
    Serial.print("-> MO TRINH DUYET TRUY CAP DASHBOARD: http://");
    Serial.println(WiFi.localIP());
    Serial.println("------------------------------------------------");

    server.on("/", handleRoot);
    server.on("/style.css", handleStyle);
    server.on("/api/status", handleApiStatus);
    server.on("/api/control", handleApiControl);
    server.begin();

    // Vẽ giao diện tĩnh lên OLED
    oled_needs_update = true;
    updateOLED();

    // 8. Khởi động Edge Impulse Voice AI
    run_classifier_init();
    if (!microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE)) {
        Serial.println("ERR: Khong the cap phat bo nho buffer am thanh!");
        return;
    }
    Serial.println("-> Edge Impulse Voice AI da san sang!");
}

// ==========================================
// VÒNG LẶP CHÍNH LOOP
// ==========================================
void loop() {
    server.handleClient();
    updateOLED();

    unsigned long currentMillis = millis();

    // 1. TỰ ĐỘNG ĐÓNG CỬA SAU 5 GIÂY NẾU ĐANG MỞ
    if (stateDoor && doorAutoCloseTime > 0 && currentMillis >= doorAutoCloseTime) {
        setDoor(false);
        doorAutoCloseTime = 0;
        Serial.println("-> [AUTO] Da tu dong dong cua sau 5 giay!");
    }

    // 2. ĐỌC THẺ TỪ RFID RC522
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        String uidStr = "";
        for (byte i = 0; i < rfid.uid.size; i++) {
            char buf[4];
            sprintf(buf, "%02X", rfid.uid.uidByte[i]);
            uidStr += buf;
            if (i < rfid.uid.size - 1) uidStr += " ";
        }
        lastRfidCard = uidStr;
        Serial.printf("\n>>> [RFID] Quet thanh cong the UID: %s -> MO CUA 5S!\n", lastRfidCard.c_str());

        // Mở cửa và hẹn giờ đóng sau 5 giây
        setDoor(true);
        doorAutoCloseTime = millis() + 5000;

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }

    // 3. ĐỌC CÁC CẢM BIẾN AN NINH & MÔI TRƯỜNG (Mỗi 500ms)
    static unsigned long last_sensor_read = 0;
    if (currentMillis - last_sensor_read >= 500) {
        last_sensor_read = currentMillis;

        // Đọc PIR
        statePIR = (digitalRead(PIN_PIR) == HIGH);

        // Đọc Khí Gas
        gasRawValue = analogRead(PIN_GAS);
        gasAlert = (gasRawValue > 1800);

        // Đọc Cảm biến Rung
        stateVibration = (digitalRead(PIN_VIB) == HIGH);

        // Đọc DHT22 mỗi 2.5 giây
        static unsigned long last_dht_read = 0;
        if (currentMillis - last_dht_read >= 2500) {
            last_dht_read = currentMillis;
            float t = dht.readTemperature();
            float h = dht.readHumidity();
            if (!isnan(t)) currentTemp = t;
            if (!isnan(h)) currentHumi = h;
            oled_needs_update = true;
        }
    }

    // 4. XỬ LÝ NHẬN DIỆN GIỌNG NÓI VOICE AI
    if (micEnabled && !is_speaker_playing) {
        bool m = microphone_inference_record();
        if (m) {
            int64_t energy_sum = 0;
            int current_buf = inference.buf_select ^ 1;
            for (size_t i = 0; i < EI_CLASSIFIER_SLICE_SIZE; i++) {
                int16_t val = inference.buffers[current_buf][i];
                energy_sum += abs(val);
            }
            int avg_energy = energy_sum / EI_CLASSIFIER_SLICE_SIZE;

            if (avg_energy < VOICE_ENERGY_THRESHOLD) {
                return;
            }

            signal_t signal;
            signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
            signal.get_data = &microphone_audio_signal_get_data;
            ei_impulse_result_t result = {0};

            EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, false);
            if (r == EI_IMPULSE_OK) {
                float score_bat_bep = 0.0, score_bat_khach = 0.0;
                float score_tat_bep = 0.0, score_tat_khach = 0.0;

                float max_score = 0.0;
                float second_score = 0.0;
                const char* max_label = "";
                const char* second_label = "";

                for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                    const char* lbl = result.classification[ix].label;
                    float val = result.classification[ix].value;

                    if (strcmp(lbl, "bat_den_bep") == 0) score_bat_bep = val;
                    else if (strcmp(lbl, "bat_den_khach") == 0) score_bat_khach = val;
                    else if (strcmp(lbl, "tat_den_bep") == 0) score_tat_bep = val;
                    else if (strcmp(lbl, "tat_den_khach") == 0) score_tat_khach = val;

                    if (val > max_score) {
                        second_score = max_score;
                        second_label = max_label;
                        max_score = val;
                        max_label = lbl;
                    } else if (val > second_score) {
                        second_score = val;
                        second_label = lbl;
                    }
                }

                if (max_score >= 0.50f && strcmp(max_label, "noise") != 0 && strcmp(max_label, "unknow") != 0) {
                    Serial.printf("[AI Nghe] Top 1: %-14s (%.0f%%) | Top 2: %-14s (%.0f%%)\n", 
                                  max_label, max_score * 100.0f, second_label, second_score * 100.0f);
                }

                bool valid_command = false;

                if (strcmp(max_label, "tat_den_bep") == 0) {
                    if (score_tat_bep >= CONFIDENCE_THRESHOLD && 
                        score_tat_khach <= 0.32f && 
                        (score_tat_bep - score_tat_khach) >= 0.25f) {
                        valid_command = true;
                    }
                }
                else if (strcmp(max_label, "tat_den_khach") == 0) {
                    if (score_tat_khach >= CONFIDENCE_THRESHOLD && 
                        score_tat_bep <= 0.32f && 
                        (score_tat_khach - score_tat_bep) >= 0.25f) {
                        valid_command = true;
                    }
                }
                else if (strcmp(max_label, "bat_den_bep") == 0) {
                    if (score_bat_bep >= CONFIDENCE_THRESHOLD && 
                        score_bat_khach <= 0.32f && 
                        (score_bat_bep - score_bat_khach) >= 0.25f) {
                        valid_command = true;
                    }
                }
                else if (strcmp(max_label, "bat_den_khach") == 0) {
                    if (score_bat_khach >= CONFIDENCE_THRESHOLD && 
                        score_bat_bep <= 0.32f && 
                        score_tat_khach <= 0.35f &&
                        (score_bat_khach - score_bat_bep) >= 0.25f) {
                        valid_command = true;
                    }
                }

                if (valid_command && (millis() - last_trigger_time) > COOLDOWN_TIME_MS) {
                    Serial.printf(">>> [KICH HOAT] %s (%.0f%%)\n", max_label, max_score * 100.0f);
                    handleVoiceCommand(max_label, max_score);
                    last_trigger_time = millis();
                }
            }
        }
    } else {
        delay(30);
    }
}
