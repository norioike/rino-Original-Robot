//LEDの個数
#define L_LEVEL 5　//インジケータがLの時に光るLEDの個数
#define M_LEVEL 9 //インジケータがMの時に光るLEDの個数
#define H_LEVEL 12 //インジケータがHの時に光るLEDの個数

#define MATRIX_X 16  //LEDマトリクスのX軸の方向の個数
#define MATRIX_Y 16  //LEDマトリクスのY軸方向の個数

void showVoiceLED(int val) {
  int startLeft = 256;//256番目のLEDがLEDだと
  int startRight = 268; //268番目から
  int level = 0;

  if (val == 0) {
    for (int i = 0; i < H_LEVEL; i++) {
      leds[startLeft + i] = CRGB::Black;
      leds[startRight + i] = CRGB::Black;
    }
  } else {
    switch (val) {
      case 1:
        level = L_LEVEL;
        break;
      case 2:
        level = M_LEVEL;
        break;
      case 3:
        level = H_LEVEL;
        break;
    }
    for (int i = 0; i < level; i++) {
      leds[startLeft + i] = CRGB::White;
      leds[startRight + i] = CRGB::White;
    }
    for (int i = level; i < H_LEVEL; i++) {
      leds[startLeft + i] = CRGB::Black;
      leds[startRight + i] = CRGB::Black;
    }

  }
  FastLED.show();
  //  FastLED.delay(5);
}

//LEDマトリクスに表示する絵を呼び出す(picture_preset.h)をチェック
void showPicture(uint32_t array[]) {
  for (int i = 0; i < (MATRIX_X * MATRIX_Y); i++) {

    leds[i] = CRGB(byte(array[i] >> 16), byte(array[i] >> 8), byte(array[i]));

  }
  FastLED.show();
}


void showMessage(String message, int shift_speed, int repeats, uint32_t color) {

  //  Serial.print("message is :");
  //  Serial.println(message);
  //  //messageの長さをチェック
  int length = message.length();//文字数を取得
  //  Serial.print("length is :");
  //  Serial.println(length);
  char msg[length];
  int matrix_length = 0;

  for (int i = 0; i < length; i++) {
    msg[i] = message.charAt(i) - 0x20;//1文字ずつ格納 番地だけがわかっている
    //    Serial.println(msg[i], DEC); //1つずつ取れている。0x20ずらすことで、character.h と一致する
    matrix_length += Character[(message.charAt(i) - 0x20) * 6]; //七番目は幅を表しているので、これをやると、幅がわかる
    if (i != length - 1) {
      matrix_length++;
    }
  }
  //  Serial.print("matrix length is:");
  //  Serial.println(matrix_length);//matrixの長さがわかる。
  //するとこいつは5 x matrix_lengthの配列とみなせる。
  byte matrix[matrix_length + 1]; //配列が作れるようになる
  int matrixAddress = 0;
  byte character_array_length = 6;
  for (int j = 0; j < length; j++) {
    int part_length_info = msg[j] * character_array_length;
    //    Serial.print("character length is:");
    //    Serial.println(Character[part_length_info]);

    for (int k = 1; k <= Character[part_length_info]; k++) {
      matrix[matrixAddress] = Character[part_length_info + k];
      //      Serial.println(matrix[matrixAddress], BIN); //これ見るとちゃんと取れてるね。
      matrixAddress++;
    }
    matrix[matrixAddress] = B00000000; //これは空白
    if (matrixAddress != matrix_length) {
      matrixAddress++;
    }
  }
  //  Serial.print("matrixAddress:");
  //  Serial.println(matrixAddress);
  //  Serial.println("check string");

  //実際にスクロールしてみる
  int scroll = (MATRIX_X * 2) + matrix_length; //
  int height = 8;//16列目を座標の原点とする？

  //今度は配列を作る
  int showDataArray[height][scroll] = {0};
  for (int l = 0; l < scroll; l++) {

    if (l < MATRIX_X) {
      for (int m = 0; m < height; m++) {
        showDataArray[m][l] = 0;
      }
    } else if (l <= MATRIX_X + matrix_length) {
      byte on_off_array[] = {(matrix[l - MATRIX_X]), (matrix[l - MATRIX_X] >> 1), (matrix[l - MATRIX_X] >> 2), (matrix[l - MATRIX_X] >> 3), (matrix[l - MATRIX_X] >> 4), (matrix[l - MATRIX_X] >> 5), (matrix[l - MATRIX_X] >> 6), (matrix[l - MATRIX_X] >> 7)}; //これが一列
      for (int n = 0; n < height; n++) {
        if (on_off_array[n]&B00000001 == true) {
          showDataArray[n][l] = 1;
        } else {
          showDataArray[n][l] = 0;
        }
      }
    } else {
      for (int m = 0; m < height; m++) {
        showDataArray[m][l] = 0;
      }
    }
    Serial.print(showDataArray[0][l], BIN);
    Serial.print(showDataArray[1][l], BIN);
    Serial.print(showDataArray[2][l], BIN);
    Serial.print(showDataArray[3][l], BIN);
    Serial.print(showDataArray[4][l], BIN);
    Serial.print(showDataArray[5][l], BIN);
    Serial.print(showDataArray[6][l], BIN);
    Serial.println(showDataArray[7][l], BIN);
  }

  //  Serial.println("start shows");

  resetLED();

  int base_y = 4;//

  for (int rep = 0 ; rep <= repeats; rep++) {
    for (int shift = 0; shift <= (scroll - MATRIX_X); shift++) { //全体のシフト
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < MATRIX_X; x++) {
          if (showDataArray[y][shift + x] == true) {
            leds[(MATRIX_X * (y + base_y)) + x] = CRGB(byte(color >> 8), byte(color >> 16), byte(color)); //関数は
          } else {
            leds[(MATRIX_X * (y + base_y)) + x] = CRGB::Black;
          }
        }
      }

      FastLED.show();
      delay(shift_speed);
    }
  }
}


//すべてのLEDをOFFにする
void resetLED() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(300);
}


//プリセットのPictureを呼び出す
void CheckPicture(int num) {
  Serial.print("call here and icon number is :");
  Serial.println(num);
  switch (num) {
    case ASRADA:
      showPicture(asrada);
      break;
    case RED_FLAG:
      showPicture(red_flag);
      break;
    case YELLOW_FLAG:
      showPicture(yellow_flag);
      break;
    case OIL_FLAG:
      showPicture(oil_flag);
      break;
    case BLUE_FLAG:
      showPicture(blue_flag);
      break;
    case WHITE_FLAG:
      showPicture(white_flag);
      break;
    case GREEN_FLAG:
      showPicture(green_flag);
      break;
    case PENALTY_FLAG:
      showPicture(penalty_flag);
      break;
    case ORANGE_BALL:
      showPicture(orange_ball);
      break;
    case BLACK_FLAG:
      showPicture(black_flag);
      break;
    case CHECKER_FLAG:
      showPicture(checker_flag);
      break;
    case SC_SIGNAL:
      showPicture(safety_car);
      break;
    case PIT_SIGNAL:
      showPicture(pit_in);
      break;
    case OK_SIGNAL:
      showPicture(ok_sign);
      break;
    case NO_SGINAL:
      showPicture(no_sign);
      break;
    case PIT_STOP:
      showPicture(pit_stop);
      break;
    case GO_SIGNAL:
      showPicture(go_sign);
      break;
    case SMILE:
      showPicture(smile);
      presetMotion(SMILE);
      break;
    case SAD:
      showPicture(sad);
      presetMotion(SAD);
      break;
    case ANGRY:
      showPicture(angry);
      presetMotion(ANGRY);
      break;
    case SHOCK:
      showPicture(shock);
      presetMotion(SHOCK);
      break;
    default:
      break;
  }

}
