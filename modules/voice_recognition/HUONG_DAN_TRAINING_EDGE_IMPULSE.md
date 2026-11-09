# HƯỚNG DẪN CHI TIẾT TRAINING MÔ HÌNH NHẬN DIỆN GIỌNG NÓI VỚI EDGE IMPULSE
**Phân hệ**: Điều khiển giọng nói Offline/Edge AI (Voice Control)
**Mục tiêu**: Nhận diện các câu lệnh:
1. `bat_den_khach` (Bật đèn phòng khách)
2. `tat_den_khach` (Tắt đèn phòng khách)
3. `bat_den_ngu` (Bật đèn phòng ngủ)
4. `tat_den_ngu` (Tắt đèn phòng ngủ)
5. `noise` (Tiếng ồn môi trường, tiếng quạt, tạp âm)
6. `unknown` (Các từ nói khác không phải lệnh)

---

## BƯỚC 1: THIẾT BỊ PHẦN CỨNG THU ÂM (HARDWARE MICROPHONE)
Để ESP32 nghe được giọng nói thực tế, bạn cần gắn thêm 1 module Microphone số:
- **Khuyên dùng số 1**: Module **INMP441** (Chuẩn I2S kỹ thuật số, lọc nhiễu cực tốt, giá rẻ).
  - Sơ đồ chân nối với ESP32:
    - `VDD` -> 3.3V
    - `GND` -> GND
    - `SD` (Serial Data) -> GPIO 32
    - `WS` (Word Select / L/R Clock) -> GPIO 25
    - `SCK` (Bit Clock) -> GPIO 33
    - `L/R` -> GND (kênh Left)
- **Phương án thu âm mẫu training**: Bạn có thể dùng chính chiếc điện thoại Smartphone hoặc Laptop mic để thu âm gửi lên Edge Impulse mà chưa cần cắm mic vào ESP32 ngay!

---

## BƯỚC 2: TẠO PROJECT TRÊN EDGE IMPULSE
1. Truy cập: [https://edgeimpulse.com](https://edgeimpulse.com) và đăng nhập (hoặc đăng ký tài khoản miễn phí).
2. Bấm **Create New Project**, đặt tên: `ESP32-SmartHome-VoiceControl`.
3. Chọn loại dự án: **Audio** (Recognize sounds from audio).

---

## BƯỚC 3: THU THẬP TẬP DỮ LIỆU (DATA ACQUISITION)
Mô hình AI muốn chính xác thì tập dữ liệu âm thanh phải đa dạng:

### 1. Cách thu âm nhanh nhất bằng Điện thoại / Web:
1. Vào tab **Data acquisition** trên Edge Impulse.
2. Bấm **Connect a device** -> Chọn **Show QR code**.
3. Dùng điện thoại quét mã QR -> Trình duyệt điện thoại mở ra công cụ ghi âm trực tiếp đồng bộ với tài khoản.
4. Chọn Sample length: `1000 ms` (1 giây cho mỗi từ lệnh).

### 2. Số lượng mẫu cần thu (Dataset Guidelines):
- `bat_den_khach`: ~30 - 50 mẫu (Nói các tốc độ: nhanh, chậm, trầm, bổng, cách xa 30cm, 1m).
- `tat_den_khach`: ~30 - 50 mẫu.
- `bat_den_ngu`: ~30 - 50 mẫu.
- `tat_den_ngu`: ~30 - 50 mẫu.
- `noise`: ~50 mẫu (Tiếng thở, tiếng gõ bàn, tiếng quạt chạy, tiếng im lặng).
- `unknown`: ~50 mẫu (Nói các từ khác như: 'alo', 'xin chào', 'thời tiết', 'chào bạn', v.v.).

*Mẹo*: Edge Impulse tự động chia tỉ lệ 80% Train / 20% Test.

---

## BƯỚC 4: THIẾT KẾ IMPULSE (CREATE IMPULSE)
1. Vào tab **Impulse design** -> **Create impulse**.
2. **Time series data**:
   - Window size: `1000 ms` (1 giây).
   - Window increase: `500 ms`.
   - Frequency: `16000 Hz` (Chuẩn nhận diện giọng nói 16kHz).
3. **Processing block**: Bấm *Add a processing block* -> Chọn **Audio (MFCC)**.
   *(MFCC: Mel-Frequency Cepstral Coefficients - thuật toán trích xuất đặc trưng âm thanh giọng nói con người tối ưu nhất).*
4. **Learning block**: Bấm *Add a learning block* -> Chọn **Classification (Keras)**.
5. Bấm **Save Impulse**.

---

## BƯỚC 5: TRÍCH XUẤT ĐẶC TRƯNG (MFCC PARAMETERS)
1. Bấm vào tab **MFCC** ở menu bên trái.
2. Để mặc định các thông số:
   - Number of coefficients: 13
   - Frame length: 0.02s
   - Frame stride: 0.01s
3. Bấm **Save parameters** -> Bấm **Generate features**.
4. Quan sát đồ thị 3D Feature Explorer: Nếu các cụm màu (các nhãn lệnh) tách rời nhau rõ ràng là dữ liệu rất tốt!

---

## BƯỚC 6: HUẤN LUYỆN MÔ HÌNH (TRAIN MODEL)
1. Bấm vào tab **Classifier** ở menu bên trái.
2. Cấu hình Hyperparameters:
   - Number of training cycles (Epochs): 50 - 100
   - Learning rate: 0.005
3. Network architecture:
   - Chọn mạng 1D Convolutional hoặc 2D Conv (Edge Impulse gợi ý sẵn kiến trúc siêu nhẹ cho Audio).
4. Bấm **Start training**.
5. Quan sát kết quả:
   - **Accuracy**: Đạt từ 90% - 98% là đạt chuẩn.
   - **Confusion matrix**: Kiểm tra xem nhãn nào bị nhận diện nhầm để bổ sung thêm dữ liệu.
   - **On-device performance**: Kiểm tra thời gian suy luận (Inference time ~ 30-50ms trên ESP32) và RAM sử dụng (~20-30KB RAM, cực kỳ nhẹ cho ESP32).

---

## BƯỚC 7: KIỂM THỬ (MODEL TESTING)
1. Vào tab **Model testing** -> Bấm **Classify all**.
2. Đánh giá xem trên tập dữ liệu Test chưa từng thấy, mô hình đạt độ chính xác bao nhiêu %.

---

## BƯỚC 8: XUẤT THƯ VIỆN C++ CHO ESP32 (DEPLOYMENT)
1. Vào tab **Deployment**.
2. Tìm mục **Arduino library** -> Bấm vào để chọn.
3. Ở mục Optimization: Chọn **Quantized (int8)** để giảm kích thước model xuống 4 lần mà độ chính xác gần như giữ nguyên.
4. Bấm **Build** ở cuối trang.
5. Trình duyệt sẽ tải về 1 file `.zip` (ví dụ: `ei-esp32-smarthome-voicecontrol-arduino-1.0.0.zip`).

---

## BƯỚC 9: TÍCH HỢP VÀO DỰ ÁN LỚN (INTEGRATION)
1. Trong Arduino IDE: Chọn `Sketch` -> `Include Library` -> `Add .ZIP Library...` và chọn file zip vừa tải.
2. Trong dự án của bạn (thư mục `main_firmware`), bạn chỉ cần gọi hàm nhận diện `run_classifier()` trong một task FreeRTOS riêng để khi có lệnh giọng nói, cờ điều khiển đèn sẽ lập tức được kích hoạt.
