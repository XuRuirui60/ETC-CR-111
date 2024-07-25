#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128  // OLED 寬度像素
#define SCREEN_HEIGHT 64  // OLED 高度像素
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 設定74HC595的引腳
const int dataPin = 6;   // SER (Data pin)
const int latchPin = 5;  // RCLK (Latch pin)
const int clockPin = 3;  // SRCLK (Clock pin)

// 設定各燈的持續時間
const long YRDuration = 2000;  // 黃燈持續時間（2秒）
const long GRDuration = 3000;  // 綠燈持續時間（3秒）

unsigned long previousMillis = 0;  // 上一次變更時間
int state = 0;                     // 0 - 東西向紅燈，1 - 東西向黃燈，2 - 東西向綠燈，3 - 南北向紅燈，4 - 南北向黃燈，5 - 南北向綠燈

const int CPUledPin = 9;
const int buttonUp = A2;
const int buttonEnter = A1;
const int buttonDown = A0;

unsigned long ledMillis = 0;
const long ledInterval = 100;
bool led = false;

unsigned long startMillis = 0;
const long initialDisplayDuration = 2000;
bool initialDisplayDone = false;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // 去彈跳的時間延遲
int buttonState = LOW;             // 當前按鈕狀態
int lastButtonState = LOW;         // 上一次讀取的按鈕狀態

int page = 0;

void setup() {
  pinMode(CPUledPin, OUTPUT);  // 設定 9 號腳為輸出
  pinMode(buttonUp, INPUT);
  pinMode(buttonEnter, INPUT);
  pinMode(buttonDown, INPUT);
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 一般1306 OLED的位址都是0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.clearDisplay();  // 清除畫面
  startMillis = millis();
  // 顯示初始畫面
  display.setTextSize(2);     // 設定文字大小
  display.setTextColor(1);    // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
  display.setCursor(41, 10);  // 設定起始座標
  display.print("NKHS");      // 要顯示的字串
  display.setCursor(41, 40);  // 設定起始座標
  display.print("C201");      // 要顯示的字串
  display.display();          // 要有這行才會把文字顯示出來

  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  updateShiftRegister(0b00100001);  // 初始狀態，東西向紅燈，南北向紅燈
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long interval;

  // LED 閃爍邏輯
  if (currentMillis - ledMillis >= ledInterval) {
    ledMillis = currentMillis;
    led = !led;
    digitalWrite(CPUledPin, led);
  }

  // 根據狀態來設定持續時間
  switch (state) {
    case 0:  // 東西向紅燈
    case 2:  // 東西向綠燈
      interval = GRDuration;
      break;
    case 1:  // 東西向黃燈
    case 3:  // 南北向黃燈
      interval = YRDuration;
      break;
  }

  // 根據時間來變更燈的狀態
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    state = (state + 1) % 4;  // 共有6種狀態

    switch (state) {
      case 0:                             // 東西向紅燈
        updateShiftRegister(0b00100001);  // 東西向紅燈，南北向紅燈
        break;
      case 1:                             // 東西向黃燈
        updateShiftRegister(0b00010001);  // 東西向黃燈，南北向紅燈
        break;
      case 2:                             // 東西向綠燈
        updateShiftRegister(0b00001100);  // 東西向綠燈，南北向紅燈
        break;
      case 3:                             // 南北向紅燈
        updateShiftRegister(0b00001010);  // 東西向紅燈，南北向紅燈
        break;
    }
  }

  // 顯示選單
  if (currentMillis - startMillis >= initialDisplayDuration && !initialDisplayDone) {
    int buttonUpState = digitalRead(buttonUp);
    int buttonEnterState = digitalRead(buttonEnter);
    int buttonDownState = digitalRead(buttonDown);

    switch (page) {
      case 0:
        display.clearDisplay();
        display.setTextSize(2);     // 設定文字大小
        display.setTextColor(1);    // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
        display.setCursor(41, 4);   // 設定起始座標
        display.print("Menu");      // 要顯示的字串
        display.setCursor(47, 46);  // 設定起始座標
        display.print("GYR");       // 要顯示的字串
        display.fillRect(0, 22, 128, 21, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(47, 25);  // 設定起始座標
        display.print("FAN");       // 要顯示的字串
        display.display();          // 要有這行才會把文字顯示出來
        if (buttonEnterState == LOW)
          page = 1;
        break;
      case 1:
        display.clearDisplay();
        display.setTextSize(2);     // 設定文字大小
        display.setTextColor(1);    // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
        display.setCursor(47, 1);   // 設定起始座標
        display.print("FAN");       // 要顯示的字串
        display.setCursor(29, 33);  // 設定起始座標
        display.print("Speed2");    // 要顯示的字串
        display.setCursor(29, 49);  // 設定起始座標
        display.print("Return");    // 要顯示的字串
        display.fillRect(0, 16, 128, 16, 1);
        display.setTextColor(0);
        display.setCursor(29, 17);  // 設定起始座標
        display.print("Speed1");    // 要顯示的字串
        display.display();          // 要有這行才會把文字顯示出來
        if (buttonUpState == LOW)
          page = 3;
        if (buttonDownState == LOW)
          page = 2;
        break;
      case 2:
        display.clearDisplay();
        display.setTextSize(2);     // 設定文字大小
        display.setTextColor(1);    // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
        display.setCursor(47, 1);   // 設定起始座標
        display.print("FAN");       // 要顯示的字串
        display.setCursor(29, 17);  // 設定起始座標
        display.print("Speed1");    // 要顯示的字串
        display.setCursor(29, 49);  // 設定起始座標
        display.print("Return");    // 要顯示的字串
        display.fillRect(0, 32, 128, 16, 1);
        display.setTextColor(0);
        display.setCursor(29, 33);  // 設定起始座標
        display.print("Speed2");    // 要顯示的字串
        display.display();          // 要有這行才會把文字顯示出來
        if (buttonUpState == LOW)
          page = 1;
        if (buttonDownState == LOW)
          page = 3;
        break;
      case 3:
        display.clearDisplay();
        display.setTextSize(2);     // 設定文字大小
        display.setTextColor(1);    // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
        display.setCursor(47, 1);   // 設定起始座標
        display.print("FAN");       // 要顯示的字串
        display.setCursor(29, 17);  // 設定起始座標
        display.print("Speed1");    // 要顯示的字串
        display.setCursor(29, 33);  // 設定起始座標
        display.print("Speed2");    // 要顯示的字串
        display.fillRect(0, 48, 128, 16, 1);
        display.setTextColor(0);
        display.setCursor(29, 49);  // 設定起始座標
        display.print("Return");    // 要顯示的字串
        display.display();          // 要有這行才會把文字顯示出來
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
  blueT();
  eep();
}

void updateShiftRegister(byte data) {
  // 更新74HC595
  digitalWrite(latchPin, LOW);                  // 關閉寄存器鎖存
  shiftOut(dataPin, clockPin, MSBFIRST, data);  // 移位資料
  digitalWrite(latchPin, HIGH);                 // 開啟寄存器鎖存
}


void blueT() {//藍芽接收到訊息要做的動作
  if (BT.available() > 0) {//當收到訊息
    bs = BT.readString();


    }}




void eep() {  //以BCD的方式顯示EEPROM的內容到led0~7
  int wr, mt[2], finalout;
  wr = EEPROM.read(1);  //讀取EEPROM(位置)的資料
  //以下三行是10進位轉BCD
  mt[0] = wr / 10;  //十位數，led4~7
  mt[1] = wr % 10;  //個位數，led0~3
  finalout = mt[0] * 16 + mt[1];
  digitalWrite(latchPin, LOW);                      // 送資料前要先把 latchPin 設成低電位
  shiftOut(dataPin, clockPin, LSBFIRST, finalout);  //送出資料，170就是2進位的10101010
  digitalWrite(latchPin, HIGH);                     // 送完資料後要把 latchPin 設成高電位
}
