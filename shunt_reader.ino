#include "SPI.h"
#include "TFT_eSPI.h"     // Hardware-specific library
#include "EspMQTTClient.h"
#include "esp_task_wdt.h"

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
#include "Free_Fonts.h"

// Include the header files that contain the icons
#include "image1.h"
#include "image2.h"
#include "image3.h"
#include "image4.h"
#include "carica0.h"
#include "carica1.h"
#include "carica2.h"
#include "carica3.h"
#include "disconnected.h"
#include "esp32.h"

float SOC;
float LastSOC;
float Voltage;
float LastVoltage;
float Current;
float LastCurrent;
float Power;
float LastPower;
float PVPower = 1;
float LastPVPower;
int error = 0;
int ledPin = 5;
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
unsigned long then = 0;
unsigned long intervals[6] = { 500, 500, 500, 500, 500 };
int current_interval_index = 0;

#define WDT_TIMEOUT 60

EspMQTTClient client(
  "IoT-WiFi-Casa",
  "123stella",
  "192.168.0.91",  // MQTT Broker server ip
  "andrea",   // Can be omitted if not needed
  "elsapp09",   // Can be omitted if not needed
  "ShuntReaderDisplay",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);


void setup()
{
  //Serial.begin(115200);
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch


  // Optional functionalities of EspMQTTClient
  //client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  //client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

  tft.begin();
  tft.setRotation(0);	// landscape
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true); // Swap the colour byte order when rendering
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(ledChannel, 20);
  tft.pushImage(0, 0, esp32Width, esp32Height, esp32);
  delay(5000);
}

void onConnectionEstablished()
{
  client.subscribe("solar_assistant/battery_1/state_of_charge/state", [](const String & payload) {
    SOC = payload.toFloat();
  });

  client.subscribe("solar_assistant/battery_1/current/state", [](const String & payload) {
    Current = payload.toFloat();
  });

  client.subscribe("solar_assistant/battery_1/voltage/state", [](const String & payload) {
    Voltage = payload.toFloat();
  });

  client.subscribe("solar_assistant/battery_1/power/state", [](const String & payload) {
    Power = payload.toFloat();
  });

  client.subscribe("solar_assistant/inverter_1/pv_power/state", [](const String & payload) {
    PVPower = payload.toFloat();
  });
}

void WiFiError() {
  if (error == 1) {
    ledcWrite(ledChannel, 20);
    tft.pushImage(0, 0, disconnectedWidth, disconnectedHeight, disconnected);
    delay(5000);
    client.loop();
    //Serial.println("connessione KO");
  }
}

void connessioneOk() {
  if (error == 1) {
    tft.fillScreen(TFT_BLACK);
    //tft.fillScreen(TFT_WHITE);
    tft.drawRect(0, 0, 239, 208, TFT_NAVY); //rettandolo soc
    tft.drawRect(0, 208, 161, 55, TFT_NAVY); //rettangolo A
    tft.drawRect(0, 263, 161, 56, TFT_NAVY);  //rettangolo W
    tft.drawRect(161, 208, 78, 55, TFT_NAVY); //rettangolo V
    tft.drawRect(161, 263, 78, 56, TFT_NAVY); //rettangolo PVPower
    LastSOC = -1;
    LastVoltage = -1;
    LastCurrent = -1;
    LastPower = -1;
    LastPVPower = -1;
    error = 0;
    ledcWrite(ledChannel, 100);
    //Serial.println("connessione OK");
  }
}

void loop()
{
  esp_task_wdt_reset();

  client.loop();

  if (!client.isConnected()) {
    WiFiError();
    error = 1;
  }
  else {
    connessioneOk();
  }

  if (LastSOC != SOC) {
    if (SOC < 15) {
      tft.fillRect(5, 119, 230, 88, TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setFreeFont(FSBI24);
      tft.drawFloat(SOC, 0, 65, 123, 8);
      tft.setFreeFont(FSSB24);
      tft.drawString("%", 180, 123, GFXFF);
    }
    if ((SOC < 100) && (SOC >= 15)) {
      tft.fillRect(5, 119, 230, 88, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft.setFreeFont(FSBI24);
      tft.drawFloat(SOC, 0, 65, 123, 8);
      tft.setFreeFont(FSSB24);
      tft.drawString("%", 180, 123, GFXFF);
    }
    if (SOC == 100) {
      tft.fillRect(5, 119, 230, 88, TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setFreeFont(FSBI24);
      tft.drawFloat(SOC, 0, 12, 123, 8);
      tft.setFreeFont(FSSB24);
      tft.drawString("%", 185, 123, GFXFF);
    }
  }

  if ((SOC <= 100) && (SOC >= 80) && (Current < 0)) {
    tft.pushImage(17, 5, image4Width, image4Height, image4);
  }
  if ((SOC < 80) && (SOC >= 60) && (Current < 0)) {
    tft.pushImage(17, 5, image3Width, image3Height, image3);
  }
  if ((SOC < 60) && (SOC >= 30) && (Current < 0)) {
    tft.pushImage(17, 5, image2Width, image2Height, image2);
  }
  if ((SOC < 30) && (SOC >= 0) && (Current < 0)) {
    tft.pushImage(17, 5, image1Width, image1Height, image1);
  }

  if (Current > 0) {
    unsigned long now = millis();
    if (now - then >= intervals[current_interval_index]) {
      switch (current_interval_index) {
        case 0:
          tft.pushImage(17, 5, carica0Width, carica0Height, carica0);
          break;
        case 1:
          tft.pushImage(17, 5, carica1Width, carica1Height, carica1);
          break;
        case 2:
          tft.pushImage(17, 5, carica2Width, carica2Height, carica2);
          break;
        case 3:
          tft.pushImage(17, 5, carica3Width, carica3Height, carica3);
          break;
        case 4:
          tft.pushImage(17, 5, image4Width, image4Height, image4);
          break;
        case 5:
          tft.pushImage(17, 5, image4Width, image4Height, image4);
          break;
      }
      then = now;
      current_interval_index = (current_interval_index + 1) % 5; // increment index and wrap it back to zero, if it goes to 4
    }
  }
  /*
    unsigned long currentMillis = millis();

    if (Current < 10) {
      if (currentMillis - previousMillis >= interval) {
        tft.pushImage(17, 5, carica1Width, carica1Height, carica1);
      }
      if (currentMillis - previousMillis >= interval + 500) {
        tft.pushImage(17, 5, carica2Width, carica2Height, carica2);
      }
      if (currentMillis - previousMillis >= interval + 1000) {
        tft.pushImage(17, 5, carica3Width, carica3Height, carica3);
      }
      if (currentMillis - previousMillis >= interval + 1500) {
        tft.pushImage(17, 5, image4Width, image4Height, image4);
      }
    }
  */

  if (LastVoltage != Voltage) {
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    // tft.setFreeFont(FSSB12);
    tft.drawFloat(Voltage, 1, 167, 225, 4);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    //  tft.setFreeFont(FMB12);
    tft.drawString("V", 220, 231, 2);
  }

  if (LastCurrent != Current) {
    if ((Current < 0) && (Current > -10)) {
      tft.fillRect(3, 212, 150, 50, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB12);
      tft.drawFloat(Current, 1, 5, 214, 6);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FMB12);
      tft.drawString("A", 105, 232, 4);
    }

    if ((Current <= -10) && (Current > -100)) {
      tft.fillRect(3, 212, 150, 50, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB12);
      tft.drawFloat(Current, 1, 5, 214, 6);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB12);
      tft.drawString("A", 125, 232, 4);
    }

    if (Current <= -100) {
      tft.fillRect(3, 212, 150, 50, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB12);
      tft.drawFloat(Current, 0, 5, 214, 6);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FMB12);
      tft.drawString("A", 105, 232, 4);
    }


    if ((Current < 10) && (Current >= 0)) {
      tft.fillRect(3, 212, 150, 50, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB12);
      tft.drawFloat(Current, 1, 25, 214, 6);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB12);
      tft.drawString("A", 105, 232, 4);
    }

    if ((Current >= 10) && (Current < 100)) {

      tft.fillRect(3, 212, 150, 50, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB12);
      tft.drawFloat(Current, 1, 5, 214, 6);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FMB12);
      tft.drawString("A", 105, 232, 4);
    }

    if (Current >= 100) {
      tft.fillRect(3, 212, 150, 50, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB12);
      tft.drawFloat(Current, 0, 5, 214, 6);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FMB12);
      tft.drawString("A", 105, 232, 4);
    }

    if ((Current <= 0.2) && (Current >= -0.2)) {
      if ((SOC <= 100) && (SOC >= 80)) {
        tft.pushImage(17, 5, image4Width, image4Height, image4);
      }
      if ((SOC < 80) && (SOC >= 60)) {
        tft.pushImage(17, 5, image3Width, image3Height, image3);
      }
      if ((SOC < 60) && (SOC >= 30)) {
        tft.pushImage(17, 5, image2Width, image2Height, image2);
      }
      if ((SOC < 30) && (SOC >= 0)) {
        tft.pushImage(17, 5, image1Width, image1Height, image1);
      }
      ledcWrite(ledChannel, 10);
    }
    else {
      ledcWrite(ledChannel, 100);
    }
  }

  if (LastPower != Power) {
    if ((Power < 1000) && (Power >= 100)) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power, 0, 15, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("W", 105, 288, 4);
    }

    if ((Power < 100) && (Power >= 10)) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power, 0, 45, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("W", 105, 288, 4);
    }
    if ((Power < 10) && (Power > 0)) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power, 0, 45, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("W", 105, 288, 4);
    }

    if (Power >= 1000) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power / 1000, 2, 5, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("KW", 105, 288, 4);
    }

    if ((Power <= -100) && (Power > -1000)) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power, 0, 5, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("W", 105, 288, 4);
    }

    if ((Power <= -10) && (Power > -100)) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power, 0, 25, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("W", 105, 288, 4);
    }

    if ((Power <= 0) && (Power > -10)) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power, 0, 45, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("W", 105, 288, 4);
    }

    if (Power <= -1000) {
      tft.fillRect(3, 265, 157, 53, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //tft.setFreeFont(FSSB18);
      tft.drawFloat(Power / 1000, 2, 5, 270, 6);
      //tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FMB18);
      tft.drawString("KW", 120, 288, 4);
    }
  }

  if (LastPVPower != PVPower) {

    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("Produzione", 166, 267, 2);

    if ((PVPower > 0) && (PVPower < 10)) {
      tft.fillRect(164, 283, 73, 33, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FSSB12);
      tft.drawFloat(PVPower, 0, 185, 290, 4);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //  tft.setFreeFont(FMB12);
      tft.drawString("W", 220, 296, 2);
    }
    if ((PVPower >= 10) && (PVPower < 100)) {
      tft.fillRect(164, 283, 73, 33, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FSSB12);
      tft.drawFloat(PVPower, 0, 175, 290, 4);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //  tft.setFreeFont(FMB12);
      tft.drawString("W", 220, 296, 2);
    }
    if ((PVPower >= 100) && (PVPower < 1000)) {
      tft.fillRect(164, 283, 73, 33, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FSSB12);
      tft.drawFloat(PVPower, 0, 170, 290, 4);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //  tft.setFreeFont(FMB12);
      tft.drawString("W", 220, 296, 2);
    }
    if (PVPower >= 1000) {
      tft.fillRect(164, 283, 73, 33, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      // tft.setFreeFont(FSSB12);
      tft.drawFloat(PVPower / 1000, 1, 173, 290, 4);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      //  tft.setFreeFont(FMB12);
      tft.drawString("KW", 215, 296, 2);
    }

    if (PVPower == 0) {
      tft.fillRect(164, 283, 73, 33, TFT_BLACK);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft.drawString("0", 188, 290, 4);
      tft.drawString("W", 220, 296, 2);
    }
  }

  LastSOC = SOC;
  LastVoltage = Voltage;
  LastCurrent = Current;
  LastPower = Power;
  LastPVPower = PVPower;
}
