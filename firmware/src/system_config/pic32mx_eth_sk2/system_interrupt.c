/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

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

#include "system/common/sys_common.h"
#include "../apps/tcpip/tcpip_tcp_server_TP5_IpGen/firmware/src/app.h"
#include "../apps/tcpip/tcpip_tcp_server_TP5_IpGen/firmware/src/appgen.h"
#include "system_definitions.h"
#include "Generateur.h"
#include "GesPec12.h"
#include <stdint.h>
#include "bsp.h"

// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************

void __ISR(_TIMER_2_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance0(void) {
    DRV_TMR_Tasks(sysObj.drvTmr0);
}

// timer 1 configure pour interrupt toutes les 1 ms
// anti-rebond et appgen

void __ISR(_TIMER_1_VECTOR, ipl3AUTO) IntHandlerDrvTmrInstance2(void) {
    static uint16_t wait3Secondes = 0;
    static uint8_t wait10cycle = 0;

    ScanPec12(PEC12_A, PEC12_B, PEC12_PB);

    ScanS9(S_OK);

    LED1_W = !LED1_R;

    if (wait3Secondes >= WAITFOR3SECONDES) {
        if (wait10cycle >= WAITFOR10CYCLES) {

            wait3Secondes = 2998;
            APPGEN_UpdateState(APPGEN_STATE_SERVICE_TASKS);
            wait10cycle = 0;
        } else {
            wait10cycle++;
        }
    } else {
        wait3Secondes++;
    }

    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_TIMER_1);
}

void __ISR(_TIMER_3_VECTOR, ipl7AUTO) IntHandlerDrvTmrInstance1(void) {
    LED0_W = 1;
    GENSIG_Execute();
    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_TIMER_3);
    LED0_W = 0;
}

void __ISR(_USB_1_VECTOR, ipl1AUTO) _IntHandlerUSBInstance0(void) {
    DRV_USBFS_Tasks_ISR(sysObj.drvUSBObject);
}

void __ISR(_ETH_VECTOR, ipl5AUTO) _IntHandler_ETHMAC(void) {
    DRV_ETHMAC_Tasks_ISR((SYS_MODULE_OBJ) 0);
}

/* This function is used by ETHMAC driver */
bool SYS_INT_SourceRestore(INT_SOURCE src, int level) {
    if (level) {
        SYS_INT_SourceEnable(src);
    }

    return level;
}

/*******************************************************************************
 End of File
 */
