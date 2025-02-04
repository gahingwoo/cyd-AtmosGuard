### Gambaran Keseluruhan Projek Jam Cuaca Cyd  

Ini ialah **ESP32-Cheap-Yellow-Display** yang menggabungkan **pemantauan persekitaran dalaman dan luaran** serta **peringatan cuaca**. Sesuai untuk kegunaan rumah atau pejabat.  

---  

## 🌟 Ciri Utama  

### 📟 1. Paparan Maklumat  

📌 **Skrin Kiri:**  
- **Masa** dan **tarikh** semasa (diselaraskan dengan pelayan NTP untuk ketepatan)  
- **Suhu** dan **kelembapan** dalaman (pengesanan masa nyata melalui sensor DHT22)  

📌 **Skrin Kanan:**  
- **Lokasi bandar** (anggaran lokasi berdasarkan IP)  
- **Keadaan cuaca luaran** (cerah, mendung, hujan, salji, dll.)  
- **Suhu dan kelembapan luaran** (data masa nyata dari API Open-Meteo)  

📌 **Cadangan Pintar di Bawah:**  
- Memberi **peringatan pakaian dan perjalanan** berdasarkan cuaca dan masa  
- Contoh: *"☔ Sekarang hujan, jangan lupa bawa payung!"*  

---  

## 📡 2. Pengumpulan Data  

- **Suhu & kelembapan dalaman**: Sensor DHT22 (disambungkan ke IO22)  
- **Cuaca luaran**: API Open-Meteo untuk kemas kini cuaca masa nyata  
- **Penyegerakan masa**: Pelayan NTP untuk memastikan ketepatan jam  

---  

## 🔔 3. Pemberitahuan Pintar  

- **Peringatan berdasarkan masa** (pagi / tengah hari / petang / malam)  
- **Makluman cuaca** (cerah, mendung, berkabus, hujan, salji, ribut petir)  
- **Amaran suhu melampau**:  
  - *Lebih 35°C* → Amaran gelombang haba  
  - *Bawah 5°C* → Amaran cuaca sejuk  

---  

## ⚙ 4. Automasi & Penyegaran Pintar  

- **Auto sambung semula WiFi** untuk mengelakkan masalah rangkaian  
- **Kemas kini berjadual:**  
  - **Masa**: Dikemas kini secara berkala  
  - **Cuaca**: Dapatkan data pada selang masa tertentu  
  - **Data sensor**: Dikemas kini hanya apabila nilai berubah untuk menjimatkan sumber  

---  

## 🚨 5. Pengendalian Ralat Yang Boleh Dipercayai  

- **Pemantauan sambungan WiFi**: Jika terputus, skrin akan memaparkan **"Ralat Sambungan Rangkaian"**  

---  

## ⚙ Fail Konfigurasi (PlatformIO)  

Projek ini dibina menggunakan **PlatformIO**, menyokong pelbagai konfigurasi perkakasan. Anda boleh memilih persekitaran (`env`) yang berbeza untuk penyusunan berdasarkan jenis skrin:  

- **cyd**: Untuk skrin ILI9341  
- **cyd2usb**: Untuk skrin ST7789 dengan penyongsangan RGB dan pelarasan warna BGR  

Selain itu, fail konfigurasi `platformio.ini` mengandungi **kelayakan WiFi, kunci API cuaca, tetapan zon waktu, dan selang kemas kini**, yang boleh disesuaikan mengikut keperluan anda.