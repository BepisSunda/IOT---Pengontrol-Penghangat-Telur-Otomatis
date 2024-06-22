#define BLYNK_TEMPLATE_ID "TMPL6bjDqc82i"
#define BLYNK_TEMPLATE_NAME "TUBES"
#define DHTTYPE DHT11

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "1303210181";
const char* password = "1303210181";
char auth[] = "1AskO7TFrbwvzVn3sHbrOfM6koy3YYWa"; // Ganti dengan token auth Blynk Anda

// GPIO pin definitions
const int kipas = D4; // GPIO untuk relay kipas
const int lampu = D3; // GPIO untuk relay lampu
const int sensorSuhu = D1; // GPIO untuk sensor suhu dan kelembaban

DHT dht(sensorSuhu, DHTTYPE); // Inisialisasi sensor DHT

BlynkTimer timer;
bool fanManual = false; // Flag untuk indikasi pengendalian manual kipas
float lastTemperature = 0.0; // Menyimpan suhu terakhir yang terbaca
int lastSwitchState = -1; // Menyimpan status saklar terakhir

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);

  pinMode(lampu, OUTPUT); // Set pin mode untuk relay lampu
  pinMode(kipas, OUTPUT); // Set pin mode untuk relay kipas

  dht.begin(); // Inisialisasi sensor DHT

  digitalWrite(lampu, HIGH); // Mulai dengan lampu menyala
  digitalWrite(kipas, LOW); // Mulai dengan kipas mati (aktif rendah)

  // Setup handler untuk pin virtual Blynk
  Blynk.virtualWrite(V3, LOW); // Inisialisasi keadaan saklar V3 menjadi OFF (0)
  Blynk.virtualWrite(V2, 0.0); // Inisialisasi V2 dengan nilai suhu (opsional)

  timer.setInterval(1000L, updateSuhu); // Set interval untuk pembaruan suhu
  Blynk.syncVirtual(V3); // Sync state of V3 at startup
}

void loop() {
  Blynk.run(); // Jalankan Blynk
  timer.run(); // Jalankan timer Blynk
}

// Handler Blynk untuk Pin Virtual V3 untuk mengontrol manual kipas
BLYNK_WRITE(V3) {
  int switchState = param.asInt();

  // Update switch state only if it has changed
  if (switchState != lastSwitchState) {
    lastSwitchState = switchState;
    Serial.print("Switch V3 state: ");
    Serial.println(switchState);

    fanManual = (switchState == 1); // Update fanManual state based on switchState

    // Update fan state immediately based on manual control
    if (fanManual) {
      digitalWrite(kipas, LOW); // Hidupkan relay kipas (aktif rendah)
      Serial.println("Fan is ON (manual)");
    } else {
      updateFanState(); // Update fan state based on temperature
    }
  }
}

void updateSuhu() {
  float suhu = dht.readTemperature(); // Baca suhu dalam derajat Celsius

  // Periksa apakah pembacaan gagal dan keluar (untuk mencoba lagi).
  if (isnan(suhu)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  lastTemperature = suhu; // Simpan suhu terakhir yang terbaca

  Serial.print("Temperature: "); // Label untuk serial monitor
  Serial.print(suhu); // Nilai suhu untuk serial monitor
  Serial.println(" °C"); // Unit untuk serial monitor

  Blynk.virtualWrite(V2, suhu); // Kirim nilai suhu ke Blynk

  // Update fan and lamp state based on temperature
  if (!fanManual) { // Only update fan state automatically if not in manual mode
    updateFanState();
  }
  updateLampState(suhu);
}

void updateFanState() {
  if (lastTemperature >= 33 || fanManual) {
    digitalWrite(kipas, LOW); // Hidupkan relay kipas (aktif rendah)
    Serial.println("Fan is ON to decrease temperature");
  } else {
    digitalWrite(kipas, HIGH); // Matikan relay kipas (aktif rendah)
    Serial.println("Fan is OFF to maintain temperature");
  }
}

void updateLampState(float suhu) {
  if (suhu < 29) {
    digitalWrite(lampu, HIGH); // Hidupkan relay lampu untuk menaikkan suhu
    Serial.println("Lamp is ON to increase temperature");
  } else if (suhu > 33) {
    digitalWrite(lampu, LOW); // Matikan relay lampu
    Serial.println("Lamp is OFF to decrease temperature");
  }
}
