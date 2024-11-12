| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# Example: Solenoid WiFi Control

This example is built on the Espressif Wifi Control example. 

The board this is based on is something similar to this: https://devices.esphome.io/devices/ESP32E-Relay-X8

## GPIO functions:

| GPIO                         | Direction | Assigned Pin                                           |
| ---------------------------- | --------- | ------------------------------------------------------ |
| RELAY_OUTPUT_1               | output    | 32                                                     |
| RELAY_OUTPUT_2               | output    | 33                                                     |
| RELAY_OUTPUT_3               | output    | 25                                                     |
| RELAY_OUTPUT_4               | output    | 26                                                     |
| RELAY_OUTPUT_5               | output    | 27                                                     |
| RELAY_OUTPUT_6               | output    | 14                                                     |
| RELAY_OUTPUT_7               | output    | 12                                                     |
| RELAY_OUTPUT_8               | output    | 13                                                     |

## Test:
 1. Connect the development board to your computer.
 2. Update the Wifi SSID and password
 3. Build and flash the project to the board.
 4. Send curl request to update outputs
    ### 4.1 Using Linux:
        # GET request to read all outputs
        curl http://<esp32-ip>/outputs
        # Returns: {"outputs":[0,0,0,0,0,0,0,0]}
        
        # POST request to set an output
        curl -X POST -d '{"output":0,"state":1}' http://<esp32-ip>/output
        # Sets output 0 to HIGH

    ### 4.2 Using Windows Powershell: 
        # GET request to read all outputs
        Invoke-WebRequest -Uri "http://<esp32-ip>/outputs" -Method GET

        # POST request to set an output
        $body = '{"output":0,"state":1}'
        Invoke-WebRequest -Uri "http://<esp32-ip>/output" -Method POST -Body $body -ContentType "application/json"

## How to use example

Before project configuration and build, be sure to set the correct chip target using `idf.py set-target <chip_name>`.

### Hardware Required

* A development board with any Espressif SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming
* Some jumper wires to connect GPIOs.

### Configure the project

### Build and Flash

Build the project and flash it to the board, then run the monitor tool to view the serial output:

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.


## Troubleshooting

Blah
