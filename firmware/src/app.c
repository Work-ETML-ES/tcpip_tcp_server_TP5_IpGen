/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

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

#include "app.h"
#include "Mc32DriverLcd.h"
#include "appgen.h"
#include "Mc32gest_SerComm.h"
#define SERVER_PORT 9760

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

APP_DATA appData;
APPGEN_DATA appRJ45Stat;

bool SaveTodo = false;

S_ParamGen LocalParamGen;
S_ParamGen RemoteParamGen;
APPGEN_IPADDR appgen_ipAddr;
APPGEN_DATA affichageIP;

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
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize(void) {
    /* Place the App state machine in its initial state. */
    appData.state = APP_TCPIP_WAIT_INIT;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void) {
    SYS_STATUS tcpipStat;
    const char *netName, *netBiosName;
    static IPV4_ADDR dwLastIP[2] = {
        {-1},
        {-1}
    };
    IPV4_ADDR ipAddr;
    int i, nNets;
    TCPIP_NET_HANDLE netH;

    // static int16_t wait5Secondes = 0;

    SYS_CMD_READY_TO_READ();
    switch (appData.state) {
        case APP_TCPIP_WAIT_INIT:
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
            if (tcpipStat < 0) { // some error occurred
                SYS_CONSOLE_MESSAGE(" APP: TCP/IP stack initialization failed!\r\n");

                //ajout SCA 
                lcd_gotoxy(1, 4);
                printf_lcd("TCP/IP error !");

                appData.state = APP_TCPIP_ERROR;
            } else if (tcpipStat == SYS_STATUS_READY) {
                // now that the stack is ready we can check the
                // available interfaces
                nNets = TCPIP_STACK_NumberOfNetworksGet();
                for (i = 0; i < nNets; i++) {

                    netH = TCPIP_STACK_IndexToNet(i);
                    netName = TCPIP_STACK_NetNameGet(netH);
                    netBiosName = TCPIP_STACK_NetBIOSName(netH);

#if defined(TCPIP_STACK_USE_NBNS)
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS enabled\r\n", netName, netBiosName);
#else
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS disabled\r\n", netName, netBiosName);
#endif  // defined(TCPIP_STACK_USE_NBNS)

                }
                appData.state = APP_TCPIP_WAIT_FOR_IP;

            }
            break;

        case APP_TCPIP_WAIT_FOR_IP:
            appRJ45Stat.rj45Stat = false;
            // if the IP address of an interface has changed
            // display the new value on the system console
            nNets = TCPIP_STACK_NumberOfNetworksGet();

            for (i = 0; i < nNets; i++) {
                netH = TCPIP_STACK_IndexToNet(i);
                if (!TCPIP_STACK_NetIsReady(netH)) {
                    return; // interface not ready yet!
                }
                ipAddr.Val = TCPIP_STACK_NetAddress(netH);
                if (dwLastIP[i].Val != ipAddr.Val) {
                    affichageIP.ipState = true;
                    SYS_CONSOLE_MESSAGE(TCPIP_STACK_NetNameGet(netH));
                    SYS_CONSOLE_MESSAGE(" IP Address: ");
                    SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);

                    APPGEN_SetIP(ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                }
                appData.state = APP_TCPIP_OPENING_SERVER;
            }
            break;
        case APP_TCPIP_OPENING_SERVER:
        {

            SYS_CONSOLE_PRINT("Waiting for Client Connection on port: %d\r\n", SERVER_PORT);
            appData.socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, SERVER_PORT, 0);
            if (appData.socket == INVALID_SOCKET) {
                SYS_CONSOLE_MESSAGE("Couldn't open server socket\r\n");
                break;
            }
            // N�cessaire si on veut que TCPIP_TCP_IsConnected() d�tecte d�connexion du c�ble 
            appData.keepAlive.keepAliveEnable = true;
            appData.keepAlive.keepAliveTmo = 1000;
            //[ms] / 0 => valeur par d�faut 
            appData.keepAlive.keepAliveUnackLim = 2;
            //[nb de tentatives] / 0 => valeur par d�faut 
            TCPIP_TCP_OptionsSet(appData.socket, TCP_OPTION_KEEP_ALIVE, &(appData.keepAlive));

            appData.state = APP_TCPIP_WAIT_FOR_CONNECTION;
        }
            break;

        case APP_TCPIP_WAIT_FOR_CONNECTION:
        {
            if (!TCPIP_TCP_IsConnected(appData.socket)) {
                return;
            } else {
                // We got a connection
                appRJ45Stat.rj45Stat = true;
                appData.state = APP_TCPIP_SERVING_CONNECTION;
                SYS_CONSOLE_MESSAGE("Received a connection\r\n");
            }
        }
            break;

        case APP_TCPIP_SERVING_CONNECTION:
        {
            if (!TCPIP_TCP_IsConnected(appData.socket)) {
                appData.state = APP_TCPIP_CLOSING_CONNECTION;
                SYS_CONSOLE_MESSAGE("Connection was closed\r\n");
                break;
            }
            int16_t wMaxGet, wMaxPut, wCurrentChunk;
            uint16_t w, w2;
            uint8_t AppBuffer[32];
            // Figure out how many bytes have been received and how many we can transmit.
            wMaxGet = TCPIP_TCP_GetIsReady(appData.socket); // Get TCP RX FIFO byte count
            wMaxPut = TCPIP_TCP_PutIsReady(appData.socket); // Get TCP TX FIFO free space

            // Make sure we don't take more bytes out of the RX FIFO than we can put into the TX FIFO
            if (wMaxPut < wMaxGet)
                wMaxGet = wMaxPut;

            // Process all bytes that we can
            // This is implemented as a loop, processing up to sizeof(AppBuffer) bytes at a time.
            // This limits memory usage while maximizing performance.  Single byte Gets and Puts are a lot slower than multibyte GetArrays and PutArrays.
            wCurrentChunk = sizeof (AppBuffer);
            for (w = 0; w < wMaxGet; w += sizeof (AppBuffer)) {
                // Make sure the last chunk, which will likely be smaller than sizeof(AppBuffer), is treated correctly.
                if (w + sizeof (AppBuffer) > wMaxGet)
                    wCurrentChunk = wMaxGet - w;

                // Transfer the data out of the TCP RX FIFO and into our local processing buffer.
                wCurrentChunk = TCPIP_TCP_ArrayGet(appData.socket, AppBuffer, wCurrentChunk);
                GetMessage((int8_t*) AppBuffer, &RemoteParamGen, &SaveTodo);

                //                // Perform the "ToUpper" operation on each data byte
                //                for (w2 = 0; w2 < wCurrentChunk; w2++) {
                //                    i = AppBuffer[w2];
                //                    if (i >= 'a' && i <= 'z') {
                //                        i -= ('a' - 'A');
                //                        AppBuffer[w2] = i;
                //                    } else if (i == '\e') //escape
                //                    {
                //                        appRJ45Stat.rj45Stat = false;
                //                        appData.state = APP_TCPIP_CLOSING_CONNECTION;
                //                        SYS_CONSOLE_MESSAGE("Connection was closed\r\n");
                //                    }
                //                }

                if (AppBuffer[0] == '!' && strchr((char*) AppBuffer, '#') != NULL) {
                    SendMessage((int8_t*) AppBuffer, &RemoteParamGen, SaveTodo);
                } else {
                    const char* msg = "mauvaise trame envoy�";
                    strcpy((char*) AppBuffer, msg);
                    wCurrentChunk = strlen(msg); // longueur de la cha�ne � envoyer
                }

                // Transfer the data out of our local processing buffer and into the TCP TX FIFO.
                SYS_CONSOLE_PRINT("Server Sending %s\r\n", AppBuffer);
                TCPIP_TCP_ArrayPut(appData.socket, AppBuffer, wCurrentChunk);

                // No need to perform any flush.  TCP data in TX FIFO will automatically transmit itself after it accumulates for a while.  If you want to decrease latency (at the expense of wasting network bandwidth on TCP overhead), perform and explicit flush via the TCPFlush() API.
            }
        }
            break;
        case APP_TCPIP_CLOSING_CONNECTION:
        {
            appRJ45Stat.rj45Stat = false;
            // Close the socket connection.
            TCPIP_TCP_Close(appData.socket);
            appData.socket = INVALID_SOCKET;
            appData.state = APP_TCPIP_WAIT_FOR_IP;

        }
            break;
        default:
            break;
    }
}




/*******************************************************************************
 End of File
 */

