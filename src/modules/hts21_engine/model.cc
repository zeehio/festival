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
/*    model.c : read model and search pdf from models                */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "festival.h"

#include "defaults.h"
#include "misc.h"
#include "model.h"
#include "global.h"

/* LoadModelFiles : load model files from files to pdf array */
void LoadModelFiles(ModelSet *ms)
{
   int i, j, k;

   /*-------------------- load pdfs for duration --------------------*/
   /* read the number of states & the number of pdfs (leaf nodes) */
   fread(&ms->nstate,  sizeof(int), 1, ms->fp[DUR]);
   if (EST_BIG_ENDIAN) ms->nstate = SWAPINT(ms->nstate);
   fread(&ms->ndurpdf, sizeof(int), 1, ms->fp[DUR]);
   if (EST_BIG_ENDIAN) ms->ndurpdf = SWAPINT(ms->ndurpdf);

   ms->durpdf = walloc(float *,ms->ndurpdf+2);
   
   /* read pdfs (mean & variance) */
   for (i=1; i<=ms->ndurpdf; i++) {
      ms->durpdf[i] = walloc(float,2*ms->nstate+2);
      fread(ms->durpdf[i]+2, sizeof(float), 2*ms->nstate, ms->fp[DUR]);
      if (EST_BIG_ENDIAN)
	      swap_bytes_float(ms->durpdf[i]+2,2*ms->nstate);
   }

   /*-------------------- load pdfs for mcep --------------------*/
   /* read vector size for spectrum */
   fread(&ms->mcepvsize, sizeof(int), 1, ms->fp[MCP]);
   if (EST_BIG_ENDIAN) ms->mcepvsize = SWAPINT(ms->mcepvsize);
   ms->nmceppdf = walloc(int,ms->nstate);

   /* read the number of pdfs for each state position */
   fread(ms->nmceppdf, sizeof(int), ms->nstate, ms->fp[MCP]);
   if (EST_BIG_ENDIAN) swap_bytes_int(ms->nmceppdf,ms->nstate);
   ms->mceppdf = walloc(float **,ms->nstate+2);
   
   /* read pdfs (mean, variance) */
   for (i=2; i<=ms->nstate+1; i++) {
      ms->mceppdf[i] = walloc(float *,ms->nmceppdf[i-2]+2);
      for (j=1; j<=ms->nmceppdf[i-2]; j++) {
         ms->mceppdf[i][j] = walloc(float,ms->mcepvsize*2);
         fread(ms->mceppdf[i][j], sizeof(float), ms->mcepvsize*2, ms->fp[MCP]);
	 if (EST_BIG_ENDIAN)
		 swap_bytes_float(ms->mceppdf[i][j],ms->mcepvsize*2);
      }
   } 

   /*-------------------- load pdfs for log F0 --------------------*/
   /* read the number of streams for f0 modeling */
   fread(&ms->lf0stream, sizeof(int), 1, ms->fp[LF0]);
   if (EST_BIG_ENDIAN) ms->lf0stream = SWAPINT(ms->lf0stream);
   ms->nlf0pdf = walloc(int,ms->nstate+2);
   /* read the number of pdfs for each state position */
   fread(ms->nlf0pdf, sizeof(int), ms->nstate, ms->fp[LF0]);
   if (EST_BIG_ENDIAN) swap_bytes_int(ms->nlf0pdf,ms->nstate);
   ms->lf0pdf = walloc(float ***,ms->nstate+3);
   
   /* read pdfs (mean, variance & weight) */
   for (i=2; i<=ms->nstate+1; i++) {
      ms->lf0pdf[i] = walloc(float **,ms->nlf0pdf[i-2]+1);
      for (j=1; j<=ms->nlf0pdf[i-2]; j++) {
         ms->lf0pdf[i][j] = walloc(float *,ms->lf0stream+1);
         for (k=1; k<=ms->lf0stream; k++) {
            ms->lf0pdf[i][j][k] = walloc(float,4);
            fread(ms->lf0pdf[i][j][k], sizeof(float), 4, ms->fp[LF0]);
	    if (EST_BIG_ENDIAN)
		 swap_bytes_float(ms->lf0pdf[i][j][k],4);
         }
      }
   } 
}

/* FindDurPDF : find duration pdf from pdf array */
void FindDurPDF (Model *m, ModelSet *ms, float rho, int diffdur)
{
   float data, mean, variance;
   int s, idx; 

   idx = m->durpdf;

   m->dur = walloc(int,ms->nstate+2);
   m->totaldur = 0;
   
   for (s=2; s<=ms->nstate+1; s++) {
      mean = ms->durpdf[idx][s];
      variance = ms->durpdf[idx][ms->nstate+s];
      data = mean + rho*variance;
      
      if (data < 0.0) data = 0.0;
         
      m->dur[s] = (int) (data+diffdur+0.5);
      m->totaldur += m->dur[s];
      diffdur += (int)(data-(float)m->dur[s]);
   }
}

/* FindLF0PDF : find required pdf for log F0 from pdf array */
void FindLF0PDF (int s, Model *m, ModelSet *ms, float uvthresh)
{
   int idx, stream;
   float *weight;

   idx = m->lf0pdf[s];

   if (m->lf0mean[s]) wfree(m->lf0mean[s]);
   m->lf0mean[s]     = walloc(float,ms->lf0stream+1);
   if (m->lf0variance[s]) wfree(m->lf0variance[s]);
   m->lf0variance[s] = walloc(float,ms->lf0stream+1);
   
   for (stream=1; stream<=ms->lf0stream; stream++) {
      m->lf0mean    [s][stream] = ms->lf0pdf[s][idx][stream][0];
      m->lf0variance[s][stream] = ms->lf0pdf[s][idx][stream][1];
      weight = ms->lf0pdf[s][idx][stream]+2;
      
      if (stream==1) {
         if (weight[0] > uvthresh)
            m->voiced[s] = 1;
         else
            m->voiced[s] = 0;
      }
   }
}

/* FindMcpPDF : find pdf for mel-cepstrum from pdf array */
void FindMcpPDF (int s, Model *m, ModelSet *ms)
{
   int idx;
   
   idx = m->mceppdf[s];

   m->mcepmean[s] = ms->mceppdf[s][idx];
   m->mcepvariance[s] = ms->mceppdf[s][idx]+ms->mcepvsize;
}

void InitModelSet (ModelSet *ms)
{
   ms->fp[DUR] = NULL;
   ms->fp[LF0] = NULL;
   ms->fp[MCP] = NULL;
   
   return;
} 

void DeleteModelSet(ModelSet *ms)
{
    int i,j,k;
    
    for (i=1; i<=ms->ndurpdf; i++)
	wfree(ms->durpdf[i]);
    wfree(ms->durpdf);

    for (i=2; i<=ms->nstate+1; i++)
    {
	for (j=1; j<=ms->nmceppdf[i-2]; j++)
	    wfree(ms->mceppdf[i][j]);
	wfree(ms->mceppdf[i]);
    }
    wfree(ms->nmceppdf);
    wfree(ms->mceppdf);

    for (i=2; i<=ms->nstate+1; i++)
    {
	for (j=1; j<=ms->nlf0pdf[i-2]; j++)
	{
	    for (k=1; k <=ms->lf0stream; k++)
		wfree(ms->lf0pdf[i][j][k]);
	    wfree(ms->lf0pdf[i][j]);
	}
	wfree(ms->lf0pdf[i]);
    }
    wfree(ms->nlf0pdf);
    wfree(ms->lf0pdf);
}

/* -------------------- End of "model.c" -------------------- */

