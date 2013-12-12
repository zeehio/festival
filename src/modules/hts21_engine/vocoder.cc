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
/*   vocoder.c : mel-cepstral vocoder                                */
/*              (pulse/noise excitation & MLSA filter)               */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "EST_walloc.h"

#include "misc.h"
#include "model.h"
#include "defaults.h"
#include "global.h"
#include "vocoder.h"

void init_vocoder(int m, VocoderSetup *vs)
{
   vs->fprd = FPERIOD;
   vs->iprd = IPERIOD;
   vs->seed = SEED;
   vs->pd   = PADEORDER;

   vs->next = SEED;
   vs->gauss = GAUSS;

   vs->pade[ 0]=1.0;
   vs->pade[ 1]=1.0; vs->pade[ 2]=0.0;
   vs->pade[ 3]=1.0; vs->pade[ 4]=0.0;      vs->pade[ 5]=0.0;
   vs->pade[ 6]=1.0; vs->pade[ 7]=0.0;      vs->pade[ 8]=0.0;      vs->pade[ 9]=0.0;
   vs->pade[10]=1.0; vs->pade[11]=0.4999273; vs->pade[12]=0.1067005; vs->pade[13]=0.01170221; vs->pade[14]=0.0005656279;
   vs->pade[15]=1.0; vs->pade[16]=0.4999391; vs->pade[17]=0.1107098; vs->pade[18]=0.01369984; vs->pade[19]=0.0009564853;
   vs->pade[20]=0.00003041721;

   vs->rate=RATE;
                  
   vs->c = walloc(double,3*(m+1)+3*(vs->pd+1)+vs->pd*(m+2));
   
   vs->p1 = -1;
   vs->sw = 0;
   vs->x  = 0x55555555;
}

void vocoder (double p, float *mc, int m, FILE *rawfp, globalP *gp, VocoderSetup *vs)
{
   double inc, x;
   int i, j, k; 
   short xs;
   double a = gp->ALPHA;
   
   if (p!=0.0) 
      p = vs->rate / p;  /* f0 -> pitch */
   
   if (vs->p1 < 0) {
      if (vs->gauss & (vs->seed != 1)) vs->next = srnd ((unsigned)vs->seed);

      vs->p1   = p;
      vs->pc   = vs->p1;
      vs->cc   = vs->c + m + 1;
      vs->cinc = vs->cc + m + 1;
      vs->d1   = vs->cinc + m + 1;
      
      mc2b(mc, vs->c, m, a);
      
      return;
   }

   mc2b(mc, vs->cc, m, a); 
   
   for (k=0; k<=m; k++)
      vs->cinc[k] = (vs->cc[k]-vs->c[k])*(double)vs->iprd/(double)vs->fprd;

   if (vs->p1!=0.0 && p!=0.0) {
      inc = (p-vs->p1)*(double)vs->iprd/(double)vs->fprd;
   }
   else {
      inc = 0.0;
      vs->pc = p;
      vs->p1 = 0.0;
   }

   for (j=vs->fprd, i=(vs->iprd+1)/2; j--;) {
      if (vs->p1 == 0.0) {
          if (vs->gauss)
             x = (double) nrandom(vs);
          else
             x = mseq(vs);
      }
      else {
          if ((vs->pc += 1.0)>=vs->p1) {
             x = sqrt (vs->p1);
             vs->pc = vs->pc - vs->p1;
          }
          else
              x = 0.0;
      }

      x *= exp(vs->c[0]);

      x = mlsadf(x, vs->c, m, a, vs->pd, vs->d1, vs);
      xs = (short) x;

      fwrite(&xs, sizeof(short), 1, rawfp);

      fflush(stdout);

      if (!--i) {
         vs->p1 += inc;
         for (k=0;k<=m;k++) vs->c[k] += vs->cinc[k];
         i = vs->iprd;
      }
   }
   
   vs->p1 = p;
   movem(vs->cc,vs->c,m+1);
}

double mlsafir (double x, double *b, int m, double a, double *d)
{
   double y = 0.0;
   double aa;
   register int i;
	
   aa = 1 - a*a;

   d[0] = x;
   d[1] = aa*d[0] + a*d[1];
	
   for (i=2; i<=m; i++) {
      d[i] = d[i] + a*(d[i+1]-d[i-1]);
      y += d[i]*b[i];
   }
	
   for (i=m+1; i>1; i--) d[i] = d[i-1];
	
   return (y);
}

double mlsadf1(double x, double *b, int m, double a, int pd, double *d, VocoderSetup *vs)
{
   double v, out = 0.0, *pt, aa;
   register int i;

   aa = 1 - a*a;
   pt = &d[pd+1];

   for (i=pd; i>=1; i--) {
      d[i] = aa*pt[i-1] + a*d[i];
      pt[i] = d[i] * b[1];
      v = pt[i] * vs->ppade[i];
		
      x += (1 & i) ? v : -v;
      out += v;
   }
	
   pt[0] = x;
   out += x;
	
   return(out);
}

double mlsadf2 (double x, double *b, int m, double a, int pd, double *d, VocoderSetup *vs)
{
   double v, out = 0.0, *pt, aa;
   register int i;
    
   aa = 1 - a*a;
   pt = &d[pd * (m+2)];
	
   for (i=pd; i>=1; i--) {
      pt[i] = mlsafir (pt[i-1], b, m, a, &d[(i-1)*(m+2)]);
      v = pt[i] * vs->ppade[i];

      x  += (1&i) ? v : -v;
      out += v;
   }
    
   pt[0] = x;
   out  += x;
	
   return(out);
}

double mlsadf(double x, double *b, int m, double a, int pd, double *d, VocoderSetup *vs)
{

   vs->ppade = &(vs->pade[pd*(pd+1)/2]);
    
   x = mlsadf1 (x, b, m, a, pd, d, vs);
   x = mlsadf2 (x, b, m, a, pd, &d[2*(pd+1)], vs);

   return (x);
}

double nrandom (VocoderSetup *vs)
{
    unsigned long rr;
   if (vs->sw == 0) {
      vs->sw = 1;
      do {
	  rr = vs->next;
         vs->r1 = 2 * rnd(&rr) - 1;
         vs->r2 = 2 * rnd(&rr) - 1;
	 vs->next = rr;
         vs->s  = vs->r1 * vs->r1 + vs->r2 * vs->r2;
      } while (vs->s > 1 || vs->s == 0);

      vs->s = sqrt (-2 * log(vs->s) / vs->s);
      return ( vs->r1 * vs->s );
   }
   else {
      vs->sw = 0;
      return ( vs->r2 * vs->s );
   }
}

double rnd (unsigned long *next)
{
   double r;

   *next = *next * 1103515245L + 12345;
   r = (*next / 65536L) % 32768L;

   return ( r/RANDMAX ); 
}

unsigned long srnd ( unsigned long seed )
{
   return (seed);
}


int mseq (VocoderSetup *vs)
{
   register int x0, x28;

   vs->x >>= 1;

   if (vs->x & B0)
      x0 = 1;
   else
      x0 = -1;

   if (vs->x & B28)
      x28 = 1;
   else
      x28 = -1;

   if (x0 + x28)
      vs->x &= B31_;
   else
      vs->x |= B31;

   return (x0);
}

void mc2b( float *mc, double *b, int m, double a)
{
   b[m] = mc[m];
    
   for (m--; m>=0; m--)
      b[m] = mc[m] - a * b[m+1];
}

/* -------------------- End of "vocoder.c" -------------------- */ 
