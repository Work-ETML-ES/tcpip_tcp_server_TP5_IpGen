// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms



// Librairie inclues
#include <stdint.h>                   
#include <stdbool.h>
#include "MenuGen.h"
#include "Mc32DriverLcd.h"
#include "appgen.h"
#include "GesPec12.h"
#include "Mc32NVMUtil.h"
#include <math.h>
#include "Generateur.h"


//---------------------------------------------------------------------------------
// Définition des constantes pour l'affichage des types de signaux sur le menu LCD
// Chaque chaîne représente le nom d'un signal affiché à l'écran.
//---------------------------------------------------------------------------------
const char MenuFormes[4][21] = {
    "Sinus",
    "Triangle",
    "DentDeScie",
    "Carre"
};

// Structure pour les traitements du Pec12
S_Pec12_Descriptor Pec12;
S_9_Descriptor S9;

APPGEN_IPADDR appgen_ipAddr;
APPGEN_DATA initialisationState;

//---------------------------------------------------------------------------------
// Fonction : MENU_Initialize
// Description : Affiche les valeurs initiales des paramètres sur le LCD.
// Paramètre d'entrée :
//   - pParam : pointeur vers la structure S_ParamGen contenant les paramètres initiaux.
//---------------------------------------------------------------------------------

void MENU_Initialize(S_ParamGen *pParam) {
    lcd_gotoxy(2, 1);
    printf_lcd("Forme = %10s  ", MenuFormes[pParam->Forme]);

    lcd_gotoxy(2, 2);
    printf_lcd("Freq [Hz] =  %4d   ", pParam->Frequence);

    lcd_gotoxy(2, 3);
    printf_lcd("Ampl [mV] = %5d   ", pParam->Amplitude);

    lcd_gotoxy(2, 4);
    printf_lcd("Offset [mV] = %5d ", pParam->Offset);
}

//---------------------------------------------------------------------------------
// Fonction : MENU_Execute
// Description : Gère l'exécution cyclique du menu. Elle permet à l'utilisateur
//               de sélectionner et modifier les paramètres du signal (forme, fréquence,
//               amplitude et offset) via différents états.
// Paramètre d'entrée :
//   - pParam : pointeur vers la structure S_ParamGen contenant les paramètres
//              généraux du signal.
//---------------------------------------------------------------------------------

void MENU_Execute(S_ParamGen *pParam, bool local) {

    // Etat courant du menu. Initialisé à la sélection de la forme (SEL_FORME).
    static MENU_STATE menuState = SEL_FORME;
    static MENU_STATE previousMenuState = SET_FORME;
    static S_ParamGen tempParams;

    // Flag d'initialisation (premier passage dans la fonction).
    static uint8_t isInitializedLocal = 0;
    static uint8_t isInitializedRemote = 0;

    static uint8_t compteur = 0;

    //  MENU_Execute ? mode « remote » (local == false)

    if (local == false) {
        if (appRJ45Stat.usbStatSave == false) {
            if (!isInitializedRemote) {

                //Désactivation PEC12
                Pec12.InactivityDuration = 0;
                Pec12ClearInactivity();

                // Affiche les paramètres initiaux sur le LCD.
                lcd_putc('\f');
                MENU_Initialize(pParam);
                isInitializedLocal = 0;
                isInitializedRemote = 1;
            }

            lcd_gotoxy(1, 1);
            printf_lcd("#Forme =");
            lcd_gotoxy(1, 2);
            printf_lcd("#Freq [Hz] =");
            lcd_gotoxy(1, 3);
            printf_lcd("#Ampl [mV] =");
            lcd_gotoxy(1, 4);
            printf_lcd("#Offset [mV] =");

            AfficheMenu(pParam);
            // Mise à jour du signal et de sa période avec les nouveaux paramètres
            GENSIG_UpdateSignal(pParam);
            GENSIG_UpdatePeriode(pParam);
        }
    } else {
        // Initialisation du menu lors du premier appel
        if (!isInitializedLocal || initialisationState.initialisationMenu == true) {
            initialisationState.initialisationMenu = false;
            Pec12.InactivityDuration = 0;
            Pec12ClearInactivity();

            // Affiche les paramètres initiaux sur le LCD.
            lcd_putc('\f');
            MENU_Initialize(pParam);

            // Sauvegarde les paramètres actuels dans la structure temporaire.
            tempParams = *pParam;

            isInitializedRemote = 0;
            isInitializedLocal = 1;
        }

        //Si save pas actif alors gestion menu
        if (menuState != SAVE) {

            // Si l'état est impair, nous sommes en mode "SET" (modification)
            if (menuState % 2) {
                // Affiche les valeurs modifiables (version temporaire) sur le LCD.
                AfficheMenu(&tempParams);
                // Gère les actions de modification et met à jour l'état du menu.
                menuState = GestSettingMenu(menuState, &tempParams, pParam);
            }// Sinon, nous sommes en mode de sélection
            else {
                // Passage en mode modification si l'utilisateur appuie sur OK.
                if (Pec12IsOK()) {
                    menuState++;
                } else if (Pec12IsPlus()) {
                    menuState += 2;
                    // Si on dépasse le dernier état, on revient à la première option.
                    if (menuState >= SAVE) {
                        menuState = SEL_FORME;
                    }
                } else if (Pec12IsMinus()) {
                    menuState -= 2;
                    // Si le menu dépasse la borne inférieure, on passe à la dernière option.
                    if (menuState > SAVE) {
                        menuState = SEL_OFFSET;
                    }
                }
            }
            if (S9IsOK() || S9IsESC()) {
                // Indiquer une activité
                Pec12ClearInactivity();
                // Clear de S9
                S9ClearESC();
                S9ClearOK();
                // Passer à l'état de sauvegarde
                menuState = SAVE;
            }
        }
        // Si le changement d'état est significatif (différence de 2 ou plus),
        // on efface le marqueur affiché sur le LCD pour éviter les résidus.
        if ((abs(previousMenuState - menuState)) >= 2) {
            lcd_gotoxy(1, 1);
            printf_lcd(" ");
            lcd_gotoxy(1, 2);
            printf_lcd(" ");
            lcd_gotoxy(1, 3);
            printf_lcd(" ");
            lcd_gotoxy(1, 4);
            printf_lcd(" ");
        }

        // Mise à jour de l'état précédent avec l'état courant.
        previousMenuState = menuState;

        // Affichage "*"
        //   Si "*" alors on est entrain de choisir
        //   SI "?" alors on a choisi
        switch (menuState) {
            case SEL_FORME:
                lcd_gotoxy(1, 1);
                printf_lcd("*");
                break;
            case SET_FORME:
                lcd_gotoxy(1, 1);
                printf_lcd("?");
                break;
            case SEL_FREQU:
                lcd_gotoxy(1, 2);
                printf_lcd("*");
                break;
            case SET_FREQU:
                lcd_gotoxy(1, 2);
                printf_lcd("?");
                break;
            case SEL_AMPL:
                lcd_gotoxy(1, 3);
                printf_lcd("*");
                break;
            case SET_AMPL:
                lcd_gotoxy(1, 3);
                printf_lcd("?");
                break;
            case SEL_OFFSET:
                lcd_gotoxy(1, 4);
                printf_lcd("*");
                break;
            case SET_OFFSET:
                lcd_gotoxy(1, 4);
                printf_lcd("?");
                break;
            case SAVE:
                // Vérification de l'appui long
                if (S9IsESC()) {
                    compteur++; // Incrémentation du compteur

                    //Exécuter la sauvegarde
                    NVM_WriteBlock((uint32_t*) pParam, sizeof (S_ParamGen));
                    //Affichage d'un message de confirmation de save
                    lcd_ClearLine(3);
                    lcd_gotoxy(1, 2);
                    printf_lcd("    Sauvegarde OK   ");
                    Pec12ClearInactivity(); // Réinitialiser l'inactivité
                }// Toutes autres actions
                else if (S9IsOK() || Pec12IsESC() || Pec12IsMinus() || Pec12IsOK() || Pec12IsPlus()) {
                    compteur++; // Incrémentation du compteur
                    lcd_ClearLine(3);
                    // Annuler la sauvegarde
                    lcd_gotoxy(1, 2);
                    printf_lcd(" Sauvegarde ANNULEE ");
                    Pec12ClearInactivity(); // Réinitialiser l'inactivité
                }

                // Boucle d'attente de 2 secondes
                if (compteur > 0) {
                    compteur++; // Continuer l'incrémentation
                    if (compteur > 200) {
                        // Réinitialiser l'affichage du menu
                        MENU_Initialize(pParam);
                        compteur = 0; // Remise à zéro du compteur
                        menuState = SEL_FORME; // Retour à l'état initial
                    }
                } else {
                    // Afficher la question de la sauvegarde
                    lcd_putc('\f');

                    lcd_gotoxy(1, 2);
                    printf_lcd("    Sauvegarde ?    ");
                    lcd_gotoxy(1, 3);
                    printf_lcd("    (Appui long)    ");
                }
                break;
            default:
                break;
        }

        // Réinitialise les indicateurs d'événements (touches) du système Pec12
        Pec12ClearESC();
        Pec12ClearOK();
        Pec12ClearPlus();
        Pec12ClearMinus();

        S9ClearESC();
        S9ClearOK();
    }
}

//---------------------------------------------------------------------------------
// Fonction : AfficheMenu
// Description : Affiche uniquement les valeurs modifiables du menu en mode setting.
// Paramètre d'entrée :
//   - pParam : pointeur vers la structure S_ParamGen (temporaire) contenant les valeurs à modifier.
//---------------------------------------------------------------------------------

void AfficheMenu(S_ParamGen *pParam) {
    // Affiche le nom de la forme de signal modifiée
    lcd_gotoxy(10, 1);
    printf_lcd("%10s", MenuFormes[pParam->Forme]);

    // Affiche la valeur de la fréquence modifiée
    lcd_gotoxy(15, 2);
    printf_lcd("%4d", pParam->Frequence);

    // Affiche la valeur de l'amplitude modifiée
    lcd_gotoxy(14, 3);
    printf_lcd("%5d", pParam->Amplitude);

    // Affiche la valeur de l'offset modifiée
    lcd_gotoxy(16, 4);
    printf_lcd("%5d", pParam->Offset);
}

//---------------------------------------------------------------------------------
// Fonction : GestSettingMenu
// Description : Gère les modifications apportées aux paramètres en mode setting.
//               En fonction des actions de l'utilisateur (boutons OK, ESC, Plus, Moins),
//               la fonction ajuste la valeur du paramètre en cours de modification.
// Paramètres d'entrée :
//   - menuState : état courant du menu (de type MENU_STATE)
//   - tempData  : pointeur vers la structure temporaire contenant les paramètres modifiables
//   - pParam    : pointeur vers la structure principale des paramètres (pour sauvegarde/restauration)
// Retour :
//   - Le nouvel état du menu après traitement des actions.
//---------------------------------------------------------------------------------

MENU_STATE GestSettingMenu(MENU_STATE menuState, S_ParamGen *tempData, S_ParamGen *pParam) {
    // Si l'utilisateur confirme la modification en appuyant sur OK
    if (Pec12IsOK()) {
        // Sauvegarde la valeur modifiée dans la structure principale
        *pParam = *tempData;
        // Retour en mode sélection (état précédent = mode setting - 1)
        menuState--;
        // Met à jour l'affichage avec la valeur validée
        AfficheMenu(tempData);
        // Mise à jour du signal et de sa période avec les nouveaux paramètres
        GENSIG_UpdateSignal(pParam);
        GENSIG_UpdatePeriode(pParam);
    }// Si l'utilisateur annule la modification en appuyant sur ESC
    else if (Pec12IsESC()) {
        // Restaure la valeur initiale dans la structure temporaire
        *tempData = *pParam;
        // Retour en mode sélection
        menuState--;
        // Affiche les valeurs restaurées sur le LCD
        AfficheMenu(pParam);
    }// Si l'utilisateur augmente la valeur en appuyant sur moins
    else if (Pec12IsMinus()) {
        switch (menuState) {
            case SET_FORME:
                // Passage à la forme suivante si l'on n'est pas déjà à la limite supérieure
                if (tempData->Forme < SignalCarre) {
                    tempData->Forme++;
                } else {
                    tempData->Forme = SignalCarre;
                }
                break;
            case SET_FREQU:
                // Augmente la fréquence d'un pas ; rebouclage à la valeur minimale si on dépasse FREQUENCE_MAX
                if (tempData->Frequence < FREQUENCE_MAX) {
                    tempData->Frequence += FREQUENCE_MIN;
                } else {
                    tempData->Frequence = FREQUENCE_MIN;
                }
                break;
            case SET_AMPL:
                // Augmente l'amplitude d'un pas ; rebouclage à AMPLITUDE_MIN si la valeur maximale est atteinte
                if (tempData->Amplitude < AMPLITUDE_MAX) {
                    tempData->Amplitude += PAS_AMPLITUDE;
                } else {
                    tempData->Amplitude = AMPLITUDE_MIN;
                }
                break;
            case SET_OFFSET:
                // Augmente l'offset d'un pas ; on ne dépasse pas OFFSET_MAX
                if (tempData->Offset < OFFSET_MAX) {
                    tempData->Offset += PAS_OFFSET;
                } else {
                    tempData->Offset = OFFSET_MAX;
                }
                break;
            default:
                break;
        }
    }// Si l'utilisateur diminue la valeur en appuyant sur plus
    else if (Pec12IsPlus()) {
        switch (menuState) {
            case SET_FORME:
                // Passage à la forme précédente si possible, sinon maintien à SignalSinus
                if (tempData->Forme > SignalSinus) {
                    tempData->Forme--;
                } else {
                    tempData->Forme = SignalSinus;
                }
                break;
            case SET_FREQU:
                // Diminue la fréquence d'un pas ; rebouclage à FREQUENCE_MAX si la valeur minimale est atteinte
                if (tempData->Frequence > FREQUENCE_MIN) {
                    tempData->Frequence -= FREQUENCE_MIN;
                } else {
                    tempData->Frequence = FREQUENCE_MAX;
                }
                break;
            case SET_AMPL:
                // Diminue l'amplitude d'un pas ; rebouclage à AMPLITUDE_MAXsi la valeur minimale est atteinte
                if (tempData->Amplitude > AMPLITUDE_MIN) {
                    tempData->Amplitude -= PAS_AMPLITUDE;
                } else {
                    tempData->Amplitude = AMPLITUDE_MAX;
                }
                break;
            case SET_OFFSET:
                // Diminue l'offset d'un pas ; on ne descend pas en dessous de OFFSET_MIN
                if (tempData->Offset > OFFSET_MIN) {
                    tempData->Offset -= PAS_OFFSET;
                } else {
                    tempData->Offset = OFFSET_MIN;
                }
                break;
            default:
                break;
        }
    }
    // Retourne l'état mis à jour du menu
    return menuState;
}