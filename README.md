# Nuki Opener for ESPHome (ESP32)
[![Build Component](https://github.com/uriyacovy/ESPHome_nuki_lock/actions/workflows/build.yaml/badge.svg)](https://github.com/uriyacovy/ESPHome_nuki_lock/actions/workflows/build.yaml)

This module builds an ESPHome lock platform for Nuki Opener (nuki_opener) that creates the following entities in Home Assistant:
- Lock 
- Binary Sensor: Is Paired
- Binary Sensor: Is Connected
- Binary Sensor: Critical Battery 
- Sensor: Battery Level

The lock entity is updated whenever the look changes state (via Nuki App, HA, or manually) using Nuki BT advertisement mechanism.

![screenshot](https://user-images.githubusercontent.com/1754967/183266065-d1a6e9fe-d7f7-4295-9c0d-4bf9235bf4cd.png)

## How to use
Add the following to the ESPHome yaml file:

```yaml
esphome:
  libraries:
  - Preferences
  - https://github.com/uriyacovy/NukiBleEsp32#dev

external_components:
  - source: github://erix/ESPHome_nuki_opener

esp32:
  board: "esp32dev"  # Or whatever other board you're using
  framework:
    type: arduino
    version: 2.0.3
    platform_version: 4.4.0

lock:
  # Required:
  - platform: nuki_opener
    name: Nuki Opener
    is_connected: 
      name: "Nuki Connected"
    is_paired: 
      name: "Nuki Paired"      
  # Optional:
    battery_critical:
      name: "Nuki Battery Critical"
    battery_level:
      name: "Nuki Battery Level"
```

After running ESPHome (esphome run <yamlfile.yaml>), the module will actively try to pair to Nuki.
To set Nuki for paring mode, press the button for 5 seconds until the led turns on.
Once Nuki is paired, the new ESPHome entities will get the updated state.

## Unparing Nuki
To unpair Nuki, add the following to ESPHome yaml file below `platform: nuki_lock` section and run ESPHome again:
```yaml
    unpair: true
```

## Dependencies
The module depends on the work done by [I-Connect](https://github.com/I-Connect), https://github.com/I-Connect/NukiBleEsp32

## Tested Hardware
- ESP32 wroom
- Nuki Opener

