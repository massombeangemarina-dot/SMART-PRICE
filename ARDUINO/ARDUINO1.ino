#include <Keypad.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(A0, A1); 

#define PIN_LED_ROUGE 10
#define PIN_BOUTON    11
#define PIN_LED_VERT  12
#define PIN_BUZZER    13

const byte LIGNES = 4;
const byte COLONNES = 4;

char touches[LIGNES][COLONNES] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte brochesLignes[LIGNES]     = {6, 7, 8, 9};
byte brochesColonnes[COLONNES] = {5, 4, 3, 2};

Keypad clavier = Keypad(makeKeymap(touches), brochesLignes, brochesColonnes, LIGNES, COLONNES);

struct Article {
  String nom;
  float prix;
  int quantite;
};

Article panier[20];
int nombreArticles = 0;
int indexSelection = 0;

String articleEnCoursNom = "";
float articleEnCoursPrix = 0.0;
String quantiteSaisieStr = "";
bool enAttenteQuantite = false;
String quantiteEditionPanier = "";

bool estAllume = true;
bool dernierEtatBouton = HIGH;

void setup() {
  // Communication PC à 115200 Bauds (Ultra-rapide)
  Serial.begin(115200);   
  espSerial.begin(9600);  

  pinMode(PIN_LED_ROUGE, OUTPUT);
  pinMode(PIN_LED_VERT, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BOUTON, INPUT_PULLUP);

  digitalWrite(PIN_LED_ROUGE, LOW);
  digitalWrite(PIN_LED_VERT, LOW);

  bipSucces();

  Serial.println(F("\n=========================================="));
  Serial.println(F("    === TABLEAU ACTIF ===   "));
  Serial.println(F("=========================================="));
  delay(500);

  afficherPanierSerie();
}

void loop() {
  // 1. Interrupteur ON/OFF (Bouton poussoir)
  bool etatBouton = digitalRead(PIN_BOUTON);
  if (dernierEtatBouton == HIGH && etatBouton == LOW) {
    estAllume = !estAllume;
    
    if (estAllume) {
      bipSucces();
      afficherPanierSerie();
    } else {
      bipErreur();
      digitalWrite(PIN_LED_ROUGE, LOW);
      digitalWrite(PIN_LED_VERT, LOW);
      effacerEcranSerie();
      Serial.println(F("+-------------------------------------------------------+"));
      Serial.println(F("|            DISPOSITIF ETEINT / EN VEILLE              |"));
      Serial.println(F("|        Appuyez sur le bouton pour rallumer.           |"));
      Serial.println(F("+-------------------------------------------------------+"));
    }
    delay(300);
  }
  dernierEtatBouton = etatBouton;

  if (!estAllume) return;

  // 2. Saisie Clavier 4x4
  char touche = clavier.getKey();
  if (touche) {
    bipTouche();
    traiterClavier(touche);
  }

  // 3. Réception ESP32
  if (espSerial.available()) {
    String reponse = espSerial.readStringUntil('\n');
    reponse.trim();

    if (reponse.startsWith("OK;")) {
      digitalWrite(PIN_LED_ROUGE, LOW);
      digitalWrite(PIN_LED_VERT, HIGH);
      bipSucces();

      int i1 = reponse.indexOf(';');
      int i2 = reponse.indexOf(';', i1 + 1);

      articleEnCoursNom = reponse.substring(i1 + 1, i2);
      articleEnCoursPrix = reponse.substring(i2 + 1).toFloat();
      
      quantiteSaisieStr = "1";
      enAttenteQuantite = true;

      afficherSaisieQuantiteSerie();
    } 
    else if (reponse.startsWith("ERREUR;")) {
      digitalWrite(PIN_LED_VERT, LOW);
      digitalWrite(PIN_LED_ROUGE, HIGH);
      bipErreur();
      Serial.println(F("\n[!] Aucun article reconnu sur l'etiquette."));
    }
  }
}

void declencherScan() {
  digitalWrite(PIN_LED_VERT, LOW);
  digitalWrite(PIN_LED_ROUGE, HIGH);
  espSerial.println("S");
  Serial.println(F("\n[ARDUINO] Demande de Scan envoyee..."));
}

void traiterClavier(char touche) {
  if (enAttenteQuantite) {
    if (touche >= '0' && touche <= '9') {
      if (quantiteSaisieStr == "1" || quantiteSaisieStr == "0") quantiteSaisieStr = ""; 
      quantiteSaisieStr += touche;
      afficherSaisieQuantiteSerie();
    }
    else if (touche == '*') {
      quantiteSaisieStr = "0";
      afficherSaisieQuantiteSerie();
    }
    else if (touche == '#') {
      int qte = quantiteSaisieStr.toInt();
      if (qte <= 0) qte = 1;

      panier[nombreArticles].nom = articleEnCoursNom;
      panier[nombreArticles].prix = articleEnCoursPrix;
      panier[nombreArticles].quantite = qte;
      nombreArticles++;

      enAttenteQuantite = false;
      digitalWrite(PIN_LED_VERT, LOW);
      afficherPanierSerie();
    }
  } 
  else {
    if (touche >= '0' && touche <= '9') {
      if (nombreArticles > 0) {
        quantiteEditionPanier += touche;
        panier[indexSelection].quantite = quantiteEditionPanier.toInt();
        afficherPanierSerie();
      }
    }
    else {
      quantiteEditionPanier = "";

      switch (touche) {
        case 'A':
        case 'a':
          declencherScan();
          break;

        case 'B':
        case 'b':
          if (nombreArticles > 0) {
            for (int i = indexSelection; i < nombreArticles - 1; i++) {
              panier[i] = panier[i + 1];
            }
            nombreArticles--;
            if (indexSelection >= nombreArticles && indexSelection > 0) indexSelection--;
            bipErreur();
            afficherPanierSerie();
          }
          break;

        case 'C':
        case 'c':
          if (indexSelection > 0) {
            indexSelection--;
            afficherPanierSerie();
          }
          break;

        case 'D':
        case 'd':
          if (indexSelection < nombreArticles - 1) {
            indexSelection++;
            afficherPanierSerie();
          }
          break;

        case '*':
          if (nombreArticles > 0) {
            panier[indexSelection].quantite = 1;
            afficherPanierSerie();
          }
          break;

        case '#':
          afficherPanierSerie();
          break;
      }
    }
  }
}

void effacerEcranSerie() {
  for (int i = 0; i < 25; i++) {
    Serial.println();
  }
}

void imprimerFormate(String texte, int largeur) {
  Serial.print(texte);
  for (int i = texte.length(); i < largeur; i++) {
    Serial.print(" ");
  }
}

void afficherPanierSerie() {
  effacerEcranSerie();
  Serial.println(F("+----+----------------------+------+------------+---------------+"));
  Serial.println(F("| N* | ARTICLE              | QTE  | PRIX U     | PRIX T        |"));
  Serial.println(F("+----+----------------------+------+------------+---------------+"));

  if (nombreArticles == 0) {
    Serial.println(F("|                  (LE PANIER EST VIDE)                         |"));
    Serial.println(F("+----+----------------------+------+------------+---------------+"));
    Serial.println(F("\n -> Appuyez sur 'A' ou sur le bouton pour scanner un article."));
  } 
  else {
    float totalGeneral = 0;
    for (int i = 0; i < nombreArticles; i++) {
      float totalArticle = panier[i].prix * panier[i].quantite;
      totalGeneral += totalArticle;

      Serial.print(F("|"));
      if (i == indexSelection) Serial.print(F(">"));
      else Serial.print(F(" "));

      imprimerFormate(String(i + 1), 3);
      Serial.print(F("| "));
      
      String nomAffiche = panier[i].nom;
      if (nomAffiche.length() > 20) nomAffiche = nomAffiche.substring(0, 17) + "...";
      imprimerFormate(nomAffiche, 21);
      
      Serial.print(F("| "));
      imprimerFormate(String(panier[i].quantite), 5);
      
      Serial.print(F("| "));
      imprimerFormate(String((long)panier[i].prix) + " FCFA", 11);
      
      Serial.print(F("| "));
      imprimerFormate(String((long)totalArticle) + " FCFA", 14);
      
      Serial.println(F("|"));
    }
    
    Serial.println(F("+----+----------------------+------+------------+---------------+"));
    Serial.print(F("| TOTAL GENERAL : "));
    imprimerFormate(String((long)totalGeneral) + " FCFA", 44);
    Serial.println(F("|"));
    Serial.println(F("+---------------------------------------------------------------+"));
    Serial.println(F("\n [C/D] Naviguer | [Chiffres] Saisir Qte | [B] Suppr | [*] Reset 1"));
  }
}

void afficherSaisieQuantiteSerie() {
  effacerEcranSerie();
  Serial.println(F("+---------------------------------------------------------------+"));
  Serial.println(F("|                    NOUVEL ARTICLE DETECTE                     |"));
  Serial.println(F("+-----------------------+---------------------------------------+"));
  Serial.print(F("| ARTICLE               | "));
  imprimerFormate(articleEnCoursNom, 38);
  Serial.println(F("|"));
  
  Serial.print(F("| PRIX UNITAIRE         | "));
  imprimerFormate(String((long)articleEnCoursPrix) + " FCFA", 38);
  Serial.println(F("|"));
  
  Serial.print(F("| QUANTITE SAISIE       | "));
  imprimerFormate(quantiteSaisieStr, 38);
  Serial.println(F("|"));
  Serial.println(F("+-----------------------+---------------------------------------+"));
  Serial.println(F("\n [Chiffres] Tapez la quantité | [*] Effacer | [#] Valider"));
}

void bipTouche() { tone(PIN_BUZZER, 2000, 40); }
void bipSucces() { tone(PIN_BUZZER, 1500, 150); }
void bipErreur() { tone(PIN_BUZZER, 400, 300); }