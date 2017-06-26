#include <Nefry.h>
//#include <Adafruit_NeoPixel.h>//included in Nefry.h
#include <ArduinoJson.h>
#include "sound.hpp"

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NUMPIXELS      3
#define PIN            2

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define SOUND_PIN 5

char states[2] = {0};
enum ENUM_STATE{
  NON,
  GREEN,
  RED,
  YELLOW,
  BLUE
};

bool blink = true;

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setPixelColor(0, pixels.Color(0,0,0));
  pixels.setPixelColor(1, pixels.Color(0,0,0));
  pixels.setPixelColor(2, pixels.Color(0,0,0));
  pixels.show();
  Serial.begin(115200);

  // Nefryパラメータ
  Nefry.setConfHtmlStr("JenkinsURL",0); // JenkinsのURL
  Nefry.setConfHtmlStr("ProjectName",1);// プロジェクト名
  
  Nefry.setProgramName("JenkinsXFD");   // プログラム名
  Nefry.setIndexLink("JenkinsXfd","/JenkinsXfd");  // menuにLink追加
  
  // ページアクセス時の動作
  Nefry.getWebServer()->on("/JenkinsXfd", [&]() {
    ESP8266WebServer* server = Nefry.getWebServer();
    // 応答
    server->send(200, "text/html", "OK");
    
    // URL引数
    String action = server->arg("action");
    String jenkinsStatus = "";
    if(action == "start"){
      // build開始で青点滅
      states[2] = states[1];
      states[1] = states[0];
      states[0] = BLUE;
      blink = true;
    }else{
      // 状態取得
      Nefry.ndelay(1000);
      jenkinsStatus = getJenkinsStatus();
    }
    // 200:OK 応答送信
    String content = F(
      "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
      "<title>JenkinsXfd</title>"
      "</head><body><div><h1>JenkinsXfd</h1><br/><pre>");
      content += jenkinsStatus;
      content += F("</pre><br/><a href=\"/\">Back to top</a></div></body></html>");
//    Nefry.getWebServer()->send(200, "text/html", content);
  });

  // 初回の状態更新
  getJenkinsStatus();
}

void loop() {
  Nefry.ndelay(500);
  Serial.println("loop()");
  showNeoPixel();
}

void showNeoPixel(){
  // NeoPixel表示
  for(int i = 0; i <= 2; i++){
    int r=0,g=0,b=0;
    switch (states[i]) {
      case GREEN: g=255; break;
      case RED:   r=255; break;
      case YELLOW:r=g=255; break;
      case BLUE:
        if(blink)b=255;
        blink = blink ? false : true;
        break;
      case NON: break; // all 0
      default: break;
    }
    pixels.setPixelColor(2-i, pixels.Color(r,g,b));
  }
  pixels.show();
  
}

// JenkinsからBuild結果の取得
String getJenkinsStatus(){
  const int BUFFER_SIZE = JSON_OBJECT_SIZE(50) + JSON_ARRAY_SIZE(1);
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  String body = "";
  HTTPClient http;
  String jenkinsURL = Nefry.getConfStr(0); // JenkinsのURL
  String projectName = Nefry.getConfStr(1);// プロジェクト名
  
  String host = jenkinsURL + "/job/" + projectName + "/api/json?depth=1&tree=builds[id,result]{0,3}";
  Serial.println(host);
  http.begin(host);
  int httpCode = http.GET();
  Nefry.ndelay(100);

  if (httpCode < 0) {
    body = http.errorToString(httpCode);
  } else {
    body = http.getString();
  }
  http.end();
  
  // jsonデコード
  JsonObject& root = jsonBuffer.parseObject(body);
  if (!root.success()) {
    Serial.println("parseObject() failed");
  }
  
  for(int i = 0; i <= 2; i++){
    String id = root["builds"][i]["id"];
    String result = root["builds"][i]["result"];
    Serial.println(id);
    Serial.println(result);

    if(result == "SUCCESS"){
      Nefry.println("SUCCESS");
      states[i] = GREEN;
    }else if(result == "UNSTABLE"){
      Nefry.println("UNSTABLE");
      states[i] = YELLOW;
    }else{
      // FAILURE,ABORTED,NOT_BUILT
      Nefry.println("BuildFail");
      states[i] = RED;
    }
  }

  showNeoPixel();

  // 音を鳴らす
  if(GREEN == states[0]){
    PlayOkSounds(SOUND_PIN);
  }else{
    PlayNgSounds(SOUND_PIN);
  }
    
  Nefry.println(body);
  Nefry.println("End /JenkinsXfd");
  return body;
}

