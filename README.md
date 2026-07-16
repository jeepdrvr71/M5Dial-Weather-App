# M5Dial Weather Tracker 🌤️

A smart, rotary-controlled weather dashboard built for the M5Stack M5Dial. 

## Features
* **Rotary Navigation:** Use the dial to scroll through a master list of cities.
* **Favorites System:** Save up to 5 favorite cities to quick-access memory using the touchscreen.
* **Smart Screensaver:** Automatically cycles through your favorite cities, your home location, and the current time if left idle for 15 seconds.
* **NTP Time Sync:** Accurate network time protocol integration.

## Hardware Required
* [M5Stack M5Dial](https://docs.m5stack.com/en/core/M5Dial)
* [USB C Power Supply](https://www.amazon.com/Charger-Adapter-Compatible-Raspberry-Devices/dp/B0FPF3PBGV/ref=sr_1_13?adgrpid=191556522892&dib=eyJ2IjoiMSJ9.MpMHtYnovVaLlQty07hNppZMDpK7nKEUfT2aPyoPRWiOWf1YuLwi1M45Iycl__Yz7LtyFqWsMbhRIt70NgRLvb1vwbewRnH3t-3tZNeb1S07YwGCUw7uGtwkRFpsQ7aXaD0DAcs8GxZZY26YJf-E13KJtI82H3MdjDsknzEWZhPwCOp6VtCtGp1oMTqhmx_PJJniUP0bEl1ppYg2svOMfV37xataL4oVUBEFbEsDUfA.j0nPSdggPNVL9uR2eUyAvgH1_FjUSQRIVUk9VoXTCpU&dib_tag=se&hvadid=779537095428&hvdev=c&hvexpln=0&hvlocphy=1019668&hvnetw=g&hvocijid=12638810370932848464--&hvqmt=e&hvrand=12638810370932848464&hvtargid=kwd-1842224188535&hydadcr=21946_13654678_2283796&keywords=6v+usb+c+charger&mcid=810788ea945c31bab0c83235210f4fb1&qid=1783454876&sr=8-13)
* USB-C Cable

## Software Dependencies
Install these via the Arduino Library Manager:
* `M5Dial` by M5Stack
* `ArduinoJson` by Benoit Blanchon

## Setup Instructions
1. Get a free API key from [OpenWeatherMap](https://openweathermap.org/api).
2. Rename `secrets_template.h` to `secrets.h`.
3. Enter your Wi-Fi SSID, Password, and OpenWeather API key into `secrets.h`.
4. Open `M5Dial_Weather_Tracker.ino` and change `const String currentLocation = "Wixom";` to your actual home city.
5. Compile and upload to your M5Dial!

## Usage
* **Turn Dial:** Scroll through cities.
* **Press Dial:** Fetch weather for the selected city.
* **Hold Dial (1 sec):** Swap between "All Cities" and "Favorites" lists.
* **Tap Screen (+ Add Fav):** While viewing a city's weather, tap the yellow button to save it.

Copyright (c) 2026 J Botica

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
