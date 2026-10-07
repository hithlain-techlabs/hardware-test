#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Definição dos pinos I2C
#define I2C_SDA 8
#define I2C_SCL 9

Adafruit_BMP280 bmp; // Instância do sensor BMP280

// Função auxiliar para escanear o barramento I2C caso o sensor não responda
void scanI2C() {
  Serial.println("\n--- Iniciando I2C Scanner ---");
  byte count = 0;
  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Dispositivo I2C encontrado no endereco: 0x%02X\n", address);
      count++;
    }
  }
  if (count == 0) {
    Serial.println("Nenhum dispositivo I2C encontrado. Verifique as conexoes fisicas!");
  }
  Serial.println("--- Fim do scan ---\n");
}

void setup() {
  Serial.begin(115200);
  delay(1500); // Aguarda a porta serial estabilizar

  Serial.println("\n====================================");
  Serial.println("    Teste do Sensor BMP280 - ESP32   ");
  Serial.println("====================================");

  // Inicializa a comunicacao I2C com os pinos customizados
  Wire.begin(I2C_SDA, I2C_SCL);

  // O BMP280 costuma utilizar o endereco 0x76 ou 0x77
  bool status = bmp.begin(0x76);
  if (!status) {
    Serial.println("Nao foi possivel encontrar o BMP280 no endereco 0x76. Tentando 0x77...");
    status = bmp.begin(0x77);
  }

  if (!status) {
    Serial.println("ERRO: Sensor BMP280 nao encontrado!");
    Serial.println("Dicas:");
    Serial.println("1. Verifique se SDA esta no pino 8 e SCL no pino 9");
    Serial.println("2. Verifique se VCC esta em 3V3 e GND em GND");
    scanI2C();
    while (1) {
      delay(1000);
    }
  }

  Serial.println("Sensor BMP280 inicializado com sucesso!");

  /* Configuracoes recomendadas para medicoes gerais */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Modo de operacao */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressao oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtro IIR */
                  Adafruit_BMP280::STANDBY_MS_500); /* Tempo em standby */
}

void loop() {
  float temperatura = bmp.readTemperature();
  float pressao = bmp.readPressure() / 100.0F; // Converte Pa para hPa
  float altitude = bmp.readAltitude(1013.25);   // 1013.25 hPa e o padrao ao nivel do mar

  Serial.printf("Temperatura: %.2f °C | Pressao: %.2f hPa | Altitude Aprox.: %.2f m\n",
                temperatura, pressao, altitude);

  delay(2000);
}