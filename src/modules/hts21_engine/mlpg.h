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
/*    mlpg.h : speech parameter generation from pdf sequence         */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */

#define INFTY ((double) 1.0e+38)
#define INFTY2 ((double) 1.0e+19)
#define INVINF ((double) 1.0e-38)
#define INVINF2 ((double) 1.0e-19)

#define WLEFT 0
#define WRIGHT 1

typedef struct _DWin {
   int num;           /* number of static + deltas */
   char **fn;         /* delta window coefficient file */
   int **width;       /* width [0..num-1][0(left) 1(right)] */
   float **coef;      /* coefficient [0..num-1][length[0]..length[1]] */
   float **coefr;     /* pointers to the memory being allocated */
   int maxw[2];       /* max width [0(left) 1(right)] */
   int max_L;
} DWin;

typedef struct _SMatrices {
   double **mseq;   /* sequence of mean vector */
   double **ivseq;	 /* sequence of invarsed variance vector */
   double *g;			
   double **R;
   double *r; 
} SMatrices;

typedef struct _PStream {
   int vSize;
   int order;
   int T;
   int width;
   DWin dw;
   float **par;     /* output parameter vector */
   SMatrices sm;
} PStream;

void pdf2speech(FILE *, FILE *, FILE *, PStream *, PStream *, globalP *, ModelSet *, UttModel *, VocoderSetup *);
void InitDWin (PStream *);

/* -------------------- End of "mlpg.h" -------------------- */
