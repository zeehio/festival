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
/*    model.h : model definition                                     */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */

typedef struct _Model {  /* HMM handler */
   char *name;            /* the name of this HMM */
   int durpdf;            /* duration pdf index for this HMM */
   int *lf0pdf;           /* mel-cepstrum pdf indexes for each state of this HMM */  
   int *mceppdf;          /* log f0 pdf indexes for each state of this HMM */
   int *dur;              /* duration for each state of this HMM */
   int totaldur;          /* total duration of this HMM */
   float **lf0mean;       /* mean vector of log f0 pdfs for each state of this HMM */
   float **lf0variance;   /* variance (diag) elements of log f0 for each state of this HMM */
   float **mcepmean;      /* mean vector of mel-cepstrum pdfs for each state of this HMM */
   float **mcepvariance;  /* variance (diag) elements of mel-cepstrum for each state of this HMM */
   HTS_Boolean *voiced;       /* voiced/unvoiced decision for each state of this HMM */
   struct _Model *next;   /* pointer to next HMM */
} Model; 


typedef struct _UttModel { /* Utterance model handler */
   Model *mhead;
   Model *mtail;
   int nModel;
   int nState;
   int totalframe;
} UttModel;


typedef struct _ModelSet { /* HMM set handler */
   int nstate;
   int lf0stream;
   int mcepvsize;
   int ndurpdf;
   int *nmceppdf;
   int *nlf0pdf;
   float **durpdf,***mceppdf,****lf0pdf;
   FILE *fp[3];
} ModelSet;


void LoadModelFiles (ModelSet *);
void FindDurPDF (Model *, ModelSet *, float, int );
void FindLF0PDF (int, Model *, ModelSet *, float);
void FindMcpPDF (int, Model *, ModelSet *);
void InitModelSet (ModelSet *);
void DeleteModelSet(ModelSet *ms);

/* -------------------- End of "model.h" -------------------- */
