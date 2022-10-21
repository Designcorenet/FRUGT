#include "arduino.h"
#include "PlotControl.h"

PlotControl::PlotControl(){
  first = NULL; 
  sign = 1.0;
  subS = NULL;
}

void PlotControl::initParPointers(pltVal_t *p){  
    pPar = p;                          // pointer Variable setzen
    pVal = &((*p).val);                // pointer auf Wert
    pLimit = NULL;                     // nicht auf Limits zeigen
    subS = NULL;                       // keinen sub-Wert anzeigen
    subsubS = NULL;
}

void PlotControl::addVar(pltVal_t *v, char id[idLength], char stz){      // Variable zum Plotten und Kontrollieren hinzufügen
  if(first){                          // pointer first ist schon gesetzt: v anhängen
    pltVal_t *elem = first;               // Variablenreihe durchgehen: elem zeigt auf erste Variable...
    while((*elem).nxt){                   // ... solange noch eine weitere kommt (also bei der aktuellen .nxt noch weiter zeigt)
      elem = (*elem).nxt;                 // ... elem auf nächste zeigen lassen
    }                                     // jetzt zeigt elem auf die letzte Variable
    (*elem).nxt = v;                      // dort v anhängen
  } else{                             // erste Variable: first ist noch null
    first = v;                            // v als erste Variable setzen
    initParPointers(v);                   // v zur aktuellen Variable machen und alle pointer auf den Wert initialisieren
  }
  strcpy((*v).id, id);                    // übergebene Werte setzen: Name muss kopiert werden...
  (*v).stz = stz;                         // ...char für Steuerung
  (*v).val = 1.0;                         // default Anfangswerte: Wert
  (*v).stp = 1.1;                         // Änderungsschrittweite 10% nach oben
  (*v).skale = 1.0;                       // Einheit für Plot
  (*v).plotR = {-3E38, 3E38};             // Bereich für Plotten (unendlich, also ohne Schranke)
  (*v).valR = {-3E38, 3E38};              // erlaubter Bereich für Wert (ebenfalls ohne Schranke)
}

void PlotControl::setLimits(limit_t *l, float _min, float _max){
  (*l)._min = _min;
  (*l)._max = _max;
}

pltVal_t *PlotControl::serPrintId(pltVal_t *wert){             // Legende des Wertes an Serial Plotter. Name und ggf. Skalierung
  Serial.print((*wert).id);
  if((*wert).skale > 1.0){        // Skalierung wenn > 1 (z.B. "p/5.0")
    Serial.print("/");
    Serial.print((*wert).skale, 0);
  }
  if((*wert).skale < 1.0){        // Skalierung wenn < 1 (z.B. bei 0.1: "i*10.0")
    Serial.print("*");
    Serial.print(1.0 / (*wert).skale, 0);
  }                            // wenn == 1 keine Skalierung
  if(pPar == wert){            // wenn die Variable gerade ausgewählt ist:  
    if(subS){                  // falls Subwerte angewählt sind...
      Serial.print(*subS);     // deren Namen anhängen
    }
    if(subsubS){                  // falls SubSub-Werte angewählt sind...
      Serial.print(*subsubS);     // deren Namen auch noch anhängen
    }
    Serial.print("[");         // Wert in eckigen Klammern...
    Serial.print(*pVal);       // wert ohne plotClip()!
    Serial.print("]");
  }
  if((*wert).nxt){            // wenn noch eine kommt..
    Serial.print("\t");       // tab dran
  } else{                     // ansonsten..
    Serial.println();         // Zeilenende
  }
  return (*wert).nxt;         // nxt-Zeiger zurück liefern für die Schleife in serPrintIds()
}

void PlotControl::serPrintIds(){     // ganze Legende an Serial Plotter
  pltVal_t *elem = first;            // auf erste Variable zeigen
  while(elem = serPrintId(elem));    // alle IDs ausdrucken (bis die Funktion NULL zurück liefert)
}

pltVal_t *PlotControl::serPrintVal(pltVal_t *wert){   // Wert einer Variablen an Serial
  Serial.print(plotClip(wert));                      // Wert auf Plotbereich eingrenzen
  if((*wert).nxt){                                    // kommt danach noch eine Variable?
    Serial.print("\t");                               // nxt != NULL ja: tab anhängen
  } else{                                             // nein (nxt == NULL):
    Serial.println();                                 // Zeilenende
  }
  return (*wert).nxt;                                 // pointer auf nächste Variable zurück (für Schleife in serPrintVals)
}

void PlotControl::serPrintVals(){                  // alle Werte an Serial Plotter
  for(int j = 0; j < pg; j++){            // Schleife: bei "Flush" wird der Wert pg-mal ausgegeben, um störende Spikes schnell aus dem Plotfenster zu schieben
    pltVal_t *elem = first;
    while(elem = serPrintVal(elem));      // alle IDs ausdrucken (bis die Funktion NULL zurück liefert)
  }
  if(pg > 1) pg = 1;                      // Plotgeschw. wieder auf 1
}

void PlotControl::valClip(pltVal_t *w){   // clipe den Wert von w auf das Intervall [minV, maxV]; der Wert wird verändert
  if(parOff(w)) return;                            // nicht clippen, wenn stp > 0 ist (steuerbarer Parameter) und dieser gerade abgeschaltet ist.
  if((*w).val > (*w).valR._max) (*w).val = (*w).valR._max;
  if((*w).val < (*w).valR._min) (*w).val = (*w).valR._min;
}

float PlotControl::plotClip(pltVal_t *w){   // liefere den Wert von w geclipt auf das Intervall [minD, maxD] und skaliert ohne den Wert zu verändern
  float y = getParVal(w);            // 0 falls der Parameter gerade aus ist, sonst dessen Wert
  if(y > (*w).plotR._max) return (*w).plotR._max / (*w).skale;  // auf plotR  eingegrenzt und skaliert
  if(y < (*w).plotR._min) return (*w).plotR._min / (*w).skale;
  return y / (*w).skale;
}

boolean PlotControl::parOff(pltVal_t *par){   // ist es ein steuerbarer Parameter (stp > 0) und er ist gerade abgeschaltet (z.B. I-Wert aus, val<0)
  return (*par).stp > 0.0 && (*par).val < 0.0;
}

float PlotControl::getParVal(pltVal_t *par){   // für Ausgabe von Regelparametern. Die kann man vorübergehend abschalten, ohne ihren Wert zu verlieren, indem man ihren Wert negativ macht
  if(parOff(par)) {    
    return 0.0;
  } else{
    return (*par).val;
  }
}

void PlotControl::changeSkala(int8_t richtung){
  float s = *pVal;            // pVal zeigt auf skale
  while(s < 0.9) s *= 10.0;   // s in Bereich 1..9 bringen, solle also danach 1, 2 oder 5 sein.
  while(s > 11.0) s /= 10.0;
  s = round(s);
  if(richtung > 0){   //Skala vergrößern:
    if(s > 1.9 && s < 2.1){
      *pVal *= 2.5;    // hoch: aus 2 wird 5
    } else{
      *pVal *= 2.0;    // aus 1 wird 2 und aus 5 wird 10
    }
  } else{             // Skala verkleinern:
    if(s > 4.9){
      *pVal /= 2.5;    // aus 5 wird 2
    } else{
      *pVal /= 2.0;    // aus 1 wird 5 und aus 2 wird 1
    }
  }
}

boolean PlotControl::setzeVar(char c){      // ist c eines der Steuerzeichen für Variablen? 
  boolean res = false;
  pltVal_t *elem = first;         // auf erste Variable zeigen
  do{                             // alle Variablen durchgehen
    if(c == (*elem).stz){         // ist es das stz dieser Variablen?
      pPar = elem;                // ja: pointer auf diese Variable setzen
      initParPointers(elem);      // Pointer initialisieren
      nSigns = 0;                 // noch keine + oder -
      res = true;                 // Var gefunden, true zurückliefern
      break;                      // Variablensuche abbrechen
    } else{
      elem = (*elem).nxt;         // nein, keine Übereinstimmung: nächste Variable
    }
  } while(elem);            // weiter bis .nxt NULL ist
  switch(c){                // auf andere Steuerzeichen testen
    case 'k':{                // Skale einstellen
      pVal = &(*pPar).skale;  // float-pointer auf skale-Komponente der aktuellen Variable
      pLimit = NULL;          // limit-Pointer muss auf null
      subS = &skaleS;         // Anzeige sub-Wert
      subsubS = NULL;         // keinen subsub anzeigen
      res = true;             // Zeichen erkannt, true zurückliefern
      break;
    }
    case 't':{                // stp einstellen
      pVal = &(*pPar).stp;    // float-pointer auf step-Komponente der aktuellen Variable
      pLimit = NULL;          // limit-Pointer muss auf null
      subS = &stpS;           // Anzeige sub-Wert
      subsubS = NULL;         // keinen subsub anzeigen
      res = true;             // Zeichen erkannt, true zurückliefern
      break;
    }
    case 'o':{                // plotR einstellen
      pLimit = &(*pPar).plotR; // limit-pointer auf plotR-Komponente der aktuellen Variable
      pVal = &(*pPar).plotR._max;  // float-pointer auf max-Komponente von PlotR (default)
      subS = &plotRS;         // Anzeige sub-Wert
      subsubS = &_maxS;       // subsub auf max
      res = true;             // Zeichen erkannt, true zurückliefern
      break;  
    }
    case 'v':{                // valR einstellen
      pLimit = &(*pPar).valR; // limit-pointer auf valR-Komponente der aktuellen Variable
      pVal = &(*pPar).plotR._max;  // float-pointer auf max-Komponente von PlotR (default)
      subS = &valRS;          // Anzeige sub-Wert
      subsubS = &_maxS;       // subsub auf max
      res = true;             // Zeichen erkannt, true zurückliefern
      break;
    }
    case '+':{                // Float vergrößern
      if(subS == &skaleS){    // es ist der skale-Wert: den ändern wir im 1-2-5-Muster
        changeSkala(1);       // nächste Stufe nach oben
      } else{                 // alle anderen...
        *pVal *= (*pPar).stp; // float-Wert um Faktor stp vergrößern
      }
      res = true;             // Zeichen erkannt, true zurückliefern
      break;
    }
    case '-':{                // Float verkleinern
      if(subS == &skaleS){    // es ist der skale-Wert: den ändern wir im 1-2-5-Muster
        changeSkala(-1);      // nächste Stufe nach unten
      } else{
        *pVal /= (*pPar).stp; // float-Wert um Faktor stp verkleinern
      }
      res = true;             // Zeichen erkannt, true zurückliefern
      break;
    }
    case '<':{                // unteres Limit einstellen
      if(!pLimit){            // falls Limit-Pointer nicht gesetzt...
        pLimit = &(*pPar).valR; // .. wird standardmäßig valR verändert
        subS = &valRS;        // Anzeigestring setzen
      }
      pVal = &(*pLimit)._min; // min auswählen
      subsubS = &_minS;       // subsub auf min
      res = true;             // Zeichen erkannt, true zurückliefern
      break; 
    }
    case '>':{                // oberes Limit einstellen
      if(!pLimit){            // falls Limit-Pointer nicht gesetzt...
        pLimit = &(*pPar).valR; // .. wird standardmäßig valR verändert
        subS = &valRS;        // Anzeigestring setzen
      }
      pVal = &(*pLimit)._max; // max auswählen
      subsubS = &_maxS;       // subsub auf min
      res = true;             // Zeichen erkannt, true zurückliefern
      break;
    }
    case 'F':{                // Flush Plotter (um Spikes loszuwerden, die die Skalierung ruinieren)
      pg = 250;               // Werte werden einmalig 250 mal ausgeben
      break;
    }
    case '.': {              // Parameter vorübergehend ausschalten
      if(!parOff(pPar)){    // wenns nicht schon off ist (achtung, trifft auch auf nicht-Parameter zu!)
        (*pPar).val = -(*pPar).val;   // Wert negativ machen; bewirkt bei Parametern, dass getParVal 0 zurückliefert
      }
      break;
    }
    case '|': {             // Parameter wieder einschalten, alter Wert wird wieder übernommen
      if(parOff(pPar)){    // wenns off ist ...
        (*pPar).val = -(*pPar).val;   // Wert wieder positiv machen; bewirkt bei Parametern, dass getParVal wieder den richtigen Wert zurückliefert
      }
      break;
    }
    default:{                 // alle anderen sollten numerische Zeichen sein
      if(isDigit(c)){
        *pVal = (c + Serial.readString()).toFloat();
        res = true;             // Zeichen erkannt, true zurückliefern
      }
    }
  }
  return res;             // kein Steuerzeichen für Variablen: false zurückliefern
}

void PlotControl::checkKeys(){
    while(Serial.available()) {     // solange Zeichen im Serial - Puffer sind...
      char c = Serial.read();       // Zeichen lesen
      if(setzeVar(c)) res = true;   // Sind gültige Steuerzeichen drin?
    }
    if(res){                        // falls ein Steuerzeichen erkannt wurde...
        serPrintIds();              // ...Legende ausgeben
        res = false;                // res wieder auf false für nächste Runde
    }
  
} // CheckKeys() 
