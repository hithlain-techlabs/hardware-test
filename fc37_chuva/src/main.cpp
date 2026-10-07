#include <Arduino.h>

// Definição dos pinos conforme as conexões
#define PIN_AO 1 // Entrada Analógica (AO do sensor conectado ao GPIO 1)
#define PIN_DO 4 // Entrada Digital (DO do sensor conectado ao GPIO 4)

void setup() {
  // Inicialização da porta serial
  Serial.begin(115200);
  delay(1000); // Aguarda estabilizar a serial

  // Configuração dos pinos
  pinMode(PIN_DO, INPUT);
  // O pino analógico no ESP32 não precisa de pinMode, mas podemos definir a resolução
  analogReadResolution(12); // Resolução padrão de 12 bits (0 a 4095)

  Serial.println("=========================================");
  Serial.println("   Teste Sensor de Chuva FC-37 + ESP32   ");
  Serial.println("=========================================");
}

void loop() {
  // Leitura analógica (0 a 4095)
  // Seco: valor alto (~4095) | Molhado/Chuva: valor baixo (tende a 0)
  int valorAnalogico = analogRead(PIN_AO);

  // Leitura digital do comparador LM393
  // HIGH (1): Seco | LOW (0): Chuva detectada (limiar do trimpot)
  int estadoDigital = digitalRead(PIN_DO);

  // Conversão para porcentagem estimada de umidade/intensidade (0% a 100%)
  // Mapeamos invertido porque quanto menor a leitura, mais água na placa
  int intensidadeChuva = map(valorAnalogico, 4095, 0, 0, 100);
  intensidadeChuva = constrain(intensidadeChuva, 0, 100);

  // Exibição dos resultados no Serial Monitor
  Serial.print("[Analógico]: ");
  Serial.print(valorAnalogico);
  Serial.print(" (");
  Serial.print(intensidadeChuva);
  Serial.print("%) | [Digital]: ");

  if (estadoDigital == LOW) {
    Serial.println("0 - CHUVA DETECTADA!");
  } else {
    Serial.println("1 - Seco (Sem chuva)");
  }

  delay(1000); // Aguarda 1 segundo entre as leituras
}