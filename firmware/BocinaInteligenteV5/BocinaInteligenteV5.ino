#include "BluetoothA2DPSink.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CONFIG LED RGB ---
#define PIN_R 14
#define PIN_G 12
#define PIN_B 13

// --- CONFIG PANTALLA OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CONFIG AUDIO I2S ---
#define I2S_LRC   25
#define I2S_BCLK 32
#define I2S_DOUT 33

// --- CONFIG PERILLA (VOLUMEN) ---
#define PIN_POT 34
int ultimoVol = -1;
unsigned long ultimaLecturaPot = 0;

// --- CONFIG BOTONES ---
#define BTN_PREV 18
#define BTN_PLAY 19
#define BTN_NEXT 23

BluetoothA2DPSink a2dp_sink;

// Variables para la música y barra de progreso autónoma
String glob_titulo = "Bocina Lista";
String glob_artista = "Conecta tu cel";
uint32_t pos_actual = 0;
uint32_t duracion_total = 180000; // 3 minutos base por defecto
unsigned long ultimoDebounce = 0;
unsigned long ultimoIncrementoBarra = 0;
uint8_t tonoArcoiris = 0;
int scrollPos = 0;
unsigned long ultimoScroll = 0;

void setRGB(int r, int g, int b) {
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void fijarColorArcoiris(uint8_t posicion) {
  posicion = 255 - posicion;
  if (posicion < 85) {
    setRGB(255 - posicion * 3, 0, posicion * 3);
  } else if (posicion < 170) {
    posicion -= 85;
    setRGB(0, posicion * 3, 255 - posicion * 3);
  } else {
    posicion -= 170;
    setRGB(posicion * 3, 255 - posicion * 3, 0);
  }
}

// Dibuja la pantalla con Barra Autónoma y Texto en Movimiento
void actualizarPantalla() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Encabezado
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("SPOTIFY BLUETOOTH");
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  
  // Título con Efecto de Movimiento (Scroll) si es muy largo
  display.setCursor(scrollPos, 18);
  display.print(glob_titulo); 
  
  // Artista fijo abajo
  display.setCursor(0, 32);
  display.print(glob_artista.substring(0, 20));
  
  // Dibujar Barra de Reproducción Inteligente
  display.drawRect(0, 50, 128, 6, SSD1306_WHITE); 
  if (duracion_total > 0) {
    int anchoBarra = map(pos_actual, 0, duracion_total, 0, 126);
    if(anchoBarra > 126) anchoBarra = 126;
    if(anchoBarra < 0) anchoBarra = 0;
    display.fillRect(1, 51, anchoBarra, 4, SSD1306_WHITE); 
  }
  
  display.display();
}

// Lector de canciones
void avisa_cancion(uint8_t id, const uint8_t *text) {
  if (id == 1) { // Título
    glob_titulo = String((char*)text);
    pos_actual = 0; // Reiniciar el progreso del ESP32 al cambiar de canción
    scrollPos = 0;  // Reiniciar posición del texto
  }
  else if (id == 2) { // Artista
    glob_artista = String((char*)text);
  }
  actualizarPantalla();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_R, OUTPUT); pinMode(PIN_G, OUTPUT); pinMode(PIN_B, OUTPUT);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_PLAY, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  
  setRGB(0, 0, 255); 

  Wire.begin(26, 27);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED fallo");
  
  actualizarPantalla();

  i2s_pin_config_t mis_pines = {
    .mck_io_num   = I2S_PIN_NO_CHANGE,
    .bck_io_num   = I2S_BCLK,
    .ws_io_num    = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num  = I2S_PIN_NO_CHANGE
  };

  a2dp_sink.set_pin_config(mis_pines);
  a2dp_sink.set_avrc_metadata_callback(avisa_cancion);
  a2dp_sink.start("Bocina Inteligente");
}

void loop() {
  esp_a2d_audio_state_t estado = a2dp_sink.get_audio_state();

  // 1. LED RGB ARCOÍRIS + PROGRESO INTERNO DE LA BARRA
  if (estado == ESP_A2D_AUDIO_STATE_STARTED) {
    fijarColorArcoiris(tonoArcoiris);
    tonoArcoiris++;
    
    // El ESP32 cuenta el tiempo de forma autónoma cada segundo
    if (millis() - ultimoIncrementoBarra >= 1000) {
      ultimoIncrementoBarra = millis();
      pos_actual += 1000;
      // Si llega al final simulado, se detiene para no desbordar
      if (pos_actual > duracion_total) pos_actual = duracion_total;
      actualizarPantalla();
    }

    // Efecto de movimiento para nombres largos
    if (glob_titulo.length() > 14 && millis() - ultimoScroll >= 350) {
      ultimoScroll = millis();
      scrollPos -= 3;
      if (scrollPos < (int)(glob_titulo.length() * -6)) { 
        scrollPos = 128; // Reinicia desde la derecha
      }
      actualizarPantalla();
    }

  } else {
    setRGB(0, 0, 255); // Azul en pausa
  }

  // 2. CONTROL DE BOTONES FÍSICOS
  if (millis() - ultimoDebounce > 350) {
    if (digitalRead(BTN_PLAY) == LOW) {
      if (estado == ESP_A2D_AUDIO_STATE_STARTED) {
        a2dp_sink.pause();
      } else {
        a2dp_sink.play();
      }
      ultimoDebounce = millis();
    }
    else if (digitalRead(BTN_NEXT) == LOW) {
      a2dp_sink.next();
      ultimoDebounce = millis();
    }
    else if (digitalRead(BTN_PREV) == LOW) {
      a2dp_sink.previous();
      ultimoDebounce = millis();
    }
  }

  // 3. LECTURA DE PERILLA (VOLUMEN)
  unsigned long tiempoActual = millis();
  if (tiempoActual - ultimaLecturaPot > 150) {
    ultimaLecturaPot = tiempoActual;
    int lectura = analogRead(PIN_POT);
    int vol = map(lectura, 0, 4095, 0, 128); 
    if (abs(vol - ultimoVol) >= 4) { 
      ultimoVol = vol;
      a2dp_sink.set_volume(ultimoVol);
    }
  }

  delay(15); 
}