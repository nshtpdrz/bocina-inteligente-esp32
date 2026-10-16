#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SpotifyArduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "secrets.h"   // copia secrets.example.h como secrets.h

// -------------------- CONFIG PANTALLA OLED --------------------
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pines I2C para tu pantalla
#define I2C_SDA 26
#define I2C_SCL 27

// -------------------- CONFIG AUDIO DAC (Fase 1) --------------------
#define PIN_AUDIO_DAC 25 

// -------------------- CONFIG WIFI --------------------
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// -------------------- TU API DE SPOTIFY CORREGIDA --------------------
const char* clientId = SPOTIFY_CLIENT_ID;
const char* clientSecret = SPOTIFY_CLIENT_SECRET;

// Tu Refresh Token definitivo que dio Status 200 en la autenticación
const char* refreshToken = SPOTIFY_REFRESH_TOKEN;

WiFiClientSecure spotifyClient;

// 🛠️ CORRECCIÓN: Usamos el constructor de 3 parámetros igual que en tu otro proyecto
SpotifyArduino spotify(spotifyClient, clientId, clientSecret);

unsigned long delaySpotify = 4000; 
unsigned long ultimaConsultaSpotify = 0;

String currentTrack = "Ninguna";
String currentArtist = "Desconocido";

void sonarTonoConexion() {
  for (int j = 0; j < 100; j++) { 
    for (int i = 0; i < 360; i += 15) {
      int valorDAC = (sin(i * 0.01745) * 127) + 128;
      dacWrite(PIN_AUDIO_DAC, valorDAC);
      delayMicroseconds(100);
    }
  }
  dacWrite(PIN_AUDIO_DAC, 0); 
}

// Función callback para procesar la canción
void alRecibirDatosSpotify(CurrentlyPlaying currentlyPlaying) {
  currentTrack = currentlyPlaying.trackName;
  currentArtist = currentlyPlaying.artists[0].artistName;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("F2: CLIENTE SPOTIFY"));
  display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
  
  display.setCursor(0, 18);
  display.print(F("Rola: ")); 
  display.println(currentTrack.substring(0, 16)); 
  
  display.setCursor(0, 38);
  display.print(F("Art: ")); 
  display.println(currentArtist.substring(0, 16));
  
  if(currentlyPlaying.isPlaying) {
    display.setCursor(110, 54);
    display.print(F("[>]"));
  }
  display.display();
}

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
  display.setCursor(15, 25);
  display.print(F("Conectando Wi-Fi..."));
  display.display();

  pinMode(PIN_AUDIO_DAC, OUTPUT);

  // Conexión Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  spotifyClient.setInsecure(); 

  // 🛠️ CORRECCIÓN: Inyectamos el refresh token en el setup de la forma que te funcionaba
  spotify.setRefreshToken(refreshToken);

  display.clearDisplay();
  display.setCursor(10, 20);
  display.print(F("¡Fase 2 Lista!"));
  display.setCursor(10, 40);
  display.print(F("Spotify Online"));
  display.display();
  
  sonarTonoConexion(); 
  delay(2000);
}

void loop() {
  unsigned long tiempoActual = millis();

  if (tiempoActual - ultimaConsultaSpotify > delaySpotify) {
    ultimaConsultaSpotify = tiempoActual;
    
    Serial.println(F("Consultando Spotify..."));

    // 🛠️ CORRECCIÓN: Quitamos el "MX" para usar la llamada simple que tenías antes
    int status = spotify.getCurrentlyPlaying(alRecibirDatosSpotify);
    
    if (status == 204) { 
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(F("F2: CLIENTE SPOTIFY"));
      display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
      display.setCursor(10, 28);
      display.print(F("Pon una cancion"));
      display.setCursor(10, 42);
      display.print(F("en tu celular..."));
      display.display();
    }
  }
}