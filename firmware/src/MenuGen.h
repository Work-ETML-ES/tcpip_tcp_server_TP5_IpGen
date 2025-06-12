#ifndef MenuGen_h
#define MenuGen_h

// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  03.02.2016
// Fichier MenuGen.h
// Gestion du menu  du g�n�rateur
// Traitement cyclique � 1 ms du Pec12


// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "DefMenuGen.h"

// D�finition des limites et des pas pour la configuration du g�n�rateur
#define FREQUENCE_MAX 2000  // Fr�quence maximale en Hz
#define FREQUENCE_MIN 20    // Fr�quence minimale en Hz
#define AMPLITUDE_MAX 10000 // Amplitude maximale
#define AMPLITUDE_MIN 0     // Amplitude minimale
#define PAS_AMPLITUDE 100   // Incr�mentation de l'amplitude
#define OFFSET_MAX 5000     // Offset maximal
#define OFFSET_MIN -5000    // Offset minimal
#define PAS_OFFSET 100      // Incr�mentation de l'offset

// �num�ration des diff�rents �tats du menu (SEL = selection & SET = setting)
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

// Ex�cution du menu, appel�e cycliquement par l'application
void MENU_Execute(S_ParamGen *pParam, bool local);

// Initialisation de l'affichage du menu
void MENU_Initialize(S_ParamGen *pParam);

// Affichage des valeurs et des options du menu
void AfficheMenu(S_ParamGen *pParam);

// Gestion des �tats de configuration des param�tres
MENU_STATE GestSettingMenu(MENU_STATE menuStat, S_ParamGen *tempData, S_ParamGen *pParam);

#endif



  
   







