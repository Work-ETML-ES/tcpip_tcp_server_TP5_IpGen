/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    appgen.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "appgen.h"
#include "Mc32DriverLcd.h"
#include "../apps/tcpip/tcpip_tcp_server_TP5_IpGen/firmware/src/system_config/pic32mx_eth_sk2/framework/driver/drv_tmr_static.h"
#include "Mc32gestSpiDac.h"
#include "MenuGen.h"
#include "GesPec12.h"
#include "Generateur.h"
#include "Mc32Debounce.h"

// Descripteur des sinaux
S_SwitchDescriptor DescrS9;

// Structure pour les traitements de S9
S_Pec12_Descriptor S9;

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
 */

APPGEN_DATA appgenData;
APPGEN_IPADDR appgen_ipAddr;
APPGEN_DATA affichageIP;
APPGEN_DATA initialisationState;

S_ParamGen LocalParamGen;
S_ParamGen RemoteParamGen;

APPGEN_DATA appRJ45Stat;
APPGEN_DATA usbStatSave;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPGEN_Initialize ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appgenData.stategen = APPGEN_STATE_INIT;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void APPGEN_Tasks ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Tasks(void) {
    static uint16_t wait5Secondes = 0;

    /* Check the application's current state. */
    switch (appgenData.stategen) {
            /* Application's initial state. */
        case APPGEN_STATE_INIT:
        {
            // Initialisation et allumage de l'affichage LCD
            lcd_init();
            lcd_bl_on();

            // Init SPI DAC
            SPI_InitLTC2604();

            // Initialisation PEC12 et S9
            Pec12Init();
            S9Init();

            // Initialisation du generateur
            GENSIG_Initialize(&LocalParamGen);

            // 5) On duplique les paramètres locaux pour la partie ?remote?
            RemoteParamGen = LocalParamGen;

            // Affichage à l'enclechement 
            lcd_gotoxy(1, 1);
            printf_lcd("TP5 IpGen 2025");
            lcd_gotoxy(1, 2);
            printf_lcd("Nasser El-Ghandour");

            // mise a jour du signal et de la periode des parametre local
            GENSIG_UpdateSignal(&LocalParamGen);
            GENSIG_UpdatePeriode(&LocalParamGen);

            // ajout init drivers timers statiques
            DRV_TMR0_Initialize();
            DRV_TMR1_Initialize();

            // Active les timers 
            DRV_TMR0_Start();
            DRV_TMR1_Start();

            APPGEN_UpdateState(APPGEN_STATE_WAIT); // Passer à l'état d'attente
            break;
        }
        case APPGEN_STATE_WAIT:
        {
            // nothing to do
            break;
        }
        case APPGEN_STATE_SERVICE_TASKS:
        {
            // Toggle de la led 2
            BSP_LEDToggle(BSP_LED_2);

            if (affichageIP.ipState == true) {
                APPGEN_DisplayStoredIP();
                if (wait5Secondes >= 500) {
                    wait5Secondes = 0;
                    affichageIP.ipState = false;
                    initialisationState.initialisationMenu = true;
                }else {
                    ++wait5Secondes;
                }

            } else {
                if (appRJ45Stat.rj45Stat) {
                    MENU_Execute(&RemoteParamGen, false);
                    BSP_LEDOn(BSP_LED_5);
                    BSP_LEDOff(BSP_LED_7);
                } else {
                    MENU_Execute(&LocalParamGen, true);
                    BSP_LEDOff(BSP_LED_5);
                    BSP_LEDOn(BSP_LED_7);
                }
            }
            // Si on doit sauver les paramètres sur USB, on lance la demande de sauvegarde
            if (appRJ45Stat.usbStatSave) {
                MENU_DemandeSave();
            }

            APPGEN_UpdateState(APPGEN_STATE_WAIT); // Passer à l'état d'attente
            break;
        }

            /* TODO: implement your application state machine.*/


            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


//------------------------------------------------------------------------------
// Mise à jour de l?état de la machine
//------------------------------------------------------------------------------

void APPGEN_UpdateState(APPGEN_STATES NewState) {
    appgenData.stategen = NewState; // Change l'état de la machine d'état
}

//------------------------------------------------------------------------------
// Affiche le message ?Sauvegarde USB? pendant ~3 s puis l?efface
//------------------------------------------------------------------------------

void MENU_DemandeSave(void) {
    static uint16_t wait3Secondes = 0;

    // On remet l?inactivité de l?encodeur à zéro
    Pec12ClearInactivity();

    // Affichage du message de sauvegarde
    lcd_putc('\f');
    lcd_gotoxy(4, 2);
    printf_lcd("Sauvegarde USB");

    // Après environ 500 appels (~3 s si TMR déclenche à 6 ms), on efface
    if (wait3Secondes >= 500) {
        lcd_putc('\f');
        appRJ45Stat.usbStatSave = false; // on désactive le flag de sauvegarde
        wait3Secondes = 0; // on réinitialise le compteur
    } else {
        wait3Secondes++; // on incrémente tant qu?on n?a pas atteint 3 s
    }
}

void APPGEN_SetIP(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) {
    appgen_ipAddr.v[0] = ip0;
    appgen_ipAddr.v[1] = ip1;
    appgen_ipAddr.v[2] = ip2;
    appgen_ipAddr.v[3] = ip3;
}

void APPGEN_DisplayStoredIP(void) {
    lcd_putc('\f');
    lcd_gotoxy(7, 2);
    printf_lcd("Adr. IP");
    lcd_gotoxy(4, 3);
    printf_lcd("%d.%d.%d.%d", appgen_ipAddr.v[0], appgen_ipAddr.v[1], appgen_ipAddr.v[2], appgen_ipAddr.v[3]);
}

/*******************************************************************************
 End of File
 */
