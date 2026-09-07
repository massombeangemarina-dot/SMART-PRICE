import cv2
import easyocr
import time
import urllib.request
import urllib.parse
import re

URL_ESP32_STREAM = "http://192.168.4.1:81/stream"
URL_CHECK_SCAN   = "http://192.168.4.1/check_scan"
URL_FLASH        = "http://192.168.4.1/flash?state="
URL_RESULTAT     = "http://192.168.4.1/resultat?msg="

print("[+] Chargement du modèle OCR (EasyOCR)...")
reader = easyocr.Reader(['fr', 'en'], gpu=False)

print("[+] Connexion au flux vidéo ESP32-CAM...")
cap = cv2.VideoCapture(URL_ESP32_STREAM)

if not cap.isOpened():
    print("[-] Erreur de connexion au flux vidéo.")
    exit()

print("\n==================================================")
print("     CAISSE AUTOMATIQUE PRÊTE (OCR DYNAMIQUE)     ")
print("==================================================")

def eteindre_flash():
    try: urllib.request.urlopen(URL_FLASH + "0", timeout=0.2)
    except: pass

def envoyer_resultat_arduino(msg):
    try:
        url = URL_RESULTAT + urllib.parse.quote(msg)
        urllib.request.urlopen(url, timeout=0.5)
    except Exception as e:
        print(f"[-] Erreur envoi vers ESP32: {e}")

cadres_detectes = []
temps_affichage_cadres = 0
dernier_check = time.time()

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    if time.time() - dernier_check > 0.08:
        dernier_check = time.time()
        try:
            req = urllib.request.urlopen(URL_CHECK_SCAN, timeout=0.1)
            resp = req.read().decode().strip()

            if resp == "SCAN":
                print("\n[>] ORDRE RECEVEUR ('S') DÉTECTÉ !")
                time.sleep(0.2)
                ret_cap, frame_capture = cap.read()
                if not ret_cap: frame_capture = frame.copy()
                eteindre_flash()

                print("[+] Analyse de l'étiquette par EasyOCR...")
                results = reader.readtext(frame_capture)
                
                nom_article = ""
                prix_article = 0.0
                cadres_detectes = []

                # Étape 1 : Parcourir tout le texte repéré
                for (bbox, text, prob) in results:
                    text_clean = text.strip()
                    print(f"    -> Texte repéré : '{text_clean}' (Confiance : {prob:.2f})")

                    p1 = tuple(map(int, bbox[0]))
                    p2 = tuple(map(int, bbox[2]))
                    cadres_detectes.append((p1, p2, text_clean))

                    # Si le texte contient des chiffres ou "F"/"FCFA", c'est le PRIX
                    if re.search(r'\d', text_clean):
                        # Extraction de tous les chiffres présents (ex: "4 500 F" -> "4500")
                        chiffres = re.sub(r'[^\d]', '', text_clean)
                        if chiffres:
                            prix_article = float(chiffres)
                    # Sinon, c'est le NOM de l'article (ex: "Tomate")
                    elif len(text_clean) > 2 and not nom_article:
                        nom_article = text_clean

                # Étape 2 : Vérification et Envoi
                if nom_article and prix_article > 0:
                    # Formatage envoyé à l'ESP32 : Nom et Prix séparés
                    msg = f"OK;{nom_article};{prix_article}\n"
                    envoyer_resultat_arduino(msg)
                    print(f"\n[+] SUCCÈS DETECTÉ !")
                    print(f"    - Nom  : {nom_article}")
                    print(f"    - Prix : {prix_article} FCFA")
                else:
                    envoyer_resultat_arduino("ERREUR;\n")
                    print("\n[-] ÉTIQUETTE INCOMPLÈTE (Nom ou Prix manquant).")

                temps_affichage_cadres = time.time() + 3.0
        except:
            pass

    # Dessin des boîtes à l'écran
    if time.time() < temps_affichage_cadres:
        for (p1, p2, txt) in cadres_detectes:
            cv2.rectangle(frame, p1, p2, (0, 255, 0), 2)
            cv2.putText(frame, txt, (p1[0], p1[1] - 10), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    cv2.imshow("Flux Caméra Caisse - EasyOCR", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()