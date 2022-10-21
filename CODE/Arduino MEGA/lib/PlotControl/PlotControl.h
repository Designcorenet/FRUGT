#ifndef PlotControl_h
#define PlotControl_h

#include "arduino.h"

/*
#define pc_VARIABLE 0       // Monstanten für die Wahl der Veränderung: Variable selbst
#define pc_RESOLUTION_R 1   // Auflösung zurücksetzen (reset)
#define pc_RESOLUTION_F 2   // Auflösung verfeinern (fine)
#define pc_RESOLUTION_C 3   // Auflösung vergröbern (coarse)
#define pc_SKALAP 4         // Skalierung der Größe vergrößern
#define pc_SKALAM 5         // Skalierung der Größe verkleinern
#define pc_OFF 6            // p, i oder d vorübergehend abschalten
#define pc_ON 7             // p, i oder d wieder einschalten mit vorherigem Wert
*/
#define idLength 7          // max Länge der angezeigten Variablennamen (inklusive des 0-Terminators)

struct limit_t{
  float _min;
  float _max;
};

struct pltVal_t{  // für Werte, die auf serial plotter angezeigt werden sollen
      char stz;       // Steuerzeichen (kleinbuchstabe; dieses Zeichen über die serielle Schnittstelle schltet die Variable aktiv für weitere Steuerbefehle)
      float val;      // Wert der Variablen
      float skale;    // Skalierung auf Display (in 1 - 2 - 5 - Schritten)
      limit_t plotR;   
      float stp;      // Standardschrittweite bei Veränderung
      limit_t valR;
      char id[idLength];     // Bezeichnung des Wertes ACHTUNG: maximale Länge+1 (für 0-Terminator) angeben!
      pltVal_t *nxt;  // pointer zur nächsten Variable in der Kette
    };                // {stz, val, skale, plotR, stp, valR, id, nxt}

void printComment();          // muss im scetch definiert werden und gibt in der Legende Parameter etc. aus.

class PlotControl
{
  public:
    PlotControl();
    void addVar(pltVal_t *v, char id[idLength], char stz);     // Variable zum Plotten und Kontrollieren hinzufügen
    void PlotControl::setLimits(limit_t *l, float _min, float _max);
    void serPrintIds();
    void serPrintVals();
    boolean parOff(pltVal_t *p);
    void valClip(pltVal_t *w);
    float getParVal(pltVal_t *par);  // liefert 0 wenn der Parameter durch OFF abgeschaltet ist, sonst den  Wert par.val
    void checkKeys();
    
  private:
    pltVal_t *first;          // pointer auf erstes Element der Kette    
    pltVal_t *pPar;           // pointer auf aktuellen Parameter
    float *pVal;              // pointer auf aktuellen Wert (val, skale oder stp)
    limit_t *pLimit;          // pointer auf aktuelles Limit (plotR oder valR)
    
    float change, sign;
    uint8_t nSigns = 0;               // Anzuahl der + oder - nach einer Variablenkennung (bei s++ z.B. 2)
    boolean res;    
    float resFaktor = sqrt(10.0);     // Faktor, um den die Schrittweite verändert wird: pro + oder - und pro F oder C
    uint8_t pg = 1;                   // Plotgeschwindigkeit, für fast-forward des Plots
    String stpS = ".step";            // Anzeige für stp-Auswahl
    String skaleS = ".scale";         // Anzeige für skale-Auswahl
    String plotRS = ".plotR";         // Anzeige für PlotR-Auswahl
    String valRS = ".valR";
    String _maxS = ".max";            // Anzeige für limit._max-Auswahl
    String _minS = ".min";            // Anzeige für limit._min-Auswahl
    String *subS, *subsubS;           // Zeiger auf anzuzeigenden Namen des struct-Wert

    void initParPointers(pltVal_t *p);
    float plotClip(pltVal_t *w);
    void adjust(uint8_t was);
    void changeSkala(int8_t richtung);
    pltVal_t *serPrintId(pltVal_t *wert);
    pltVal_t *serPrintVal(pltVal_t *wert);
    boolean setzeVar(char c);
};

#endif
