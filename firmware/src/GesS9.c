// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier GesS9.c
// Gestion du bouton S9
// Traitement cyclique à 10 ms

// Librairie inclues
#include "Mc32Debounce.h"
#include "Mc32DriverLcd.h"
#include "appgen.h"
#include "Mc32Debounce.h"


S_SwitchDescriptor DescrPB_S9;

// Structure pour les traitements du S9
S_9_Descriptor S9;

void ScanS9(bool ValS9) {
    DoDebounce(&DescrPB_S9, ValS9);

    if (ValS9 == 0) {
        S9.PressDuration++;
    } else if (DebounceIsReleased(&DescrPB_S9)) {
        // Appui long
        if (S9.PressDuration >= PRESSION_LONGUE_S9) {
            S9.OK = 0;
            S9.ESC = 1;
        }// Appui court
        else {
            S9.ESC = 0;
            S9.OK = 1;
        }
        // Remise à 0 du compteur de durée de l'appui
        S9.PressDuration = 0;
    }

    DebounceClearPressed(&DescrPB_S9);
    DebounceClearReleased(&DescrPB_S9);
}

void S9Init(void) {
    // Initialisation des descripteurs de touches S9
    DebounceInit(&DescrPB_S9);

    // Init de la structure S9
    S9.OK = 0; // événement action OK
    S9.ESC = 0; // événement action ESC
    S9.PressDuration = 0; // Pour durée pression du P.B.

} // S9

//       S9IsOK         true indique action OK

bool S9IsOK(void) {
    return (S9.OK);
}
//       S9IsESC        true indique action ESC

bool S9IsESC(void) {
    return (S9.ESC);
}
//       S9ClearOK      annule indication action OK

void S9ClearOK(void) {
    S9.OK = 0;
}
//       S9ClearESC     annule indication action ESC

void S9ClearESC(void) {
    S9.ESC = 0;
}