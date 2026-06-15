// 火箭鳥創客倉庫

#include <Wire.h>

// 位址
#define BMI088_ACC_ADDR  0x18  // SDO1 -> GND
#define BMI088_GYRO_ADDR 0x69  // SDO2 -> 3V

const int I2C0_SDA = 20;
const int I2C0_SCL = 25;

void setup() {
  Serial.begin(115200);
  
  // 等待序列埠，最多等 3 秒
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < 3000));

  Serial.println("\n--- BMI088 Arduino Debug Start ---");

  // 初始化 I2C0 腳位與速度
  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire.begin();
  Wire.setClock(100000);   // 使用標準 100kHz 確保穩定
  
  // 修正此行：使用 setTimeout 代替 setWireTimeout
  Wire.setTimeout(25);     // 設定 25ms 逾時，防止系統因 I2C 訊號異常而當機

  // 初始化 BMI088
  initBMI088();
  Serial.println("✅ BMI088 Initialization Done!");
}

void loop() {
  float accX, accY, accZ;
  float gyroX, gyroY, gyroZ;

  // 讀取數據
  if (readAccelerometer(accX, accY, accZ) && readGyroscope(gyroX, gyroY, gyroZ)) {
    // 輸出至序列埠監控視窗
    Serial.print("ACC [G]: X="); Serial.print(accX, 3);
    Serial.print(" | Y="); Serial.print(accY, 3);
    Serial.print(" | Z="); Serial.print(accZ, 3);
    
    Serial.print("  ||  GYRO [°/s]: X="); Serial.print(gyroX, 1);
    Serial.print(" | Y="); Serial.print(gyroY, 1);
    Serial.print(" | Z="); Serial.println(gyroZ, 1);
  } else {
    Serial.println("⚠️ 讀取失敗，正在嘗試重置 I2C 總線...");
    Wire.begin(); // 嘗試恢復總線
  }

  delay(100); // 10Hz 更新率
}

// --- BMI088 初始化 ---
void initBMI088() {
  // 1. 將 ACC 從延時模式喚醒
  writeRegister(BMI088_ACC_ADDR, 0x7D, 0x04);
  delay(50);
  
  // 2. 開啟 ACC 電源
  writeRegister(BMI088_ACC_ADDR, 0x7C, 0x00);
  delay(50);

  // 3. 設定 ACC 範圍為 +/- 24G
  writeRegister(BMI088_ACC_ADDR, 0x41, 0x03); 
  delay(20);
  
  // 4. 設定 GYRO 範圍為 +/- 2000 deg/s
  writeRegister(BMI088_GYRO_ADDR, 0x0F, 0x00);
  delay(20);
  
  // 5. 設定 GYRO ODR = 400Hz
  writeRegister(BMI088_GYRO_ADDR, 0x10, 0x00);
  delay(20);
}

// --- 讀取加速度計 (修正 Dummy Byte 問題) ---
bool readAccelerometer(float &x, float &y, float &z) {
  // Bosch 手冊指出：I2C 讀取 ACC 暫存器時，第一個傳回的位元組是 Dummy byte 
  // 因此我們要連續讀取 1 + 6 = 7 個位元組
  uint8_t buffer[7];
  
  if (!readRegisters(BMI088_ACC_ADDR, 0x12, 7, buffer)) {
    return false;
  }

  // buffer[0] 是 Dummy byte，有效的資料從 buffer[1] 開始
  int16_t rawX = (int16_t)((buffer[2] << 8) | buffer[1]);
  int16_t rawY = (int16_t)((buffer[4] << 8) | buffer[3]);
  int16_t rawZ = (int16_t)((buffer[6] << 8) | buffer[5]);

  // 24G 範圍轉換因子
  x = rawX * 0.0007324;
  y = rawY * 0.0007324;
  z = rawZ * 0.0007324;
  return true;
}

// --- 讀取陀螺儀 ---
bool readGyroscope(float &x, float &y, float &z) {
  uint8_t buffer[6]; // 陀螺儀不需要 Dummy Byte
  
  if (!readRegisters(BMI088_GYRO_ADDR, 0x02, 6, buffer)) {
    return false;
  }

  int16_t rawX = (int16_t)((buffer[1] << 8) | buffer[0]);
  int16_t rawY = (int16_t)((buffer[3] << 8) | buffer[2]);
  int16_t rawZ = (int16_t)((buffer[5] << 8) | buffer[4]);

  // 2000 deg/s 範圍轉換因子
  x = rawX * 0.061035;
  y = rawY * 0.061035;
  z = rawZ * 0.061035;
  return true;
}

// --- I2C 底層寫入 ---
void writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
  Wire.beginTransmission(devAddr);
  Wire.write(regAddr);
  Wire.write(data);
  Wire.endTransmission();
}

// --- I2C 底層連續讀取 (強化嚴謹度) ---
bool readRegisters(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *buffer) {
  Wire.beginTransmission(devAddr);
  Wire.write(regAddr);
  
  // 檢查發送暫存器位址是否成功
  if (Wire.endTransmission(false) != 0) {
    return false; 
  }

  // 請求資料，並確認收到的長度是否相符
  if (Wire.requestFrom(devAddr, length) == length) {
    for (uint8_t i = 0; i < length; i++) {
      buffer[i] = Wire.read();
    }
    return true;
  }
  return false;
}
