/*
 * [위성 탑재용 송신 코드 + AT 커맨드 기능]
 * - 평소: 1초마다 GPS 및 센서 데이터 전송
 * - 기능: 시리얼 모니터에 'AT_ON' 입력 시 설정 모드 진입
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_NeoPixel.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// --- 핀 설정 ---
#define NEOPIXEL_PIN  13
#define NUMPIXELS     12
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// [추가] HC-12 SET 제어 핀 (반드시 물리적으로 연결하세요: GPIO 4)
#define HC12_SET_PIN 4 

// GPS 연결 (UART1, 핀 26/27)
#define GPS_RX_PIN 26 
#define GPS_TX_PIN 27

// HC-12 연결 (UART2, 핀 16/17)
#define HC12_RX_PIN 16 
#define HC12_TX_PIN 17 

// 객체 생성
Adafruit_MPU6050 mpu;
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
TinyGPSPlus gps;

HardwareSerial GPS_Serial(1);
HardwareSerial HC12(2);

float base_pressure = 1013.25; 
unsigned long last_send_time = 0;
bool isAtMode = false; // AT 모드 상태 플래그 (false=통신, true=설정)

void setup() {
  Serial.begin(115200); // PC 디버깅용
  
  // [추가] SET 핀 설정 (초기엔 HIGH = 통신 모드)
  pinMode(HC12_SET_PIN, OUTPUT);
  digitalWrite(HC12_SET_PIN, HIGH); 

  GPS_Serial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  HC12.begin(9600, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
  
  pixels.begin();
  pixels.setBrightness(30);
  
  // 센서 초기화 (실패해도 멈추지 않고 진행하도록 수정 - 디버깅 편의)
  bool status = true;
  if (!mpu.begin()) { Serial.println("MPU Fail"); status = false; }
  if (!aht.begin()) { Serial.println("AHT Fail"); status = false; }
  if (!bmp.begin(0x76) && !bmp.begin(0x77)) { Serial.println("BMP Fail"); status = false; }

  if(!status) {
    setLedColor(255, 0, 0); // 실패: 빨강
    // 멈추지 않고 진행 (AT모드라도 쓰기 위해)
  }

  // 기압 센서가 살아있다면 기준 기압 설정
  if(status) base_pressure = bmp.readPressure() / 100.0;
  
  Serial.println("=== Satellite Ready ===");
  Serial.println("Commands:");
  Serial.println(" 1. Type 'AT_ON' to enter HC-12 Settings Mode.");
  Serial.println(" 2. Type 'AT_OFF' to return to Data Transmit Mode.");
}

void loop() {
  
  // ============================================================
  // [1] PC로부터 명령어 입력 처리 (AT 모드 전환 로직)
  // ============================================================
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // 공백/줄바꿈 제거

    // 1-1. 설정 모드 진입 명령
    if (input.equalsIgnoreCase("AT_ON")) {
      isAtMode = true;
      digitalWrite(HC12_SET_PIN, LOW); // SET 핀 LOW -> AT 모드 진입
      delay(100); // 진입 대기 (필수)
      Serial.println("\n>> [AT MODE] Entered. Sending stopped.");
      Serial.println(">> Type 'AT', 'AT+RX', 'AT+DEFAULT' etc.");
    } 
    // 1-2. 통신 모드 복귀 명령
    else if (input.equalsIgnoreCase("AT_OFF")) {
      isAtMode = false;
      digitalWrite(HC12_SET_PIN, HIGH); // SET 핀 HIGH -> 통신 모드 복귀
      delay(100); // 복귀 대기 (필수)
      Serial.println("\n>> [NORMAL MODE] Resuming transmission...");
    } 
    // 1-3. 그 외 입력 (AT 모드일 때만 HC-12로 전달)
    else {
      if (isAtMode) {
        HC12.println(input); // 사용자가 입력한 명령어(예: AT+C005)를 HC-12에 전송
      }
    }
  }

  // ============================================================
  // [2] HC-12 응답 확인 (AT 모드일 때 응답 보기)
  // ============================================================
  if (HC12.available()) {
    // AT 모드일 때 HC-12가 보낸 "OK" 같은 응답을 PC 화면에 출력
    if (isAtMode) {
      char c = HC12.read();
      Serial.write(c); 
    } else {
      // 통신 모드일 때는 버퍼 비우기 (혹시 모를 쓰레기 값 제거)
      HC12.read();
    }
  }

  // ============================================================
  // [3] 센서 데이터 전송 (AT 모드가 아닐 때만 실행!)
  // ============================================================
  if (!isAtMode) {
    // GPS 파싱 (항상 수행해야 위치를 안 잃어버림)
    while (GPS_Serial.available() > 0) {
      gps.encode(GPS_Serial.read());
    }

    if (millis() - last_send_time > 1000) {
      last_send_time = millis();
      
      // 센서 데이터 읽기
      float alt = 0, temp = 0, humi = 0;
      
      // 센서가 연결되어 있을 때만 값 읽기 (에러 방지)
      if (aht.begin()) {
        sensors_event_t humidity, temp_aht;
        aht.getEvent(&humidity, &temp_aht);
        temp = temp_aht.temperature;
        humi = humidity.relative_humidity;
      }
      if (bmp.begin()) {
        alt = bmp.readAltitude(base_pressure);
      }

      String packet = "";
      if (gps.location.isValid()) {
        packet += String(gps.location.lat(), 6) + ",";
        packet += String(gps.location.lng(), 6) + ",";
        setLedColor(0, 255, 0); 
      } else {
        packet += "0.000000,0.000000,";
        setLedColor(0, 0, 255); 
      }
      
      packet += String(alt, 2) + ",";
      packet += String(temp, 1) + ",";
      packet += String(humi, 0);
      
      HC12.println(packet); 
      Serial.print("Sending: ");
      Serial.println(packet); 
    }
  }
}

void setLedColor(int r, int g, int b) {
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}