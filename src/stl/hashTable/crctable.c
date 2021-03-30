/* vi: set ts=3 sw=3 ai autowrite : */

/***************************************************************************/
/*                                                                         */
/*    crctable.c                                                           */
/*                                                                         */
/*          Generate a CRC lookup table as described in IEEE               */
/*          standard 1003.2-1992, section E.4.9, page 880.                 */
/*          Copyright (c) 1995-1996 by Hamilton Laboratories               */
/*             Unpublished work.  All rights reserved.                     */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#include <stdio.h>

typedef unsigned long ulong;

/*
   Ethernet CRC
   Polynomial   x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 +
                       x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + 1

   Represent the polynomial as an unsigned number with each bit corresponding
   to a term of the polynomial set to one.  E.g., the x^5 term means bit 5
   (counting from the LSB as bit 0) is on.  The x^32 term is actually the
   feedback term and is "off the end" of the polynomial.
*/

#define  Ethernet_Polynomial     0x04c11db7
#define  Poly                    Ethernet_Polynomial


/* Calculate the CRC table entries.  crc(x) returns table entry x.  Use this
   routine to fill a table crctab[256] with the appropriate values.
*/

ulong crc( register ulong i )
      {
      i <<= 24;

      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);
      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);
      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);
      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);
      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);
      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);
      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);
      i = (i << 1) ^ ((i & 0x80000000) ? Poly : 0);

      return i;
      }



void main( void )
   {
   register int i;

   printf(  "/*   crctable.h:    CRC lookup table per "
                                 "IEEE 1003.2-1992 E.4.9 */\n"
            "\n"
            "/*   Warning!  Do not modify this file directly.\n"
            "  This file has been built using ..\\util\\crctable.c */\n"
            "\n\n"
            "static unsigned long crctab[] =\n"
            "\t\t{");

   printf(  "\t0x%08x, 0x%08x, 0x%08x, 0x%08x,\t\n",
      crc(0), crc(1), crc(2), crc(3));

   for (i = 4;  i < 252;  i += 4)
      printf("\t\t\t0x%08x, 0x%08x, 0x%08x, 0x%08x,\n",
         crc(i), crc(i+1), crc(i+2), crc(i+3));

   printf(  "\t\t\t0x%08x, 0x%08x, 0x%08x, 0x%08x\t};\n\n\n",
      crc(252), crc(253), crc(254), crc(255));

   printf(  "/*   To calculate a CRC, use the UpdateCRC(s, c) macro to\n"
            "  accumulatethe data into the CRC.  The s argument is the\n"
            "  accumulated CRC, c is the next character of data to added.\n"
            "  Initially, s = 0.\n"
            "*/\n");

   printf(  "#define UpdateCRC(s, c)      "
               "(((s) << 8) ^ crctab[((s) >> 24) ^ (c)])\n");
   }
