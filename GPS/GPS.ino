#include <TinyGPS++.h>
#include <HardwareSerial.h>

// 1. 객체 생성 (통역사 고용)
TinyGPSPlus gps;

// 2. 핀 번호 설정 (선생님 연결 상태)
#define RXD2 26
#define TXD2 27

void setup() {
  Serial.begin(115200); // PC와 통신 (시리얼 모니터용)
  
  // GPS와 통신 시작 (NEO-6M 기본 속도 9600)
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // 3. Serial2(GPS)에서 데이터가 들어오는 족족 읽어서
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    
    // 4. gps 객체에게 던져줍니다. (해석해!)
    // encode() 함수가 true를 반환하면 "해석 완료된 유효한 문장 하나가 완성됐다"는 뜻입니다.
    if (gps.encode(c)) {
      displayInfo(); // 정보를 출력하는 함수 호출
    }
  }
}

// 보기 좋게 출력하는 함수
void displayInfo() {
  Serial.print(F("Location: ")); 
  
  // 위도, 경도가 유효한지 확인 (위성을 못 잡으면 isValid가 false임)
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6); // 위도 (소수점 6자리)
    Serial.print(F(", "));
    Serial.print(gps.location.lng(), 6); // 경도
  } else {
    Serial.print(F("INVALID (Searching...)"));
  }

  Serial.print(F("  |  Sats: "));
  if (gps.satellites.isValid()) {
    Serial.print(gps.satellites.value()); // 잡힌 위성 개수
  } else {
    Serial.print(F("0"));
  }

  Serial.print(F("  |  Time: "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.println(); // 줄바꿈
}