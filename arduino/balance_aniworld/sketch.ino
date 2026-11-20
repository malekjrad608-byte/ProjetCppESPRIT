// ANIWORLD - Balance connectee, demonstration Wokwi (capteur 5 kg).
// Sur le montage reel, calibrer CALIBRATION avec une masse connue.
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

HX711 balance;
LiquidCrystal_I2C lcd(0x27, 16, 2);
const byte TARE = 4, MESURE = 5, LED_OK = 6, BUZZER = 7;
const float CALIBRATION = 420.0; // Wokwi: 2100 points pour 5 kg
float historique[6] = {};
byte nombre = 0, position = 0;
unsigned long derniereLecture = 0, dernierBouton = 0, messageJusqua = 0;
bool tareAvant = HIGH, mesureAvant = HIGH, stable = false;
float poids = 0;

void ligne(byte numero, const char *texte) {
  lcd.setCursor(0, numero);
  lcd.print("                ");
  lcd.setCursor(0, numero); lcd.print(texte);
}

void setup() {
  Serial.begin(115200);
  pinMode(TARE, INPUT_PULLUP); pinMode(MESURE, INPUT_PULLUP);
  pinMode(LED_OK, OUTPUT); pinMode(BUZZER, OUTPUT);
  lcd.init(); lcd.backlight();
  ligne(0, "ANIWORLD BALANCE"); ligne(1, "Capteur 0 - 5 kg");
  balance.begin(2, 3); // DT, SCK
  balance.set_scale(CALIBRATION);
  balance.set_offset(0); // Zero de la simulation; bouton TARE pour le montage reel
  Serial.println("ANIWORLD | Balance 5 kg | TARE=vert, MESURER=bleu");
  Serial.println("Cliquer le capteur HX711 pour modifier la charge simulee.");
  Serial.println("Les mesures sont envoyees au moniteur serie, pas encore a Oracle.");
}

void loop() {
  bool t = digitalRead(TARE), m = digitalRead(MESURE);
  if (millis() - dernierBouton > 180) {
    if (t == LOW && tareAvant == HIGH && balance.is_ready()) {
      balance.tare(5); nombre = 0; position = 0; stable = false;
      digitalWrite(LED_OK, LOW); dernierBouton = millis();
      ligne(1, "TARE effectuee"); messageJusqua = millis() + 1200;
      Serial.println("EVENT,TARE"); tone(BUZZER, 1000, 80);
    }
    if (m == LOW && mesureAvant == HIGH) {
      dernierBouton = millis();
      if (stable && poids > 0.01 && poids <= 5.0) {
        Serial.print("MEASURE_KG,"); Serial.println(poids, 3);
        ligne(1, "Mesure envoyee"); tone(BUZZER, 1800, 100);
      } else {
        ligne(1, "Attendre / poser"); tone(BUZZER, 300, 150);
      }
      messageJusqua = millis() + 1500;
    }
  }
  tareAvant = t; mesureAvant = m;

  if (millis() - derniereLecture < 200 || !balance.is_ready()) return;
  derniereLecture = millis();
  poids = balance.get_units(1);
  if (abs(poids) < 0.005) poids = 0;
  historique[position] = poids; position = (position + 1) % 6;
  if (nombre < 6) ++nombre;
  float minimum = historique[0], maximum = historique[0];
  for (byte i = 1; i < nombre; ++i) {
    minimum = min(minimum, historique[i]); maximum = max(maximum, historique[i]);
  }
  stable = nombre == 6 && maximum - minimum < 0.02;
  digitalWrite(LED_OK, stable && poids > 0.01 && poids <= 5.0);
  char valeur[10], texte[17]; dtostrf(poids, 6, 3, valeur);
  snprintf(texte, sizeof(texte), "Poids:%s kg", valeur); ligne(0, texte);
  if (millis() >= messageJusqua) {
    if (poids < -0.02) ligne(1, "Refaire la TARE");
    else if (poids > 5.0) ligne(1, "Hors capacite");
    else if (poids <= 0.01) ligne(1, "Poser un objet");
    else ligne(1, stable ? "STABLE: MESURER" : "Stabilisation...");
  }
  Serial.print("WEIGHT_KG,"); Serial.print(poids, 3);
  Serial.print(","); Serial.println(stable ? "STABLE" : "MOVING");
}
