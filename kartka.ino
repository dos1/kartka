#include "Inkplate.h"
#include "driver/rtc_io.h"
#include <ezTime.h>

#include "pgm.h"
#include "download.h"

#include "wifi_sign.h"
#include "wifi_connecting.h"
#include "error_sign.h"

#include "config.h"

Timezone timezone;
Inkplate display(INKPLATE_1BIT);

_Noreturn void deep_sleep(int t) {
  Serial.println(String("Entering deep sleep for ") + t + " seconds. Bye!");
  
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  display.einkOff();

  rtc_gpio_isolate(GPIO_NUM_12);
  esp_sleep_enable_timer_wakeup(t * 1000000ULL);
  esp_deep_sleep_start();
}

void show_error(void) {
  display.selectDisplayMode(INKPLATE_1BIT);
  Serial.println("Error!");
  display.drawImage(error_sign, display.width() / 2 + wifi_sign_w / 2 - error_sign_w, display.height() / 2 + wifi_sign_h / 2 - error_sign_h, error_sign_w, error_sign_h, false, true);
  display.partialUpdate();
  
  deep_sleep(60);    
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("kartka (" __DATE__ " " __TIME__ ")");
  display.begin();
  display.clearDisplay();
  display.setRotation(3);

  double voltage = display.readBattery();
  Serial.println("Voltage: " + String(voltage) + "V");
  int8_t temperature = display.readTemperature();
  Serial.println("Temperature: " + String(temperature) + "°C");
    
  display.drawImage(wifi_connecting, display.width() / 2 - wifi_connecting_w / 2, display.height() / 2 - wifi_connecting_h / 2, wifi_connecting_w, wifi_connecting_h, false, true);

  Serial.print("Connecting to \"" WIFI_AP "\"..");
  WiFi.mode(WIFI_MODE_STA);
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.begin(WIFI_AP, WIFI_PSK);
  display.display();
  
  int t = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    t++;
    if (t > 60)
      show_error();
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("Connected. Requesting " HTTP_URL "...");
  display.drawImage(wifi_sign, display.width() / 2 - wifi_sign_w / 2, display.height() / 2 - wifi_sign_h / 2, wifi_sign_w, wifi_sign_h, false, true);
  display.partialUpdate();
  
  HTTPClient http;
  http.begin(HTTP_URL);
  http.addHeader("X-kartka-voltage", String(voltage));
  http.addHeader("X-kartka-temperature", String(temperature));

  int httpCode = http.GET();
  if (httpCode == 200) {
    int32_t len = http.getSize();
    Serial.println(String("OK! (") + len + ") Downloading...");
    
    if (len > 0) {
      uint8_t *data = download(http.getStreamPtr(), len);
      if (!data) {
        Serial.println("Failed!");
        show_error();
      }
      
      uint8_t *d = data;
      Serial.println("Displaying...");
            
      int width, height, max;
      d = pgm_parse(data, len, &width, &height, &max);
      if (!d) {
        Serial.println("Couldn't decode PGM data!");
        free(data);
        show_error();
      }
      Serial.println(String("Got a ") + width + "x" + height + " image, max: " + max);

      display.selectDisplayMode(INKPLATE_3BIT);
      for (int i = 0; i < width * height; i++) {
        display.drawPixel(i % width, i / width, pgm_pixel_at(d, i, pgm_depth(max)) / (max / (double)7));
      }
      display.display();
      
      http.end();
      free(data);
      
      Serial.println("Waiting for timezone " TIMEZONE "...");
      timezone.setLocation(TIMEZONE);
      Serial.println("Waiting for NTP...");
      waitForSync(16);
      Serial.println(timezone.dateTime());

      deep_sleep((23 - timezone.hour()) * 60 * 60 + (59 - timezone.minute()) * 60 + (60 - timezone.second()));
    }
  }

  Serial.println(String("HTTP error ") + httpCode + " (" + http.getSize() + ")");
  http.end();
  show_error();
}

void loop() {
  // nuthin!
}
