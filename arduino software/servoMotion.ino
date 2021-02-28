
//servo control
void moveServoSigmoid(int servo, int motionTime) {
  if (myservo.attached() == false) { //attachされていことを確認
    AttachServo();//attatchされていなかったらattachする
  }

  if (motionTime == 0) {
    myservo.write(servo);
    return;
  }
  //リミッターを通す
  servo = CheckDeg(servo);
  int targetPulsemyservo = map(servo, 0, 180, MIN, MAX); //map関数を使って、サーボモーターの角度をパルス幅に変換

  int pulseNowmyservo = myservo.readMicroseconds();
//  Serial.println(pulseNowmyservo);

  //ここで必要なパルスの幅を計算する
  int msPulsemyservo = abs(targetPulsemyservo - pulseNowmyservo);

  //シグモイド
  //-100から100の決められた数字の中でモーションを間引く
  //式は result = 1/(1 + (exp(-0.12 * x)));

  //それぞれのステップ数を決める
  double motionStep = 200 / (motionTime / INTERVAL); //ゴールまでのステップが決まっていて、それに対してintervalで割る
  double alpha = -0.12;

  for (double i = -100; i < (motionTime / INTERVAL); i = i + motionStep) {
    //各サーボの制御を行う
    //Head_Yaw
    if (pulseNowmyservo > targetPulsemyservo) {
      myservo.writeMicroseconds(pulseNowmyservo - msPulsemyservo / (1 + (exp(alpha * i))));
    } else if (pulseNowmyservo < targetPulsemyservo) {
      myservo.writeMicroseconds(pulseNowmyservo + msPulsemyservo / (1 + (exp(alpha * i))));
    } else {
      //do nothing
    }
    //最後にdelay(1パルス分の20msec)する。
    delay(INTERVAL);
  }

  //  DetachServo();
}


// servo method
void AttachServo() {
  myservo.attach(SERVO, MIN, MAX);
}

void DetachServo() {
  delay(300);//すぐにdetachするとうまくいかないので、少しdelayを置いている。
  myservo.detach();
}

//入力値がおかしい時に修正するリミッター関数
int CheckDeg(int ServoDeg) {
  if (ServoDeg > 180) {
    ServoDeg = 180;//一旦初期値の変更をする
  } else if (ServoDeg < 0) {
    ServoDeg = 0;
  }
  return ServoDeg;
}

//初期位置に1500msecで移動する
void resetPosition() {
  moveServoSigmoid(INI_POSITION, 1500);
}


//いくつか作ったプリセット
void presetMotion(int val) {
  switch (val) {
    case SMILE:
      resetPosition();
      break;
    case SAD:
      moveServoSigmoid(INI_POSITION + 80, 1000);
      moveServoSigmoid(INI_POSITION - 80, 1500);
      moveServoSigmoid(INI_POSITION + 80, 1000);
      resetPosition();
      break;
    case ANGRY:
      moveServoSigmoid(INI_POSITION + 30, 300);
      moveServoSigmoid(INI_POSITION - 30, 600);
      moveServoSigmoid(INI_POSITION + 30, 600);
      moveServoSigmoid(INI_POSITION - 30, 600);
      moveServoSigmoid(INI_POSITION + 30, 600);
      moveServoSigmoid(INI_POSITION - 30, 600);
      resetPosition();
      break;
    case SHOCK:
      moveServoSigmoid(INI_POSITION + 60, 800);
      moveServoSigmoid(INI_POSITION - 60, 1600);
      moveServoSigmoid(INI_POSITION + 60, 1600);
      moveServoSigmoid(INI_POSITION - 60, 1600);
      moveServoSigmoid(INI_POSITION + 60, 1600);
      moveServoSigmoid(INI_POSITION - 60, 1600);
      resetPosition();
      break;
    default:
      break;
  }
}
