#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// WiFi
const char* ssid = "1303210181";
const char* password = "1303210181";

// // MQTT
// const char* mqtt_broker = "public.mqtthq.com";
// const int mqtt_port = 1883;
// WiFiClientSecure espClient;
// PubSubClient client(espClient);

// Telegram
#define BOTtoken "6493211508:AAFQ7nHsRZ2INJW1FvJuwNK4oPXlXpmVi4s"
#define CHAT_ID "1737466183"
WiFiClientSecure clientSecure;
UniversalTelegramBot bot(BOTtoken, clientSecure);

// GPI
const int kipas = D4; // GPIO for the fan
#define DHTPIN D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

long lastMsg = 0;
bool monitorTemperature = false;
unsigned long lastTempRequestTime = 0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  // client.setServer(mqtt_broker, 1883);
  // client.setCallback(callback);

  pinMode(kipas, OUTPUT);
  dht.begin();

  // Initialize Telegram Bot
  clientSecure.setInsecure(); // If certificate verification is not required
  bot.updateToken(BOTtoken);

  Serial.println("Telegram Bot Initialized");
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    Serial.print("Received message: ");
    Serial.println(text);

    if (text == "/start") {
      String control = "Selamat Datang.\n";
      control += "Gunakan Commands Di Bawah Untuk Monitoring Kandang Ayam\n\n";
      control += "/Temperatur Untuk Monitoring Suhu \n";
      control += "/Humidity Untuk Monitoring Kelembapan \n";
      control += "/suhutinggi untuk menaikan suhu \n";
      control += "/suhurendah untuk menurunkan suhu \n";
      control += "/ceksuhu untuk monitoring suhu setiap 3 detik \n";
      control += "/quitsuhu untuk keluar dari monitoring suhu \n";
      bot.sendMessage(chat_id, control, "");
    } else if (text == "/suhutinggi") {
      digitalWrite(kipas, HIGH);
      Serial.print("kipas Mati");
    } else if (text == "/suhurendah") {
      digitalWrite(kipas, LOW);
      Serial.print("kipas Menyala");
    } else if (text == "/Temperatur") {
      float t = dht.readTemperature();
      if (isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        bot.sendMessage(chat_id, "Failed to read temperature!", "");
        return;
      }
      String suhu = "Status Suhu: ";
      suhu += String(t);
      suhu += " ⁰C\n";
      bot.sendMessage(chat_id, suhu, "");
    } else if (text == "/Humidity") {
      float h = dht.readHumidity();
      if (isnan(h)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        bot.sendMessage(chat_id, "Failed to read humidity!", "");
        return;
      }
      String lembab = "Status Kelembapan: ";
      lembab += String(h);
      lembab += " %Rh\n";
      bot.sendMessage(chat_id, lembab, "");
    } else if (text == "/ceksuhu") {
      monitorTemperature = true;
      lastTempRequestTime = millis();
      bot.sendMessage(chat_id, "Monitoring suhu setiap 3 detik dimulai.", "");
    } else if (text == "/quitsuhu") {
      monitorTemperature = false;
      String control = "Monitoring suhu dihentikan. Kembali ke menu utama.\n\n";
      control += "Gunakan Commands Di Bawah Untuk Monitoring Kandang Ayam\n\n";
      control += "/Temperatur Untuk Monitoring Suhu \n";
      control += "/Humidity Untuk Monitoring Kelembapan \n";
      control += "/suhutinggi untuk menaikan suhu \n";
      control += "/suhurendah untuk menurunkan suhu \n";
      control += "/ceksuhu untuk monitoring suhu setiap 3 detik \n";
      control += "/quitsuhu untuk keluar dari monitoring suhu \n";
      bot.sendMessage(chat_id, control, "");
    }
  }
}

void loop() {
  // if (!client.connected()) {
  //   reconnect();
  // }
  // client.loop();

  long now = millis();

  // Continuous temperature monitoring every 3 seconds if requested
  if (monitorTemperature && (now - lastTempRequestTime > 3000)) {
    lastTempRequestTime = now;
    float suhu = dht.readTemperature(); // Baca suhu dari DHT sensor
    char msg[50];
    snprintf(msg, 75, "Suhu kandang: %.2f ⁰C", suhu);
    bot.sendMessage(CHAT_ID, msg, "");

    // Adjust kipas based on temperature
    if (suhu < 28) {
      digitalWrite(kipas, HIGH);
    } else if (suhu > 31) {
      digitalWrite(kipas, LOW);
    } else {
      digitalWrite(kipas, LOW);
    }
  }

  // Check for new messages from Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  Serial.print("Number of new messages: ");
  Serial.println(numNewMessages);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0; // Count connection attempts
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) { // Try for 10 seconds (20 * 500ms)
      Serial.println("\nFailed to connect to WiFi. Retrying...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      attempts = 0;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi after multiple attempts.");
  }
}

// void callback(char* topic, byte* payload, unsigned int length) {
//   String messageTemp;
  
//   for (int i = 0; i < length; i++) {
//     messageTemp += (char)payload[i];
//   }

//   if (String(topic) == "kandang/kipas") {
//     Serial.print("Pesan kipas: ");
//     Serial.println(messageTemp);
//     if (messageTemp == "on") {
//       digitalWrite(kipas, HIGH);
//     } else if (messageTemp == "off") {
//       digitalWrite(kipas, LOW);
//     }
//   }
// }

// void reconnect() {
//   while (!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
//     if (client.connect("ESP8266Client")) {
//       Serial.println("connected");
//       client.subscribe("kandang ayam/#");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       delay(5000);
//     }
//   }
// }
