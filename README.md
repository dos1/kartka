# kartka

**kartka** is a simple Arduino sketch for [Inkplate 6](https://inkplate.io/) which downloads and displays a PGM file from a specified URL over WiFi every midnight.

## Dependencies

**kartka** depends on InkplateLibrary and ezTime:

```
arduino-cli lib --additional-urls https://github.com/e-radionicacom/Croduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Croduino_Boards_index.json install InkplateLibrary ezTime
arduino-cli core --additional-urls https://github.com/e-radionicacom/Croduino-Board-Definitions-for-Arduino-IDE/raw/master/package_Croduino_Boards_index.json install Croduino_Boards:Inkplate
```

## Configuration

Before building the sketch, you need to copy `config.h.example` file into `config.h` and adjust its values.

## Building

```
arduino-cli compile --fqbn Croduino_Boards:Inkplate:Inkplate6 kartka
```

## Installing

```
arduino-cli upload -p /dev/ttyUSB0 --fqbn Croduino_Boards:Inkplate:Inkplate6 kartka
```

## License

**kartka** is licensed under the terms of the [GNU General Public License](https://www.gnu.org/licenses/#GPL) version 3 or later.

The icons come from [Adwaita Icon Theme](https://gitlab.gnome.org/GNOME/adwaita-icon-theme) by [GNOME Project](https://www.gnome.org), licensed under the terms of either the [GNU LGPL v3](https://www.gnu.org/licenses/#LGPL) or [Creative Commons Attribution-Share Alike 3.0 United States License](https://creativecommons.org/licenses/by-sa/3.0/us/).

Copyright (C) 2021 Sebastian Krzyszkowiak
