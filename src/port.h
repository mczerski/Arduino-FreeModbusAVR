/*
 * FreeModbus Libary: AVR Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *   - Initial version + ATmega168 support
 * Modfications Copyright (C) 2006 Tran Minh Hoang:
 *   - ATmega8, ATmega16, ATmega32 support
 *   - RS485 support for DS75176
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: port.h,v 1.6 2006/09/17 16:45:52 wolti Exp $
 */

#ifndef _PORT_H
#define _PORT_H

/* ----------------------- Platform includes --------------------------------*/

#include <avr/interrupt.h>

/* ----------------------- Defines ------------------------------------------*/
#define	INLINE                      inline
#define PR_BEGIN_EXTERN_C           extern "C" {
#define	PR_END_EXTERN_C             }

#define ENTER_CRITICAL_SECTION( )   cli()
#define EXIT_CRITICAL_SECTION( )    sei()

#define assert( x )

typedef char    BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef unsigned short USHORT;
typedef short   SHORT;

typedef unsigned long ULONG;
typedef long    LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#define RTS_ENABLE

/* ----------------------- RS485 specifics ----------------------------------*/
#ifdef  RTS_ENABLE

#define RTS_PIN         PB1
#define RTS_DDR         DDRB
#define RTS_PORT        PORTB

#define RTS_INIT        \
    do { \
        RTS_DDR |= _BV( RTS_PIN ); \
        RTS_PORT &= ~( _BV( RTS_PIN ) ); \
    } while( 0 );

#define RTS_HIGH        \
    do { \
        RTS_PORT |= _BV( RTS_PIN ); \
    } while( 0 );

#define RTS_LOW         \
    do { \
        RTS_PORT &= ~( _BV( RTS_PIN ) ); \
    } while( 0 );

#endif

// Make sure the ATmega328PB is backwards compatible with the 328 and 328P
#if defined(__AVR_ATmega328PB__)
// SPI
  #define SPCR SPCR0
  #define SPSR SPSR0
  #define SPDR SPDR0
  #define SPI_STC_vect SPI0_STC_vect
  #define SPI_STC_vect_num SPI0_STC_vect_num
// I2C
  #define TWBR TWBR0
  #define TWSR TWSR0
  #define TWAR TWAR0
  #define TWDR TWDR0
  #define TWCR TWCR0
  #define TWAMR TWAMR0
  #define TWI_vect TWI0_vect
  #define TWI_vect_num TWI0_vect_num
// UART
  #define USART_RX_vect USART0_RX_vect
  #define USART_RX_vect_num USART0_RX_vect_num
  #define USART_UDRE_vect USART0_UDRE_vect
  #define USART_UDRE_vect_num USART0_UDRE_vect_num
  #define USART_TX_vect USART0_TX_vect
  #define USART_TX_vect_num USART0_TX_vect_num
#endif // 328PB defs

#endif
