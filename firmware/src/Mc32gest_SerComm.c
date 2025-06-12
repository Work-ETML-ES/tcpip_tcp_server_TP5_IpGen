// Mc32Gest_SerComm.C
// fonction d'émission et de réception des message
// transmis en USB CDC
// Canevas TP4 SLO2 2015-2015


#include "app.h"
#include "Mc32gest_SerComm.h"
#include "Mc32NVMUtil.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Generateur.h"
#include "MenuGen.h"

// Fonction de reception  d'un  message
// Met à jour les paramètres du generateur a partir du message recu
// Format du message
// !S=TF=200A=5000O=+450W=0#
// !S=TF=200A=5000O=+450W=1#    // ack sauvegarde

bool GetMessage(int8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo) {
    char *pt_Forme = NULL;
    char *pt_Frequence = NULL;
    char *pt_Amplitude = NULL;
    char *pt_Offset = NULL;
    char *pt_Sauvegarde = NULL;
    Pec12ClearInactivity();

    //vérification des char en début et fin de trames
    if (USBReadBuffer[0] != '!' || strchr((char*) USBReadBuffer, '#') == NULL)
        return false;


    pt_Forme = strstr((char*) USBReadBuffer, "S=");
    pt_Frequence = strstr((char*) USBReadBuffer, "F=");
    pt_Amplitude = strstr((char*) USBReadBuffer, "A=");
    pt_Offset = strstr((char*) USBReadBuffer, "O=");
    pt_Sauvegarde = strstr((char*) USBReadBuffer, "W=");

    if (!pt_Forme || !pt_Frequence || !pt_Amplitude || !pt_Offset || !pt_Sauvegarde)
        return false;

    // Décodage de la forme
    switch (pt_Forme[2]) {
        case 'T': pParam->Forme = SignalTriangle;
            break;
        case 'S': pParam->Forme = SignalSinus;
            break;
        case 'C': pParam->Forme = SignalCarre;
            break;
        case 'D': pParam->Forme = SignalDentDeScie;
            break;
        default: return false;
    }

    // ASCII to Integer
    pParam->Frequence = atoi(pt_Frequence + 2);
    pParam->Amplitude = atoi(pt_Amplitude + 2);
    pParam->Offset = atoi(pt_Offset + 2);

    // ASCII to Integer - Save mode
    *SaveTodo = (atoi(pt_Sauvegarde + 2) == 1);

    if (*SaveTodo == true) {
        NVM_WriteBlock((uint32_t*) pParam, sizeof (S_ParamGen)); 
        appRJ45Stat.usbStatSave = true;
    }
    return true;
}


// Fonction d'envoi d'un  message
// Rempli le tampon d'émission pour USB en fonction des paramètres du générateur
// Format du message
// !S=TF=0200A=05000O=+0000WP=0#
// !S=TF=0200A=05000O=+0000WP=1#    // ack sauvegarde

void SendMessage(int8_t *USBSendBuffer, S_ParamGen *pParam, bool Saved) {
    char formeChar;
    int saveFlag;
    switch (pParam->Forme) {
        case SignalTriangle: formeChar = 'T';
            break;
        case SignalSinus: formeChar = 'S';
            break;
        case SignalCarre: formeChar = 'C';
            break;
        case SignalDentDeScie: formeChar = 'D';
            break;
        default:
            break;
    }

    if (Saved) {
        saveFlag = 1;
    } else {
        saveFlag = 0;
    }
    // Construction de la trame finale
    sprintf((char*) USBSendBuffer,"!S=%cF=%dA=%dO=%dWP=%d#", formeChar, pParam->Frequence, pParam->Amplitude, pParam->Offset, saveFlag);
}
