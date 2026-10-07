#include <Arduino.h>
#include <DHT.h>

// Pino GPIO onde o pino DATA do DHT11 está conectado
#define DHTPIN 21

// Modelo do sensor (DHT11)
#define DHTTYPE DHT11

// Cria a instância do sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000); // Aguarda estabilização da comunicação serial

  Serial.println("\n========================================");
  Serial.println("  Teste do Sensor DHT11 com ESP32");
  Serial.println("========================================");

  dht.begin();
  Serial.println("Sensor DHT11 iniciado com sucesso!");
}

void loop() {
  // O DHT11 precisa de pelo menos 1 a 2 segundos de intervalo entre leituras
  delay(2000);

  // Leitura da umidade relativa do ar (%)
  float umidade = dht.readHumidity();

  // Leitura da temperatura em graus Celsius (°C)
  float temperatura = dht.readTemperature();

  // Verifica se as leituras são válidas (se retornou NaN, ocorreu erro de leitura)
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("[AVISO] Falha ao ler do sensor DHT11! Verifique as conexões no pino 21.");
    return;
  }

  // Calcula o índice de calor / sensação térmica em Celsius
  float sensacao = dht.computeHeatIndex(temperatura, umidade, false);

  Serial.printf("Temperatura: %.1f °C | Umidade: %.1f %% | Sensação Térmica: %.1f °C\n",
                temperatura, umidade, sensacao);
}