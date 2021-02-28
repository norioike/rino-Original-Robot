#include <ESP32Servo.h>
#include <FastLED.h>
#include "picture_preset.h"
#include "character.h"
#include <Wire.h>
#include <NimBLEDevice.h>


//PIN ASSIGN
#define SERVO 27 //IO27
#define LED_DATA_PIN 26 //IO26

//SERVO SETTING
#define MIN 900 // サーボの仕様書に記載されている最小
#define MAX 2100 //サーボの仕様書に記載されている最大パルス幅
#define INI_POSITION 100 //初期の座標
#define INTERVAL 20 //サーボモーターの制御パルスを送る間隔


//UART SETTING
#define BAUDRATE 115200

//FAST LED SETTING
#define COLOR_ORDER RGB //RGB表記なのかGRB表記なのか
#define CHIPSET     WS2812B //使用するLEDの種類
#define BRIGHTNESS 30 //基準となる明るさ
#define NUM_LEDS 280 //LEDの個数


//ICON
#define ASRADA        0x00
#define RED_FLAG      0x01
#define YELLOW_FLAG   0x02
#define OIL_FLAG      0x03
#define BLUE_FLAG     0x04
#define WHITE_FLAG    0x05
#define GREEN_FLAG    0x06
#define PENALTY_FLAG  0x07
#define ORANGE_BALL   0x08
#define BLACK_FLAG    0x09
#define CHECKER_FLAG  0x0A
#define SC_SIGNAL     0x0B
#define PIT_SIGNAL    0x0C
#define OK_SIGNAL     0x0D
#define NO_SGINAL     0x0E
#define PIT_STOP      0x0F
#define GO_SIGNAL     0x10
//emoji
#define SMILE         0x20
#define SAD           0x21
#define ANGRY         0x22
#define SHOCK         0x23


//連続で処理して、16bitの設定になっている
float Volts = 0;
float Vref = 3.3 / 2;

//時間管理
#define AUDIO_INTERVAL 50 //100msecごとにチェックする
unsigned long audioCheckTime = 0;


//オーディオはI2C
//CAN通信はSPI

Servo myservo;

CRGB leds[NUM_LEDS];//LEDの配列を作る


//BLEの情報
static NimBLEServer* pServer;

void setup() {
  Serial.begin(BAUDRATE);
  // put your setup code here, to run once:
  myservo.setPeriodHertz(50); // standard 50 hz servo
  resetPosition();
  DetachServo();
  Serial.println("complete setup");


  //LEDの設定
  FastLED.addLeds<CHIPSET, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness( BRIGHTNESS );//明るさを設定

  //I2Cの設定
  ADsetup();

  //最初に
  resetLED();//
  delay(500);//ここにdelayを入れないと、エラーする傾向がある。
  showPicture(asrada);

  //BLEの設定
  BLEsetup();


  //マルチタスク用のコード
  // コア0で関数task0をstackサイズ4096,優先順位1で起動
  xTaskCreatePinnedToCore(secondLoop, "secondLoop", 8192, NULL, 1, NULL, 0);
  //xTaskCreatePinnedToCore(タスクの関数名,"タスク名",スタックメモリサイズ,NULL,タスク優先順位,タスクハンドルポインタ,Core ID);
  xTaskCreatePinnedToCore(CheckAudio, "CheckAudio", 8192, NULL, 1, NULL, 1);

  //  showMessage("Hello", 500);

}

//コアは処理が入ってないと落ちるっぽい



//LEDを動かすよ
void CheckAudio(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    //オーディオをチェックする
            if (millis() - audioCheckTime >= AUDIO_INTERVAL) {
              checkAudioSignal();
              audioCheckTime = millis();
            }
//    testSigmoid();
//        wingTest();
  }
}


void loop() {//空白でもいいんじゃない説

//  testSigmoid();
  //  wingTest();
  //  static int cnt = 0;
  //  printf("Maintask thread_cnt=%ld\n", cnt++);
  //  delay(1200);
  //  showTest();

}


//サーボモーターを動かすよ
void secondLoop(void *pvParameters) {
  (void)pvParameters;

  while (1) {
        wingTest();
//    showMessage("Ogaki Mini Maker Faire 2020!", 30, 0, 0x00AA00FF); // 引数は(文字,１ステップ移動の間隔,リピート回数,LEDの色)
  }
}



void testSigmoid() {
  moveServoSigmoid(30, 1500);
  delay(500);
  moveServoSigmoid(150, 1500);
  delay(500);
}


void wingTest() {
  showVoiceLED(0);
  delay(200);
  showVoiceLED(1);
  delay(200);
  showVoiceLED(2);
  delay(200);
  showVoiceLED(3);
  delay(200);
  showVoiceLED(2);
  delay(200);
  showVoiceLED(1);
  delay(200);
  showVoiceLED(2);
  delay(200);
  showVoiceLED(1);
  delay(200);
}

void showTest() {
  for (int i = 0; i < 18; i++) {
    CheckPicture(i);
    delay(3000);
  }

  for (int j = 0x20; j < 0x24; j++) {
    CheckPicture(j);
    delay(1000);
  }


  //  showPicture(ok_sign);
  //  delay(1000);
  //  showPicture(no_sign);
  //  delay(1000);
  //  showPicture(blue_flag);
  //  delay(1000);
  //  showPicture(pit_sign);
  //  delay(1000);
  //  showPicture(stop_mark);
  //  delay(1000);


}
