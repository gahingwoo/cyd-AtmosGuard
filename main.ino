#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <DHT.h>
#include <time.h>

// Display related
TFT_eSPI tft = TFT_eSPI();  

// Time update intervals
#ifndef DHT_UPDATE_INTERVAL
#endif
#ifndef WEATHER_UPDATE_INTERVAL
#endif
#ifndef TIME_UPDATE_INTERVAL
#endif
#ifndef TIMEZONE_OFFSET
#endif
unsigned long lastTimeUpdate = 0;
unsigned long lastWeatherUpdate = 0;

// DHT22 sensor related
#define DHTPIN 22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float local_temperature = 0;
float local_humidity = 0;
unsigned long last_dht_read = 0;

// WiFi and Network settings
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = TIMEZONE_OFFSET * 3600;
const int   daylightOffset_sec = 0;

// Weather API configuration
String latitude = WEATHER_LATITUDE;
String longitude = WEATHER_LONGITUDE;
String location = WEATHER_LOCATION;
String timezone = WEATHER_TIMEZONE;

// Weather data storage
String current_date;
String last_weather_update;
String temperature;
String humidity;
String weather_description;
int weather_code = 0;
bool is_day = true;

// Time and date storage
char timeStr[6];  // HH:MM\0
char dateStr[20];
char lastTimeStr[6] = "";
char lastDateStr[20] = "";
const char* weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Screen dimensions
const int SCREEN_WIDTH = 320;   // Screen width
const int SCREEN_HEIGHT = 240;  // Screen height

// Layout columns
const int LEFT_COLUMN = 5;      // Left column start position 
const int RIGHT_COLUMN = 165;   // Right column start position
const int COLUMN_WIDTH = 150;   // Column width
const int CENTER_GAP = 10;      // Gap between columns

// Info lines positions
const int INFO_LINE1_Y = 5;     // First line (time/city)
const int INFO_LINE2_Y = 35;    // Second line (weekday/weather)
const int INFO_LINE3_Y = 60;    // Third line (date/update time)

// Indoor sensor display positions 
const int INDOOR_TITLE_Y = 95;  // Indoor title position
const int INDOOR_TEMP_Y = 120;  // Indoor temperature position
const int INDOOR_HUM_Y = 155;   // Indoor humidity position

// Outdoor data display positions
const int OUTDOOR_TITLE_Y = 95; // Outdoor title position
const int OUTDOOR_TEMP_Y = 120; // Outdoor temperature position 
const int OUTDOOR_HUM_Y = 155;  // Outdoor humidity position

// Weather advice position
const int WEATHER_ADVICE_Y = 190; // Bottom position for advice

// Font configurations
const int TIME_FONT = 4;        // Large font for time and city
const int TITLE_FONT = 2;       // Medium font for titles
const int VALUE_FONT = 4;       // Large font for values
const int INFO_FONT = 2;        // Small font for info
const int LABEL_OFFSET = 80;    // Offset between label and value

// Buffer size
const int TEXT_BUFFER = 150;    // Text buffer size

// Display colors
const uint16_t COLOR_TITLE = TFT_ORANGE;    // Title color
const uint16_t COLOR_TIME = TFT_LIGHTGREY;  // Time color
const uint16_t COLOR_INDOOR = 0x07E0;       // Indoor values color (green)
const uint16_t COLOR_OUTDOOR = 0xFFE0;      // Outdoor values color (yellow)
const uint16_t COLOR_INFO = TFT_LIGHTGREY;  // Info text color

// Display value tracking
float last_displayed_temp = -999;
float last_displayed_hum = -999;

// WiFi status
bool wifi_connected = false;
unsigned long last_wifi_retry = 0;
const unsigned long WIFI_RETRY_INTERVAL = 60000; // 1 minute between retries

// LED control
#define LEDC_CHANNEL_0     0
#define LEDC_BASE_FREQ     5000

// Time period enumeration
enum TimeSlot {
  MORNING = 0,    // 5-10
  NOON = 1,       // 10-16
  EVENING = 2,    // 16-20
  NIGHT = 3       // 20-5
};

// Weather advice mapping table
const char* WEATHER_ADVICE_MAP[][4] = {
  // Clear sky (0-1)
  {
    "Sunny morning, great for exercise",    // Morning
    "Strong UV, use sun protection",        // Noon
    "Pleasant evening for walking",         // Evening
    "Clear sky, good for stargazing"       // Night
  },
  // Cloudy (2-3)
  {
    "Moderate morning, outdoor ok",         // Morning
    "Nice day for activities",              // Noon
    "Cool breeze, enjoy evening",           // Evening
    "Mild night ahead"                      // Night
  },
  // Fog (45-48)
  {
    "Foggy, leave early for work",         // Morning
    "Keep distance when driving",           // Noon
    "Drive slow in fog",                    // Evening
    "Very low visibility tonight"           // Night
  },
  // Rain (51-55)
  {
    "Rain expected, take umbrella",         // Morning
    "Rain continues, stay dry",             // Noon
    "Wet roads, drive carefully",           // Evening
    "Rainy night, stay warm inside"         // Night
  },
  // Snow (71-75)
  {
    "Snow! Wear warm clothes",              // Morning
    "Slippery roads, be careful",           // Noon
    "Heavy snow, leave early",              // Evening
    "Freezing night, stay warm"             // Night
  },
  // Thunderstorm (95-99)
  {
    "Thunderstorm! Stay inside",            // Morning
    "Severe weather alert",                 // Noon
    "Storm coming, head home",              // Evening
    "Dangerous weather, stay in"            // Night
  }
};

// Temperature advice mapping
const char* TEMP_ADVICE_MAP[][4] = {
  // High temperature (>=35°C)
  {
    "Hot day ahead, dress light",           // Morning
    "Extreme heat, stay hydrated",          // Noon
    "Still hot, avoid sun",                 // Evening
    "Warm night, use AC"                    // Night
  },
  // Low temperature (<=5°C)
  {
    "Very cold, dress warmly",              // Morning
    "Cold day, keep moving",                // Noon
    "Temperature dropping fast",            // Evening
    "Freezing night, heat on"               // Night
  }
};


// Read DHT22 sensor
void readDHT() {
  if (millis() - last_dht_read >= DHT_UPDATE_INTERVAL) {
    float newTemp = dht.readTemperature();
    float newHumidity = dht.readHumidity();
    
    Serial.print("DHT22 Reading - Temperature: ");
    Serial.print(newTemp, 1);
    Serial.print("°C, Humidity: ");
    Serial.print(newHumidity, 1);
    Serial.println("%");
    
    if (isnan(newTemp) || isnan(newHumidity)) {
      Serial.println("Failed to read from DHT22 sensor!");
    } else {
      local_temperature = newTemp;
      local_humidity = newHumidity;
    }
    last_dht_read = millis();
  }
}

// Format time into timeStr buffer
void formatTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    strcpy(timeStr, "??:??");
    strcpy(dateStr, "???");
    return;
  }
  sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  sprintf(dateStr, "%s", weekDays[timeinfo.tm_wday]);
}

// Update only time display
void updateTimeDisplay() {
  formatTime();
  
  // Only update display when time or date changes
  if (strcmp(timeStr, lastTimeStr) != 0) {
    // Clear time display area
    tft.fillRect(LEFT_COLUMN, INFO_LINE1_Y, COLUMN_WIDTH, 30, TFT_BLACK);
    tft.setTextColor(COLOR_TIME, TFT_BLACK);
    tft.drawString(timeStr, LEFT_COLUMN, INFO_LINE1_Y, TIME_FONT);
    strcpy(lastTimeStr, timeStr);
  }
  
  if (strcmp(dateStr, lastDateStr) != 0) {
    // Clear date display area
    tft.fillRect(LEFT_COLUMN, INFO_LINE2_Y, COLUMN_WIDTH, 30, TFT_BLACK);
    tft.setTextColor(COLOR_TIME, TFT_BLACK);
    tft.drawString(dateStr, LEFT_COLUMN, INFO_LINE2_Y, TITLE_FONT);
    strcpy(lastDateStr, dateStr);
  }
}

// Update only temperature and humidity display 
void updateSensorDisplay() {
  // Only update when values change
  if (abs(local_temperature - last_displayed_temp) >= 0.1 || 
      abs(local_humidity - last_displayed_hum) >= 1) {
    
    // Clear and update temperature display area
    tft.fillRect(LEFT_COLUMN, INDOOR_TEMP_Y - 5, TEXT_BUFFER, 35, TFT_BLACK);
    tft.setTextColor(COLOR_INDOOR, TFT_BLACK);
    String tempStr = String(local_temperature, 1) + "°C";
    tft.drawString(tempStr, LEFT_COLUMN, INDOOR_TEMP_Y, VALUE_FONT);
    tft.setTextColor(COLOR_INFO, TFT_BLACK);
    tft.drawString("TEMP", LEFT_COLUMN + LABEL_OFFSET, INDOOR_TEMP_Y + 8, INFO_FONT);
    
    // Clear and update humidity display area
    tft.fillRect(LEFT_COLUMN, INDOOR_HUM_Y - 5, TEXT_BUFFER, 35, TFT_BLACK);
    tft.setTextColor(COLOR_INDOOR, TFT_BLACK);
    String humStr = String(local_humidity, 1) + "%";
    tft.drawString(humStr, LEFT_COLUMN, INDOOR_HUM_Y, VALUE_FONT);
    tft.setTextColor(COLOR_INFO, TFT_BLACK);
    tft.drawString("RH", LEFT_COLUMN + LABEL_OFFSET, INDOOR_HUM_Y + 8, INFO_FONT);
    
    // Update last displayed values
    last_displayed_temp = local_temperature;
    last_displayed_hum = local_humidity;
  }
}

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  uint32_t duty = (LEDC_BASE_FREQ * min(value, valueMax)) / valueMax;
  ledcWrite(channel, duty);
}

// Get weather description based on code
String getWeatherDescription(int code) {
  switch (code) {
    case 0: return "CLEAR SKY";
    case 1: return "MAINLY CLEAR";
    case 2: return "PARTLY CLOUDY";
    case 3: return "OVERCAST";
    case 45: return "FOG";
    case 48: return "DEPOSITING RIME FOG";
    case 51: case 53: case 55: return "DRIZZLE";
    case 56: case 57: return "FREEZING DRIZZLE";
    case 61: case 63: case 65: return "RAIN";
    case 66: case 67: return "FREEZING RAIN";
    case 71: case 73: case 75: return "SNOW";
    case 77: return "SNOW GRAINS";
    case 80: case 81: case 82: return "RAIN SHOWERS";
    case 85: case 86: return "SNOW SHOWERS";
    case 95: return "THUNDERSTORM";
    case 96: case 99: return "THUNDERSTORM WITH HAIL";
    default: return "UNKNOWN";
  }
}

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && 
         millis() - startAttemptTime < 20000) { // 20 seconds timeout
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    // Configure NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    return true;
  } else {
    WiFi.disconnect();
    wifi_connected = false;
    return false;
  }
}

String getWeatherAdvice(int code, float temp, int hour) {
  String advice = "Advice: ";
  
  // Determine time period
  TimeSlot slot;
  if (hour >= 5 && hour < 12) slot = MORNING;
  else if (hour >= 12 && hour < 18) slot = NOON;
  else if (hour >= 18 && hour < 22) slot = EVENING;
  else slot = NIGHT;
  
  // Handle extreme temperatures first
  if (temp >= 35) {
    return advice + TEMP_ADVICE_MAP[0][slot];
  } else if (temp <= 5) {
    return advice + TEMP_ADVICE_MAP[1][slot];
  }
  
  // Handle weather conditions
  int weatherIndex;
  if (code <= 1) weatherIndex = 0;
  else if (code <= 3) weatherIndex = 1;
  else if (code <= 48) weatherIndex = 2;
  else if (code <= 55) weatherIndex = 3;
  else if (code <= 75) weatherIndex = 4;
  else weatherIndex = 5;
  
  // return advice + WEATHER_ADVICE_MAP[weatherIndex][slot];
  return WEATHER_ADVICE_MAP[weatherIndex][slot];
}

void updateDisplay() {
  // Clear screen
  tft.fillScreen(TFT_BLACK);
  
  if (!wifi_connected) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WiFi Disconnected", LEFT_COLUMN, INFO_LINE1_Y, 2);
    tft.drawString("Retrying in 1 minute...", LEFT_COLUMN, INFO_LINE2_Y, 2);
    return;
  }
  
  // Left column: Time and indoor data
  // ================================
  
  // [Rest of the code remains the same...]
  // Display time and date
  formatTime();
  tft.setTextColor(COLOR_TIME, TFT_BLACK);
  tft.drawString(timeStr, LEFT_COLUMN, INFO_LINE1_Y, TIME_FONT);      // Time
  tft.drawString(dateStr, LEFT_COLUMN, INFO_LINE2_Y, TITLE_FONT);     // Weekday
  tft.drawString(current_date, LEFT_COLUMN, INFO_LINE3_Y, TITLE_FONT); // Date
  strcpy(lastTimeStr, timeStr);
  strcpy(lastDateStr, dateStr);
  
  // Indoor data title
  tft.setTextColor(COLOR_TITLE, TFT_BLACK);
  tft.drawString("INDOOR", LEFT_COLUMN, INDOOR_TITLE_Y, TITLE_FONT);
  
  // Indoor temperature and humidity
  tft.setTextColor(COLOR_INDOOR, TFT_BLACK);
  String tempStr = String(local_temperature, 1) + "°C";
  String humStr = String(local_humidity, 1) + "%";
  
  tft.drawString(tempStr, LEFT_COLUMN, INDOOR_TEMP_Y, VALUE_FONT);
  tft.drawString(humStr, LEFT_COLUMN, INDOOR_HUM_Y, VALUE_FONT);
  
  // Indoor labels
  tft.setTextColor(COLOR_INFO, TFT_BLACK);
  tft.drawString("TEMP", LEFT_COLUMN + LABEL_OFFSET, INDOOR_TEMP_Y + 8, INFO_FONT);
  tft.drawString("RH", LEFT_COLUMN + LABEL_OFFSET, INDOOR_HUM_Y + 8, INFO_FONT);
  
  last_displayed_temp = local_temperature;
  last_displayed_hum = local_humidity;
  
  // Right column: Outdoor data
  // =========================
  
  // Display location, weather and update time in right column top
  tft.setTextColor(COLOR_TIME, TFT_BLACK);
  tft.drawString(location, RIGHT_COLUMN, INFO_LINE1_Y, TIME_FONT);     // City
  
  // Weather description (limited to one line)
  String desc = weather_description;
  if (desc.length() > 25) {
    desc = desc.substring(0, 22) + "...";
  }
  tft.drawString(desc, RIGHT_COLUMN, INFO_LINE2_Y, TITLE_FONT);       // Weather
  
  // Update time
  String updateStr = "Updated: " + last_weather_update;
  tft.drawString(updateStr, RIGHT_COLUMN, INFO_LINE3_Y, TITLE_FONT);  // Update time
  
  // Outdoor data title
  tft.setTextColor(COLOR_TITLE, TFT_BLACK);
  tft.drawString("OUTDOOR", RIGHT_COLUMN, OUTDOOR_TITLE_Y, TITLE_FONT);
  
  // Outdoor temperature and humidity
  tft.setTextColor(COLOR_OUTDOOR, TFT_BLACK);
  String outTempStr = temperature + "°C";
  String outHumStr = humidity + "%";
  
  tft.drawString(outTempStr, RIGHT_COLUMN, OUTDOOR_TEMP_Y, VALUE_FONT);
  tft.drawString(outHumStr, RIGHT_COLUMN, OUTDOOR_HUM_Y, VALUE_FONT);
  
  // Outdoor labels
  tft.setTextColor(COLOR_INFO, TFT_BLACK);
  tft.drawString("TEMP", RIGHT_COLUMN + LABEL_OFFSET, OUTDOOR_TEMP_Y + 8, INFO_FONT);
  tft.drawString("RH", RIGHT_COLUMN + LABEL_OFFSET, OUTDOOR_HUM_Y + 8, INFO_FONT);

  // Display weather advice
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  int currentHour = timeinfo.tm_hour;

  String advice = getWeatherAdvice(weather_code, temperature.toFloat(),timeinfo.tm_hour);
  tft.setTextColor(COLOR_INFO, TFT_BLACK);
  tft.drawString(advice, LEFT_COLUMN, WEATHER_ADVICE_Y, INFO_FONT);
}

void getWeatherData() {
  if (!wifi_connected) {
    return;
  }

  HTTPClient http;
  String url = String("http://api.open-meteo.com/v1/forecast?latitude=") + latitude + 
              "&longitude=" + longitude + 
              "&current=temperature_2m,relative_humidity_2m,is_day,weather_code" +
              "&timezone=" + timezone;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      const char* datetime = doc["current"]["time"];
      temperature = doc["current"]["temperature_2m"].as<String>();
      humidity = doc["current"]["relative_humidity_2m"].as<String>();
      is_day = doc["current"]["is_day"].as<int>();
      weather_code = doc["current"]["weather_code"].as<int>();
      
      String datetime_str = String(datetime);
      int splitIndex = datetime_str.indexOf('T');
      current_date = datetime_str.substring(0, splitIndex);
      last_weather_update = datetime_str.substring(splitIndex + 1, splitIndex + 9);
      
      weather_description = getWeatherDescription(weather_code);
      
      // Update display with new data
      updateDisplay();
    }
  }
  http.end();
}

void setup() {
  // Initialize TFT_eSPI
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  // tft.setRotation(2);

  Serial.begin(115200);
  Serial.println("\nInitializing...");
  
  // Initialize DHT22
  dht.begin();
  Serial.println("DHT22 Initialized");
  
  // Test DHT22 reading
  float test_temp = dht.readTemperature();
  float test_humidity = dht.readHumidity();
  Serial.print("Initial DHT22 Reading - Temperature: ");
  Serial.print(test_temp, 1);
  Serial.print("°C, Humidity: ");
  Serial.print(test_humidity, 1);
  Serial.println("%");
  if (isnan(test_temp) || isnan(test_humidity)) {
    Serial.println("DHT22 reading failed! Please check wiring:");
    Serial.println("1. Check if DHT22 is properly connected to PIN 22");
    Serial.println("2. Make sure VCC is connected to 3.3V");
    Serial.println("3. Make sure GND is connected to GND");
    Serial.println("4. Check if a 10K pull-up resistor is connected between VCC and data pin");
  }
  
  // Initialize LED control
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, 8);
  ledcAttachPin(TFT_BL, LEDC_CHANNEL_0);
  
  // Set initial brightness
  uint32_t brightnessValue = (100 * 255) / 100;
  ledcAnalogWrite(LEDC_CHANNEL_0, brightnessValue);
  
  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Show connecting message
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connecting to WiFi...", LEFT_COLUMN, INFO_LINE1_Y, 2);
  
  // Initial WiFi connection attempt
  if (connectWiFi()) {
    getWeatherData();
  } else {
    updateDisplay();
  }
  
  // Set Timezone
  configTime(TIMEZONE_OFFSET * 3600, 0, "pool.ntp.org");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Read DHT22 sensor
  readDHT();
  
  // Update only sensor display every 2 seconds
  if (currentMillis - last_dht_read >= DHT_UPDATE_INTERVAL) {
    updateSensorDisplay();  // Only update the temperature and humidity display
    last_dht_read = currentMillis;
  }
  
  // Update time display every minute
  if (wifi_connected && currentMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    updateTimeDisplay();
    lastTimeUpdate = currentMillis;
  }
  
  // Check WiFi connection and try to reconnect if necessary
  if (!wifi_connected && 
      currentMillis - last_wifi_retry >= WIFI_RETRY_INTERVAL) {
    if (connectWiFi()) {
      getWeatherData();
      updateDisplay();  // Update the entire display only after reconnection
    }
    last_wifi_retry = currentMillis;
  }
  
  // Update weather if connected
  if (wifi_connected && currentMillis - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
    getWeatherData();
    updateDisplay();  // Update the entire display only after new weather data is obtained
    lastWeatherUpdate = currentMillis;
  }
  
  delay(100);
}
