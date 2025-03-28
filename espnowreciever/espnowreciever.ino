/**
 * @file      ESP-NOW Receiver with Button-Controlled Display
 * @author    Updated by ChatGPT
 */

#include "rm67162.h"
#include <TFT_eSPI.h>   
#include <WiFi.h>
#include <esp_now.h>
#include "../e46.h"

// **Define the display**
#define WIDTH  536
#define HEIGHT 240
#define PINBALL_YELLOW 0xFB00
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// **Define buttons for navigation**
#define BUTTON_NEXT  PIN_BUTTON_1  // Go to next value
#define BUTTON_PREV  PIN_BUTTON_2  // Go to previous value

#define DEBOUNCE_TIME  150   // Milliseconds
unsigned long lastButtonPress = 0;

// **Create a struct instance to store received data**
CarData carData;

#define PARAM_LEN 5
// **Parameter names for display**
const char* paramNames[] = {
    //"RPM", "Oil Temp", "Water Temp", "Accel", "Brake", "Clutch", "Speed", "Ratio", "FUELR", "FUELL"
    "RPM", "Oil Temp", "Water Temp", "Speed", "Ratio"
};

// **Keep track of which parameter is displayed**
int currentParamIndex = 0;
int prevParamIndex = 0;

// **ESP-NOW Receive Callback**
void onReceive(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
    if (len == sizeof(CarData)) {
        memcpy(&carData, incomingData, sizeof(carData));
        //Serial.println("New data received!");
        //updateDisplay();
    }
}

// **Initialize ESP-NOW**
void setupESPNow() {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed!");
        ESP.restart();
    }
    esp_now_register_recv_cb(onReceive);
}

void checkButtons() {
    if (millis() - lastButtonPress < DEBOUNCE_TIME) return;  // Ignore rapid presses

    if (digitalRead(BUTTON_NEXT) == LOW) {
        lastButtonPress = millis();
        currentParamIndex = (currentParamIndex + 1) % PARAM_LEN;
        //updateDisplay();
    }
    if (digitalRead(BUTTON_PREV) == LOW) {
        lastButtonPress = millis();
        currentParamIndex = (currentParamIndex - 1 + PARAM_LEN) % PARAM_LEN;
        //updateDisplay();
    }
}

// **Update the display to show the current parameter**
void updateDisplay() {
  if(carData.state.rpm > 5500) {
    spr.fillSprite(TFT_RED);
    spr.setTextColor(TFT_BLACK);
  } else {
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(PINBALL_YELLOW);
  }
  spr.setTextSize(7);
    
  // Draw the parameter name
  spr.setCursor(50, 0);
  spr.printf("%s", paramNames[currentParamIndex]);

  // Draw the corresponding value
  spr.setTextSize(7);
  spr.setCursor(50, 100);

  switch (currentParamIndex) {
    case 0: spr.printf("%d", carData.state.rpm); break;
    case 1: spr.printf("%d C", carData.state.oilTemp); break;
    case 2: spr.printf("%d C", carData.state.waterTemp); break;
    //case 3: spr.printf("%d%%", carData.state.accel); break;
    //case 4: spr.printf("%d%%", carData.state.brake); break;
    //case 5: spr.printf("%d%%", carData.state.clutch); break;
    case 3: spr.printf("%.1f km/h", carData.state.speed); break;
    case 4: spr.printf("%.1f", carData.state.rpm*1.0/carData.state.speed); break;
    //case 8: spr.printf("%d ?", carData.state.fuelr); break;
    //case 9: spr.printf("%d ?", carData.state.fuell); break;
  }

  lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
}

// **Setup function**
void setup() {
    Serial.begin(115200);

    // **Initialize Display**
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
    rm67162_init();
    lcd_setRotation(3);
    spr.createSprite(WIDTH, HEIGHT);
    spr.setSwapBytes(1);

    // **Initialize Buttons**
    pinMode(BUTTON_NEXT, INPUT_PULLUP);
    pinMode(BUTTON_PREV, INPUT_PULLUP);

    // **Initialize ESP-NOW**
    setupESPNow();

    // **Initial screen message**
    updateDisplay();
}

// **Loop - Check button presses to change displayed parameter**
void loop() {
    checkButtons();
    updateDisplay();
    delay(30);  // Short delay to prevent excessive CPU usage
}
