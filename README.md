# kartka

**kartka** is a simple Arduino sketch for [Inkplate 6](https://inkplate.io/) which downloads and displays a PGM file from a specified URL over WiFi every midnight.

## Dependencies

**kartka** depends on [InkplateLibrary](https://www.arduino.cc/reference/en/libraries/inkplatelibrary/) and [ezTime](https://www.arduino.cc/reference/en/libraries/eztime/):

```
arduino-cli lib --additional-urls https://github.com/SolderedElectronics/Dasduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Dasduino_Boards_index.json install InkplateLibrary ezTime
arduino-cli core --additional-urls https://github.com/SolderedElectronics/Dasduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Dasduino_Boards_index.json install Inkplate_Boards:esp32
```

## Configuration

Before building the sketch, you need to copy `config.h.example` file into `config.h` and adjust its values.

## Building

```
arduino-cli compile --fqbn Inkplate_Boards:esp32:Inkplate6 kartka
```

## Installing

```
arduino-cli upload -p /dev/ttyUSB0 --fqbn Inkplate_Boards:esp32:Inkplate6 kartka
```

## License

**kartka** is licensed under the terms of the [GNU General Public License](https://www.gnu.org/licenses/#GPL) version 3 or later.

The icons come from [Adwaita Icon Theme](https://gitlab.gnome.org/GNOME/adwaita-icon-theme) by [GNOME Project](https://www.gnome.org), licensed under the terms of either the [GNU LGPL v3](https://www.gnu.org/licenses/#LGPL) or [Creative Commons Attribution-Share Alike 3.0 United States License](https://creativecommons.org/licenses/by-sa/3.0/us/).

Copyright (C) 2021-2025 Sebastian Krzyszkowiak
