/**
 * @file      ESP-NOW Receiver for LilyGO T-Display S3 AMOLED
 * @author    Updated by ChatGPT
 * @license   MIT
 */

#include "rm67162.h"
#include <TFT_eSPI.h>   // https://github.com/Bodmer/TFT_eSPI
#include <WiFi.h>
#include <esp_now.h>
#include "../e46.h"
// **Define the display**
#define WIDTH  536
#define HEIGHT 240
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// **Create a struct instance to store received data**
CarData receivedData;

// **ESP-NOW Receive Callback**
void onReceive(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
    if (len == sizeof(CarData)) {
        memcpy(&receivedData, incomingData, sizeof(receivedData));

        Serial.printf("Received - RPM: %d, Oil Temp: %.1f, Water Temp: %.1f, Accel: %d%%, Brake: %d%%, Clutch: %d%%, Speed: %.1f km/h\n",
                      receivedData.rpm, receivedData.oilTemp, receivedData.waterTemp,
                      receivedData.accel, receivedData.brake, receivedData.clutch, receivedData.speed);

        // **Update display with new data**
        updateDisplay();
    } else {
        Serial.println("Error: Incorrect data size received!");
    }
}

// **Initialize ESP-NOW**
void setupESPNow() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed!");
        ESP.restart();
    }
    esp_now_register_recv_cb(onReceive);
}

// **Update the display with received data**
void updateDisplay() {
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_WHITE);
    spr.setTextSize(2);
    spr.setCursor(10, 20);

    spr.printf("RPM: %d\n", receivedData.rpm);
    spr.printf("Oil Temp: %.1f C\n", receivedData.oilTemp);
    spr.printf("Water Temp: %.1f C\n", receivedData.waterTemp);
    spr.printf("Accel: %d%%\n", receivedData.accel);
    spr.printf("Brake: %d%%\n", receivedData.brake);
    spr.printf("Clutch: %d%%\n", receivedData.clutch);
    spr.printf("Speed: %.1f km/h\n", receivedData.speed);

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

    // **Initialize ESP-NOW**
    setupESPNow();

    // **Initial screen message**
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_WHITE);
    spr.setTextSize(2);
    spr.setCursor(10, 100);
    spr.drawString("Waiting for ESP-NOW data...", 50, 100);
    lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
}

// **Loop (keeps system running)**
void loop() {
    delay(1000);  // Screen updates when data is received
}
