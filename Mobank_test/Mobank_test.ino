#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Test"
#define WIFI_PASSWORD "Anonymous@123"
#define API_KEY "AIzaSyB-zcwprPPuYw85YOB0phYmvh1tq3zmBqE"
#define DATABASE_URL "https://mobank-9efbd-default-rtdb.asia-southeast1.firebasedatabase.app/"

//Infrared PIN
int IR_IN = 34;
int IR_OUT = 35;

int LED_PIN = 2;

//Stepper PIN
const int dirPin = 13;
const int stepPin = 14;
int stepsPerRevolution = 200;

FirebaseData fbdo;
FirebaseAuth auth;  
FirebaseConfig config;

unsigned long senDataPrevMillis = 0;
bool signupOK = false;
bool kondisi_luar;
bool kondisi_dalam;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(IR_IN, INPUT); //INPUT_PULLUP * INPUT_PULLDOWN
  pinMode(IR_OUT, INPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.print("Connecting ti Wi-Fi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("signUp OK");
    signupOK = true;
  }else{
    Serial.printf("%s/n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  digitalWrite(LED_PIN, HIGH);
}

int kondisi_reject;

void loop() {
  if(Firebase.ready() && signupOK && (millis() - senDataPrevMillis > 5000 || senDataPrevMillis == 0)){
    senDataPrevMillis = millis();

    //IR_OUT
    int sensorValue1 = analogRead(IR_OUT);
    float distance1 = sensorValue1 / 9.766; //convert sensor value to distance
    Serial.print("Distance IN: ");
    Serial.println(distance1);

    if(distance1 < 100){
      kondisi_luar = true;
    }else{
      kondisi_luar = false;
    }

    if (Firebase.RTDB.setBool(&fbdo, "data_mobank/IR_OUT/", kondisi_luar)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //IR_IN
    int sensorValue2 = analogRead(IR_IN);
    float distance2 = sensorValue2 / 9.766; //convert sensor value to distance
    Serial.print("Distance OUT: ");
    Serial.println(distance2);

    if(distance2 < 100){
      kondisi_dalam = true;
    }else{
      kondisi_dalam = false;
    }
    if (Firebase.RTDB.setBool(&fbdo, "data_mobank/IR_IN/", kondisi_dalam)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
      else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    // Proses Konveyor
    if(distance1 < 100 && distance2 > 100){
      stepsPerRevolution = 20000000;
      digitalWrite(dirPin, HIGH);
      for(int x = 0; x < stepsPerRevolution; x++)
      {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(500);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(500);
        sensorValue1 = analogRead(IR_OUT);
        distance1 = sensorValue1 / 9.766;
        sensorValue2 = analogRead(IR_IN);
        distance2 = sensorValue2 / 9.766; //convert sensor value to distance
        if(/*distance1 > 100 ||*/ distance2 < 100){
          kondisi_dalam = true;
          stepsPerRevolution = 0;
        }else{
          kondisi_dalam = false;
        }
      }
    }
    else if(distance1 > 100 && distance2 > 100){
      stepsPerRevolution = 0;
    }
    else if(distance1 > 100 || distance2 < 100){
      stepsPerRevolution = 0;
    }

    //Proses accept/reject
    if (Firebase.RTDB.getInt(&fbdo, "data_mobank/Uang_reject/")){
      kondisi_reject = fbdo.intData();
    }

    switch(kondisi_reject){
      case 1:
        kondisi_dalam = false;
        if (Firebase.RTDB.setBool(&fbdo, "data_mobank/IR_IN/", kondisi_dalam)){
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        }
        else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }

        stepsPerRevolution = 1800;
        digitalWrite(dirPin, HIGH);
        for(int x = 0; x < stepsPerRevolution; x++){
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(500);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(500);
        }

        stepsPerRevolution = 0;
        kondisi_reject = 3;
        if (Firebase.RTDB.setInt(&fbdo, "data_mobank/Uang_reject/", kondisi_reject)){
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        }
        else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
      break;

      case 2:
        stepsPerRevolution = 1600;
        digitalWrite(dirPin, LOW);
        for(int x = 0; x < stepsPerRevolution; x++){
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(500);
          digitalWrite(stepPin, LOW);
          delayMicroseconds(500);
        }
        
        stepsPerRevolution = 0;
        kondisi_reject = 3;
        if (Firebase.RTDB.setInt(&fbdo, "data_mobank/Uang_reject/", kondisi_reject)){
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        }
        else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
      break;

      default:
        stepsPerRevolution = 0;
        kondisi_reject = 3;
        if (Firebase.RTDB.setInt(&fbdo, "data_mobank/Uang_reject/", kondisi_reject)){
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
        }
        else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
    }
  }
//endl
}