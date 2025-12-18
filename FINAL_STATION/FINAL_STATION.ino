/*
 * [지상국 최종 수신 코드 - 심플 버전]
 * - 기능: HC-12 데이터 수신 및 OLED 표시 (자세 제어 제외)
 * - 표시 항목: 고도, 온도, 습도, 위도, 경도
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

// --- OLED 설정 (128x64) ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- HC-12 설정 ---
#define HC12_RX_PIN 4 
#define HC12_TX_PIN 5
#define HC12_SET_PIN 2

HardwareSerial HC12(2);

// 데이터 저장 변수
String lat = "0.00";
String lon = "0.00";
String alt = "0.0";
String temp = "0";
String humi = "0";

void setup() {
  Serial.begin(115200);

  // 1. OLED 초기화
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED Init Failed"));
    for(;;);
  }
  
  // 로딩 화면
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.println(F("Ground Station"));
  display.setCursor(35, 40);
  display.println(F("Ready..."));
  display.display();
  delay(1000);

  // 2. SET 핀 설정 (통신 모드)
  pinMode(HC12_SET_PIN, OUTPUT);
  digitalWrite(HC12_SET_PIN, HIGH);

  // 3. HC-12 통신 시작 
  // (AT+FU1, AT+B2400 등으로 설정했다면 2400으로, 기본이면 9600으로 맞추세요)
  HC12.begin(9600, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN); 
}
 
void loop() {
  // HC-12 데이터 수신
  if (HC12.available()) {
    String packet = HC12.readStringUntil('\n');
    packet.trim(); // 공백 제거
    Serial.println(packet); // PC 시리얼 모니터 로깅용

    // 데이터 파싱 (최소 5개 데이터가 있는지 확인)
    if (packet.length() > 5) {
      // 순서: 위도(0), 경도(1), 고도(2), 온도(3), 습도(4) ... (뒤에 더 있어도 무시함)
      lat  = getValue(packet, ',', 0);
      lon  = getValue(packet, ',', 1);
      alt  = getValue(packet, ',', 2);
      temp = getValue(packet, ',', 3);
      humi = getValue(packet, ',', 4);
      
      updateScreen(); // 화면 갱신
    }
  }
}

// --- 화면 그리기 ---
void updateScreen() {
  display.clearDisplay();

  // 1. 상단 타이틀
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("== CANSAT RX =="));
  display.drawLine(0, 9, 128, 9, WHITE); // 가로줄

  // 2. 고도 (가장 크게 강조)
  display.setCursor(0, 16);
  display.setTextSize(1);
  display.print(F("ALT: "));
  
  display.setTextSize(2); // 글자 크기 2배
  display.setCursor(35, 14); // 위치 조정
  display.print(alt);
  
  display.setTextSize(1);
  display.print(F("m"));

  // 3. 온도 / 습도
  display.setCursor(0, 38);
  display.print(F("Temp: ")); display.print(temp); display.print(F(" C"));
  
  display.setCursor(0, 48);
  display.print(F("Humi: ")); display.print(humi); display.print(F(" %"));

  // 4. GPS 좌표 (맨 아래)
  display.setCursor(0, 56);
  if(lat.toFloat() == 0.0) {
    display.print(F("GPS Searching..."));
  } else {
    // 공간 부족 시 소수점 줄여서 표시
    display.print(lat); display.print(F(",")); display.print(lon);
  }

  display.display();
}

// --- 문자열 파싱 함수 ---
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "0";
}