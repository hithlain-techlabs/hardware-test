#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

// Definição dos pinos I2C
#define I2C_SDA 8
#define I2C_SCL 9

BH1750 lightMeter;

void setup() {
  Serial.begin(115200);
  delay(1000); // Aguarda estabilização da serial

  Serial.println("\n--- Teste do Sensor de Luminosidade BH1750 ---");

  // Inicializa o I2C nos pinos especificados (SDA=8, SCL=9)
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicializa o sensor BH1750
  // Endereço padrão: 0x23 (quando ADDR está desconectado ou em GND)
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
    Serial.println("Sensor BH1750 inicializado com sucesso!");
  } else {
    Serial.println("Erro ao inicializar o BH1750! Verifique as conexões.");
    Serial.println("Dica: Se o pino ADDR do módulo estiver no 3.3V, o endereço pode ser 0x5C.");
  }
}

void loop() {
  // Leitura do nível de luminosidade em lux
  float lux = lightMeter.readLightLevel();

  Serial.print("Luminosidade: ");
  Serial.print(lux);
  Serial.println(" lx");

  delay(1000);
}