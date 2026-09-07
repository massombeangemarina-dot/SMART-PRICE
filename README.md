# 🛒 SMART-PRICE / PANIER INTELLIGENT

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Microcontroller](https://img.shields.io/badge/Hardware-Arduino%20Uno%20%7C%20ESP32--CAM-green)
![Python](https://img.shields.io/badge/Python-3.8%2B-yellow)
![OpenCV](https://img.shields.io/badge/Library-OpenCV%20%26%20EasyOCR-red)

**SMART-PRICE** est un système embarqué complet pour panier d'achat intelligent. Il associe un **Arduino Uno**, un **ESP32-CAM**, un **écran TFT ST7789** et un **moteur de reconnaissance optique de caractères (OCR) sous Python** pour permettre le scan visuel instantané d'étiquettes de prix, la gestion dynamique du panier et le calcul en temps réel.

---



## 📸 Aperçu du Système

* **Interface Utilisateur** : UI Dark Mode sur écran TFT 2.4" ST7789 (240x320) via SPI logiciel.
* **Scan Visuel** : Capture d'image via ESP32-CAM et traitement EasyOCR sur PC/Serveur.
* **Interface de Contrôle** : Navigabilité complète via un clavier matriciel 4x4, avertisseur sonore (Buzzer) et LEDs d'état.
* **Gestion d'Énergie** : Bouton physique de mise en veille / réveil synchrone sur l'ESP32-CAM.

---

Le projet étant en cours de développement, cette version( Codes téléversés) fonctionne sans écran et les résultats sont visibles via le moniteur série d'arduino IDE.


## 🏗️ Architecture Technique

Le projet repose sur une architecture maître-esclave distribuée :

[ Clavier 4x4 ] ────┐
[ Écran ST7789 ] ───┼──> (Arduino Uno) <──[SoftwareSerial]──> (ESP32-CAM) <──[Wi-Fi HTTP]──> [ Script Python OCR ]
[ Buzzer / LEDs ] ──┘     (Gestion UI)                           (Streaming)                     (Traitement d'image)


1. **Arduino Uno** : Gère l'affichage UI, les saisies clavier, le calcul du total et les signaux sonores/lumineux.
2. **ESP32-CAM** : Diffuse un flux vidéo en direct via Wi-Fi (AP) et sert de passerelle entre l'Arduino et le serveur Python via `HardwareSerial`.
3. **Moteur Python (PC)** : Intercepte le flux vidéo, effectue l'analyse OCR lors d'un scan (extraction du nom et du prix) et renvoie le résultat formaté `NOM:PRIX` à l'ESP32-CAM.

---

## 🔌 Schéma de Câblage (Pinout)

### 1. Arduino Uno $\leftrightarrow$ Écran TFT ST7789 (SPI Logiciel)
| Broche ST7789 | Broche Arduino Uno | Description |
| :--- | :--- | :--- |
| **CS** | `A5` | Chip Select |
| **RST** | `A4` | Reset |
| **DC** | `A3` | Data / Command |
| **MOSI (SDA)** | `A2` | Data In |
| **SCLK (SCL)** | `A1` | Clock |

### 2. Communication Série (Arduino Uno $\leftrightarrow$ ESP32-CAM)
> **Note** : Attention à la logique croisée RX/TX.

| Composant Source | Broche Source | Composant Cible | Broche Cible |
| :--- | :--- | :--- | :--- |
| **Arduino Uno** | `Pin 13` (TX) | **ESP32-CAM** | `GPIO 14` (RX2) |
| **ESP32-CAM** | `GPIO 15` (TX2) | **Arduino Uno** | `A0` (RX) |
| **Masse (GND)** | `GND` | **Masse (GND)** | `GND` |

### 3. Périphériques Arduino Uno
* **Clavier 4x4** :
  * Lignes (Rows) $\rightarrow$ `Pin 2`, `Pin 4`, `Pin 5`, `Pin 6`
  * Colonnes (Cols) $\rightarrow$ `Pin 7`, `Pin 8`, `Pin 9`, `Pin 10`
* **Buzzer** $\rightarrow$ `Pin 3`
* **LED Rouge** $\rightarrow$ `Pin 11`
* **LED Verte** $\rightarrow$ `Pin 12`

### 4. Périphériques ESP32-CAM
* **Bouton Veille/Réveil** $\rightarrow$ `GPIO 16` (en PULLUP interne vers GND)
* **Flash LED** $\rightarrow$ `GPIO 4`

---

## ⌨️ Commandes du Clavier Matriciel

| Touche | Action |
| :---: | :--- |
| **`A`** | Déclencher une prise de vue / Scan OCR |
| **`B`** | Supprimer l'article sélectionné du panier |
| **`C`** | Naviguer vers le haut dans le panier |
| **`D`** | Naviguer vers le bas dans le panier |
| **`0` - `9`** | Saisir une nouvelle quantité |
| **`*`** | Effacer la saisie de quantité en cours |
| **`#`** | Valider la nouvelle quantité OU afficher l'écran récapitulatif/Total |

---

## 🚀 Installation et Déploiement

### Prérequis Logiciels
* [Arduino IDE](https://www.arduino.cc/en/software)
* Python 3.8 ou supérieur

### 1. Configuration Arduino & ESP32-CAM
1. Installez les bibliothèques Arduino suivantes via le gestionnaire de bibliothèques :
   * `Adafruit GFX Library`
   * `Adafruit ST7789 Library`
   * `Keypad` by Mark Stanley, Alexander Brevig
2. Téléversez le code `Arduino_SmartCart.ino` sur l'Arduino Uno.
3. Téléversez le code `CameraWebServer.ino` sur l'ESP32-CAM (pensez à maintenir `GPIO 0` au `GND` pendant le téléversement, puis à le débrancher et faire un Reset).

### 2. Configuration du Serveur Python OCR
1. Clonez ce dépôt :
   ```bash
   git clone [https://github.com/votre-compte/SMART-PRICE.git](https://github.com/votre-compte/SMART-PRICE.git)
   cd SMART-PRICE
