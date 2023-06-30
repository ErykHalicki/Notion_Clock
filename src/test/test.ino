#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  // Initialize the serial port
  Serial.begin(115200);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Scan for nearby networks
  
}

void loop() {
  // Do nothing
	int numNetworks = WiFi.scanNetworks();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nearby networks:");
  for (int i = 0; i < numNetworks; i++) {
    lcd.setCursor(0, i+1);
    lcd.print(WiFi.SSID(i));
  }
delay(1000);
}
