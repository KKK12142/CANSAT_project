#include <Wire.h>
#include <Adafruit_AHTX0.h>  // AHT20 라이브러리
#include <Adafruit_BMP280.h> // BMP280 라이브러리

// 객체 생성
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // 시리얼 연결 대기

  Serial.println(F("AHT20 + BMP280 Test Start"));

  // 1. AHT20 (온습도) 초기화
  if (!aht.begin()) {
    Serial.println(F("AHT20 not found! Check wiring"));
    while (1) delay(10);
  }
  Serial.println(F("AHT20 Found!"));

  // 2. BMP280 (기압/고도) 초기화
  // 중요: 알리발 모듈은 주소가 0x76인 경우가 많고, 정품은 0x77입니다.
  // 아래 코드는 0x76으로 시도해보고 안 되면 0x77로 다시 시도합니다.
  if (!bmp.begin(0x76)) {
    Serial.println(F("BMP280 init failed at 0x76, trying 0x77..."));
    if (!bmp.begin(0x77)) {
       Serial.println(F("BMP280 not found! Check wiring"));
       while (1) delay(10);
    }
  }
  Serial.println(F("BMP280 Found!"));
  // BMP280 설정 (노이즈 감소 및 성능 최적화)
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  // --- AHT20 데이터 읽기 ---
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // 온습도 데이터 가져오기

  // --- BMP280 데이터 읽기 ---
  float pressure = bmp.readPressure(); // 파스칼(Pa) 단위
  float altitude = bmp.readAltitude(1013.25); // 해수면 기압 기준 고도 계산

  // --- 출력 ---
  Serial.print(F("Temp(AHT): ")); 
  Serial.print(temp.temperature); 
  Serial.print(F(" C,  "));

  Serial.print(F("Humi(AHT): ")); 
  Serial.print(humidity.relative_humidity); 
  Serial.print(F(" %,  "));

  Serial.print(F("Press(BMP): ")); 
  Serial.print(pressure / 100); // hPa(헥토파스칼)로 변환
  Serial.print(F(" hPa,  "));

  Serial.print(F("Alt(BMP): ")); 
  Serial.print(altitude); 
  Serial.println(F(" m"));

  delay(2000); // 2초마다 갱신
}