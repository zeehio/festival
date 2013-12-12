/*  ---------------------------------------------------------------  */
/*      The HMM-Based Speech Synthesis System (HTS): version 1.1b    */
/*                        HTS Working Group                          */
/*                                                                   */
/*                   Department of Computer Science                  */
/*                   Nagoya Institute of Technology                  */
/*                                and                                */
/*    Interdisciplinary Graduate School of Science and Engineering   */
/*                   Tokyo Institute of Technology                   */
/*                      Copyright (c) 2001-2003                      */
/*                        All Rights Reserved.                       */
/*                                                                   */
/*  Permission is hereby granted, free of charge, to use and         */
/*  distribute this software and its documentation without           */
/*  restriction, including without limitation the rights to use,     */
/*  copy, modify, merge, publish, distribute, sublicense, and/or     */
/*  sell copies of this work, and to permit persons to whom this     */
/*  work is furnished to do so, subject to the following conditions: */
/*                                                                   */
/*    1. The code must retain the above copyright notice, this list  */
/*       of conditions and the following disclaimer.                 */
/*                                                                   */
/*    2. Any modifications must be clearly marked as such.           */
/*                                                                   */    
/*  NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF TECHNOLOGY,  */
/*  HTS WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM    */
/*  ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL       */
/*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF        */
/*  TECHNOLOGY, HTS WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE    */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY        */
/*  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  */
/*  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS   */
/*  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR          */
/*  PERFORMANCE OF THIS SOFTWARE.                                    */
/*                                                                   */
/*  ---------------------------------------------------------------  */
/*    misc.c : miscellaneous functions (from SPTK)                   */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "festival.h"
#include "misc.h"

FILE *getfp (char *name, char *opt)
{
   FILE *fp;
   
   if ((fp=fopen(name, opt)) == NULL) {
      fprintf (stderr, "Can't open '%s'!\n", name);
      festival_error();
   }
   return (fp);
}

void GetToken (FILE *fp, char *buff)
{
   char c;
   int i;
   HTS_Boolean squote = 0;
   HTS_Boolean dquote = 0;

   c = fgetc (fp);

   while (isspace(c))
      c = fgetc (fp);
      
   if (c=='\'') {  /* single quote case */
      c = fgetc (fp);
      squote = 1;
   }
   
   if (c=='\"') {  /*double quote case */
      c = fgetc (fp);
      dquote = 1;
   }
   
   if (c==',') {   /*special character ',' */
      strcpy (buff, ",");
      return; 
   }
   
   i = 0;
   while (1) {
      buff[i++] = c;
      c = fgetc (fp);
      if (squote && c == '\'') break;
      if (dquote && c == '\"') break;
      if (!(squote || dquote || isgraph(c)) ) break;
   }
   
   buff[i]=0;
}

void movem (double *a, double *b, int nitem)
{
   register long i;
  
   i = nitem;
  
   if (a>b)
      while (i--) *b++ = *a++;
   else {
      a += i; b += i;
      while (i--) *--b = *--a;
   }
}

/* -------------------- End of "misc.c" -------------------- */
