### Cyd-AtmosGuard (ESP32 Weather) Clock Project Overview  

This is an **ESP32-Cheap-Yellow-Display-based weather clock** that integrates **indoor and outdoor environment monitoring** and **weather reminders**. It’s perfect for home or office use.  

![AtmosGuard Weather Clock](picture.png)

## 🌐 Language Options

- [粵語](README_CANTON.md)
- [简体中文](README_CN.md)
- [Bahasa Melayu](README_MS.md)

---

## 🌟 Core Features  

### 📟 1. Display Information  

📌 **Left Screen:**  
- Current **time** and **date** (synchronized with NTP servers for accuracy)  
- Indoor **temperature** and **humidity** (real-time detection via DHT22 sensor)  

📌 **Right Screen:**  
- **City location** (approximate location based on IP)  
- **Outdoor weather** conditions (sunny, cloudy, rainy, snowy, etc.)  
- **Outdoor temperature and humidity** (real-time data from Open-Meteo API)  

📌 **Bottom Smart Suggestions:**  
- Provides **clothing and travel reminders** based on the weather and time of day  
- Example: *"☔ It's raining now, don't forget to bring an umbrella!"*  

---

## 📡 2. Data Collection  

- **Indoor temperature & humidity**: DHT22 sensor (connected to IO22)  
- **Outdoor weather**: Open-Meteo API for real-time weather updates  
- **Time synchronization**: NTP servers to ensure clock accuracy  

---

## 🔔 3. Smart Notifications  

- **Time-based reminders** (morning / noon / evening / night)  
- **Weather-aware alerts** (sunny, cloudy, foggy, rainy, snowy, thunderstorms)  
- **Extreme temperature warnings**:  
  - *Above 35°C* → Heatwave alert  
  - *Below 5°C* → Cold weather warning  

---

## ⚙ 4. Automation & Smart Refresh  

- **Auto WiFi reconnect** to prevent network issues  
- **Scheduled updates:**  
  - **Time**: Periodically refreshed  
  - **Weather**: Fetched at set intervals  
  - **Sensor data**: Updated only when values change to save resources  

---

## 🚨 5. Reliable Error Handling  

- **WiFi connection monitoring**: If disconnected, the screen displays a **“Network Connection Error”**  

---

## ⚙ Configuration File (PlatformIO)  

This project is built with **PlatformIO**, supporting multiple hardware configurations. You can select different environments (`env`) for compilation based on their screen type:  

- **cyd**: For ILI9341 screens  
- **cyd2usb**: For ST7789 screens with RGB inversion and BGR color order adjustments  

Additionally, the `platformio.ini` configuration file includes **WiFi credentials, weather API keys, time zone settings, and update intervals**, which you can customize based on their needs.  