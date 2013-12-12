/*********************************************************************/
/*                                                                   */
/*            Nagoya Institute of Technology, Aichi, Japan,          */
/*       Nara Institute of Science and Technology, Nara, Japan       */
/*                                and                                */
/*             Carnegie Mellon University, Pittsburgh, PA            */
/*                      Copyright (c) 2003-2004                      */
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
/*    2. Any modifications must be clearly marked as such.           */
/*    3. Original authors' names are not deleted.                    */
/*                                                                   */    
/*  NAGOYA INSTITUTE OF TECHNOLOGY, NARA INSTITUTE OF SCIENCE AND    */
/*  TECHNOLOGY, CARNEGIE MELLON UNIVERSITY, AND THE CONTRIBUTORS TO  */
/*  THIS WORK DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,  */
/*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, */
/*  IN NO EVENT SHALL NAGOYA INSTITUTE OF TECHNOLOGY, NARA           */
/*  INSTITUTE OF SCIENCE AND TECHNOLOGY, CARNEGIE MELLON UNIVERSITY, */
/*  NOR THE CONTRIBUTORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR      */
/*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM   */
/*  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,  */
/*  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN        */
/*  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*          Author :  Tomoki Toda (tomoki@ics.nitech.ac.jp)          */
/*          Date   :  June 2004                                      */
/*                                                                   */
/*          Modified by Alan W Black (awb@cs.cmu.edu) Jan 2006       */
/*          taken from festvox/src/vc/ back into Festival            */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*  Subroutine for Speech Synthesis                                  */
/*                                                                   */
/*-------------------------------------------------------------------*/

#ifndef __MLSA_RESYNTHESIS_H
#define __MLSA_RESYNTHESIS_H

#define ALPHA 0.42

typedef struct DVECTOR_STRUCT {
    long length;
    double *data;
    double *imag;
} *DVECTOR;

typedef struct DMATRIX_STRUCT {
    long row;
    long col;
    double **data;
    double **imag;
} *DMATRIX;

#define XBOOL int
#define XTRUE 1
#define XFALSE 0

#define NODATA NULL

#define FABS(x) ((x) >= 0.0 ? (x) : -(x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static DVECTOR xdvalloc(long length);
static DVECTOR xdvcut(DVECTOR x, long offset, long length);
static void xdvfree(DVECTOR vector);
static double dvmax(DVECTOR x, long *index);
static double dvmin(DVECTOR x, long *index);
static DMATRIX xdmalloc(long row, long col);
static void xdmfree(DMATRIX matrix);

DVECTOR synthesis_body(DMATRIX mcep, DVECTOR f0v, DVECTOR dpow,
                       double fs, double framem);
static void waveampcheck(DVECTOR wav, XBOOL msg_flag);

#define RANDMAX 32767 
#define   B0         0x00000001
#define   B28        0x10000000
#define   B31        0x80000000
#define   B31_       0x7fffffff
#define   Z          0x00000000

typedef enum {MFALSE, MTRUE} Boolean;

typedef struct _VocoderSetup {
   
   int fprd;
   int iprd;
   int seed;
   int pd;
   unsigned long next;
   Boolean gauss;
   double p1;
   double pc;
   double pj;
   double pade[21];
   double *ppade;
   double *c, *cc, *cinc, *d1;
   double rate;
   
   int sw;
   double r1, r2, s;
   
   int x;
   
   /* for postfiltering */
   int size;
   double *d; 
   double *g;
   double *mc;
   double *cep;
   double *ir;
   int o;
   int irleng;
   
} VocoderSetup;

static void init_vocoder(double fs, int framel, int m, VocoderSetup *vs);
static void vocoder(double p, double *mc, int m, double a, double beta,
		    VocoderSetup *vs, double *wav, long *pos);
static void vocoder(double p, double *mc, double dpow, int m, double a,
		    double beta, VocoderSetup *vs, double *wav, long *pos);
static double mlsadf(double x, double *b, int m, double a, int pd, double *d,
		     VocoderSetup *vs);
static double mlsadf1(double x, double *b, int m, double a, int pd, double *d,
		      VocoderSetup *vs);
static double mlsadf2(double x, double *b, int m, double a, int pd, double *d,
		      VocoderSetup *vs);
static double mlsafir (double x, double *b, int m, double a, double *d);
static double nrandom (VocoderSetup *vs);
static double rnd (unsigned long *next);
static unsigned long srnd (unsigned long seed);
static int mseq (VocoderSetup *vs);
static void mc2b (double *mc, double *b, int m, double a);
static double b2en (double *b, int m, double a, VocoderSetup *vs);
static void b2mc (double *b, double *mc, int m, double a);
static void freqt (double *c1, int m1, double *c2, int m2, double a,
		   VocoderSetup *vs);
static void c2ir (double *c, int nc, double *h, int leng);


#if 0
static DVECTOR get_dpowvec(DMATRIX rmcep, DMATRIX cmcep);
static double get_dpow(double *rmcep, double *cmcep, int m, double a,
		       VocoderSetup *vs);
#endif
static void free_vocoder(VocoderSetup *vs);

#endif /* __RESYNTHESIS_SUB_H */
