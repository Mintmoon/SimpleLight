/*
 * aPLib compression library  -  the smaller the better :)
 *
 * C depacker
 *
 * Copyright (c) 1998-2003 by Joergen Ibsen / Jibz
 * All Rights Reserved
 */

#include "aplib.h"

/* global variables used */
unsigned char *aP_d_output;
unsigned char *aP_d_input;
unsigned char aP_d_tagbyte;
unsigned long aP_d_tagpos;

/* input functions */
unsigned long aP_d_getbit()
{
   unsigned long tmp;

   if (!aP_d_tagpos--)
   {
      aP_d_tagpos = 7;
      aP_d_tagbyte = *aP_d_input;
      aP_d_input++;
   }

   tmp = (aP_d_tagbyte >> 7) & 0x01;
   aP_d_tagbyte <<= 1;

   return (tmp);
}

unsigned long aP_d_getgamma()
{
   unsigned long result = 1;

   do {
      result = (result << 1) + aP_d_getbit();
   } while (aP_d_getbit());

   return (result);
}

/* the main depacking function */
unsigned long aP_depack(unsigned char *source, unsigned char *destination)
{
   unsigned long offs, len, R0, LWM;
   signed long done;
   signed long i;

   aP_d_input = source;
   aP_d_output = destination;

   LWM = 0;
   aP_d_tagpos = 0;

   *aP_d_output = *aP_d_input;
   aP_d_output++;
   aP_d_input++;

   done = 0;

   while (!done)
   {
      if (aP_d_getbit())
      {
         if (aP_d_getbit())
         {
            if (aP_d_getbit())
            {
               offs = 0;
               for (i = 4; i; i--) offs = (offs << 1) + aP_d_getbit();

               if (offs)
               {
                  *aP_d_output = *(aP_d_output - offs);
                  aP_d_output++;
               } else {
                  *aP_d_output = 0x00;
                  aP_d_output++;
               }
               LWM = 0;
            } else {
               offs = *aP_d_input;
               aP_d_input++;

               len = 2 + (offs & 0x0001);
               offs >>= 1;

               if (offs)
               {
                  for (; len; len--)
                  {
                     *aP_d_output = *(aP_d_output - offs);
                     aP_d_output++;
                  }
               } else done = 1;
               R0 = offs;
               LWM = 1;
            }
         } else {
            offs = aP_d_getgamma();

            if ((LWM == 0) && (offs == 2))
            {
               offs = R0;

               len = aP_d_getgamma();

               for (; len; len--)
               {
                  *aP_d_output = *(aP_d_output - offs);
                  aP_d_output++;
               }
            } else {
               if (LWM == 0) offs -= 3; else offs -= 2;
               offs <<= 8;
               offs += *aP_d_input;
               aP_d_input++;

               len = aP_d_getgamma();

               if (offs >= 32000) len++;
               if (offs >= 1280) len++;
               if (offs < 128) len += 2;

               for (; len; len--)
               {
                  *aP_d_output = *(aP_d_output - offs);
                  aP_d_output++;
               }
               R0 = offs;
            }
            LWM = 1;
         }
      } else {
         *aP_d_output = *aP_d_input;
         aP_d_output++;
         aP_d_input++;
         LWM = 0;
      }
   }

   return (aP_d_output - destination);
}
