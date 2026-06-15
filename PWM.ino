// 火箭鳥創客倉庫
// 定義空心杯馬達連接的 GPIO 腳位
const int M1_PIN = 0;  // M1 控制腳
const int M2_PIN = 11; // M2 控制腳
const int M3_PIN = 14; // M3 控制腳
const int M4_PIN = 28; // M4 控制腳

// PWM 參數設定
const int PWM_FREQ = 20000;  // PWM 頻率設定為 20kHz (超出人耳聽覺範圍，馬達不會發出尖叫聲)
const int PWM_RANGE = 255;   // 解析度設為 0-255 (8-bit)

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- RP2354A Coreless Motor PWM Test ---");

  // 初始化所有馬達腳位為輸出模式
  pinMode(M1_PIN, OUTPUT);
  pinMode(M2_PIN, OUTPUT);
  pinMode(M3_PIN, OUTPUT);
  pinMode(M4_PIN, OUTPUT);

  // 設定 RP2354A (Pico 2 核心) 的 PWM 頻率與範圍
  analogWriteFreq(PWM_FREQ);
  analogWriteRange(PWM_RANGE);

  // 初始狀態：全部馬達關閉
  analogWrite(M1_PIN, 0);
  analogWrite(M2_PIN, 0);
  analogWrite(M3_PIN, 0);
  analogWrite(M4_PIN, 0);
  
  Serial.println("PWM 初始化完成，開始馬達輪流測試...");
}

void loop() {
  // 輪流測試 M1 ~ M4
  testMotor(M1_PIN, "M1 (GPIO 0)");
  testMotor(M2_PIN, "M2 (GPIO 11)");
  testMotor(M3_PIN, "M3 (GPIO 14)");
  testMotor(M4_PIN, "M4 (GPIO 28)");
  
  delay(1000); // 每一輪測試完暫停 1 秒
}

// 輔助函式：讓指定馬達漸強與漸弱
void testMotor(int pin, String motorName) {
  Serial.println("現在測試: " + motorName);

  // 1. 漸強加速 (Duty cycle 從 0 到 255)
  for (int duty = 0; duty <= PWM_RANGE; duty++) {
    analogWrite(pin, duty);
    delay(5); // 調整此延時可改變加速快慢
  }
  
  delay(500); // 在最高速運轉 0.5 秒

  // 2. 漸弱減速 (Duty cycle 從 255 到 0)
  for (int duty = PWM_RANGE; duty >= 0; duty--) {
    analogWrite(pin, duty);
    delay(5);
  }

  // 確保完全關閉
  analogWrite(pin, 0);
  delay(300); // 停頓一下再換下一個馬達
}
