#ifndef WEB_ASSETS_H
#define WEB_ASSETS_H

// GIAO DIỆN HTML HIỆN ĐẠI (SMART HOME PRO DASHBOARD - NHÓM 8)
const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-S3 Smart Home Pro</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <header>
            <div class="logo-icon">🏠</div>
            <h1>SMART HOME PRO</h1>
            <p class="subtitle">Bảng Điều Khiển Nhà Thông Minh - Nhóm 8</p>
        </header>

        <!-- KHU VỰC THIẾT BỊ ĐIỀU KHIỂN -->
        <h2 class="section-title">⚡ ĐIỀU KHIỂN THIẾT BỊ</h2>
        <main class="dashboard">
            <!-- Đèn Phòng Khách -->
            <div class="card" id="card-khach">
                <div class="card-icon" id="icon-khach">💡</div>
                <div class="card-info">
                    <h3>Đèn Phòng Khách</h3>
                    <p class="status-text" id="status-khach">Đang tắt</p>
                </div>
                <div class="switch-wrapper">
                    <label class="switch">
                        <input type="checkbox" id="toggle-khach" onchange="toggleDevice('khach', this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
            </div>

            <!-- Đèn Phòng Bếp -->
            <div class="card" id="card-bep">
                <div class="card-icon" id="icon-bep">🍳</div>
                <div class="card-info">
                    <h3>Đèn Phòng Bếp</h3>
                    <p class="status-text" id="status-bep">Đang tắt</p>
                </div>
                <div class="switch-wrapper">
                    <label class="switch">
                        <input type="checkbox" id="toggle-bep" onchange="toggleDevice('bep', this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
            </div>

            <!-- Cửa Thông Minh (Motor Servo) -->
            <div class="card" id="card-door">
                <div class="card-icon" id="icon-door">🚪</div>
                <div class="card-info">
                    <h3>Cửa Thông Minh</h3>
                    <p class="status-text" id="status-door">Đang đóng</p>
                </div>
                <div class="switch-wrapper">
                    <label class="switch switch-door">
                        <input type="checkbox" id="toggle-door" onchange="toggleDevice('door', this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
            </div>

            <!-- Loa Phản Hồi Giọng Nói (Mute/Unmute) -->
            <div class="card" id="card-spk">
                <div class="card-icon" id="icon-spk">🔊</div>
                <div class="card-info">
                    <h3>Loa Giọng Nói</h3>
                    <p class="status-text" id="status-spk">Đang bật tiếng</p>
                </div>
                <div class="switch-wrapper">
                    <label class="switch switch-spk">
                        <input type="checkbox" id="toggle-spk" checked onchange="toggleDevice('spk', this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
            </div>

            <!-- Microphone Voice AI -->
            <div class="card" id="card-mic">
                <div class="card-icon" id="icon-mic">🎙️</div>
                <div class="card-info">
                    <h3>Microphone AI</h3>
                    <p class="status-text" id="status-mic">Đang lắng nghe</p>
                </div>
                <div class="switch-wrapper">
                    <label class="switch switch-mic">
                        <input type="checkbox" id="toggle-mic" checked onchange="toggleDevice('mic', this.checked)">
                        <span class="slider"></span>
                    </label>
                </div>
            </div>
        </main>

        <!-- KHU VỰC CẢM BIẾN MÔI TRƯỜNG & AN NINH -->
        <h2 class="section-title">📊 GIÁM SÁT CẢM BIẾN</h2>
        <div class="sensors-grid">
            <!-- Nhiệt độ DHT22 -->
            <div class="sensor-box">
                <div class="sensor-icon">🌡️</div>
                <div class="sensor-data">
                    <span class="sensor-val" id="val-temp">--</span><span class="sensor-unit">°C</span>
                </div>
                <div class="sensor-label">Nhiệt độ (DHT22)</div>
            </div>

            <!-- Độ ẩm DHT22 -->
            <div class="sensor-box">
                <div class="sensor-icon">💧</div>
                <div class="sensor-data">
                    <span class="sensor-val" id="val-humi">--</span><span class="sensor-unit">%</span>
                </div>
                <div class="sensor-label">Độ ẩm (DHT22)</div>
            </div>

            <!-- Chuyển động PIR -->
            <div class="sensor-box" id="box-pir">
                <div class="sensor-icon" id="icon-pir">🚶</div>
                <div class="sensor-status" id="status-pir">Không có người</div>
                <div class="sensor-label">Chuyển động PIR</div>
            </div>

            <!-- Cảm biến Khí Gas -->
            <div class="sensor-box" id="box-gas">
                <div class="sensor-icon" id="icon-gas">⚠️</div>
                <div class="sensor-status" id="status-gas">An toàn</div>
                <div class="sensor-label">Khí Gas (MQ-2)</div>
            </div>

            <!-- Cảm biến Rung -->
            <div class="sensor-box" id="box-vib">
                <div class="sensor-icon" id="icon-vib">📳</div>
                <div class="sensor-status" id="status-vib">Bình thường</div>
                <div class="sensor-label">Cảm biến Rung</div>
            </div>

            <!-- Thẻ từ RFID RC522 -->
            <div class="sensor-box rfid-box">
                <div class="sensor-icon">💳</div>
                <div class="sensor-status rfid-uid" id="val-rfid">Chưa quẹt thẻ</div>
                <div class="sensor-label">Thẻ từ RFID (RC522)</div>
            </div>
        </div>

        <footer>
            <div class="network-badge">
                <span class="dot-online"></span> Đã kết nối Wi-Fi ESP32-S3
            </div>
            <p class="tech-stack">Voice AI • RFID RC522 • DHT22 • PIR • Gas • Rung</p>
        </footer>
    </div>

    <script>
        function toggleDevice(device, state) {
            const val = state ? 1 : 0;
            fetch(`/api/control?device=${device}&state=${val}`)
                .then(res => res.json())
                .then(data => updateUI(data))
                .catch(err => console.error(err));
        }

        function updateUI(data) {
            // Đèn khách
            document.getElementById('toggle-khach').checked = data.khach;
            document.getElementById('status-khach').innerText = data.khach ? "Đang bật" : "Đang tắt";
            document.getElementById('card-khach').className = data.khach ? "card active-khach" : "card";

            // Đèn bếp
            document.getElementById('toggle-bep').checked = data.bep;
            document.getElementById('status-bep').innerText = data.bep ? "Đang bật" : "Đang tắt";
            document.getElementById('card-bep').className = data.bep ? "card active-bep" : "card";

            // Cửa Servo
            document.getElementById('toggle-door').checked = data.door;
            document.getElementById('status-door').innerText = data.door ? "Đang mở cửa (90°)" : "Đang đóng cửa";
            document.getElementById('card-door').className = data.door ? "card active-door" : "card";

            // Loa
            document.getElementById('toggle-spk').checked = data.spk;
            document.getElementById('status-spk').innerText = data.spk ? "Đang bật tiếng" : "Đã tắt loa (Mute)";
            document.getElementById('card-spk').className = data.spk ? "card active-spk" : "card";

            // Mic
            document.getElementById('toggle-mic').checked = data.mic;
            document.getElementById('status-mic').innerText = data.mic ? "Đang lắng nghe" : "Đã tắt mic";
            document.getElementById('card-mic').className = data.mic ? "card active-mic" : "card";

            // DHT22
            document.getElementById('val-temp').innerText = (data.temp !== undefined && data.temp > -50) ? data.temp.toFixed(1) : "--";
            document.getElementById('val-humi').innerText = (data.humi !== undefined && data.humi >= 0) ? data.humi.toFixed(1) : "--";

            // PIR
            const boxPir = document.getElementById('box-pir');
            const txtPir = document.getElementById('status-pir');
            if (data.pir) {
                boxPir.className = "sensor-box alert-box";
                txtPir.innerText = "Phát hiện người!";
            } else {
                boxPir.className = "sensor-box";
                txtPir.innerText = "Không có người";
            }

            // Gas
            const boxGas = document.getElementById('box-gas');
            const txtGas = document.getElementById('status-gas');
            if (data.gas_alert) {
                boxGas.className = "sensor-box danger-box";
                txtGas.innerText = "CẢNH BÁO GAS RÒ RỈ!";
            } else {
                boxGas.className = "sensor-box";
                txtGas.innerText = "An toàn (" + data.gas + ")";
            }

            // Rung
            const boxVib = document.getElementById('box-vib');
            const txtVib = document.getElementById('status-vib');
            if (data.vib) {
                boxVib.className = "sensor-box alert-box";
                txtVib.innerText = "PHÁT HIỆN RUNG!";
            } else {
                boxVib.className = "sensor-box";
                txtVib.innerText = "Bình thường";
            }

            // RFID
            if (data.rfid && data.rfid.length > 0) {
                document.getElementById('val-rfid').innerText = data.rfid;
            }
        }

        function syncStatus() {
            fetch('/api/status')
                .then(res => res.json())
                .then(data => updateUI(data))
                .catch(e => {});
        }
        setInterval(syncStatus, 1500);
        window.onload = syncStatus;
    </script>
</body>
</html>
)rawliteral";

// FILE CSS (DARK MODE THEME HIỆN ĐẠI, HIỂN THỊ CẢM BIẾN DẠNG LƯỚI CARD)
const char PAGE_STYLE[] PROGMEM = R"rawliteral(
:root {
    --bg-color: #0f172a;
    --card-bg: #1e293b;
    --card-hover: #334155;
    --text-primary: #f8fafc;
    --text-secondary: #94a3b8;
    --accent-blue: #38bdf8;
    --accent-green: #22c55e;
    --accent-yellow: #eab308;
    --accent-red: #ef4444;
}
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
}
body {
    background-color: var(--bg-color);
    color: var(--text-primary);
    min-height: 100vh;
    display: flex;
    justify-content: center;
    align-items: center;
    padding: 20px 10px;
}
.container {
    width: 100%;
    max-width: 480px;
}
header {
    text-align: center;
    margin-bottom: 20px;
}
.logo-icon {
    font-size: 36px;
    margin-bottom: 6px;
}
h1 {
    font-size: 22px;
    letter-spacing: 2px;
    font-weight: 700;
}
.subtitle {
    color: var(--text-secondary);
    font-size: 13px;
    margin-top: 4px;
}
.section-title {
    font-size: 14px;
    color: var(--text-secondary);
    letter-spacing: 1px;
    margin: 18px 0 10px 4px;
    text-transform: uppercase;
}
.dashboard {
    display: flex;
    flex-direction: column;
    gap: 12px;
}
.card {
    background-color: var(--card-bg);
    border-radius: 14px;
    padding: 14px 16px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    border: 1px solid rgba(255, 255, 255, 0.05);
    transition: all 0.25s ease;
}
.card-icon {
    font-size: 26px;
    margin-right: 14px;
}
.card-info {
    flex-grow: 1;
}
.card-info h3 {
    font-size: 16px;
    font-weight: 600;
}
.status-text {
    font-size: 12px;
    color: var(--text-secondary);
    margin-top: 2px;
}
.card.active-khach {
    border-color: rgba(34, 197, 94, 0.4);
    box-shadow: 0 0 15px rgba(34, 197, 94, 0.15);
}
.card.active-bep {
    border-color: rgba(234, 179, 8, 0.4);
    box-shadow: 0 0 15px rgba(234, 179, 8, 0.15);
}
.card.active-door {
    border-color: rgba(56, 189, 248, 0.4);
    box-shadow: 0 0 15px rgba(56, 189, 248, 0.2);
}
.card.active-spk {
    border-color: rgba(168, 85, 247, 0.4);
}
.card.active-mic {
    border-color: rgba(56, 189, 248, 0.3);
}

/* SWITCH TOGGLE */
.switch {
    position: relative;
    display: inline-block;
    width: 48px;
    height: 26px;
}
.switch input {
    opacity: 0;
    width: 0;
    height: 0;
}
.slider {
    position: absolute;
    cursor: pointer;
    top: 0; left: 0; right: 0; bottom: 0;
    background-color: #334155;
    transition: .3s;
    border-radius: 26px;
}
.slider:before {
    position: absolute;
    content: "";
    height: 20px;
    width: 20px;
    left: 3px;
    bottom: 3px;
    background-color: white;
    transition: .3s;
    border-radius: 50%;
}
input:checked + .slider {
    background-color: var(--accent-green);
}
.switch-door input:checked + .slider {
    background-color: var(--accent-blue);
}
.switch-spk input:checked + .slider {
    background-color: #a855f7;
}
.switch-mic input:checked + .slider {
    background-color: var(--accent-blue);
}
input:checked + .slider:before {
    transform: translateX(22px);
}

/* SENSORS GRID */
.sensors-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
}
.sensor-box {
    background-color: var(--card-bg);
    border-radius: 14px;
    padding: 14px;
    text-align: center;
    border: 1px solid rgba(255, 255, 255, 0.05);
    transition: all 0.25s ease;
}
.rfid-box {
    grid-column: span 2;
}
.sensor-icon {
    font-size: 26px;
    margin-bottom: 4px;
}
.sensor-data {
    font-size: 22px;
    font-weight: 700;
    color: var(--accent-blue);
}
.sensor-unit {
    font-size: 14px;
    font-weight: 400;
    color: var(--text-secondary);
    margin-left: 2px;
}
.sensor-label {
    font-size: 11px;
    color: var(--text-secondary);
    margin-top: 4px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}
.sensor-status {
    font-size: 14px;
    font-weight: 600;
    color: var(--accent-green);
    margin: 4px 0;
}
.rfid-uid {
    color: var(--accent-yellow);
    font-family: monospace;
    font-size: 16px;
    letter-spacing: 2px;
}
.alert-box {
    border-color: rgba(234, 179, 8, 0.6);
    background-color: rgba(234, 179, 8, 0.08);
}
.alert-box .sensor-status {
    color: var(--accent-yellow);
}
.danger-box {
    border-color: rgba(239, 68, 68, 0.8);
    background-color: rgba(239, 68, 68, 0.15);
    animation: pulse-danger 1s infinite alternate;
}
.danger-box .sensor-status {
    color: var(--accent-red);
}
@keyframes pulse-danger {
    from { box-shadow: 0 0 5px rgba(239, 68, 68, 0.4); }
    to { box-shadow: 0 0 18px rgba(239, 68, 68, 0.8); }
}

footer {
    text-align: center;
    margin-top: 24px;
}
.network-badge {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    background-color: var(--card-bg);
    padding: 6px 14px;
    border-radius: 20px;
    font-size: 12px;
    color: var(--text-secondary);
}
.dot-online {
    width: 8px;
    height: 8px;
    background-color: var(--accent-green);
    border-radius: 50%;
    box-shadow: 0 0 8px var(--accent-green);
}
.tech-stack {
    font-size: 11px;
    color: var(--text-secondary);
    margin-top: 8px;
}
</style>
)rawliteral";

#endif
