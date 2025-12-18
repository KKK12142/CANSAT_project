#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C 방식 초기화 (핀 번호 입력 불필요, Wire 라이브러리가 알아서 21, 22 찾음)
#define OLED_RESET     -1 // 리셋 핀이 없으면 -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  // 0x3C는 I2C 주소입니다. (안 되면 0x3D로 바꿔보세요)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Hello ESP32 I2C!");
  display.display();
}

void loop() {
//이부분에 이제 센서에서 수집한 데이터를 (GPS정보, 온습도, 고도, 기압등...) 표시를 해줘야함

}