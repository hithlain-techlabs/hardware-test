#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

/*
 * ==============================================================================
 *  PROJETO INTEGRADO DE TESTE DE SENSORES - ESP32-S3
 * ==============================================================================
 *  Conexões da Protoboard (conforme sensores.txt):
 * 
 *  - Barramento de Alimentação:
 *      3V3 -> VCC de todos os sensores
 *      GND -> GND de todos os sensores
 * 
 *  - DHT11 (Temperatura & Umidade):
 *      DATA / OUT -> GPIO 21
 * 
 *  - FC-37 (Sensor de Chuva):
 *      AO (Analógico) -> GPIO 1
 *      DO (Digital)   -> GPIO 4
 * 
 *  - Barramento I2C (Compartilhado):
 *      SDA -> GPIO 8 (BMP280 + BH1750)
 *      SCL -> GPIO 9 (BMP280 + BH1750)
 * ==============================================================================
 */

// --- Definição dos Pinos ---
#define PIN_DHT 21
#define DHTTYPE DHT11

#define PIN_CHUVA_AO 1
#define PIN_CHUVA_DO 4

#define I2C_SDA 8
#define I2C_SCL 9

// --- Instâncias dos Sensores ---
DHT dht(PIN_DHT, DHTTYPE);
Adafruit_BMP280 bmp;
BH1750 lightMeter;

// --- Flags de Status dos Sensores ---
bool bmpAtivo = false;
bool bh1750Ativo = false;

// Intervalo de leitura (DHT11 requer ao menos 2 segundos entre leituras)
const unsigned long INTERVALO_LEITURA = 2000;
unsigned long ultimaLeitura = 0;

// Função auxiliar para escanear dispositivos I2C
void scanI2C() {
  Serial.println("\n[I2C] Escaneando barramento I2C (SDA=8, SCL=9)...");
  byte totalEncontrados = 0;
  for (byte endereco = 1; endereco < 127; endereco++) {
    Wire.beginTransmission(endereco);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  -> Dispositivo encontrado no endereco: 0x%02X\n", endereco);
      totalEncontrados++;
    }
  }
  if (totalEncontrados == 0) {
    Serial.println("  [AVISO] Nenhum dispositivo I2C detectado. Verifique os pinos SDA/SCL e alimentacao.");
  }
  Serial.println("[I2C] Fim da varredura.\n");
}

void setup() {
  Serial.begin(115200);
  delay(1500); // Aguarda estabilização da conexão serial

  Serial.println("\n==================================================");
  Serial.println("  Inicializando Sistema Integrado de Sensores     ");
  Serial.println("==================================================");

  // 1. Configuração I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // 2. Inicialização do DHT11
  Serial.print("[DHT11] Inicializando no pino ");
  Serial.print(PIN_DHT);
  Serial.println("...");
  dht.begin();
  Serial.println("[DHT11] Pronto!");

  // 3. Configuração do Sensor de Chuva FC-37
  Serial.println("[FC-37] Configurando pinos de chuva (AO=1, DO=4)...");
  pinMode(PIN_CHUVA_DO, INPUT);
  analogReadResolution(12); // Resolução de 12 bits (0 a 4095)
  Serial.println("[FC-37] Pronto!");

  // 4. Inicialização do Barômetro BMP280 (Tenta 0x76, depois 0x77)
  Serial.println("[BMP280] Inicializando...");
  if (bmp.begin(0x76)) {
    bmpAtivo = true;
    Serial.println("[BMP280] Conectado no endereco 0x76!");
  } else if (bmp.begin(0x77)) {
    bmpAtivo = true;
    Serial.println("[BMP280] Conectado no endereco 0x77!");
  } else {
    Serial.println("[BMP280] [ERRO] Nao foi possivel detectar o BMP280!");
  }

  if (bmpAtivo) {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,
                    Adafruit_BMP280::SAMPLING_X16,
                    Adafruit_BMP280::FILTER_X16,
                    Adafruit_BMP280::STANDBY_MS_500);
  }

  // 5. Inicialização do Sensor de Luminosidade BH1750 (Tenta 0x23, depois 0x5C)
  Serial.println("[BH1750] Inicializando...");
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
    bh1750Ativo = true;
    Serial.println("[BH1750] Conectado no endereco 0x23!");
  } else if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C, &Wire)) {
    bh1750Ativo = true;
    Serial.println("[BH1750] Conectado no endereco 0x5C!");
  } else {
    Serial.println("[BH1750] [ERRO] Nao foi possivel detectar o BH1750!");
  }

  // Se algum sensor I2C falhou, roda scanner para diagnóstico
  if (!bmpAtivo || !bh1750Ativo) {
    scanI2C();
  }

  Serial.println("\n--- Inicializacao concluida. Coletando dados... ---\n");
}

void loop() {
  unsigned long agora = millis();
  if (agora - ultimaLeitura < INTERVALO_LEITURA) {
    return;
  }
  ultimaLeitura = agora;

  Serial.println("------------------- LEITURA DOS SENSORES -------------------");

  // --- Leitura do DHT11 ---
  float dhtUmid = dht.readHumidity();
  float dhtTemp = dht.readTemperature();
  if (isnan(dhtUmid) || isnan(dhtTemp)) {
    Serial.println("• DHT11:   [FALHA DE LEITURA] Verifique a conexao no pino 21.");
  } else {
    float sensacao = dht.computeHeatIndex(dhtTemp, dhtUmid, false);
    Serial.printf("• DHT11:   Temp: %.1f °C | Umidade: %.1f %% | Sensacao: %.1f °C\n",
                  dhtTemp, dhtUmid, sensacao);
  }

  // --- Leitura do BMP280 ---
  if (bmpAtivo) {
    float bmpTemp = bmp.readTemperature();
    float bmpPressao = bmp.readPressure() / 100.0F; // Pa para hPa
    float altitude = bmp.readAltitude(1013.25);     // 1013.25 hPa no nivel do mar
    Serial.printf("• BMP280:  Temp: %.2f °C | Pressao: %.2f hPa | Alt. Aprox: %.1f m\n",
                  bmpTemp, bmpPressao, altitude);
  } else {
    Serial.println("• BMP280:  [SENSOR OFFLINE / NAO DETECTADO]");
  }

  // --- Leitura do BH1750 ---
  if (bh1750Ativo) {
    float lux = lightMeter.readLightLevel();
    Serial.printf("• BH1750:  Luminosidade: %.1f lx\n", lux);
  } else {
    Serial.println("• BH1750:  [SENSOR OFFLINE / NAO DETECTADO]");
  }

  // --- Leitura do FC-37 (Chuva) ---
  int valorAnalogico = analogRead(PIN_CHUVA_AO);
  int estadoDigital = digitalRead(PIN_CHUVA_DO);

  // Mapeia 4095 (seco) -> 0% e 0 (muita água) -> 100%
  int intensidadeChuva = map(valorAnalogico, 4095, 0, 0, 100);
  intensidadeChuva = constrain(intensidadeChuva, 0, 100);

  const char* statusChuva = (estadoDigital == LOW) ? "CHUVA DETECTADA" : "Seco (Sem chuva)";
  Serial.printf("• FC-37:   Analog: %d (%d%%) | Digital (DO): %d [%s]\n",
                valorAnalogico, intensidadeChuva, estadoDigital, statusChuva);

  Serial.println();
}