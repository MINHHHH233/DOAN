#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Định nghĩa chân
#define VIBRATION_SENSOR_PIN 3  // PD3
#define BUZZER_PIN 5            // PD5
#define LED_PIN 8               // PB0

// Khai báo Serial
SoftwareSerial simSerial(10, 11); // SIM: RX ← TX SIM, TX → RX SIM
SoftwareSerial gpsSerial(6, 7);   // GPS: RX ← TX GPS, TX → RX GPS

TinyGPSPlus gps;

bool alertSent = false;

void setup() {
  pinMode(VIBRATION_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  simSerial.begin(9600);
  gpsSerial.begin(9600);

  delay(1000);
  Serial.println("Hệ thống khởi động...");
}

void loop() {
  // Đọc dữ liệu GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  int vibration = digitalRead(VIBRATION_SENSOR_PIN);

  if (vibration == HIGH && !alertSent) {
    Serial.println("Phát hiện rung, kích hoạt báo động...");

    // Còi và LED nhấp nháy 10 giây
    for (int i = 0; i < 20; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(250);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      delay(250);
    }

    // Chuẩn bị tọa độ
    float lat = 0.0, lng = 0.0;
    if (gps.location.isValid()) {
      lat = gps.location.lat();
      lng = gps.location.lng();
    }

    // Soạn và gửi SMS
    sendSMS(lat, lng);
    alertSent = true;
  }

  // Cho phép gửi lại sau 30s nếu tiếp tục bị rung
  static unsigned long lastReset = millis();
  if (millis() - lastReset > 30000) {
    alertSent = false;
    lastReset = millis();
  }
}

// Hàm gửi SMS
void sendSMS(float lat, float lng) {
  simSerial.println("AT");
  delay(500);

  simSerial.println("AT+CMGF=1"); // Chế độ text
  delay(500);

  simSerial.println("AT+CMGS=\"0326511246\"");
  delay(500);

  if (lat != 0.0 && lng != 0.0) {
    simSerial.print("Xe bạn đang bị rung! Vị trí: https://maps.google.com/?q=");
    simSerial.print(lat, 6);
    simSerial.print(",");
    simSerial.print(lng, 6);
  } else {
    simSerial.print("Xe bạn đang bị rung! Không lấy được vị trí GPS.");
  }

  delay(500);
  simSerial.write(26); // Ký tự kết thúc tin nhắn (Ctrl+Z)
  delay(5000);

  Serial.println("Đã gửi tin nhắn SMS.");
}
