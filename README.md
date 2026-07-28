# esphome_hunter_wifi

A big thank you for the original coders: [Sebastien](https://github.com/seb821/OpenSprinkler-Firmware-Hunter) and [Ecodina](https://github.com/ecodina/hunter-wifi)

Release 3.0.0 is Updated for compatibility with esp-idf.

# ESPHome Hunter WiFi

Make your traditional Hunter irrigation controller smart with ESPHome.

This external component connects an ESPHome device to a Hunter irrigation
controller and exposes its stations/zones to Home Assistant, allowing local
automation and control without replacing the existing irrigation hardware.

## Why?

Many Hunter irrigation systems are reliable irrigation controllers but have
limited connectivity and automation possibilities.

This project adds a local smart interface:

- Keep your existing Hunter irrigation hardware
- Add Home Assistant integration
- Create automations based on weather, sensors, time, presence, etc.
- Avoid cloud dependencies

## Design goals

The goal is not to replace the Hunter controller logic.

The Hunter controller remains responsible for controlling irrigation outputs.
This component provides a smart local interface so Home Assistant can interact
with the existing system.

## Installation

Add the external component to your ESPHome configuration:
Something like:

```yaml
esphome:
  name: hunter_wifi

esp32:
  board: esp32dev
  framework:
    type: esp-idf

external_components:
  - source:
      type: git
      url: https://github.com/pipacsba/esphome_hunter_wifi
    components: [hunter_wifi]

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

hunter_wifi:
```

Before:

Rain sensor / buttons
        |
        |
Hunter controller
        |
        |
Sprinkler valves


After:

                 Home Assistant
                       |
                   ESPHome API
                       |
                    ESP32
                       |
                  Hunter controller
                       |
                       |
               Sprinkler valves

## Use cases

Examples:

- Water the garden only when nobody is home
- Skip watering after rain detected by Home Assistant
- Adjust watering based on soil moisture sensors
- Create custom irrigation schedules
- Combine irrigation with other smart home automations


## Requirements

- ESP32 device
- ESPHome with ESP-IDF framework


## License

Given that [Sebastien](https://github.com/seb821/OpenSprinkler-Firmware-Hunter) and [Ecodina](https://github.com/ecodina/hunter-wifi) published their code under the GNU GPL v3, this project follows the same license.

    Hunter Roam - WiFi
        
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
