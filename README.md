# 🏠 HỆ THỐNG SMART HOME IOT NÂNG CAO - NHÓM 8

> **Đồ án môn học:** Internet of Things Nâng Cao  
> **Giảng viên hướng dẫn:** ThS. Trịnh Đình Yến  
> **Nhóm thực hiện:** Nhóm 8  
> **Nền tảng phần cứng:** ESP32-S3 DevKitC-1 (Xtensa Dual-Core 240MHz, Vector AI)

---

## 🌟 Giới Thiệu Tổng Quan

Hệ thống **Smart Home IoT Nâng Cao** là giải pháp nhà thông minh tích hợp trí tuệ nhân tạo tại biên (**Edge AI / TinyML**) hoạt động **hoàn toàn Offline không phụ thuộc vào Internet**, kết hợp công nghệ xác thực an ninh thẻ từ **RFID RC522**, tự động đóng mở cửa bằng **Motor Servo SG90**, hệ thống đa cảm biến môi trường và bảng điều khiển **Web Dashboard Dark Mode** thời gian thực.

### ✨ Các Tính Năng Cốt Lõi:
1. **Nhận diện giọng nói Offline (Edge AI Voice):** Nhận diện trực tiếp khẩu lệnh tiếng Việt (*"Bật đèn phòng khách"*, *"Tắt đèn phòng khách"*, *"Bật đèn phòng bếp"*, *"Tắt đèn phòng bếp"*, *"Tắt micro"*) qua Micro số I2S INMP441, độ trễ phản hồi < 50ms, bảo mật riêng tư tuyệt đối.
2. **Phản hồi âm thanh tiếng Việt tự nhiên:** Phát câu nói thông báo qua Mạch khuếch đại Class-D I2S MAX98357A (kèm công tắc Mute tắt tiếng trên Web).
3. **Kiểm soát cửa tự động RFID RC522:** Quẹt thẻ từ tần số 13.56MHz để mở cửa tự động bằng Servo SG90 (góc 90°), tích hợp bộ đếm tự động đóng cửa an toàn sau 5 giây.
4. **Hệ thống cảm biến an toàn & môi trường:**
   - **DHT22:** Giám sát liên tục nhiệt độ (°C) và độ ẩm (%).
   - **PIR:** Cảm biến hồng ngoại phát hiện người chuyển động trong phòng.
   - **MQ-2:** Cảm biến đo nồng độ khí Gas, phát hiện rò rỉ khí gas/khói và báo động.
   - **SW-420:** Cảm biến rung động phát hiện cạy phá cửa, va đập.
5. **Màn hình OLED SSD1306 0.96":** Hiển thị 3 vùng trực quan: Dải vàng Nhóm 8, thông số mạng & AI, hai hộp trạng thái đèn thời gian thực.
6. **Web Dashboard Dark Mode:** Giao diện điều khiển từ xa responsive trên điện thoại & máy tính, đồng bộ RESTful API JSON bất đồng bộ mỗi 1.5s.

---

## 📌 Sơ Đồ Kết Nối Chân Phần Cứng (ESP32-S3)

| Thiết Bị / Module | Chân Tín Hiệu | Chân Nối ESP32-S3 | Nguồn Cấp |
| :--- | :--- | :--- | :--- |
| **LED Phòng Khách** | Anode (+) | **GPIO 5** | 3.3V |
| **LED Phòng Bếp** | Anode (+) | **GPIO 18** | 3.3V |
| **Motor Servo Cửa (SG90)** | Tín hiệu PWM (Vàng/Cam) | **GPIO 4** | 5V & GND |
| **Micro I2S INMP441** | `SD`, `WS`, `SCK` | **GPIO 12, 15, 17** | 3.3V & GND |
| **Loa I2S MAX98357A** | `DIN`, `BCLK`, `LRC` | **GPIO 42, 2, 1** | 5V & GND |
| **Màn hình OLED SSD1306** | `SDA`, `SCL` | **GPIO 8, GPIO 9** | 3.3V & GND |
| **Module Thẻ từ RFID RC522** | `SS`, `SCK`, `MOSI`, `MISO`, `RST` | **GPIO 10, 13, 11, 47, 21** | **3.3V (Bắt buộc)** |
| **Cảm biến nhiệt ẩm DHT22** | `Data` | **GPIO 16** | 3.3V / 5V |
| **Cảm biến chuyển động PIR** | `OUT` | **GPIO 7** | 5V & GND |
| **Cảm biến Khí Gas MQ-2** | `A0` (Analog ADC1) | **GPIO 6** | 5V & GND |
| **Cảm biến Rung SW-420** | `D0` (Digital) | **GPIO 14** | 3.3V / 5V |

---

## 📂 Cấu Trúc Thư Mục

```text
IoTPro/
├── main_firmware/               # Mã nguồn chính nạp vào ESP32-S3
│   ├── src/main.cpp             # Toàn bộ logic điều khiển, FreeRTOS, Voice AI & Web
│   ├── include/
│   │   ├── web_assets.h         # Giao diện Web HTML5/CSS3 Dark Mode & API
│   │   └── audio_prompts.h      # Dữ liệu âm thanh phản hồi tiếng Việt 24kHz PCM
│   └── platformio.ini           # Cấu hình biên dịch PlatformIO & thư viện
├── modules/                     # Các module mã nguồn kiểm tra từng phần cứng độc lập
│   ├── access_control/          # Test RFID & Servo
│   ├── display_ui/              # Test OLED
│   ├── environment_safety/      # Test DHT22, Gas, PIR, Rung
│   └── voice_recognition/       # Test Micro & Loa I2S
├── docs/                        # Tài liệu báo cáo hoàn chỉnh
│   ├── Bao_Cao_Do_An_IoT_Nang_Cao_Nhom_8.docx   # Báo cáo học thuật chuẩn A4
│   ├── Bao_Cao_Do_An_IoT_Nang_Cao_Nhom_8.pptx   # Slide thuyết trình 19 trang
│   ├── Flowchart.drawio                         # Lưu đồ thuật toán
│   └── SoDoNguyenLi.fzz                         # Sơ đồ mạch Fritzing
└── README.md                    # Tài liệu giới thiệu dự án
```

---

## 🚀 Hướng Dẫn Biên Dịch & Nạp Code

1. Cài đặt **Visual Studio Code** và tiện ích mở rộng **PlatformIO IDE**.
2. Mở thư mục `IoTPro/main_firmware` trong VS Code.
3. Cắm cáp USB Type-C kết nối ESP32-S3 với máy tính (Cổng COM tự động nhận diện).
4. Nhấn biểu tượng mũi tên **Upload (`→`)** ở thanh dưới VS Code để nạp code.
5. Mở **Serial Monitor** (tốc độ `115200` baud) để xem địa chỉ IP Wi-Fi và truy cập Web Dashboard trên trình duyệt: `http://<IP_ESP32>`.
