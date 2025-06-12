#ifndef Generateur_h
#define Generateur_h

// TP3 MenuGen 2016
// C. HUBER  03.02.2016
// Fichier Generateur.h
// Prototypes des fonctions du générateur  de signal

// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
#include <math.h>
#include <stdint.h>
#include "DefMenuGen.h"

// Définition des constantes
#define MAX_ECH 100 // Nombre d'échantillons
#define MOITIE_ECH 50 //Moitié des échantillons
#define VAL_MAX_PAS 65535   // Nombre de pas maximum de convertion
#define TRANSFORMATION_VALEUR_TIMER3 800000 // Valeur de fréquence du timer 3
#define MAX_AMPLITUDE 10000 // Amplitude maximum
#define MOITIE_AMPLITUDE 5000   // Moitier de l'amplitude maximum


// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam);

// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam);

// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam);

// Execution du générateur en envoient les valeurs calculées au dac
void  GENSIG_Execute(void);


#endif