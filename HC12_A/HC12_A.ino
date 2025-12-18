#include <HardwareSerial.h>

// --- 핀 번호 정의 (PCB 고정: 16, 17) ---
// 만약 통신이 안 되면 16, 17 숫자를 서로 바꿔보세요 (Swap Test)
#define HC12_RX_PIN 16 // ESP32가 받는 핀 (HC-12의 TX와 연결)
#define HC12_TX_PIN 17 // ESP32가 보내는 핀 (HC-12의 RX와 연결)

// HC-12용 시리얼 객체 생성 (UART 1번 사용)
HardwareSerial HC12(1); 

// 비동기 딜레이를 위한 타이머 변수
unsigned long last_send_time = 0;
const long interval = 1000; // 1초 간격 전송

void setup() {
  Serial.begin(115200);      // PC 시리얼 모니터 (빠르게)
  
  // HC-12 통신 시작
  // ESP32는 핀 매핑이 자유로우므로 begin()에서 핀을 지정합니다.
  HC12.begin(9600, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
  
  Serial.println(F("=== Dual-Way Communication Ready ==="));
  Serial.println(F("Data Sent -> [TX] / Data Received -> [RX]"));
}

void loop() {
  // ==========================================
  // 1. 수신 모드 (항상 귀를 열고 있음)
  // ==========================================
  if (HC12.available()) {
      // 데이터가 들어오면 '\n' (줄바꿈)이 나올 때까지 읽음
      String receivedData = HC12.readStringUntil('\n');
      
      // 수신된 데이터 출력 (구분을 위해 [RX] 태그 추가)
      Serial.print(F("[RX Recv]: "));
      Serial.println(receivedData);
  }

  // ==========================================
  // 2. 송신 모드 (1초마다 한 번씩 실행)
  // ==========================================
  unsigned long current_time = millis();
  
  if (current_time - last_send_time >= interval) {
      last_send_time = current_time; // 시간 갱신

      // --- 임시 센서 데이터 생성 (테스트용) ---
      // 실제 센서 연결 시 이 부분을 실제 값(gps.lat 등)으로 교체
      float temperature = 25.4 + random(0, 10) / 10.0; // 약간의 변화를 줌
      float altitude = 15.3 + random(0, 5);
      double latitude = 37.5665;
      double longitude = 126.9780;

      // --- 데이터 패킷 조립 (CSV 포맷) ---
      String packet = "";
      packet += String(latitude, 6) + ",";
      packet += String(longitude, 6) + ",";
      packet += String(temperature, 1) + ",";
      packet += String(altitude, 1);

      // --- 전송 (반드시 println 사용) ---
      HC12.println(packet); 

      // 내가 보낸 데이터 확인용 출력 ([TX] 태그 추가)
      Serial.print(F("[TX Sent]: "));
      Serial.println(packet);
  }
}