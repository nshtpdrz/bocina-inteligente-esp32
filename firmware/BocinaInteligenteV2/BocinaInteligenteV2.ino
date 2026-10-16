#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Audio.h" // Asegúrate de tener instalada la librería ESP32-audioI2S
#include "secrets.h"   // copia secrets.example.h como secrets.h

// -------------------- CONFIG PANTALLA OLED --------------------
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define I2C_SDA 26
#define I2C_SCL 27

// -------------------- CONFIG WIFI --------------------
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// -------------------- CONFIG AUDIO --------------------
Audio audio;

void setup() {
  Serial.begin(115200);

  // Inicializar Pantalla OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 falló"));
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.print(F("Conectando Wi-Fi..."));
  display.display();

  // Conexión Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clearDisplay();
  display.setCursor(10, 20);
  display.print(F("¡Wi-Fi Conectado!"));
  display.setCursor(10, 40);
  display.print(F("Iniciando Audio..."));
  display.display();
  delay(1500);

  // 🛠️ CONFIGURACIÓN DEL DAC INTERNO CORREGIDA
  // Pasando -1, -1, 25 la librería activa el audio analógico en el Pin 25 directo
  audio.setPinout(-1, -1, 25); 
  audio.setVolume(21); // Rango de volumen de 0 a 21

  // Estación de radio pública en formato MP3 para la prueba de streaming
  audio.connecttohost("http://stream.zeno.fm/06697a1w07zhv"); 

  display.clearDisplay();
  display.setCursor(0, 20);
  display.print(F("Streaming Wi-Fi:"));
  display.setCursor(0, 40);
  display.print(F("Sonando en Pin 25..."));
  display.display();
}

void loop() {
  // Es vital mantener el loop libre de "delays" para que el audio no se corte
  audio.loop(); 
}

// ---- Funciones de estado para el monitor serie y actualización de pantalla ----
void audio_showstation(const char *info){
    Serial.print("Estación: "); Serial.println(info);
}

void audio_showstreamtitle(const char *info){
    Serial.print("Rola actual: "); Serial.println(info);
    
    // Muestra en la pantalla OLED el título de la canción que viene del stream
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(F("STREAMING WI-FI"));
    display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
    display.setCursor(0, 25);
    display.print(info);
    display.display();
}