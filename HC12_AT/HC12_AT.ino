/*
 * [HC-12 설정 및 디버깅용 패스스루 코드]
 * - 기능: 시리얼 모니터의 명령어를 HC-12로 전달하고, 응답을 출력
 * - 사용법: 업로드 후 시리얼 모니터에서 'AT' 입력
 */
#include <HardwareSerial.h>

const int HC12_SET_PIN = 4;  // SET 핀 (LOW: AT모드, HIGH: 통신모드)
const int HC12_RX_PIN = 16;  // ESP32 RX <- HC-12 TX
const int HC12_TX_PIN = 17;  // ESP32 TX -> HC-12 RX

HardwareSerial HC12(2);

void setup() {
  Serial.begin(115200); // PC와는 빠르게 통신
  
  // HC-12 초기값은 9600입니다. 만약 설정을 바꿨었다면 그 속도에 맞춰야 합니다.
  // 모르겠으면 9600으로 시도해보고 안 되면 다른 속도로 바꿔보세요.
  HC12.begin(9600, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);

  pinMode(HC12_SET_PIN, OUTPUT);
  
  // [중요] AT 커맨드 모드 진입
  digitalWrite(HC12_SET_PIN, LOW); 
  delay(200); // 모드 진입 안정화 대기

  Serial.println("--- HC-12 AT Command Mode ---");
  Serial.println("Type 'AT' to check connection.");
  Serial.println("Type 'AT+DEFAULT' to reset.");
}

void loop() {
  // PC -> HC-12
  if (Serial.available()) {
    HC12.write(Serial.read());
  }

  // HC-12 -> PC
  if (HC12.available()) {
    Serial.write(HC12.read());
  }
}