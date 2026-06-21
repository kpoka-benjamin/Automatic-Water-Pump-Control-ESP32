#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Définition des broches
#define RELAY_PIN 26

#define LED_POMPE 33
#define LED_HIGH  32
#define LED_LOW   25

#define LEVEL_SENSOR 34

// Variables du système
bool modeAuto = true;
bool pompeON = false;

// Seuils du capteur de niveau
int seuilBas  = 600;
int seuilHaut = 1900;

String lastEtat = "";

void setup() {

  Serial.begin(115200);

  // Nom Bluetooth visible sur le téléphone
  SerialBT.begin("ESP32_POMPE");

  pinMode(RELAY_PIN, OUTPUT);

  pinMode(LED_POMPE, OUTPUT);
  pinMode(LED_HIGH, OUTPUT);
  pinMode(LED_LOW, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);

  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Systeme Pompe");

  lcd.setCursor(0, 1);
  lcd.print("Initialisation");

  delay(2000);
  lcd.clear();
}

void loop() {

  // Lecture de la valeur analogique du capteur
  int niveau = analogRead(LEVEL_SENSOR);

  Serial.println(niveau);

  delay(300);

  // Réception des commandes Bluetooth
  if (SerialBT.available()) {

    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();

    if (cmd == "AUTO") {
      modeAuto = true;
    }

    if (cmd == "1") {
      modeAuto = false;
      pompeON = true;
    }

    if (cmd == "0") {
      modeAuto = false;
      pompeON = false;
    }
  }

  bool niveauBas = (niveau < seuilBas);
  bool niveauHaut = (niveau > seuilHaut);

  // Gestion automatique de la pompe
  if (modeAuto) {

    if (niveauBas) {
      pompeON = true;
    }

    else if (niveauHaut) {
      pompeON = false;
    }
  }

  // Commande du relais et de la LED pompe
  digitalWrite(RELAY_PIN, pompeON ? HIGH : LOW);
  digitalWrite(LED_POMPE, pompeON ? HIGH : LOW);

  // Indication visuelle du niveau d'eau
  digitalWrite(LED_LOW, niveauBas);
  digitalWrite(LED_HIGH, niveauHaut);

  // Affichage sur le LCD
  lcd.setCursor(0, 0);
  lcd.print(modeAuto ? "Mode: AUTO    " : "Mode: MANUEL  ");

  lcd.setCursor(0, 1);

  if (pompeON)
    lcd.print("Pompe: ON     ");
  else
    lcd.print("Pompe: OFF    ");

  // Construction du message à envoyer à l'application
  String etat = "";

  if (niveauBas)
    etat = "NIVEAU=BAS";
  else if (niveauHaut)
    etat = "NIVEAU=HAUT";
  else
    etat = "NIVEAU=MOYEN";

  etat += pompeON ? ";POMPE=ON" : ";POMPE=OFF";
  etat += modeAuto ? ";MODE=AUTO" : ";MODE=MANUEL";

  // Envoi uniquement si l'état a changé
  if (etat != lastEtat) {

    SerialBT.println(etat);

    lastEtat = etat;
  }
}