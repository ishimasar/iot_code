#include <IOXhop_FirebaseESP32.h>
// Set these to run example.
#define WIFI_SSID "HUMAX-78BB8"
#define WIFI_PASSWORD "LECkLTFJXmX5L"
#define FIREBASE_DB_URL "https://self-line-notify-default-rtdb.firebaseio.com/"
#define FIREBASE_DB_DATA_PATH "/messages"
#define FIREBASE_DB_DATA_KEY "/mobile1ToHome"

#include <ESP32_SD_ShinonomeFNT.h>
#include <ESP32_SD_UTF8toSJIS.h>
#include <M5Stack.h>
#include <WiFi.h>

const char* UTF8SJIS_file = "/font/Utf8Sjis.tbl";
const char* Shino_Zen_Font_file = "/font/shnmk16.bdf";
const char* Shino_Half_Font_file = "/font/shnm8x16.bdf";
const uint8_t CS_SD = 4;
const uint8_t PIXEL_SIZE = 2; //拡大倍率
const uint8_t ROW_MARGIN = 2; //行間
const uint8_t ROW_WIDTH = 320 / 8 / PIXEL_SIZE;

ESP32_SD_ShinonomeFNT SFR(CS_SD, 24000000);
ESP32_SD_UTF8toSJIS UTS;

void shinonomePrint(int x, int y, uint8_t buf[][16], int txtMax, uint16_t color) {
  for (int i=0; i <txtMax; i++) {
    for (int j=0; j < 16; j++) {
      for (int k=0; k < 8; k++)  {
        if(bitRead(buf[i][j],7-k)) {
          M5.Lcd.fillRect((x + 8 * i + k) * PIXEL_SIZE,  (y + j) * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, color);
        }
      }
    }
  }
}

void clearScreen(){
    uint8_t font_buf[20][16];
    uint16_t sj_length;
    M5.Lcd.fillScreen(BLACK);
    SFR.SD_Shinonome_Init3F(UTF8SJIS_file, Shino_Half_Font_file, Shino_Zen_Font_file);
    sj_length = SFR.StrDirect_ShinoFNT_readALL("とどいていません", font_buf);
    SFR.SD_Shinonome_Close3F();
    shinonomePrint(15, 50, font_buf, sj_length, TFT_WHITE);
}

void showMessage(String data){

  uint16_t sj_length, pos = 0;
  uint8_t sj_txt[1024];
  uint8_t row = 0, cp = 0, doLater = 0;
  uint8_t buf[2][16] = {0};

  M5.Lcd.fillScreen(WHITE);

  SFR.SD_Shinonome_Init3F(UTF8SJIS_file, Shino_Half_Font_file, Shino_Zen_Font_file);
  sj_length = UTS.UTF8_to_SJIS(data, sj_txt);
  Serial.print("sj_length: ");
  Serial.println(sj_length);
  
  do{ //行のループ
    int column = 0;
    while(column < ROW_WIDTH){ //文字のループ

      if(doLater == 1){ //前行の末尾の文字をまだ出力してない場合はここで出力
        shinonomePrint(column * 8, row * (16 + ROW_MARGIN) + ROW_MARGIN, buf, cp, TFT_BLACK);
        column += cp;
        doLater = 0;
      }
      
      cp = SFR.Sjis_inc_FntRead(sj_txt, sj_length, &pos, buf);
      
      if(column == ROW_WIDTH -1 && cp == 2){ //残り1文字のとき全角文字が来てしまったら次の行に回す
        doLater = 1;
      } else {
        shinonomePrint(column * 8, row * (16 + ROW_MARGIN) + ROW_MARGIN , buf, cp, TFT_BLACK);
        column += cp; 
      }
      
      if(pos == 0){break;} //Sjis_inc_FntReadの結果、posに0を入っていたら全文字終わったということ
    }
    row++;
  } while(pos > 0);
  
  SFR.SD_Shinonome_Close3F();
  M5.Speaker.tone(440, 200);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  delay(500);
  
  Firebase.begin(FIREBASE_DB_URL);
  
  Firebase.stream(FIREBASE_DB_DATA_PATH, [](FirebaseStream stream) {
    String eventType = stream.getEvent();
    eventType.toLowerCase();
     
    Serial.print("event: ");
    Serial.println(eventType);
    if (eventType == "put") {
      Serial.print("json: ");
      Serial.println(stream.getDataString());
      String path = stream.getPath();
      String data = stream.getDataString();
      if (path.equals(FIREBASE_DB_DATA_KEY)) {
        Serial.print("data: ");
        Serial.println(data);
        showMessage(data);
      }
    }
  });

  M5.begin();
  clearScreen();
}

void loop() {
  if(M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
    clearScreen();
  }
  M5.update();
}
