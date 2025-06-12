#ifndef MenuGen_h
#define MenuGen_h

// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  03.02.2016
// Fichier MenuGen.h
// Gestion du menu  du générateur
// Traitement cyclique à 1 ms du Pec12


// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "DefMenuGen.h"

// Définition des limites et des pas pour la configuration du générateur
#define FREQUENCE_MAX 2000  // Fréquence maximale en Hz
#define FREQUENCE_MIN 20    // Fréquence minimale en Hz
#define AMPLITUDE_MAX 10000 // Amplitude maximale
#define AMPLITUDE_MIN 0     // Amplitude minimale
#define PAS_AMPLITUDE 100   // Incrémentation de l'amplitude
#define OFFSET_MAX 5000     // Offset maximal
#define OFFSET_MIN -5000    // Offset minimal
#define PAS_OFFSET 100      // Incrémentation de l'offset

// Énumération des différents états du menu (SEL = selection & SET = setting)
typedef enum 
{
    SEL_FORME = 0,
    SET_FORME,
    SEL_FREQU,
    SET_FREQU,
    SEL_AMPL,
    SET_AMPL,
    SEL_OFFSET,
    SET_OFFSET,
    SAVE
}MENU_STATE;

// Exécution du menu, appelée cycliquement par l'application
void MENU_Execute(S_ParamGen *pParam, bool local);

// Initialisation de l'affichage du menu
void MENU_Initialize(S_ParamGen *pParam);

// Affichage des valeurs et des options du menu
void AfficheMenu(S_ParamGen *pParam);

// Gestion des états de configuration des paramètres
MENU_STATE GestSettingMenu(MENU_STATE menuStat, S_ParamGen *tempData, S_ParamGen *pParam);

#endif



  
   







