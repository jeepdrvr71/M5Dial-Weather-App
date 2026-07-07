# M5Dial Weather Tracker 🌤️

A smart, rotary-controlled weather dashboard built for the M5Stack M5Dial. 

## Features
* **Rotary Navigation:** Use the dial to scroll through a master list of cities.
* **Favorites System:** Save up to 5 favorite cities to quick-access memory using the touchscreen.
* **Smart Screensaver:** Automatically cycles through your favorite cities, your home location, and the current time if left idle for 15 seconds.
* **NTP Time Sync:** Accurate network time protocol integration.

## Hardware Required
* [M5Stack M5Dial](https://docs.m5stack.com/en/core/M5Dial)
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