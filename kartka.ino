#include "Inkplate.h"
#include "driver/rtc_io.h"
#include <ezTime.h>

#include "wifi_sign.h"
#include "wifi_connecting.h"
#include "error_sign.h"

#include "config.h"

Timezone timezone;
Inkplate display(INKPLATE_1BIT);

uint8_t *downloadFile(WiFiClient *s, int32_t len) {
  uint8_t *buffer = (uint8_t *)ps_malloc(len);
  uint8_t *buff = buffer;
  while (len > 0) {
    size_t size = s->available();
    if (size) {
      int c = s->readBytes(buff, ((size > len) ? len : size));
      if (len > 0) {
        len -= c;
      }
      buff += c;
    }
    yield();
  }
  return buffer;
}

void deep_sleep(int t) {
  t = 10;
  Serial.println(String("Entering deep sleep for ") + t + " seconds. Bye!");
  delay(100);
  
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  display.einkOff();

  rtc_gpio_isolate(GPIO_NUM_12);
  esp_sleep_enable_timer_wakeup(t * 1000000ULL);
  esp_deep_sleep_start();
}

_Noreturn void show_error(void) {
  Serial.println("Error!");
  display.drawImage(error_sign, display.width() / 2 + wifi_sign_w / 2 - error_sign_w, display.height() / 2 + wifi_sign_h / 2 - error_sign_h, error_sign_w, error_sign_h, false, true);
  display.partialUpdate();
  
  deep_sleep(60);    
}

void setup() {
  Serial.begin(115200);
  display.begin();
  display.clearDisplay();
  
  display.setRotation(3);
  display.setTextSize(2);
  
  display.drawImage(wifi_connecting, display.width() / 2 - wifi_connecting_w / 2, display.height() / 2 - wifi_connecting_h / 2, wifi_connecting_w, wifi_connecting_h, false, true);

  Serial.print("Connecting to \"" WIFI_AP "\"..");
  WiFi.mode(WIFI_MODE_STA);
  WiFi.setHostname("kindluino");
  WiFi.begin(WIFI_AP, WIFI_PSK);
  WiFi.setAutoReconnect(true);
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
  Serial.println("Connected. Downloading " HTTP_URL "...");
  display.drawImage(wifi_sign, display.width() / 2 - wifi_sign_w / 2, display.height() / 2 - wifi_sign_h / 2, wifi_sign_w, wifi_sign_h, false, true);
  display.partialUpdate();
  
  HTTPClient http;
  http.getStream().setNoDelay(true);
  http.getStream().setTimeout(1);
  http.begin(HTTP_URL);

  int httpCode = http.GET();
  if (httpCode == 200) {
    int32_t len = http.getSize();
    Serial.println(String("OK! (") + len + ")");
    
    if (len > 0) {
      uint8_t *data = downloadFile(http.getStreamPtr(), len);
      Serial.println("Displaying...");
            
      display.selectDisplayMode(INKPLATE_3BIT);
      for (int i = 0; i < len; i++) {
        display.drawPixel(i % 600, i / 600, data[i] / 32);
      }
      display.display();
      http.end();
      
      Serial.println("Waiting for timezone...");
      timezone.setLocation("Europe/Warsaw");
      Serial.println("Waiting for NTP...");
      waitForSync(10);
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
