/*
 * Copyright (C) 2021-2022 Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <Inkplate.h>
#include <driver/rtc_io.h>
#include <rom/rtc.h>
#include <WiFiMulti.h>
#include <ezTime.h>
#include <assert.h>

#include "wifi_sign.h"
#include "wifi_connecting_sign.h"
#include "error_sign.h"
#include "battery_sign.h"

#include "config.h"

#ifndef CONFIG_WIFI_SSID
#error "You must provide a config.h file with configuration - see config.h.example"
#endif

Inkplate display(INKPLATE_1BIT);
WiFiMulti wifiMulti;
Timezone timezone;

bool quiet = false;
RTC_DATA_ATTR int recovery = 0;

_Noreturn void deep_sleep(int t) {
  if (t < 0) {
    Serial.println("Entering infinite deep sleep. Bye!");
  } else {
    Serial.println(String("Entering deep sleep for ") + t + " seconds. Bye!");
  }
  Serial.println();

  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  display.einkOff();

  rtc_gpio_isolate(GPIO_NUM_12);
  if (t >= 0) {
    esp_sleep_enable_timer_wakeup(t * 1000000ULL);
  }
  esp_deep_sleep_start();
}

void show_error(void) {
  display.selectDisplayMode(INKPLATE_1BIT);
  Serial.println("Error!");
  display.drawImage(error_sign, display.width() / 2 + wifi_sign_w / 2 - error_sign_w, display.height() / 2 + wifi_sign_h / 2 - error_sign_h, error_sign_w, error_sign_h);
  if (quiet) {
    display.display();
  } else {
    display.partialUpdate();
  }
  deep_sleep(60 * pow(2, min(recovery++, 9)));
}

int get_sleep_time(void) {
  return (24 - timezone.hour()) * 60 * 60 + (00 - timezone.minute()) * 60 + (00 - timezone.second());
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("kartka (" __DATE__ " " __TIME__ ")");
  Serial.println("Copyright (C) 2021-2022 Sebastian Krzyszkowiak <dos@dosowisko.net>");
  Serial.println("This program comes with ABSOLUTELY NO WARRANTY.");
  Serial.println("This is free software, and you are welcome to redistribute it and/or modify");
  Serial.println("it under the terms of the GNU General Public License as published by");
  Serial.println("the Free Software Foundation, either version 3 of the License, or");
  Serial.println("(at your option) any later version.");
  Serial.println("https://gitlab.com/dos1/kartka");
  Serial.println();

  display.begin();
  display.clearDisplay();
  display.setRotation(CONFIG_ROTATION);

  if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET && !recovery) {
    quiet = true;
    Serial.println("Starting in quiet mode.");
    Serial.println();
  }

  if (recovery) {
    Serial.println("Starting in recovery mode: " + String(recovery));
    Serial.println();
  }

  double voltage = display.readBattery();
  Serial.println("Voltage: " + String(voltage) + "V");
  int8_t temperature = display.readTemperature();
  Serial.println("Temperature: " + String(temperature) + "°C");
  Serial.println();

  if (voltage < CONFIG_LOW_BATTERY) {
    Serial.println("Low battery! Abort!");
    display.drawImage(battery_sign, display.width() / 2 - battery_sign_w / 2, display.height() / 2 - battery_sign_h / 2, battery_sign_w, battery_sign_h);
    display.display();
    deep_sleep(-1);
  }

  display.drawImage(wifi_connecting, display.width() / 2 - wifi_connecting_w / 2, display.height() / 2 - wifi_connecting_h / 2, wifi_connecting_w, wifi_connecting_h);

  setServer(CONFIG_NTP_SERVER);

  Serial.print("Connecting to WiFi...");
  WiFi.mode(WIFI_MODE_STA);
  WiFi.setHostname(CONFIG_WIFI_HOSTNAME);
  const char* aps[] = {CONFIG_WIFI_SSID};
  const char* psks[] = {CONFIG_WIFI_PSK};
  static_assert(sizeof(aps) == sizeof(psks), "Invalid access point configuration");
  for (int i=0; i<sizeof(aps)/sizeof(aps[0]); i++) {
    wifiMulti.addAP(aps[i], psks[i]);
  }
  if (!quiet) {
    display.display();
  }

  int t = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
    t++;
    if (t > 30)
      show_error();
    Serial.print(".");
    delay(1000);
  }

  Serial.println();
  Serial.println("Connected to \"" + WiFi.SSID() + "\"");
  display.drawImage(wifi_sign, display.width() / 2 - wifi_sign_w / 2, display.height() / 2 - wifi_sign_h / 2, wifi_sign_w, wifi_sign_h);
  if (!quiet) {
    display.partialUpdate();
  }

  Serial.println("Waiting for timezone " CONFIG_TIMEZONE "...");
  if (!timezone.setCache(0) && !timezone.setLocation(CONFIG_TIMEZONE)) {
    Serial.println("Could not retrieve timezone information!");
    show_error();
  }
  Serial.println("Waiting for NTP...");
  if (!waitForSync(10)) {
    Serial.println("Could not retrieve current time!");
    show_error();
  }
  Serial.println(timezone.dateTime());

  int sleep_time = get_sleep_time();
  if (quiet && timezone.hour() != 0) {
    Serial.println("Woke up too early!");
    if (sleep_time < 15) {
      Serial.println(String("Sleeping for ") + (sleep_time + 1) + " seconds.");
      delay((sleep_time + 1) * 1000);
      sleep_time = get_sleep_time();
    } else {
      deep_sleep(sleep_time);
    }
  }

  Serial.println("Requesting \"" CONFIG_HTTP_URL "\"...");

  HTTPClient http;
  http.begin(CONFIG_HTTP_URL);
  http.addHeader("X-kartka-voltage", String(voltage));
  http.addHeader("X-kartka-temperature", String(temperature));
  http.addHeader("X-kartka-recovery", String(recovery));
  http.addHeader("X-kartka-quiet", quiet ? "true" : "false");

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
      Serial.println("Decoding...");

      int width, height, max;
      d = pgm_parse(data, len, &width, &height, &max);
      if (!d) {
        Serial.println("Couldn't decode PGM data!");
        free(data);
        show_error();
      }
      Serial.println(String("PGM image: ") + width + "x" + height + " (" + max + ")");

      Serial.println("Drawing...");

      display.selectDisplayMode(INKPLATE_3BIT);
      for (int i = 0; i < width * height; i++) {
        display.drawPixel((display.width() - width) / 2 + (i % width), (display.height() - height) / 2 + (i / width), pgm_pixel_at(d, i, pgm_depth(max)) * 7 / max);
      }

      Serial.println("Displaying...");
      display.display();
      
      http.end();
      free(data);

      recovery = 0;
      deep_sleep(sleep_time);
    }
  }

  Serial.println(String("HTTP error ") + httpCode + " (" + http.getSize() + ")");
  http.end();
  show_error();
}

void loop() {
  // nuthin!
}
