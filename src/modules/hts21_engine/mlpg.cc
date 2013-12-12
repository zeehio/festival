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
/*    mlpg.c : speech parameter generation from pdf sequence         */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include "festival.h"

#include "defaults.h"
#include "misc.h"
#include "model.h"
#include "global.h"
#include "vocoder.h"
#include "mlpg.h"

double finv (double x)
{
   if (x >= INFTY2) return 0.0;
   if (x <= -INFTY2) return 0.0;
   if (x <= INVINF2 && x >= 0) return INFTY;
   if (x >= -INVINF2 && x < 0) return -INFTY;
   
   return 1.0/x;
}

double *dcalloc(int x)
{
    return walloc(double,x);
}

double **ddcalloc(int x, int y)
{
   register int i;
   double **ptr;

   ptr = walloc(double *,x);
	
   for (i=0; i<x; i++)
      ptr[i] = dcalloc(y);
   
   return(ptr);
}

float *fcalloc(int x)
{
   return walloc(float,x);
}

float **ffcalloc(int x, int y)
{
   register int i;
   float **ptr;
   
   ptr = walloc(float *,x);
	
   for (i=0; i<x; i++)
      ptr[i] = fcalloc(y);
   
   return(ptr);
}

int str2farray (char *c, float **x)
{
   int i, size, sp;
   char *p, *buf;
   
   while (isspace(*c))
      c++;
   
   if (*c == '\0') {
      *x = NULL;
      return (0);
   }

   size = 1;
   sp = 0;
  
   for (p = c; *p != '\0'; p++) {
      if (!isspace (*p)) {
         if (sp == 1) {
            size++;
            sp = 0;
         }
      }
      else
         sp = 1;
   }

   buf = walloc(char,strlen(c));

   *x = walloc(float,size);

  for (i=0; i<size; i++)
     (*x)[i] = (float)strtod (c, &c);

  return (size);
}

/*----------------------------------------------------------------
	matrix calcuration functions
----------------------------------------------------------------*/

/* calc_R_and_r : calcurate R=W'U^{-1}W and r=W'U^{-1}M */
void calc_R_and_r(PStream *pst, int m)
{
   register int i, j, k, l, n;
   double   wu;
   
   for (i=0; i<pst->T; i++) {
		pst->sm.r[i]    = pst->sm.ivseq[i][m] * pst->sm.mseq[i][m];
		pst->sm.R[i][0] = pst->sm.ivseq[i][m];
      
		for (j=1; j<pst->width; j++)
         pst->sm.R[i][j]=0.0;
      
      for (j=1; j<pst->dw.num; j++)
         for (k=pst->dw.width[j][0]; k<=pst->dw.width[j][1]; k++) {
            n = i+k;
            if ( (n>=0) && (n<pst->T) && (pst->dw.coef[j][-k]!=0.0) ) {
               l = j*(pst->order+1)+m;
               wu = pst->dw.coef[j][-k] * pst->sm.ivseq[n][l];
               pst->sm.r[i] += wu*pst->sm.mseq[n][l]; 
            
               for (l=0; l<pst->width; l++) {
                  n = l-k;
                  if ( (n<=pst->dw.width[j][1]) && (i+l<pst->T) && (pst->dw.coef[j][n] != 0.0) )
                     pst->sm.R[i][l] += wu * pst->dw.coef[j][n];
               }
            }
         }
   }
}

/* Cholesky : Cholesky factorization of Matrix R */
void Cholesky(PStream *pst)
{
   register int i, j, k;
	
   pst->sm.R[0][0] = sqrt(pst->sm.R[0][0]);

   for (i=1; i<pst->width; i++)
      pst->sm.R[0][i] /= pst->sm.R[0][0];

	for (i=1; i<pst->T; i++) {
      for (j=1; j<pst->width; j++)
         if (i-j >= 0)
            pst->sm.R[i][0] -= pst->sm.R[i-j][j] * pst->sm.R[i-j][j];
         
      pst->sm.R[i][0] = sqrt(pst->sm.R[i][0]);
         
      for (j=1; j<pst->width; j++) {
			for (k=0; k<pst->dw.max_L; k++)
            if (j!=pst->width-1)
               pst->sm.R[i][j] -= pst->sm.R[i-k-1][j-k]*pst->sm.R[i-k-1][j+1];
            
         pst->sm.R[i][j] /= pst->sm.R[i][0];
      }
   }
}

/* Cholesky_forward : forward substitution to solve linear equations */
void Cholesky_forward(PStream *pst)
{
   register int i, j;
   double hold;
   
   pst->sm.g[0] = pst->sm.r[0] / pst->sm.R[0][0];

   for (i=1; i<pst->T; i++) {
      hold = 0.0;
      for (j=1; j<pst->width; j++) {
         if (i-j >= 0)
            hold += pst->sm.R[i-j][j]*pst->sm.g[i-j];
      }
      pst->sm.g[i] = (pst->sm.r[i]-hold)/pst->sm.R[i][0];
   }
}

/* Cholesky_backward : backward substitution to solve linear equations */
void Cholesky_backward(PStream *pst, int m)
{
   register int i, j;
   double hold;
   
   pst->par[pst->T-1][m] = pst->sm.g[pst->T-1] / pst->sm.R[pst->T-1][0];

   for (i=pst->T-2; i>=0; i--) {
      hold = 0.0;
      for (j=1; j<pst->width; j++) {
         if (pst->sm.R[i][j] != 0.0)
            hold += pst->sm.R[i][j]*pst->par[i+j][m];
      }
		pst->par[i][m] = (float)((pst->sm.g[i] - hold) / pst->sm.R[i][0]);
   }
}

/* generate parameter sequence from pdf sequence */
void mlpg(PStream *pst)
{	
   int m;

   for (m=0; m<=pst->order; m++) {
      calc_R_and_r(pst,m);
      Cholesky(pst);
      Cholesky_forward(pst);
      Cholesky_backward(pst,m);
   }
}


/* InitPStream : Initialise PStream for parameter generation */
void InitPStream(PStream *pst)
{
   pst->width	   = pst->dw.max_L*2+1;  /* band width of R */

   pst->sm.mseq  = ddcalloc(pst->T, pst->vSize);
   pst->sm.ivseq = ddcalloc(pst->T, pst->vSize);
   pst->sm.g     = dcalloc (pst->T);
   pst->sm.R     = ddcalloc(pst->T, pst->width);
   pst->sm.r     = dcalloc (pst->T);
   pst->par      = ffcalloc(pst->T,pst->order+1);
}

/* FreePStream : Free PStream */
void FreePStream(PStream *pst)
{
   register int t;
   
   for (t=0; t<pst->T; t++) {
      wfree(pst->sm.mseq[t]);
      wfree(pst->sm.ivseq[t]);
      wfree(pst->sm.R[t]);
      wfree(pst->par[t]);
   }

   for (t=0; t<pst->dw.num; t++)
       wfree(pst->dw.width[t]);
   wfree(pst->dw.width);
   wfree(pst->dw.coefr[0]);
   for (t=1; t<pst->dw.num; t++)
       wfree(pst->dw.coefr[t]);
   wfree(pst->dw.coefr);
   wfree(pst->dw.coef);
	
   wfree(pst->sm.mseq);
   wfree(pst->sm.ivseq);
   wfree(pst->sm.R);
   wfree(pst->sm.g);
   wfree(pst->sm.r);
   wfree(pst->par);
}

/* pdf2speech : parameter generation from pdf sequence */
void pdf2speech( FILE *rawfp, FILE *lf0fp, FILE *mcepfp, 
                  PStream *mceppst, PStream *lf0pst, globalP *gp, ModelSet *ms, UttModel *um, VocoderSetup *vs)
{
   int frame, mcepframe, lf0frame;
   int state, lw, rw, k, n;
   Model *m;
   HTS_Boolean nobound, *voiced;
   
   float f0;

   lf0pst->vSize = ms->lf0stream;
   lf0pst->order = 0;
   mceppst->vSize = ms->mcepvsize;
   mceppst->order = mceppst->vSize / mceppst->dw.num - 1;

   InitDWin(lf0pst);
   InitDWin(mceppst);

   mcepframe  = 0;
   lf0frame = 0;
 
   voiced = walloc(HTS_Boolean,um->totalframe+1);
   
   for (m=um->mhead; m!=um->mtail ; m=m->next) {
      for (state=2; state<=ms->nstate+1; state++) {
         for (frame=1; frame<=m->dur[state]; frame++) {
            voiced[mcepframe++] = m->voiced[state];
            if (m->voiced[state]) {
               lf0frame++;
            }
         }
      }
   }
         
   mceppst->T = mcepframe;
   lf0pst->T  = lf0frame;
  
   InitPStream(mceppst);      
   InitPStream(lf0pst);

   mcepframe = 0;
   lf0frame  = 0;

   for (m=um->mhead; m!=um->mtail; m=m->next) {
      for (state=2; state<=ms->nstate+1; state++) {
         for (frame=1; frame<=m->dur[state]; frame++) {
            for (k=0; k<ms->mcepvsize; k++) {
               mceppst->sm.mseq[mcepframe][k]  = m->mcepmean[state][k];
               mceppst->sm.ivseq[mcepframe][k] = finv(m->mcepvariance[state][k]);
            }
            for (k=0; k<ms->lf0stream; k++) {
               lw = lf0pst->dw.width[k][WLEFT];
               rw = lf0pst->dw.width[k][WRIGHT];
               nobound = (HTS_Boolean)1;
               
               for (n=lw; n<=rw;n++)
                  if (mcepframe+n<0 || um->totalframe<mcepframe+n)
                     nobound = (HTS_Boolean)0;
                  else
		      nobound = (HTS_Boolean)((int)nobound & voiced[mcepframe+n]);
                  
               if (voiced[mcepframe]) {
                  lf0pst->sm.mseq[lf0frame][k] = m->lf0mean[state][k+1];
                  if (nobound || k==0)
                     lf0pst->sm.ivseq[lf0frame][k] = finv(m->lf0variance[state][k+1]);
                  else
                     lf0pst->sm.ivseq[lf0frame][k] = 0.0;
               }
            }
            if (voiced[mcepframe])
               lf0frame++;
            mcepframe++;
         }
      }
   }

   mlpg(mceppst);
   
   if (lf0frame>0)
      mlpg(lf0pst);

   lf0frame = 0;
   
   if (gp->XIMERA && lf0fp!=NULL)
      fprintf(lf0fp, "# FrameShift=%dms\n", (FPERIOD*1000)/RATE);
   
   for (mcepframe=0; mcepframe<mceppst->T; mcepframe++) { 
      if (voiced[mcepframe])
         f0 = gp->F0_STD * exp(lf0pst->par[lf0frame++][0]) + gp->F0_MEAN;  
      else                  
         f0 = 0.0;

      if (mcepfp != NULL)
         fwrite(mceppst->par[mcepframe], sizeof(float), mceppst->order+1, mcepfp);
      if (lf0fp != NULL) {
         if (gp->XIMERA)  
            fprintf(lf0fp, "%.1f 1\n", f0);
         else 
            fwrite(&f0, sizeof(double), 1, lf0fp);
      }

      if (rawfp!=NULL)
         vocoder(f0, mceppst->par[mcepframe], mceppst->order, rawfp, gp, vs);
   }

   FreePStream(mceppst);
   FreePStream(lf0pst);
   wfree(voiced);
}

/* InitDWin : Initialise dynamic window */
void InitDWin(PStream *pst)
{   
   int i;
   int fsize, leng, fpos;
   FILE *fp;

   /* memory allocation */
   pst->dw.width = walloc(int *,pst->dw.num);
  
   for (i=0; i<pst->dw.num; i++) {
       pst->dw.width[i] = walloc(int,2);
   }

   pst->dw.coef= walloc(float *,pst->dw.num);
   /* because the pointers are moved, keep an original of the memory
      being allocated */
   pst->dw.coefr= walloc(float *,pst->dw.num);
   
   /* window for static parameter */
   pst->dw.width[0][WLEFT] = pst->dw.width[0][WRIGHT] = 0;
   pst->dw.coef[0] = fcalloc (1);
   pst->dw.coefr[0] = pst->dw.coef[0];
   pst->dw.coef[0][0] = 1;

   /* set delta coefficients */
   for (i=1; i<pst->dw.num; i++) {
      if (pst->dw.fn[i][0] == ' ')
         fsize = str2farray(pst->dw.fn[i], &(pst->dw.coef[i]));
      else {         /* read from file */
         if ((fp = fopen (pst->dw.fn[i], "r")) == NULL) {
            fprintf(stderr, "file %s not found\n", pst->dw.fn[i]);
            festival_error();
         }
            
         /* check the number of coefficients */
         fseek(fp, 0L, 2);
	 fpos = (int)ftell(fp);
         fsize = fpos/sizeof (float);
         fseek(fp, 0L, 0);

         /* read coefficients */
         pst->dw.coef[i] = fcalloc (fsize);
         pst->dw.coefr[i] = pst->dw.coef[i];
         fread(pst->dw.coef[i], sizeof(float), fsize, fp);
	 if (EST_BIG_ENDIAN)
		 swap_bytes_float(pst->dw.coef[i],fsize);

         fclose(fp);
      }

      /* set pointer */
      leng = fsize / 2;
      pst->dw.coef[i] += leng;
      pst->dw.width[i][WLEFT] = -leng;
      pst->dw.width[i][WRIGHT] = leng;
         
      if (fsize % 2 == 0)
         pst->dw.width[i][WRIGHT]--;
   }
 
   pst->dw.maxw[WLEFT] = pst->dw.maxw[WRIGHT] = 0;
      
   for (i=0; i<pst->dw.num; i++) {
      if (pst->dw.maxw[WLEFT] > pst->dw.width[i][WLEFT])
         pst->dw.maxw[WLEFT] = pst->dw.width[i][WLEFT];
      if (pst->dw.maxw[WRIGHT] < pst->dw.width[i][WRIGHT])
         pst->dw.maxw[WRIGHT] = pst->dw.width[i][WRIGHT];
   }

   /* calcurate max_L to determine size of band matrix */
   if ( pst->dw.maxw[WLEFT] >= pst->dw.maxw[WRIGHT] )
      pst->dw.max_L = pst->dw.maxw[WLEFT];
   else
      pst->dw.max_L = pst->dw.maxw[WRIGHT];

}

/* -------------------- End of "mlpg.c" -------------------- */
