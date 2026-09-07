# 🛒 SMART-PRICE / PANIER INTELLIGENT

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Microcontroller](https://img.shields.io/badge/Hardware-Arduino%20Uno%20%7C%20ESP32--CAM-green)
![Python](https://img.shields.io/badge/Python-3.8%2B-yellow)
![OpenCV](https://img.shields.io/badge/Library-OpenCV%20%26%20EasyOCR-red)

**SMART-PRICE** est un système embarqué complet pour panier d'achat intelligent. Il associe un **Arduino Uno**, un **ESP32-CAM**, un **écran TFT ST7789** et un **moteur de reconnaissance optique de caractères (OCR) sous Python** pour permettre le scan visuel instantané d'étiquettes de prix, la gestion dynamique du panier et le calcul en temps réel.

---

Le projet étant en cours de développement, cette version( Codes téléversés) fonctionne sans écran et les résultats sont visibles via le moniteur série d'arduino IDE.

## 📸 Aperçu du Système

* **Interface Utilisateur** : UI Dark Mode sur écran TFT 2.4" ST7789 (240x320) via SPI logiciel.
* **Scan Visuel** : Capture d'image via ESP32-CAM et traitement EasyOCR sur PC/Serveur.
* **Interface de Contrôle** : Navigabilité complète via un clavier matriciel 4x4, avertisseur sonore (Buzzer) et LEDs d'état.
* **Gestion d'Énergie** : Bouton physique de mise en veille / réveil synchrone sur l'ESP32-CAM.

---

## 🏗️ Architecture Technique

Le projet repose sur une architecture maître-esclave distribuée :
