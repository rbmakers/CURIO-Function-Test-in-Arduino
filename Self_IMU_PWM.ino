// 火箭鳥創客倉庫

#include <Wire.h>

// ==========================================
// 📌 硬體腳位與參數定義
// ==========================================
// 1. 三色狀態 LED 腳位
const int LED_A = 7;
const int LED_B = 8;
const int LED_C = 9;

// 2. 四路空心杯馬達 PWM 腳位
const int M1_PIN = 0;
const int M2_PIN = 11;
const int M3_PIN = 14;
const int M4_PIN = 28;

// 3. I2C0 與 BMI088 定義 (3.0V 低雜訊邏輯準位)
const int I2C0_SDA = 20;
const int I2C0_SCL = 25;
#define BMI088_ACC_ADDR  0x18  // SDO1 -> GND
#define BMI088_GYRO_ADDR 0x69  // SDO2 -> 3V (V_DDIO)

// PWM 參數
const int PWM_FREQ = 20000;   // 20kHz 超音波頻率，防止馬達共振尖叫
const int PWM_RANGE = 255;    // 8-bit 解析度 (0 ~ 255)

void setup() {
  // ----------------------------------------
  // Step 1: 優先初始化 LED 並執行開機自我測試
  // ----------------------------------------
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);
  runLED_SelfTest(); 

  // ----------------------------------------
  // Step 2: 初始化序列埠
  // ----------------------------------------
  Serial.begin(115200);
  delay(1500); // 留時間給電腦 CDC 驅動握手
  Serial.println("\n=============================================");
  Serial.println("     CURIO Flight Controller Comprehensive Test");
  Serial.println("=============================================");

  // ----------------------------------------
  // Step 3: 初始化 I2C0 介面與 BMI088 感測器
  // ----------------------------------------
  Serial.println("正在初始化 I2C0 (SDA:20, SCL:25)...");
  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire.begin();
  Wire.setClock(100000);   // 100kHz 標準速度
  Wire.setTimeout(25);     // 25ms 逾時防死當

  Serial.println("正在喚醒並設定 BMI088 暫存器...");
  initBMI088();
  Serial.println("✅ BMI088 初始化成功！");

  // ----------------------------------------
  // Step 4: 初始化馬達 PWM 配置
  // ----------------------------------------
  pinMode(M1_PIN, OUTPUT);
  pinMode(M2_PIN, OUTPUT);
  pinMode(M3_PIN, OUTPUT);
  pinMode(M4_PIN, OUTPUT);

  analogWriteFreq(PWM_FREQ);
  analogWriteRange(PWM_RANGE);

  // 初始全關閉
  analogWrite(M1_PIN, 0);
  analogWrite(M2_PIN, 0);
  analogWrite(M3_PIN, 0);
  analogWrite(M4_PIN, 0);
  
  Serial.println("✅ PWM 馬達驅動核心就緒！");
  Serial.println("---------------------------------------------");
  Serial.println("開始進入主測試循環：馬達動態控制 + IMU 即時讀取...\n");
}

void loop() {
  // 依序輪流測試 M1 ~ M4，並在測試中持續讀取顯示 IMU 數據
  runMotorWithIMU(M1_PIN, "M1 (GPIO 0)");
  runMotorWithIMU(M2_PIN, "M2 (GPIO 11)");
  runMotorWithIMU(M3_PIN, "M3 (GPIO 14)");
  runMotorWithIMU(M4_PIN, "M4 (GPIO 28)");

  delay(1000); // 每一輪大循環結束，停頓 1 秒
}

// ==========================================
// 🛸 核心整合功能：馬達驅動的同時讀取 IMU
// ==========================================
void runMotorWithIMU(int motorPin, String motorName) {
  Serial.println(">>> 啟動測試: " + motorName);
  float ax, ay, az, gx, gy, gz;

  // 1. 漸強加速段
  for (int duty = 0; duty <= PWM_RANGE; duty += 5) {
    analogWrite(motorPin, duty);
    
    // 在加速的過程中，實時讀取並顯示一組 BMI088 數據
    if (readAccelerometer(ax, ay, az) && readGyroscope(gx, gy, gz)) {
      printSystemData(motorName, duty, ax, ay, az, gx, gy, gz);
    }
    delay(15); // 控制加速平滑度
  }

  // 2. 最高速維持段 (運轉 0.3 秒)
  analogWrite(motorPin, PWM_RANGE);
  for (int i = 0; i < 6; i++) {
    if (readAccelerometer(ax, ay, az) && readGyroscope(gx, gy, gz)) {
      printSystemData(motorName, PWM_RANGE, ax, ay, az, gx, gy, gz);
    }
    delay(50);
  }

  // 3. 漸弱減速段
  for (int duty = PWM_RANGE; duty >= 0; duty -= 5) {
    analogWrite(motorPin, duty);
    
    if (readAccelerometer(ax, ay, az) && readGyroscope(gx, gy, gz)) {
      printSystemData(motorName, duty, ax, ay, az, gx, gy, gz);
    }
    delay(15);
  }

  // 確保完全關閉該馬達，並停頓一下
  analogWrite(motorPin, 0);
  Serial.println("<<< 結束測試: " + motorName + "\n");
  delay(400);
}

// 數據格式化輸出
void printSystemData(String mName, int duty, float ax, float ay, float az, float gx, float gy, float gz) {
  // 動態閃爍系統心跳燈 (LED_B) 提示運作中
  digitalWrite(LED_B, (duty % 10 == 0) ? HIGH : LOW);

  Serial.print("[" + mName + " PWM:" + String(duty) + "] ");
  Serial.print("ACC[G]: X="); Serial.print(ax, 3);
  Serial.print(" Y="); Serial.print(ay, 3);
  Serial.print(" Z="); Serial.print(az, 3);
  Serial.print(" | GYRO[°/s]: X="); Serial.print(gx, 1);
  Serial.print(" Y="); Serial.print(gy, 1);
  Serial.print(" Z="); Serial.println(gz, 1);
}

// ==========================================
// 🛠️ 飛控板開機 LED 自我測試副程式 (POST)
// ==========================================
void runLED_SelfTest() {
  // [階段 1] 快速流水燈 (3 輪)
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_A, HIGH); delay(60); digitalWrite(LED_A, LOW);
    digitalWrite(LED_B, HIGH); delay(60); digitalWrite(LED_B, LOW);
    digitalWrite(LED_C, HIGH); delay(60); digitalWrite(LED_C, LOW);
  }
  delay(150);

  // [階段 2] 慢速交替閃爍 (2 輪)
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_A, HIGH); digitalWrite(LED_B, LOW); digitalWrite(LED_C, HIGH);
    delay(200);
    digitalWrite(LED_A, LOW); digitalWrite(LED_B, HIGH); digitalWrite(LED_C, LOW);
    delay(200);
  }
  digitalWrite(LED_B, LOW);
  delay(150);

  // [階段 3] 三燈全大亮就緒提示
  digitalWrite(LED_A, HIGH); digitalWrite(LED_B, HIGH); digitalWrite(LED_C, HIGH);
  delay(400);
  digitalWrite(LED_A, LOW);  digitalWrite(LED_B, LOW);  digitalWrite(LED_C, LOW);
  delay(200);
}

// ==========================================
// 📡 BMI088 底層暫存器控制副程式
// ==========================================
void initBMI088() {
  writeRegister(BMI088_ACC_ADDR, 0x7D, 0x04);  delay(50); // 喚醒 ACC
  writeRegister(BMI088_ACC_ADDR, 0x7C, 0x00);  delay(50); // ACC 電源開啟
  writeRegister(BMI088_ACC_ADDR, 0x41, 0x03);  delay(20); // ACC 範圍 +/- 24G
  writeRegister(BMI088_GYRO_ADDR, 0x0F, 0x00); delay(20); // GYRO 範圍 +/- 2000 °/s
  writeRegister(BMI088_GYRO_ADDR, 0x10, 0x00); delay(20); // GYRO ODR = 400Hz
}

bool readAccelerometer(float &x, float &y, float &z) {
  uint8_t buffer[7]; // 包含 1 位元組 Dummy Byte
  if (!readRegisters(BMI088_ACC_ADDR, 0x12, 7, buffer)) return false;
  
  int16_t rawX = (int16_t)((buffer[2] << 8) | buffer[1]);
  int16_t rawY = (int16_t)((buffer[4] << 8) | buffer[3]);
  int16_t rawZ = (int16_t)((buffer[6] << 8) | buffer[5]);
  
  x = rawX * 0.0007324; // 24G 因子
  y = rawY * 0.0007324;
  z = rawZ * 0.0007324;
  return true;
}

bool readGyroscope(float &x, float &y, float &z) {
  uint8_t buffer[6];
  if (!readRegisters(BMI088_GYRO_ADDR, 0x02, 6, buffer)) return false;
  
  int16_t rawX = (int16_t)((buffer[1] << 8) | buffer[0]);
  int16_t rawY = (int16_t)((buffer[3] << 8) | buffer[2]);
  int16_t rawZ = (int16_t)((buffer[5] << 8) | buffer[4]);
  
  x = rawX * 0.061035; // 2000 deg/s 因子
  y = rawY * 0.061035;
  z = rawZ * 0.061035;
  return true;
}

void writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
  Wire.beginTransmission(devAddr);
  Wire.write(regAddr);
  Wire.write(data);
  Wire.endTransmission();
}

bool readRegisters(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *buffer) {
  Wire.beginTransmission(devAddr);
  Wire.write(regAddr);
  if (Wire.endTransmission(false) != 0) return false;
  
  if (Wire.requestFrom(devAddr, length) == length) {
    for (uint8_t i = 0; i < length; i++) {
      buffer[i] = Wire.read();
    }
    return true;
  }
  return false;
}