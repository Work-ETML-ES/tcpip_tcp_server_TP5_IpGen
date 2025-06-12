#ifndef Generateur_h
#define Generateur_h

// TP3 MenuGen 2016
// C. HUBER  03.02.2016
// Fichier Generateur.h
// Prototypes des fonctions du g�n�rateur  de signal

// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
#include <math.h>
#include <stdint.h>
#include "DefMenuGen.h"

// D�finition des constantes
#define MAX_ECH 100 // Nombre d'�chantillons
#define MOITIE_ECH 50 //Moiti� des �chantillons
#define VAL_MAX_PAS 65535   // Nombre de pas maximum de convertion
#define TRANSFORMATION_VALEUR_TIMER3 800000 // Valeur de fr�quence du timer 3
#define MAX_AMPLITUDE 10000 // Amplitude maximum
#define MOITIE_AMPLITUDE 5000   // Moitier de l'amplitude maximum


// Initialisation du  g�n�rateur
void  GENSIG_Initialize(S_ParamGen *pParam);

// Mise � jour de la periode d'�chantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam);

// Mise � jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam);

// Execution du g�n�rateur en envoient les valeurs calcul�es au dac
void  GENSIG_Execute(void);


#endif