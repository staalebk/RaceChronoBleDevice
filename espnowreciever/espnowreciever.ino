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
CarData receivedData;

// **Parameter names for display**
const char* paramNames[] = {
    "RPM", "Oil Temp", "Water Temp", "Accel", "Brake", "Clutch", "Speed", "Ratio"
};

// **Keep track of which parameter is displayed**
int currentParamIndex = 0;

// **ESP-NOW Receive Callback**
void onReceive(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
    if (len == sizeof(CarData)) {
        memcpy(&receivedData, incomingData, sizeof(receivedData));
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
        currentParamIndex = (currentParamIndex + 1) % 8;
        //updateDisplay();
    }
    if (digitalRead(BUTTON_PREV) == LOW) {
        lastButtonPress = millis();
        currentParamIndex = (currentParamIndex - 1 + 8) % 8;
        //updateDisplay();
    }
}

// **Update the display to show the current parameter**
void updateDisplay() {
  if(receivedData.rpm > 5500) {
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
    case 0: spr.printf("%d", receivedData.rpm); break;
    case 1: spr.printf("%.1f C", receivedData.oilTemp); break;
    case 2: spr.printf("%.1f C", receivedData.waterTemp); break;
    case 3: spr.printf("%d%%", receivedData.accel); break;
    case 4: spr.printf("%d%%", receivedData.brake); break;
    case 5: spr.printf("%d%%", receivedData.clutch); break;
    case 6: spr.printf("%.1f km/h", receivedData.speed); break;
    case 7: spr.printf("Ratio %.1f", receivedData.rpm/receivedData.speed); break;
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
    lcd_setRotation(1);
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
