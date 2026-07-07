#include <M5Dial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// --- Configuration ---
#include "secrets.h" // Loads your Wi-Fi and API credentials

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const String apiKey = SECRET_API_KEY;

// Location & Time Settings
const String currentLocation = "Wixom";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // Eastern Standard Time (UTC-5)
const int   daylightOffset_sec = 3600; // +1 Hour for Daylight Saving Time (EDT)

// Master list of available cities
String cities[] = {"Wixom", "Detroit", "New York", "London", "Tokyo", "Sydney", "Paris", "Cairo"};
const int numCities = sizeof(cities) / sizeof(cities[0]);

// Favorites Storage (Max 5)
String favorites[5];
int favCount = 0;

// --- State Management ---
enum State { SELECT_CITY, FETCH_WEATHER, SHOW_WEATHER, AUTO_SCROLL };
State currentState = SELECT_CITY;

bool browsingFavorites = false; 
int currentCityIndex = 0;
long oldEncoderPos = -999;
unsigned long lastInteractionTime = 0;

// Auto-Scroll Variables
enum AutoStep { STEP_FAVS, STEP_CURRENT, STEP_TIME };
AutoStep currentAutoStep = STEP_FAVS;
int autoFavIndex = 0;
unsigned long stepStartTime = 0;
int stepDuration = 0;
bool forceNextStep = false;

// Weather Data Variables
float temperature = 0.0;
String weatherDesc = "";

// Helper to figure out which city we are currently looking at manually
String getSelectedCity() {
  if (!browsingFavorites) {
    return cities[currentCityIndex];
  } else {
    if (currentCityIndex < favCount) {
      return favorites[currentCityIndex];
    } else {
      return currentLocation;
    }
  }
}

void setup() {
  M5Dial.begin();
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold12pt7b);
  
  // Connect to Wi-Fi
  M5Dial.Display.fillScreen(TFT_BLACK);
  M5Dial.Display.setTextColor(TFT_WHITE);
  M5Dial.Display.drawString("Connecting Wi-Fi...", 120, 120);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Init Network Time
  M5Dial.Display.drawString("Syncing Time...", 120, 150);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(2000); // Give NTP a moment to sync
  
  M5Dial.Encoder.write(0);
  lastInteractionTime = millis();
  drawCitySelector();
}

void loop() {
  M5Dial.update();
  
  // Track interactions to wake up or reset the idle timer
  long newEncoderPos = M5Dial.Encoder.read() / 4;
  auto touch = M5Dial.Touch.getDetail();
  
  if (newEncoderPos != oldEncoderPos || M5Dial.BtnA.wasPressed() || touch.isPressed()) {
    lastInteractionTime = millis();
    
    // Wake up from Auto-Scroll
    if (currentState == AUTO_SCROLL) {
      currentState = SELECT_CITY;
      oldEncoderPos = newEncoderPos; 
      drawCitySelector();
      delay(200); // Debounce wake up
      return; 
    }
  }

  // Enter Auto-Scroll if idle for 15 seconds
  if (currentState != AUTO_SCROLL && (millis() - lastInteractionTime > 15000)) {
    currentState = AUTO_SCROLL;
    currentAutoStep = STEP_FAVS;
    autoFavIndex = 0;
    forceNextStep = true;
    if (favCount == 0) currentAutoStep = STEP_CURRENT; // Skip favs if none exist
  }

  int maxIndex = browsingFavorites ? (favCount + 1) : numCities;

  switch (currentState) {
    
    case SELECT_CITY: {
      if (M5Dial.BtnA.pressedFor(1000)) {
        browsingFavorites = !browsingFavorites;
        currentCityIndex = 0;
        M5Dial.Encoder.write(0);
        drawCitySelector();
        delay(500); 
        break;
      }

      if (newEncoderPos != oldEncoderPos) {
        currentCityIndex = newEncoderPos % maxIndex;
        if (currentCityIndex < 0) currentCityIndex += maxIndex;
        drawCitySelector();
        oldEncoderPos = newEncoderPos;
      }

      if (M5Dial.BtnA.wasReleased()) {
        currentState = FETCH_WEATHER;
      }
      break;
    }

    case FETCH_WEATHER: {
      M5Dial.Display.fillScreen(TFT_BLACK);
      M5Dial.Display.setTextColor(TFT_YELLOW);
      M5Dial.Display.drawString("Fetching...", 120, 120);
      
      String targetCity = getSelectedCity();
      getWeather(targetCity);
      
      currentState = SHOW_WEATHER;
      drawWeatherManual();
      break;
    }

    case SHOW_WEATHER: {
      String activeCity = getSelectedCity();
      
      if (activeCity != currentLocation && touch.isPressed()) {
        if (touch.y > 140 && touch.y < 190 && touch.x > 60 && touch.x < 180) {
          addCityToFavorites(activeCity);
          drawWeatherManual(); 
          delay(300);
        }
      }

      if (M5Dial.BtnA.wasPressed()) {
        M5Dial.Encoder.write(currentCityIndex * 4); 
        currentState = SELECT_CITY;
        drawCitySelector();
      }
      break;
    }

    case AUTO_SCROLL: {
      if (forceNextStep || (millis() - stepStartTime >= stepDuration)) {
        forceNextStep = false;
        stepStartTime = millis();
        
        if (currentAutoStep == STEP_FAVS) {
          if (favCount > 0) {
            getWeather(favorites[autoFavIndex]);
            drawWeatherAuto(favorites[autoFavIndex], "Favorite City");
            stepDuration = 10000; // Show fav for 10 seconds
            
            autoFavIndex++;
            if (autoFavIndex >= favCount) {
              currentAutoStep = STEP_CURRENT;
            }
          } else {
            currentAutoStep = STEP_CURRENT;
            forceNextStep = true;
          }
          
        } else if (currentAutoStep == STEP_CURRENT) {
          getWeather(currentLocation);
          drawWeatherAuto(currentLocation, "Current Location");
          stepDuration = 15000; // Show current for 15 seconds
          currentAutoStep = STEP_TIME;
          
        } else if (currentAutoStep == STEP_TIME) {
          drawTimeAuto();
          stepDuration = 15000; // Show time for 15 seconds
          currentAutoStep = STEP_FAVS;
          autoFavIndex = 0;
          if (favCount == 0) currentAutoStep = STEP_CURRENT;
        }
      }
      break;
    }
  }
}

// --- Favorites Management ---
void addCityToFavorites(String city) {
  for (int i = 0; i < favCount; i++) {
    if (favorites[i] == city) return; 
  }
  if (favCount >= 5) {
    for (int i = 0; i < 4; i++) favorites[i] = favorites[i+1];
    favorites[4] = city;
    favCount = 5;
  } else {
    favorites[favCount] = city;
    favCount++;
  }
}

// --- UI Functions ---
void drawCitySelector() {
  M5Dial.Display.fillScreen(TFT_BLACK);
  M5Dial.Display.setTextColor(TFT_LIGHTGRAY);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold9pt7b);
  
  M5Dial.Display.drawString(browsingFavorites ? "Browsing: FAVORITES" : "Browsing: ALL CITIES", 120, 40);
  
  M5Dial.Display.setTextColor(TFT_CYAN);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold18pt7b);
  M5Dial.Display.drawString(getSelectedCity(), 120, 110);
  
  if (browsingFavorites && currentCityIndex == favCount) {
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextColor(TFT_ORANGE);
    M5Dial.Display.drawString("(Current Location)", 120, 145);
  }
  
  M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
  M5Dial.Display.setTextColor(TFT_DARKGRAY);
  M5Dial.Display.drawString("Hold dial to swap lists", 120, 200);
}

void drawWeatherManual() {
  M5Dial.Display.fillScreen(TFT_BLACK);
  String activeCity = getSelectedCity();
  
  M5Dial.Display.setTextColor(TFT_CYAN);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold12pt7b);
  M5Dial.Display.drawString(activeCity, 120, 45);
  
  M5Dial.Display.setTextColor(TFT_WHITE);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold18pt7b);
  M5Dial.Display.drawString(String(temperature, 1) + " F", 120, 95);
  
  M5Dial.Display.setTextColor(TFT_GREEN);
  M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
  M5Dial.Display.drawString(weatherDesc, 120, 140);
  
  if (activeCity != currentLocation) {
    bool isFav = false;
    for(int i=0; i<favCount; i++) { if(favorites[i] == activeCity) isFav = true; }
    
    if (isFav) {
      M5Dial.Display.fillRoundRect(60, 160, 120, 30, 6, TFT_DARKGREEN);
      M5Dial.Display.setTextColor(TFT_WHITE);
      M5Dial.Display.drawString("Saved !", 120, 175);
    } else {
      M5Dial.Display.drawRoundRect(60, 160, 120, 30, 6, TFT_YELLOW);
      M5Dial.Display.setTextColor(TFT_YELLOW);
      M5Dial.Display.drawString("+ Add Fav", 120, 175);
    }
  } else {
    M5Dial.Display.setTextColor(TFT_ORANGE);
    M5Dial.Display.drawString("Home Base", 120, 175);
  }
}

// Minimal UI for Auto-Scroll mode (no buttons)
void drawWeatherAuto(String city, String label) {
  M5Dial.Display.fillScreen(TFT_BLACK);
  
  M5Dial.Display.setTextColor(TFT_ORANGE);
  M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
  M5Dial.Display.drawString(label, 120, 35);

  M5Dial.Display.setTextColor(TFT_CYAN);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold18pt7b);
  M5Dial.Display.drawString(city, 120, 80);
  
  M5Dial.Display.setTextColor(TFT_WHITE);
  M5Dial.Display.drawString(String(temperature, 1) + " F", 120, 130);
  
  M5Dial.Display.setTextColor(TFT_GREEN);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold12pt7b);
  M5Dial.Display.drawString(weatherDesc, 120, 180);
}

void drawTimeAuto() {
  M5Dial.Display.fillScreen(TFT_BLACK);
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    M5Dial.Display.setTextColor(TFT_RED);
    M5Dial.Display.setTextFont(&fonts::FreeSansBold12pt7b);
    M5Dial.Display.drawString("Time Sync Error", 120, 120);
    return;
  }
  
  char timeStr[10];
  strftime(timeStr, sizeof(timeStr), "%I:%M %p", &timeinfo);
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a, %b %d", &timeinfo); 
  
  M5Dial.Display.setTextColor(TFT_ORANGE);
  M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
  M5Dial.Display.drawString("Current Time", 120, 35);
  
  M5Dial.Display.setTextColor(TFT_CYAN);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold12pt7b);
  M5Dial.Display.drawString(dateStr, 120, 90);
  
  M5Dial.Display.setTextColor(TFT_WHITE);
  M5Dial.Display.setTextFont(&fonts::FreeSansBold18pt7b);
  M5Dial.Display.drawString(timeStr, 120, 140);
  
  M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
  M5Dial.Display.setTextColor(TFT_DARKGRAY);
  M5Dial.Display.drawString("Turn dial to wake", 120, 205);
}

void getWeather(String city) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = "https://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=imperial";
    
    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
      String payload = http.getString();
      JsonDocument doc;
      deserializeJson(doc, payload);
      temperature = doc["main"]["temp"];
      const char* desc = doc["weather"][0]["main"];
      weatherDesc = String(desc);
    } else {
      weatherDesc = "API Error";
    }
    http.end();
  }
}void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
