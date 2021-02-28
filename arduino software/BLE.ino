
#define GATT "0000FE03-0000-1000-8000-00805F9B34FB"
#define TX "F04EB177-3005-43A7-AC61-A390DDF83076"
#define RX "2BEEA05B-1879-4BB4-8A2F-72641F82420B"


NimBLECharacteristic* TxCharacteristic;
NimBLECharacteristic* RxCharacteristic;

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
      Serial.println("Client connected");
      Serial.println("Multi-connect support: start advertising");
      NimBLEDevice::startAdvertising();
    };
    /** Alternative onConnect() method to extract details of the connection.
        See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
    */
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
      Serial.print("Client address: ");
      Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
      /** We can use the connection handle here to ask for different connection parameters.
          Args: connection handle, min connection interval, max connection interval
          latency, supervision timeout.
          Units; Min/Max Intervals: 1.25 millisecond increments.
          Latency: number of intervals allowed to skip.
          Timeout: 10 millisecond increments, try for 5x interval time for best results.
      */
      pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };
    void onDisconnect(NimBLEServer* pServer) {
      Serial.println("Client disconnected - start advertising");
      NimBLEDevice::startAdvertising();
    };

    /********************* Security handled here **********************
    ****** Note: these are the same return values as defaults ********/
    bool onConfirmPIN(uint32_t pass_key) {
      Serial.print("The passkey YES/NO number: ");
      Serial.println(pass_key);
      /** Return false if passkeys don't match. */
      return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc) {
      /** Check that encryption was successful, if not we disconnect the client */
      if (!desc->sec_state.encrypted) {
        NimBLEDevice::getServer()->disconnect(desc->conn_handle);
        Serial.println("Encrypt connection failed - disconnecting client");
        return;
      }
      Serial.println("Starting BLE work!");
    };
};



/** Handler class for characteristic actions */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) {
      Serial.print(pCharacteristic->getUUID().toString().c_str());
      Serial.print(": onRead(), value: ");
      Serial.println(pCharacteristic->getValue().c_str());//生データを取得する
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
      Serial.print(pCharacteristic->getUUID().toString().c_str());
      Serial.print(": onWrite(), value: ");
      const char* char_val = pCharacteristic->getValue().c_str(); //data を使うとstring 型　c_str()でconst char* 型ポインタ
      String str_val = pCharacteristic->getValue().data(); //data を使うとstring 型　c_str()でconst char* 型ポインタ

      //call command
      CheckBLECommand(char_val, str_val);

    };
    /** Called before notification or indication is sent,
        the value can be changed here before sending if desired.
    */
    void onNotify(NimBLECharacteristic* pCharacteristic) {
      Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
        The value returned in code is the NimBLE host return code.
    */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
      String str = ("Notification/Indication status code: ");
      str += status;
      str += ", return code: ";
      str += code;
      str += ", ";
      str += NimBLEUtils::returnCodeToString(code);
      Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
      String str = "Client ID: ";
      str += desc->conn_handle;
      str += " Address: ";
      str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
      if (subValue == 0) {
        str += " Unsubscribed to ";
      } else if (subValue == 1) {
        str += " Subscribed to notfications for ";
      } else if (subValue == 2) {
        str += " Subscribed to indications for ";
      } else if (subValue == 3) {
        str += " Subscribed to notifications and indications for ";
      }
      str += std::string(pCharacteristic->getUUID()).c_str();

      Serial.println(str);
    };
};

/** Handler class for descriptor actions */
class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
      std::string dscVal((char*)pDescriptor->getValue(), pDescriptor->getLength());
      Serial.print("Descriptor witten value:");
      Serial.println(dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor) {
      Serial.print(pDescriptor->getUUID().toString().c_str());
      Serial.println(" Descriptor read");
    };
};


/** Define callback instances globally to use for multiple Charateristics \ Descriptors */
static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;


void BLEsetup() {
  /** sets device name */
  NimBLEDevice::init("AstRo-1");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); // use passkey
  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |BLE_SM_PAIR_AUTHREQ_SC*/BLE_SM_PAIR_AUTHREQ_SC );

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* AlexaGattService = pServer->createService(GATT);
  TxCharacteristic = AlexaGattService->createCharacteristic(
                       TX,
                       NIMBLE_PROPERTY::WRITE |
                       NIMBLE_PROPERTY::WRITE_ENC   // only allow writing if paired / encrypted
                     );
  TxCharacteristic->setCallbacks(&chrCallbacks);

  RxCharacteristic = AlexaGattService->createCharacteristic(
                       RX,
                       NIMBLE_PROPERTY::READ |
                       NIMBLE_PROPERTY::NOTIFY |
                       NIMBLE_PROPERTY::READ_ENC
                     );
  RxCharacteristic->setCallbacks(&chrCallbacks);//コールバックに行くようにする。

  /** Note a 0x2902 descriptor MUST NOT be created as NimBLE will create one automatically
      if notification or indication properties are assigned to a characteristic.
  */

  /** Start the services when finished creating all Characteristics and Descriptors */
  AlexaGattService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  /** Add the services to the advertisment data **/
  pAdvertising->addServiceUUID(AlexaGattService->getUUID());
  /** If your device is battery powered you may consider setting scan response
      to false as it will extend battery life at the expense of less data sent.
  */
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  Serial.println("Advertising Started");
}


void testRead() {
  RxCharacteristic->setValue(0x123456);
  RxCharacteristic->notify();
}



#define MOVE_SERVO 0x01
#define SHOW_ICON 0x02
#define SHOW_MESSAGE 0x03

//プロトコルを定義
/*
    0byte -> コマンド
    1byte ->　長さ(byteの長さ)
    2byte~ -> Payload
*/

void CheckBLECommand(const char* raw, String value) {

  char res[value.length()];
  value.toCharArray(res, value.length());
  if (sizeof(res) <= 2) { //エラーや
    return;
  }

  int command = raw[0];
  int payload_length = raw[1];
  Serial.println("size check");
  Serial.println(payload_length);
  Serial.println(sizeof(res));

  if (payload_length + 2 != sizeof(res)) {
    //おかしなデータなのでreturnする
    Serial.println("size error");
    return;
  }
  Serial.println(command);
  Serial.println(res[2], HEX);


  switch (command) {
    case MOVE_SERVO:
      if (sizeof(res) != 5) {
        return;
      }
      Serial.print("angle :");
      Serial.print((int)value[2]);
      Serial.print(",time :");
      Serial.println((int)((value[3] << 8) + value[4]));
      moveServoSigmoid((int)value[2], (int)((value[3] << 8) + value[4]));
      break;

    case SHOW_ICON:
      if (sizeof(res) != 3) {
        return;
      }
      CheckPicture(raw[2]);
      break;
    default:
      break;
  };
}
