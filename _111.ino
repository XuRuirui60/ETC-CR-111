#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128  // OLED 顯示器的寬度（像素）
#define SCREEN_HEIGHT 64  // OLED 顯示器的高度（像素）
#define OLED_RESET -1     // OLED 的重置引腳（-1 表示共用 Arduino 的重置引腳）
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 設定74HC595移位寄存器的引腳
const int dataPin = 6;   // SER (數據引腳)
const int latchPin = 5;  // RCLK (鎖存引腳)
const int clockPin = 3;  // SRCLK (時鐘引腳)

// 設定燈光的持續時間
const long YRDuration = 2000;  // 黃燈持續時間（2秒）
const long GRDuration = 3000;  // 綠燈持續時間（3秒）

unsigned long previousMillis = 0;  // 上一次變更時間
int state = 0;                     // 交通燈狀態：0 - 東西向紅燈，1 - 東西向黃燈，2 - 東西向綠燈，3 - 南北向紅燈，4 - 南北向黃燈，5 - 南北向綠燈

const int CPUledPin = 9;  // CPU狀態指示燈引腳
const int buttonUp = A2;  // 向上按鈕引腳
const int buttonEnter = A1; // 確認按鈕引腳
const int buttonDown = A0; // 向下按鈕引腳

unsigned long ledMillis = 0;  // LED閃爍時間
const long ledInterval = 100; // LED閃爍間隔（毫秒）
bool led = false;            // LED狀態

unsigned long startMillis = 0;  // 計時器起始時間
const long initialDisplayDuration = 2000;  // 初始顯示時間（2秒）
bool initialDisplayDone = false;  // 初始顯示是否完成

int page = 0;  // 當前顯示頁面

SoftwareSerial BT(0, 1);  // 藍牙通訊引腳（RX, TX）
String bs;  // 藍牙接收到的訊息
bool GYRLED = false;  // 控制交通燈是否啟動

void setup() {
  pinMode(CPUledPin, OUTPUT);  // 設定 CPU LED 引腳為輸出
  pinMode(buttonUp, INPUT);    // 設定向上按鈕引腳為輸入
  pinMode(buttonEnter, INPUT); // 設定確認按鈕引腳為輸入
  pinMode(buttonDown, INPUT);  // 設定向下按鈕引腳為輸入
  Serial.begin(9600);          // 開啟串口通訊
  BT.begin(9600);              // 初始化藍牙通訊

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 初始化 OLED 顯示器
    Serial.println(F("SSD1306 allocation failed"));  // 如果初始化失敗，輸出錯誤信息
    for (;;)
      ;  // 不繼續執行，進入無限循環
  }
  display.clearDisplay();  // 清除顯示畫面
  startMillis = millis();  // 記錄起始時間
  display.setTextSize(2);     // 設定文字大小
  display.setTextColor(1);    // 設定文字顏色（1 表示預設顏色）
  display.setCursor(41, 10);  // 設定文字起始座標
  display.print("NKHS");      // 顯示字串 "NKHS"
  display.setCursor(41, 40);  // 設定文字起始座標
  display.print("C201");      // 顯示字串 "C201"
  display.display();          // 更新顯示內容

  pinMode(dataPin, OUTPUT);   // 設定 74HC595 的數據引腳為輸出
  pinMode(latchPin, OUTPUT);  // 設定 74HC595 的鎖存引腳為輸出
  pinMode(clockPin, OUTPUT);  // 設定 74HC595 的時鐘引腳為輸出
  updateShiftRegister(0b00000000);  // 設定初始狀態（東西向紅燈，南北向紅燈）
}

void loop() {
  unsigned long currentMillis = millis();  // 當前時間
  unsigned long interval;

  blueT();  // 處理藍牙訊息
  Serial.println(GYRLED);  // 輸出交通燈狀態

  if (currentMillis - ledMillis >= ledInterval) {  // 根據間隔時間來閃爍 LED
    ledMillis = currentMillis;
    led = !led;
    digitalWrite(CPUledPin, led);
  }

  if (GYRLED == true) {  // 如果啟動交通燈
    switch (state) {
      case 0:  // 東西向紅燈和南北向紅燈
      case 2:  // 東西向綠燈和南北向紅燈
        interval = GRDuration;
        break;
      case 1:  // 東西向黃燈和南北向紅燈
      case 3:  // 南北向黃燈和東西向紅燈
        interval = YRDuration;
        break;
    }

    if (currentMillis - previousMillis >= interval) {  // 根據時間變更交通燈狀態
      previousMillis = currentMillis;
      state = (state + 1) % 4;

      switch (state) {
        case 0:  // 東西向紅燈，南北向紅燈
          updateShiftRegister(0b00100001);
          break;
        case 1:  // 東西向黃燈，南北向紅燈
          updateShiftRegister(0b00010001);
          break;
        case 2:  // 東西向綠燈，南北向紅燈
          updateShiftRegister(0b00001100);
          break;
        case 3:  // 南北向紅燈，東西向紅燈
          updateShiftRegister(0b00001010);
          break;
      }
    }
  }

  if (currentMillis - startMillis >= initialDisplayDuration && !initialDisplayDone) {  // 顯示選單
    int buttonUpState = digitalRead(buttonUp);      // 讀取向上按鈕狀態
    int buttonEnterState = digitalRead(buttonEnter); // 讀取確認按鈕狀態
    int buttonDownState = digitalRead(buttonDown);  // 讀取向下按鈕狀態

    switch (page) {
      case 0:
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(1);
        display.setCursor(41, 4);
        display.print("Menu");
        display.setCursor(47, 46);
        display.print("GYR");
        display.fillRect(0, 22, 128, 21, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(47, 25);
        display.print("FAN");
        display.display();
        if (buttonEnterState == LOW)
          page = 1;
        break;
      case 1:
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(1);
        display.setCursor(47, 1);
        display.print("FAN");
        display.setCursor(29, 33);
        display.print("Speed2");
        display.setCursor(29, 49);
        display.print("Return");
        display.fillRect(0, 16, 128, 16, 1);
        display.setTextColor(0);
        display.setCursor(29, 17);
        display.print("Speed1");
        display.display();
        if (buttonUpState == LOW)
          page = 3;
        if (buttonDownState == LOW)
          page = 2;
        break;
      case 2:
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(1);
        display.setCursor(47, 1);
        display.print("FAN");
        display.setCursor(29, 17);
        display.print("Speed1");
        display.setCursor(29, 49);
        display.print("Return");
        display.fillRect(0, 32, 128, 16, 1);
        display.setTextColor(0);
        display.setCursor(29, 33);
        display.print("Speed2");
        display.display();
        if (buttonUpState == LOW)
          page = 1;
        if (buttonDownState == LOW)
          page = 3;
        break;
      case 3:
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(1);
        display.setCursor(47, 1);
        display.print("FAN");
        display.setCursor(29, 17);
        display.print("Speed1");
        display.setCursor(29, 33);
        display.print("Speed2");
        display.fillRect(0, 48, 128, 16, 1);
        display.setTextColor(0);
        display.setCursor(29, 49);
        display.print("Return");
        display.display();
        if (buttonUpState == LOW)
          page = 2;
        if (buttonEnterState == LOW)
          page = 0;
        if (buttonDownState == LOW)
          page = 1;
        break;
      default:
        break;
    }
  }
}

// 更新74HC595移位寄存器的狀態
void updateShiftRegister(byte data) {
  digitalWrite(latchPin, LOW);      // 設定鎖存引腳為低電位（開始傳輸數據）
  shiftOut(dataPin, clockPin, MSBFIRST, data);  // 將數據傳送到移位寄存器
  digitalWrite(latchPin, HIGH);     // 設定鎖存引腳為高電位（完成數據傳輸）
}

// 處理藍牙訊息
void blueT() {
  if (BT.available() > 0) {  // 如果有藍牙資料可讀
    String bs = BT.readString();  // 讀取藍牙資料
    Serial.println(bs);          // 輸出藍牙資料
    if (bs[0] == 'T') {  // 如果資料以 'T' 開頭，啟動交通燈
      GYRLED = true;
      previousMillis = millis();  // 重設計時器
    } else if (bs[0] == 'P') {  // 如果資料以 'P' 開頭，停用交通燈
      GYRLED = false;
      updateShiftRegister(0b00000000);  // 設定所有燈為關閉
    } else if (bs[0] == 'W') {  // 如果資料以 'W' 開頭，寫入 EEPROM
      for (int i = 1; i <= 4; i++) {
        EEPROM.write(i - 1, bs[i]);
      }
    } else if (bs[0] == 'R') {  // 如果資料以 'R' 開頭，讀取 EEPROM
      for (int i = 1; i <= 4; i++) {
        byte value = EEPROM.read(i);
        BT.print(value);
        BT.print(' ');
      }
      BT.println();
    }
  }
}
