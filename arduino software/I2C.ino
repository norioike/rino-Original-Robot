//MCP3425(AD変換)の設定
#define MCP3425  0x68
#define REGISTOR 0b10011000
#define ALPHA 0.1
float before = 0;
byte wingLevel = 0;
bool firstTime = true;

#define DOWN -1
#define UP 1

#define ERROR_SIZE 50000

void ADsetup() {
  //I2Cの設定
  Wire.begin();
  Wire.beginTransmission(MCP3425);
  Wire.write(REGISTOR);
  Wire.endTransmission();
}

float checkAudioSignal() {
  Wire.requestFrom(MCP3425, 2);//2byte分のデータを取得する。
  while (Wire.available() < 2) {}
  float value =  (Wire.read() << 8) + Wire.read();

  if (value >= ERROR_SIZE) {
    return 0;
  }

  float res = ALPHA * before + (1 - ALPHA) * value;

  if (firstTime) { //初回時のLED点灯回避
    before = value;
    firstTime = false;
  }
  ConvertLEDLevel(res, before);  //ここでLEDのレベルを分類する。
  before = value;
  Serial.println(res);
}

void ConvertLEDLevel(float value, float before) {
  float sound_res  = value - before;
  if (sound_res <= DOWN) {
     if (wingLevel > 0 ) {
      wingLevel--;

      if(wingLevel == 0 && value > 0){
        wingLevel = 1;
      }
    }
  } else if (sound_res >= UP) {
    if (wingLevel < 3) {
      wingLevel++;
    }
  } else if (sound_res == 0) {
    if (wingLevel > 0) {
      wingLevel--;
    }
  }

  showVoiceLED(wingLevel);
}
