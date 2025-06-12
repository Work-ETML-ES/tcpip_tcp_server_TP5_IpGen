/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    Pour Tp3 Menu et generateur de signal .

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END

#ifndef _APPGEN_H
#define _APPGEN_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#include "DefMenuGen.h"    
#include "GesPec12.h"
#include <stdbool.h>

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
#define WAITFOR3SECONDES 2999
#define WAITFOR10CYCLES 9

#define PRESSION_LONGUE_S9 499

//#define S9 PORTGbits.RG12
// *****************************************************************************

/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
 */

typedef enum {
    /* Application's state machine's initial state. */
    APPGEN_STATE_INIT = 0,
    APPGEN_STATE_WAIT,
    APPGEN_STATE_SERVICE_TASKS

} APPGEN_STATES;

typedef struct {
    uint8_t v[4];
} APPGEN_IPADDR;

//extern APPGEN_IPADDR appgen_ipAddr; 

void APPGEN_SetIP(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3);
void APPGEN_DisplayStoredIP(void);
// *****************************************************************************

/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct {
    /* The application's current state */
    APPGEN_STATES stategen;
    bool rj45Stat;
    bool usbStatSave;
    bool ipState;
    bool initialisationMenu; 

    /* TODO: Define any additional data used by the application. */

} APPGEN_DATA;

//variable externe
extern APPGEN_DATA appRJ45Stat;
extern APPGEN_DATA affichageIP;
extern APPGEN_DATA usbStatSave;
extern APPGEN_DATA initialisationState;
//Structure pour les événements du switch S9
typedef struct {
    uint8_t OK : 1; // événement action OK
    uint8_t ESC : 1; // événement action ESC
    uint16_t PressDuration; // Pour durée pression du P.B.
} S_9_Descriptor;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the 
    application in its initial state and prepares it to run so that its 
    APP_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
 */

void APPGEN_Initialize(void);


/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APPGEN_Tasks(void);
void APPGEN_UpdateState(APPGEN_STATES NewState);
void MENU_DemandeSave(void);

// Initialisation de S9
void S9Init(void);

// Scanne du switch S9
void ScanS9(bool ValS9);

//       S9IsOK         true indique action OK
bool S9IsOK(void);
//       S9IsESC        true indique action ESC
bool S9IsESC(void);
//       S9ClearOK      annule indication action OK
void S9ClearOK(void);
//       S9ClearESC     annule indication action ESC
void S9ClearESC(void);

#endif /* _APPGEN_H */
/*******************************************************************************
 End of File
 */