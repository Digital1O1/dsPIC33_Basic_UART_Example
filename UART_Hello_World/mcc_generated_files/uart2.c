/**
  UART2 Generated Driver File 

  @Company
    Microchip Technology Inc.

  @File Name
    uart2.c

  @Summary
    This is the generated source file for the UART2 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for driver for UART2. 
    Generation Information : 
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  dsPIC33CH512MP508
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB             :  MPLAB X v5.45
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "xc.h"
#include "uart2.h"

/**
  Section: Data Type Definitions
*/

/** UART Driver Queue Status

  @Summary
    Defines the object required for the status of the queue.
*/

static uint8_t * volatile rxTail;
static uint8_t *rxHead;
static uint8_t *txTail;
static uint8_t * volatile txHead;
static bool volatile rxOverflowed;

/** UART Driver Queue Length

  @Summary
    Defines the length of the Transmit and Receive Buffers

*/

/* We add one extra byte than requested so that we don't have to have a separate
 * bit to determine the difference between buffer full and buffer empty, but
 * still be able to hold the amount of data requested by the user.  Empty is
 * when head == tail.  So full will result in head/tail being off by one due to
 * the extra byte.
 */
#define UART2_CONFIG_TX_BYTEQ_LENGTH (8+1)
#define UART2_CONFIG_RX_BYTEQ_LENGTH (8+1)

/** UART Driver Queue

  @Summary
    Defines the Transmit and Receive Buffers

*/

static uint8_t txQueue[UART2_CONFIG_TX_BYTEQ_LENGTH];
static uint8_t rxQueue[UART2_CONFIG_RX_BYTEQ_LENGTH];

void (*UART2_TxDefaultInterruptHandler)(void);
void (*UART2_RxDefaultInterruptHandler)(void);

/**
  Section: Driver Interface
*/

void UART2_Initialize(void)
{
    IEC1bits.U2TXIE = 0;
    IEC1bits.U2RXIE = 0;

    // URXEN disabled; RXBIMD RXBKIF flag when Break makes low-to-high transition after being low for at least 23/11 bit periods; UARTEN enabled; MOD Asynchronous 8-bit UART; UTXBRK disabled; BRKOVR TX line driven by shifter; UTXEN disabled; USIDL disabled; WAKE disabled; ABAUD disabled; BRGH enabled; 
    // Data Bits = 8; Parity = None; Stop Bits = 1 Stop bit sent, 1 checked at RX;
    U2MODE = (0x8080 & ~(1<<15));  // disabling UART ON bit
    // STSEL 1 Stop bit sent, 1 checked at RX; BCLKMOD disabled; SLPEN disabled; FLO Off; BCLKSEL FOSC/2; C0EN disabled; RUNOVF disabled; UTXINV disabled; URXINV disabled; HALFDPLX disabled; 
    U2MODEH = 0x00;
    // OERIE disabled; RXBKIF disabled; RXBKIE disabled; ABDOVF disabled; OERR disabled; TXCIE disabled; TXCIF disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; CERIF disabled; PERIE disabled; 
    U2STA = 0x00;
    // URXISEL RX_ONE_WORD; UTXBE enabled; UTXISEL TX_BUF_EMPTY; URXBE enabled; STPMD disabled; TXWRE disabled; 
    U2STAH = 0x22;
    // BaudRate = 9600; Frequency = 4000000 Hz; BRG 103; 
    U2BRG = 0x67;
    // BRG 0; 
    U2BRGH = 0x00;
    // P1 0; 
    U2P1 = 0x00;
    // P2 0; 
    U2P2 = 0x00;
    // P3 0; 
    U2P3 = 0x00;
    // P3H 0; 
    U2P3H = 0x00;
    // TXCHK 0; 
    U2TXCHK = 0x00;
    // RXCHK 0; 
    U2RXCHK = 0x00;
    // T0PD 1 ETU; PTRCL disabled; TXRPT Retransmit the error byte once; CONV Direct logic; 
    U2SCCON = 0x00;
    // TXRPTIF disabled; TXRPTIE disabled; WTCIF disabled; WTCIE disabled; BTCIE disabled; BTCIF disabled; GTCIF disabled; GTCIE disabled; RXRPTIE disabled; RXRPTIF disabled; 
    U2SCINT = 0x00;
    // ABDIF disabled; WUIF disabled; ABDIE disabled; 
    U2INT = 0x00;
    
    txHead = txQueue;
    txTail = txQueue;
    rxHead = rxQueue;
    rxTail = rxQueue;
   
    rxOverflowed = false;

    UART2_SetTxInterruptHandler(&UART2_Transmit_CallBack);

    UART2_SetRxInterruptHandler(&UART2_Receive_CallBack);

    IEC1bits.U2RXIE = 1;
    
    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    U2MODEbits.UARTEN = 1;   // enabling UART ON bit
    U2MODEbits.UTXEN = 1;
    U2MODEbits.URXEN = 1;
}

/**
    Maintains the driver's transmitter state machine and implements its ISR
*/

void UART2_SetTxInterruptHandler(void (* interruptHandler)(void))
{
    if(interruptHandler == NULL)
    {
        UART2_TxDefaultInterruptHandler = &UART2_Transmit_CallBack;
    }
    else
    {
        UART2_TxDefaultInterruptHandler = interruptHandler;
    }
} 


void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2TXInterrupt ( void )
{
    if(UART2_TxDefaultInterruptHandler)
    {
        UART2_TxDefaultInterruptHandler();
    }
    
    if(txHead == txTail)
    {
        IEC1bits.U2TXIE = 0;
    }
    else
    {
        IFS1bits.U2TXIF = 0;

        while(!(U2STAHbits.UTXBF == 1))
        {
            U2TXREG = *txHead++;

            if(txHead == (txQueue + UART2_CONFIG_TX_BYTEQ_LENGTH))
            {
                txHead = txQueue;
            }

            // Are we empty?
            if(txHead == txTail)
            {
                break;
            }
        }
    }
}

void __attribute__ ((weak)) UART2_Transmit_CallBack ( void )
{ 

}

void UART2_SetRxInterruptHandler(void (* interruptHandler)(void))
{
    if(interruptHandler == NULL)
    {
        UART2_RxDefaultInterruptHandler = &UART2_Receive_CallBack;
    }
    else
    {
        UART2_RxDefaultInterruptHandler = interruptHandler;
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2RXInterrupt( void )
{
    if(UART2_RxDefaultInterruptHandler)
    {
        UART2_RxDefaultInterruptHandler();
    }
    
    IFS1bits.U2RXIF = 0;

    while(!(U2STAHbits.URXBE == 1))
    {
        *rxTail = U2RXREG;

        // Will the increment not result in a wrap and not result in a pure collision?
        // This is most often condition so check first
        if ( ( rxTail    != (rxQueue + UART2_CONFIG_RX_BYTEQ_LENGTH-1)) &&
             ((rxTail+1) != rxHead) )
        {
            rxTail++;
        } 
        else if ( (rxTail == (rxQueue + UART2_CONFIG_RX_BYTEQ_LENGTH-1)) &&
                  (rxHead !=  rxQueue) )
        {
            // Pure wrap no collision
            rxTail = rxQueue;
        } 
        else // must be collision
        {
            rxOverflowed = true;
        }
    }
}

void __attribute__ ((weak)) UART2_Receive_CallBack(void)
{

}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2EInterrupt ( void )
{
    if ((U2STAbits.OERR == 1))
    {
        U2STAbits.OERR = 0;
    }
    
    IFS3bits.U2EIF = 0;
}

/* ISR for UART Event Interrupt */

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U2EVTInterrupt ( void )
{
    /* Add handling for UART events here */

    IFS11bits.U2EVTIF = false;
}

/**
  Section: UART Driver Client Routines
*/

uint8_t UART2_Read( void)
{
    uint8_t data = 0;

    while (rxHead == rxTail )
    {
    }
    
    data = *rxHead;

    rxHead++;

    if (rxHead == (rxQueue + UART2_CONFIG_RX_BYTEQ_LENGTH))
    {
        rxHead = rxQueue;
    }
    return data;
}

void UART2_Write( uint8_t byte)
{
    while(UART2_IsTxReady() == 0)
    {
    }

    *txTail = byte;

    txTail++;
    
    if (txTail == (txQueue + UART2_CONFIG_TX_BYTEQ_LENGTH))
    {
        txTail = txQueue;
    }

    IEC1bits.U2TXIE = 1;
}

bool UART2_IsRxReady(void)
{    
    return !(rxHead == rxTail);
}

bool UART2_IsTxReady(void)
{
    uint16_t size;
    uint8_t *snapshot_txHead = (uint8_t*)txHead;
    
    if (txTail < snapshot_txHead)
    {
        size = (snapshot_txHead - txTail - 1);
    }
    else
    {
        size = ( UART2_CONFIG_TX_BYTEQ_LENGTH - (txTail - snapshot_txHead) - 1 );
    }
    
    return (size != 0);
}

bool UART2_IsTxDone(void)
{
    if(txTail == txHead)
    {
        return (bool)U2STAbits.TRMT;
    }
    
    return false;
}


/*******************************************************************************

  !!! Deprecated API !!!
  !!! These functions will not be supported in future releases !!!

*******************************************************************************/

static uint8_t UART2_RxDataAvailable(void)
{
    uint16_t size;
    uint8_t *snapshot_rxTail = (uint8_t*)rxTail;
    
    if (snapshot_rxTail < rxHead) 
    {
        size = ( UART2_CONFIG_RX_BYTEQ_LENGTH - (rxHead-snapshot_rxTail));
    }
    else
    {
        size = ( (snapshot_rxTail - rxHead));
    }
    
    if(size > 0xFF)
    {
        return 0xFF;
    }
    
    return size;
}

static uint8_t UART2_TxDataAvailable(void)
{
    uint16_t size;
    uint8_t *snapshot_txHead = (uint8_t*)txHead;
    
    if (txTail < snapshot_txHead)
    {
        size = (snapshot_txHead - txTail - 1);
    }
    else
    {
        size = ( UART2_CONFIG_TX_BYTEQ_LENGTH - (txTail - snapshot_txHead) - 1 );
    }
    
    if(size > 0xFF)
    {
        return 0xFF;
    }
    
    return size;
}

unsigned int __attribute__((deprecated)) UART2_ReadBuffer( uint8_t *buffer ,  unsigned int numbytes)
{
    unsigned int rx_count = UART2_RxDataAvailable();
    unsigned int i;
    
    if(numbytes < rx_count)
    {
        rx_count = numbytes;
    }
    
    for(i=0; i<rx_count; i++)
    {
        *buffer++ = UART2_Read();
    }
    
    return rx_count;    
}

unsigned int __attribute__((deprecated)) UART2_WriteBuffer( uint8_t *buffer , unsigned int numbytes )
{
    unsigned int tx_count = UART2_TxDataAvailable();
    unsigned int i;
    
    if(numbytes < tx_count)
    {
        tx_count = numbytes;
    }
    
    for(i=0; i<tx_count; i++)
    {
        UART2_Write(*buffer++);
    }
    
    return tx_count;  
}

UART2_TRANSFER_STATUS __attribute__((deprecated)) UART2_TransferStatusGet (void )
{
    UART2_TRANSFER_STATUS status = 0;
    uint8_t rx_count = UART2_RxDataAvailable();
    uint8_t tx_count = UART2_TxDataAvailable();
    
    switch(rx_count)
    {
        case 0:
            status |= UART2_TRANSFER_STATUS_RX_EMPTY;
            break;
        case UART2_CONFIG_RX_BYTEQ_LENGTH:
            status |= UART2_TRANSFER_STATUS_RX_FULL;
            break;
        default:
            status |= UART2_TRANSFER_STATUS_RX_DATA_PRESENT;
            break;
    }
    
    switch(tx_count)
    {
        case 0:
            status |= UART2_TRANSFER_STATUS_TX_FULL;
            break;
        case UART2_CONFIG_RX_BYTEQ_LENGTH:
            status |= UART2_TRANSFER_STATUS_TX_EMPTY;
            break;
        default:
            break;
    }

    return status;    
}

uint8_t __attribute__((deprecated)) UART2_Peek(uint16_t offset)
{
    uint8_t *peek = rxHead + offset;
    
    while(peek > (rxQueue + UART2_CONFIG_RX_BYTEQ_LENGTH))
    {
        peek -= UART2_CONFIG_RX_BYTEQ_LENGTH;
    }
    
    return *peek;
}

bool __attribute__((deprecated)) UART2_ReceiveBufferIsEmpty (void)
{
    return (UART2_RxDataAvailable() == 0);
}

bool __attribute__((deprecated)) UART2_TransmitBufferIsFull(void)
{
    return (UART2_TxDataAvailable() == 0);
}

uint32_t __attribute__((deprecated)) UART2_StatusGet (void)
{
    uint32_t statusReg = U2STAH;
    return ((statusReg << 16 ) | U2STA);
}

unsigned int __attribute__((deprecated)) UART2_TransmitBufferSizeGet(void)
{
    if(UART2_TxDataAvailable() != 0)
    { 
        if(txHead > txTail)
        {
            return((txHead - txTail) - 1);
        }
        else
        {
            return((UART2_CONFIG_TX_BYTEQ_LENGTH - (txTail - txHead)) - 1);
        }
    }
    return 0;
}

unsigned int __attribute__((deprecated)) UART2_ReceiveBufferSizeGet(void)
{
    if(UART2_RxDataAvailable() != 0)
    {
        if(rxHead > rxTail)
        {
            return((rxHead - rxTail) - 1);
        }
        else
        {
            return((UART2_CONFIG_RX_BYTEQ_LENGTH - (rxTail - rxHead)) - 1);
        } 
    }
    return 0;
}

void __attribute__((deprecated)) UART2_Enable(void)
{
    U2MODEbits.UARTEN = 1;
    U2MODEbits.UTXEN = 1; 
    U2MODEbits.URXEN = 1;
}

void __attribute__((deprecated)) UART2_Disable(void)
{
    U2MODEbits.UARTEN = 0;
    U2MODEbits.UTXEN = 0; 
    U2MODEbits.URXEN = 0;
}
