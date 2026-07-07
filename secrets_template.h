#### 3. The Template File (`secrets_template.h`)
Because we are going to tell GitHub to ignore your actual `secrets.h` file, you need to provide a blank template so other people who download your code know how to set it up.
```cpp
// Rename this file to secrets.h and enter your actual credentials
#define SECRET_SSID "YOUR_WIFI_SSID"
#define SECRET_PASS "YOUR_WIFI_PASSWORD"
#define SECRET_API_KEY "YOUR_OPENWEATHER_API_KEY"
