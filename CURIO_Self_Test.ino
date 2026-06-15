// 火箭鳥創客倉庫
// 定義三個測試 LED 的 GPIO 腳位
const int LED_A = 7;
const int LED_B = 8;
const int LED_C = 9;

void setup() {
  // 1. 初始化所有 LED 腳位為輸出模式
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);

  // 確保開機時先全部熄滅
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_C, LOW);

  // 2. 啟動序列埠（供偵錯使用）
  Serial.begin(115200);
  delay(500); // 稍微等待硬體電壓穩定
  Serial.println("\n--- CURIO Flight Controller Booting ---");
  Serial.println("執行硬體自我測試 (POST)...");

  // 3. 執行開機自我測試程式 (Self-Test Sequence)
  runLED_SelfTest();

  Serial.println("✅ 自我測試完成！系統進入主循環。");
}

void loop() {
  // 自我測試完成後，進入正常運作模式
  // 這裡可以放你之前的 BMI088 讀取或馬達控制程式
  
  // 主循環中，我們可以讓中間的 LED_B (GPIO 8) 當作「系統心跳燈 (Heartbeat)」慢速閃爍
  digitalWrite(LED_B, HIGH);
  delay(500);
  digitalWrite(LED_B, LOW);
  delay(500);
}

// ==========================================
// 🛠️ 飛控板開機 LED 自我測試核心功能
// ==========================================
void runLED_SelfTest() {
  
  // --- 【第一階段】快速交替流水燈 (3 輪) ---
  // 視覺效果：快速閃爍，代表正在偵測硬體核心
  Serial.println("[1/3] 正在檢測 GPIO 輸出電路...");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_A, HIGH); delay(60); digitalWrite(LED_A, LOW);
    digitalWrite(LED_B, HIGH); delay(60); digitalWrite(LED_B, LOW);
    digitalWrite(LED_C, HIGH); delay(60); digitalWrite(LED_C, LOW);
  }
  delay(200); // 稍作停頓

  // --- 【第二階段】慢速交替沉穩閃爍 (2 輪) ---
  // 視覺效果：慢速交替，模擬系統核心與感測器初始化
  Serial.println("[2/3] 核心功能與時脈確認中...");
  for (int i = 0; i < 2; i++) {
    // A 燈與 C 燈亮，B 燈滅
    digitalWrite(LED_A, HIGH);
    digitalWrite(LED_B, LOW);
    digitalWrite(LED_C, HIGH);
    delay(250);

    // 狀態反轉：B 燈亮，A 與 C 燈滅
    digitalWrite(LED_A, LOW);
    digitalWrite(LED_B, HIGH);
    digitalWrite(LED_C, LOW);
    delay(250);
  }
  // 關閉所有燈
  digitalWrite(LED_B, LOW);
  delay(200);

  // --- 【第三階段】安全確認鎖定 ---
  // 視覺效果：三燈全亮 0.5 秒隨即熄滅，代表 Ready 訊號
  Serial.println("[3/3] 電源與週邊電壓正常，系統就緒！");
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_C, HIGH);
  delay(500); 
  
  // 全滅
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_C, LOW);
  delay(300);
}
