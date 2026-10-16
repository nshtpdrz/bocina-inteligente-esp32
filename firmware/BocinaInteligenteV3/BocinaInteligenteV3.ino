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
#define I2S_LRC  25
#define I2S_BCLK 32
#define I2S_DOUT 33

BluetoothA2DPSink a2dp_sink;

void setRGB(int r, int g, int b) {
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void avisa_cancion(uint8_t id, const uint8_t *text) {
  if (id == 1) {
    String titulo = String((char*)text);
    Serial.print("Sonando: "); Serial.println(titulo);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("BLUETOOTH:");
    display.drawFastHLine(0, 11, 128, SSD1306_WHITE);
    display.setCursor(0, 25);
    display.print(titulo);
    display.display();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  setRGB(0, 0, 255);

  Wire.begin(26, 27);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED fallo");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.print("Buscando Bluetooth...");
  display.display();

  // Configurar pines I2S
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

  Serial.println("Bluetooth listo!");
  Serial.println("Busca 'Bocina Inteligente' en tu celular");

  display.clearDisplay();
  display.setCursor(0, 20);
  display.print("Bocina Inteligente");
  display.setCursor(0, 40);
  display.print("Listo para conectar");
  display.display();
}

void loop() {
  // Cambia color LED según estado
  esp_a2d_audio_state_t estado = a2dp_sink.get_audio_state();

  if (estado == ESP_A2D_AUDIO_STATE_STARTED) {
    setRGB(0, 255, 0);   // Verde = reproduciendo
  } else {
    setRGB(0, 0, 255);   // Azul = esperando
  }

  delay(100);
}