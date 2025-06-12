// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du g�n�rateur
// Traitement cyclique � 10 ms



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
// D�finition des constantes pour l'affichage des types de signaux sur le menu LCD
// Chaque cha�ne repr�sente le nom d'un signal affich� � l'�cran.
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
// Description : Affiche les valeurs initiales des param�tres sur le LCD.
// Param�tre d'entr�e :
//   - pParam : pointeur vers la structure S_ParamGen contenant les param�tres initiaux.
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
// Description : G�re l'ex�cution cyclique du menu. Elle permet � l'utilisateur
//               de s�lectionner et modifier les param�tres du signal (forme, fr�quence,
//               amplitude et offset) via diff�rents �tats.
// Param�tre d'entr�e :
//   - pParam : pointeur vers la structure S_ParamGen contenant les param�tres
//              g�n�raux du signal.
//---------------------------------------------------------------------------------

void MENU_Execute(S_ParamGen *pParam, bool local) {

    // Etat courant du menu. Initialis� � la s�lection de la forme (SEL_FORME).
    static MENU_STATE menuState = SEL_FORME;
    static MENU_STATE previousMenuState = SET_FORME;
    static S_ParamGen tempParams;

    // Flag d'initialisation (premier passage dans la fonction).
    static uint8_t isInitializedLocal = 0;
    static uint8_t isInitializedRemote = 0;

    static uint8_t compteur = 0;

    //  MENU_Execute ? mode � remote � (local == false)

    if (local == false) {
        if (appRJ45Stat.usbStatSave == false) {
            if (!isInitializedRemote) {

                //D�sactivation PEC12
                Pec12.InactivityDuration = 0;
                Pec12ClearInactivity();

                // Affiche les param�tres initiaux sur le LCD.
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
            // Mise � jour du signal et de sa p�riode avec les nouveaux param�tres
            GENSIG_UpdateSignal(pParam);
            GENSIG_UpdatePeriode(pParam);
        }
    } else {
        // Initialisation du menu lors du premier appel
        if (!isInitializedLocal || initialisationState.initialisationMenu == true) {
            initialisationState.initialisationMenu = false;
            Pec12.InactivityDuration = 0;
            Pec12ClearInactivity();

            // Affiche les param�tres initiaux sur le LCD.
            lcd_putc('\f');
            MENU_Initialize(pParam);

            // Sauvegarde les param�tres actuels dans la structure temporaire.
            tempParams = *pParam;

            isInitializedRemote = 0;
            isInitializedLocal = 1;
        }

        //Si save pas actif alors gestion menu
        if (menuState != SAVE) {

            // Si l'�tat est impair, nous sommes en mode "SET" (modification)
            if (menuState % 2) {
                // Affiche les valeurs modifiables (version temporaire) sur le LCD.
                AfficheMenu(&tempParams);
                // G�re les actions de modification et met � jour l'�tat du menu.
                menuState = GestSettingMenu(menuState, &tempParams, pParam);
            }// Sinon, nous sommes en mode de s�lection
            else {
                // Passage en mode modification si l'utilisateur appuie sur OK.
                if (Pec12IsOK()) {
                    menuState++;
                } else if (Pec12IsPlus()) {
                    menuState += 2;
                    // Si on d�passe le dernier �tat, on revient � la premi�re option.
                    if (menuState >= SAVE) {
                        menuState = SEL_FORME;
                    }
                } else if (Pec12IsMinus()) {
                    menuState -= 2;
                    // Si le menu d�passe la borne inf�rieure, on passe � la derni�re option.
                    if (menuState > SAVE) {
                        menuState = SEL_OFFSET;
                    }
                }
            }
            if (S9IsOK() || S9IsESC()) {
                // Indiquer une activit�
                Pec12ClearInactivity();
                // Clear de S9
                S9ClearESC();
                S9ClearOK();
                // Passer � l'�tat de sauvegarde
                menuState = SAVE;
            }
        }
        // Si le changement d'�tat est significatif (diff�rence de 2 ou plus),
        // on efface le marqueur affich� sur le LCD pour �viter les r�sidus.
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

        // Mise � jour de l'�tat pr�c�dent avec l'�tat courant.
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
                // V�rification de l'appui long
                if (S9IsESC()) {
                    compteur++; // Incr�mentation du compteur

                    //Ex�cuter la sauvegarde
                    NVM_WriteBlock((uint32_t*) pParam, sizeof (S_ParamGen));
                    //Affichage d'un message de confirmation de save
                    lcd_ClearLine(3);
                    lcd_gotoxy(1, 2);
                    printf_lcd("    Sauvegarde OK   ");
                    Pec12ClearInactivity(); // R�initialiser l'inactivit�
                }// Toutes autres actions
                else if (S9IsOK() || Pec12IsESC() || Pec12IsMinus() || Pec12IsOK() || Pec12IsPlus()) {
                    compteur++; // Incr�mentation du compteur
                    lcd_ClearLine(3);
                    // Annuler la sauvegarde
                    lcd_gotoxy(1, 2);
                    printf_lcd(" Sauvegarde ANNULEE ");
                    Pec12ClearInactivity(); // R�initialiser l'inactivit�
                }

                // Boucle d'attente de 2 secondes
                if (compteur > 0) {
                    compteur++; // Continuer l'incr�mentation
                    if (compteur > 200) {
                        // R�initialiser l'affichage du menu
                        MENU_Initialize(pParam);
                        compteur = 0; // Remise � z�ro du compteur
                        menuState = SEL_FORME; // Retour � l'�tat initial
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

        // R�initialise les indicateurs d'�v�nements (touches) du syst�me Pec12
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
// Param�tre d'entr�e :
//   - pParam : pointeur vers la structure S_ParamGen (temporaire) contenant les valeurs � modifier.
//---------------------------------------------------------------------------------

void AfficheMenu(S_ParamGen *pParam) {
    // Affiche le nom de la forme de signal modifi�e
    lcd_gotoxy(10, 1);
    printf_lcd("%10s", MenuFormes[pParam->Forme]);

    // Affiche la valeur de la fr�quence modifi�e
    lcd_gotoxy(15, 2);
    printf_lcd("%4d", pParam->Frequence);

    // Affiche la valeur de l'amplitude modifi�e
    lcd_gotoxy(14, 3);
    printf_lcd("%5d", pParam->Amplitude);

    // Affiche la valeur de l'offset modifi�e
    lcd_gotoxy(16, 4);
    printf_lcd("%5d", pParam->Offset);
}

//---------------------------------------------------------------------------------
// Fonction : GestSettingMenu
// Description : G�re les modifications apport�es aux param�tres en mode setting.
//               En fonction des actions de l'utilisateur (boutons OK, ESC, Plus, Moins),
//               la fonction ajuste la valeur du param�tre en cours de modification.
// Param�tres d'entr�e :
//   - menuState : �tat courant du menu (de type MENU_STATE)
//   - tempData  : pointeur vers la structure temporaire contenant les param�tres modifiables
//   - pParam    : pointeur vers la structure principale des param�tres (pour sauvegarde/restauration)
// Retour :
//   - Le nouvel �tat du menu apr�s traitement des actions.
//---------------------------------------------------------------------------------

MENU_STATE GestSettingMenu(MENU_STATE menuState, S_ParamGen *tempData, S_ParamGen *pParam) {
    // Si l'utilisateur confirme la modification en appuyant sur OK
    if (Pec12IsOK()) {
        // Sauvegarde la valeur modifi�e dans la structure principale
        *pParam = *tempData;
        // Retour en mode s�lection (�tat pr�c�dent = mode setting - 1)
        menuState--;
        // Met � jour l'affichage avec la valeur valid�e
        AfficheMenu(tempData);
        // Mise � jour du signal et de sa p�riode avec les nouveaux param�tres
        GENSIG_UpdateSignal(pParam);
        GENSIG_UpdatePeriode(pParam);
    }// Si l'utilisateur annule la modification en appuyant sur ESC
    else if (Pec12IsESC()) {
        // Restaure la valeur initiale dans la structure temporaire
        *tempData = *pParam;
        // Retour en mode s�lection
        menuState--;
        // Affiche les valeurs restaur�es sur le LCD
        AfficheMenu(pParam);
    }// Si l'utilisateur augmente la valeur en appuyant sur moins
    else if (Pec12IsMinus()) {
        switch (menuState) {
            case SET_FORME:
                // Passage � la forme suivante si l'on n'est pas d�j� � la limite sup�rieure
                if (tempData->Forme < SignalCarre) {
                    tempData->Forme++;
                } else {
                    tempData->Forme = SignalCarre;
                }
                break;
            case SET_FREQU:
                // Augmente la fr�quence d'un pas ; rebouclage � la valeur minimale si on d�passe FREQUENCE_MAX
                if (tempData->Frequence < FREQUENCE_MAX) {
                    tempData->Frequence += FREQUENCE_MIN;
                } else {
                    tempData->Frequence = FREQUENCE_MIN;
                }
                break;
            case SET_AMPL:
                // Augmente l'amplitude d'un pas ; rebouclage � AMPLITUDE_MIN si la valeur maximale est atteinte
                if (tempData->Amplitude < AMPLITUDE_MAX) {
                    tempData->Amplitude += PAS_AMPLITUDE;
                } else {
                    tempData->Amplitude = AMPLITUDE_MIN;
                }
                break;
            case SET_OFFSET:
                // Augmente l'offset d'un pas ; on ne d�passe pas OFFSET_MAX
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
                // Passage � la forme pr�c�dente si possible, sinon maintien � SignalSinus
                if (tempData->Forme > SignalSinus) {
                    tempData->Forme--;
                } else {
                    tempData->Forme = SignalSinus;
                }
                break;
            case SET_FREQU:
                // Diminue la fr�quence d'un pas ; rebouclage � FREQUENCE_MAX si la valeur minimale est atteinte
                if (tempData->Frequence > FREQUENCE_MIN) {
                    tempData->Frequence -= FREQUENCE_MIN;
                } else {
                    tempData->Frequence = FREQUENCE_MAX;
                }
                break;
            case SET_AMPL:
                // Diminue l'amplitude d'un pas ; rebouclage � AMPLITUDE_MAXsi la valeur minimale est atteinte
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
    // Retourne l'�tat mis � jour du menu
    return menuState;
}