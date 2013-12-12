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
/*   This is Zen's MLSA filter as ported by Toda to fetvox vc        */
/*   and back ported into hts/festival so we can do MLSA filtering   */
/*   If I took more time I could probably make this use the same as  */
/*   as the other code in this directory -- awb@cs.cmu.edu 03JAN06   */
/*  ---------------------------------------------------------------  */

/*********************************************************************/
/*                                                                   */
/*  Mel-cepstral vocoder (pulse/noise excitation & MLSA filter)      */
/*                                    2003/12/26 by Heiga Zen        */
/*                                                                   */
/*  Extracted from HTS and slightly modified                         */
/*   by Tomoki Toda (tomoki@ics.nitech.ac.jp)                        */
/*  June 2004                                                        */
/*  Integrate as a Voice Conversion module                           */
/*                                                                   */
/*-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <EST_walloc.h>
#include "festival.h"

#include "hts21_mlsa_resynthesis.h"

LISP hts21_mlsa_resynthesis(LISP ltrack)
{
    /* Resynthesizes a wave from given track */
    EST_Track *t;
    EST_Wave *wave = 0;
    DVECTOR w;
    DMATRIX mcep;
    DVECTOR f0v;
    int sr = 16000;
    int i,j;
    double shift;

    if ((ltrack == NULL) ||
        (TYPEP(ltrack,tc_string) &&
         (streq(get_c_string(ltrack),"nil"))))
        return siod(new EST_Wave(0,1,sr));

    t = track(ltrack);

    f0v = xdvalloc(t->num_frames());
    mcep = xdmalloc(t->num_frames(),t->num_channels()-1);

    for (i=0; i<t->num_frames(); i++)
    {
        f0v->data[i] = t->a(i,0);
        for (j=1; j<t->num_channels(); j++)
            mcep->data[i][j-1] = t->a(i,j);
    }

    if (t->num_frames() > 1)
        shift = 1000.0*(t->t(1)-t->t(0));
    else
        shift = 5.0;

    w = synthesis_body(mcep,f0v,NULL,sr,shift);

    wave = new EST_Wave(w->length,1,sr);
    
    for (i=0; i<w->length; i++)
        wave->a(i) = (short)w->data[i];

    xdmfree(mcep);
    xdvfree(f0v);
    xdvfree(w);

    return siod(wave);
}


DVECTOR synthesis_body(DMATRIX mcep,	// input mel-cep sequence
		       DVECTOR f0v,	// input F0 sequence
		       DVECTOR dpow,	// input diff-pow sequence
		       double fs,	// sampling frequency (Hz)
		       double framem)	// FFT length
{
    long t, pos;
    int framel;
    double f0;
    VocoderSetup vs;
    DVECTOR xd = NODATA;
    DVECTOR syn = NODATA;

    framel = (int)(framem * fs / 1000.0);
    init_vocoder(fs, framel, mcep->col - 1, &vs);

    // synthesize waveforms by MLSA filter
    xd = xdvalloc(mcep->row * (framel + 2));
    for (t = 0, pos = 0; t < mcep->row; t++) {
	if (t >= f0v->length) f0 = 0.0;
	else f0 = f0v->data[t];
	if (dpow == NODATA)
	    vocoder(f0, mcep->data[t], mcep->col - 1, ALPHA, 0.0, &vs,
		    xd->data, &pos);
	else
	    vocoder(f0, mcep->data[t], dpow->data[t], mcep->col - 1, ALPHA,
		    0.0, &vs, xd->data, &pos);
    }
    syn = xdvcut(xd, 0, pos);

    // normalized amplitude
    waveampcheck(syn, XFALSE);

    // memory free
    xdvfree(xd);
    free_vocoder(&vs);

    return syn;
}

#if 0
static DVECTOR get_dpowvec(DMATRIX rmcep, DMATRIX cmcep)
{
    long t;
    DVECTOR dpow = NODATA;
    VocoderSetup pvs;

    // error check
    if (rmcep->col != cmcep->col) {
	fprintf(stderr, "Error: Different number of dimensions\n");
	exit(1);
    }
    if (rmcep->row != cmcep->row) {
	fprintf(stderr, "Error: Different number of frames\n");
	exit(1);
    }

    // memory allocation
    dpow = xdvalloc(rmcep->row);
    init_vocoder(16000.0, 80, rmcep->col - 1, &pvs);

    // calculate differential power
    for (t = 0; t < rmcep->row; t++)
	dpow->data[t] = get_dpow(rmcep->data[t], cmcep->data[t],
				 rmcep->col - 1, ALPHA, &pvs);

    // memory free
    free_vocoder(&pvs);

    return dpow;
}
#endif

static void waveampcheck(DVECTOR wav, XBOOL msg_flag)
{
    double value;
    int k;

    value = MAX(FABS(dvmax(wav, NULL)), FABS(dvmin(wav, NULL)));
    if (value >= 32000.0) {
	if (msg_flag == XTRUE) {
	    fprintf(stderr, "amplitude is too big: %f\n", value);
	    fprintf(stderr, "execute normalization\n");
	}
	/* was dvscoper(wav, "*", 32000.0 / value); */
	for (k = 0; k < wav->length; k++) {
	    wav->data[k] = wav->data[k] * (32000.0/value);
	    if (wav->imag != NULL) {
		wav->imag[k] = wav->imag[k] * (32000.0/value);
	    }
	}
    }

    return;
}

static void init_vocoder(double fs, int framel, int m, VocoderSetup *vs)
{
    // initialize global parameter
    vs->fprd = framel;
    vs->iprd = 1;
    vs->seed = 1;
    vs->pd   = 5;

    vs->next =1;
    vs->gauss = MTRUE;

    vs->pade[ 0]=1.0;
    vs->pade[ 1]=1.0; vs->pade[ 2]=0.0;
    vs->pade[ 3]=1.0; vs->pade[ 4]=0.0;      vs->pade[ 5]=0.0;
    vs->pade[ 6]=1.0; vs->pade[ 7]=0.0;      vs->pade[ 8]=0.0;      vs->pade[ 9]=0.0;
    vs->pade[10]=1.0; vs->pade[11]=0.4999273; vs->pade[12]=0.1067005; vs->pade[13]=0.01170221; vs->pade[14]=0.0005656279;
    vs->pade[15]=1.0; vs->pade[16]=0.4999391; vs->pade[17]=0.1107098; vs->pade[18]=0.01369984; vs->pade[19]=0.0009564853;
    vs->pade[20]=0.00003041721;

    vs->rate = fs;
    vs->c = wcalloc(double,3 * (m + 1) + 3 * (vs->pd + 1) + vs->pd * (m + 2));
   
    vs->p1 = -1;
    vs->sw = 0;
    vs->x  = 0x55555555;
   
    // for postfiltering
    vs->mc = NULL;
    vs->o  = 0;
    vs->d  = NULL;
    vs->irleng= 64;
   
    return;
}

static void vocoder(double p, double *mc, int m, double a, double beta,
	     VocoderSetup *vs, double *wav, long *pos)
{
    double inc, x, e1, e2;
    int i, j, k; 
   
    if (p != 0.0) 
	p = vs->rate / p;  // f0 -> pitch
   
    if (vs->p1 < 0) {
	if (vs->gauss & (vs->seed != 1)) 
	    vs->next = srnd((unsigned)vs->seed);
         
	vs->p1   = p;
	vs->pc   = vs->p1;
	vs->cc   = vs->c + m + 1;
	vs->cinc = vs->cc + m + 1;
	vs->d1   = vs->cinc + m + 1;
      
	mc2b(mc, vs->c, m, a);
      
	if (beta > 0.0 && m > 1) {
	    e1 = b2en(vs->c, m, a, vs);
	    vs->c[1] -= beta * a * mc[2];
	    for (k=2;k<=m;k++)
		vs->c[k] *= (1.0 + beta);
	    e2 = b2en(vs->c, m, a, vs);
	    vs->c[0] += log(e1/e2)/2;
	}

	return;
    }

    mc2b(mc, vs->cc, m, a); 
    if (beta>0.0 && m > 1) {
	e1 = b2en(vs->cc, m, a, vs);
	vs->cc[1] -= beta * a * mc[2];
	for (k = 2; k <= m; k++)
	    vs->cc[k] *= (1.0 + beta);
	e2 = b2en(vs->cc, m, a, vs);
	vs->cc[0] += log(e1 / e2) / 2.0;
    }

    for (k=0; k<=m; k++)
	vs->cinc[k] = (vs->cc[k] - vs->c[k]) *
	    (double)vs->iprd / (double)vs->fprd;

    if (vs->p1!=0.0 && p!=0.0) {
	inc = (p - vs->p1) * (double)vs->iprd / (double)vs->fprd;
    } else {
	inc = 0.0;
	vs->pc = p;
	vs->p1 = 0.0;
    }

    for (j = vs->fprd, i = (vs->iprd + 1) / 2; j--;) {
	if (vs->p1 == 0.0) {
	    if (vs->gauss)
		x = (double) nrandom(vs);
	    else
		x = mseq(vs);
	} else {
	    if ((vs->pc += 1.0) >= vs->p1) {
		x = sqrt (vs->p1);
		vs->pc = vs->pc - vs->p1;
	    } else x = 0.0;
	}

	x *= exp(vs->c[0]);

	x = mlsadf(x, vs->c, m, a, vs->pd, vs->d1, vs);

	wav[*pos] = x;
	*pos += 1;

	if (!--i) {
	    vs->p1 += inc;
	    for (k = 0; k <= m; k++) vs->c[k] += vs->cinc[k];
	    i = vs->iprd;
	}
    }
   
    vs->p1 = p;
    memmove(vs->c,vs->cc,sizeof(double)*(m+1));
   
    return;
}

static void vocoder(double p, double *mc, double dpow, int m, double a, double beta,
	     VocoderSetup *vs, double *wav, long *pos)
{
    double inc, x, e1, e2;
    int i, j, k; 
   
    if (p != 0.0) 
	p = vs->rate / p;  // f0 -> pitch
   
    if (vs->p1 < 0) {
	if (vs->gauss & (vs->seed != 1)) 
	    vs->next = srnd((unsigned)vs->seed);
         
	vs->p1   = p;
	vs->pc   = vs->p1;
	vs->cc   = vs->c + m + 1;
	vs->cinc = vs->cc + m + 1;
	vs->d1   = vs->cinc + m + 1;
      
	mc2b(mc, vs->c, m, a);
	vs->c[0] += dpow;
      
	if (beta > 0.0 && m > 1) {
	    e1 = b2en(vs->c, m, a, vs);
	    vs->c[1] -= beta * a * mc[2];
	    for (k=2;k<=m;k++)
		vs->c[k] *= (1.0 + beta);
	    e2 = b2en(vs->c, m, a, vs);
	    vs->c[0] += log(e1/e2)/2;
	}

	return;
    }

    mc2b(mc, vs->cc, m, a);
    vs->cc[0] += dpow;
    if (beta>0.0 && m > 1) {
	e1 = b2en(vs->cc, m, a, vs);
	vs->cc[1] -= beta * a * mc[2];
	for (k = 2; k <= m; k++)
	    vs->cc[k] *= (1.0 + beta);
	e2 = b2en(vs->cc, m, a, vs);
	vs->cc[0] += log(e1 / e2) / 2.0;
    }

    for (k=0; k<=m; k++)
	vs->cinc[k] = (vs->cc[k] - vs->c[k]) *
	    (double)vs->iprd / (double)vs->fprd;

    if (vs->p1!=0.0 && p!=0.0) {
	inc = (p - vs->p1) * (double)vs->iprd / (double)vs->fprd;
    } else {
	inc = 0.0;
	vs->pc = p;
	vs->p1 = 0.0;
    }

    for (j = vs->fprd, i = (vs->iprd + 1) / 2; j--;) {
	if (vs->p1 == 0.0) {
	    if (vs->gauss)
		x = (double) nrandom(vs);
	    else
		x = mseq(vs);
	} else {
	    if ((vs->pc += 1.0) >= vs->p1) {
		x = sqrt (vs->p1);
		vs->pc = vs->pc - vs->p1;
	    } else x = 0.0;
	}

	x *= exp(vs->c[0]);

	x = mlsadf(x, vs->c, m, a, vs->pd, vs->d1, vs);

	wav[*pos] = x;
	*pos += 1;

	if (!--i) {
	    vs->p1 += inc;
	    for (k = 0; k <= m; k++) vs->c[k] += vs->cinc[k];
	    i = vs->iprd;
	}
    }
   
    vs->p1 = p;
    memmove(vs->c,vs->cc,sizeof(double)*(m+1));
   
    return;
}

static double mlsadf(double x, double *b, int m, double a, int pd, double *d, VocoderSetup *vs)
{

   vs->ppade = &(vs->pade[pd*(pd+1)/2]);
    
   x = mlsadf1 (x, b, m, a, pd, d, vs);
   x = mlsadf2 (x, b, m, a, pd, &d[2*(pd+1)], vs);

   return(x);
}

static double mlsadf1(double x, double *b, int m, double a, int pd, double *d, VocoderSetup *vs)
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

static double mlsadf2 (double x, double *b, int m, double a, int pd, double *d, VocoderSetup *vs)
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

static double mlsafir (double x, double *b, int m, double a, double *d)
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

   for (i=m+1; i>1; i--) 
      d[i] = d[i-1];

   return(y);
}

static double nrandom (VocoderSetup *vs)
{
   if (vs->sw == 0) {
      vs->sw = 1;
      do {
         vs->r1 = 2.0 * rnd(&vs->next) - 1.0;
         vs->r2 = 2.0 * rnd(&vs->next) - 1.0;
         vs->s  = vs->r1 * vs->r1 + vs->r2 * vs->r2;
      } while (vs->s > 1 || vs->s == 0);

      vs->s = sqrt (-2 * log(vs->s) / vs->s);
      
      return(vs->r1*vs->s);
   }
   else {
      vs->sw = 0;
      
      return (vs->r2*vs->s);
   }
}

static double rnd (unsigned long *next)
{
   double r;

   *next = *next * 1103515245L + 12345;
   r = (*next / 65536L) % 32768L;

   return(r/RANDMAX); 
}

static unsigned long srnd ( unsigned long seed )
{
   return(seed);
}

static int mseq (VocoderSetup *vs)
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

   return(x0);
}

// mc2b : transform mel-cepstrum to MLSA digital fillter coefficients
static void mc2b (double *mc, double *b, int m, double a)
{
   b[m] = mc[m];
    
   for (m--; m>=0; m--)
      b[m] = mc[m] - a * b[m+1];
   
   return;
}


static double b2en (double *b, int m, double a, VocoderSetup *vs)
{
   double en;
   int k;
   
   if (vs->o<m) {
      if (vs->mc != NULL)
          wfree(vs->mc);
    
      vs->mc = wcalloc(double,(m + 1) + 2 * vs->irleng);
      vs->cep = vs->mc + m+1;
      vs->ir  = vs->cep + vs->irleng;
   }

   b2mc(b, vs->mc, m, a);
   freqt(vs->mc, m, vs->cep, vs->irleng-1, -a, vs);
   c2ir(vs->cep, vs->irleng, vs->ir, vs->irleng);
   en = 0.0;
   
   for (k=0;k<vs->irleng;k++)
      en += vs->ir[k] * vs->ir[k];

   return(en);
}


// b2bc : transform MLSA digital filter coefficients to mel-cepstrum
static void b2mc (double *b, double *mc, int m, double a)
{
  double d, o;
        
  d = mc[m] = b[m];
  for (m--; m>=0; m--) {
    o = b[m] + a * d;
    d = b[m];
    mc[m] = o;
  }
  
  return;
}

// freqt : frequency transformation
static void freqt (double *c1, int m1, double *c2, int m2, double a, VocoderSetup *vs)
{
   register int i, j;
   double b;
    
   if (vs->d==NULL) {
      vs->size = m2;
      vs->d    = wcalloc(double,vs->size + vs->size + 2);
      vs->g    = vs->d+vs->size+1;
   }

   if (m2>vs->size) {
       wfree(vs->d);
      vs->size = m2;
      vs->d    = wcalloc(double,vs->size + vs->size + 2);
      vs->g    = vs->d+vs->size+1;
   }
    
   b = 1-a*a;
   for (i=0; i<m2+1; i++)
      vs->g[i] = 0.0;

   for (i=-m1; i<=0; i++) {
      if (0 <= m2)
         vs->g[0] = c1[-i]+a*(vs->d[0]=vs->g[0]);
      if (1 <= m2)
         vs->g[1] = b*vs->d[0]+a*(vs->d[1]=vs->g[1]);
      for (j=2; j<=m2; j++)
         vs->g[j] = vs->d[j-1]+a*((vs->d[j]=vs->g[j])-vs->g[j-1]);
   }

   memmove(c2,vs->g,sizeof(double)*(m2+1));
   
   return;
}

// c2ir : The minimum phase impulse response is evaluated from the minimum phase cepstrum
static void c2ir (double *c, int nc, double *h, int leng)
{
   register int n, k, upl;
   double  d;

   h[0] = exp(c[0]);
   for (n=1; n<leng; n++) {
      d = 0;
      upl = (n>=nc) ? nc-1 : n;
      for (k=1; k<=upl; k++)
         d += k*c[k]*h[n-k];
      h[n] = d/n;
   }
   
   return;
}

#if 0
static double get_dpow(double *rmcep, double *cmcep, int m, double a,
		VocoderSetup *vs)
{
    double e1, e2, dpow;

    if (vs->p1 < 0) {
	vs->p1 = 1;
	vs->cc = vs->c + m + 1;
	vs->cinc = vs->cc + m + 1;
	vs->d1   = vs->cinc + m + 1;
    }

    mc2b(rmcep, vs->c, m, a); 
    e1 = b2en(vs->c, m, a, vs);

    mc2b(cmcep, vs->cc, m, a); 
    e2 = b2en(vs->cc, m, a, vs);

    dpow = log(e1 / e2) / 2.0;

    return dpow;
}
#endif

static void free_vocoder(VocoderSetup *vs)
{
    wfree(vs->c);
    wfree(vs->mc);
    wfree(vs->d);
 
    vs->c = NULL;
    vs->mc = NULL;
    vs->d = NULL;
    vs->ppade = NULL;
    vs->cc = NULL;
    vs->cinc = NULL;
    vs->d1 = NULL;
    vs->g = NULL;
    vs->cep = NULL;
    vs->ir = NULL;
   
    return;
}

/* from vector.cc */

static DVECTOR xdvalloc(long length)
{
    DVECTOR x;

    length = MAX(length, 0);
    x = wcalloc(struct DVECTOR_STRUCT,1);
    x->data = wcalloc(double,MAX(length, 1));
    x->imag = NULL;
    x->length = length;

    return x;
}

static void xdvfree(DVECTOR x)
{
    if (x != NULL) {
	if (x->data != NULL) {
	    wfree(x->data);
	}
	if (x->imag != NULL) {
	    wfree(x->imag);
	}
	wfree(x);
    }

    return;
}

static void dvialloc(DVECTOR x)
{
    if (x->imag != NULL) {
	wfree(x->imag);
    }
    x->imag = wcalloc(double,x->length);

    return;
}

static DVECTOR xdvcut(DVECTOR x, long offset, long length)
{
    long k;
    long pos;
    DVECTOR y;
    
    y = xdvalloc(length);
    if (x->imag != NULL) {
	dvialloc(y);
    }

    for (k = 0; k < y->length; k++) {
	pos = k + offset;
	if (pos >= 0 && pos < x->length) {
	    y->data[k] = x->data[pos];
	    if (y->imag != NULL) {
		y->imag[k] = x->imag[pos];
	    }
	} else {
	    y->data[k] = 0.0;
	    if (y->imag != NULL) {
		y->imag[k] = 0.0;
	    }
	}
    }

    return y;
}

static DMATRIX xdmalloc(long row, long col)
{
    DMATRIX matrix;
    int i;

    matrix = wcalloc(struct DMATRIX_STRUCT,1);
    matrix->data = wcalloc(double *,row);
    for (i=0; i<row; i++)
        matrix->data[i] = wcalloc(double,col);
    matrix->imag = NULL;
    matrix->row = row;
    matrix->col = col;

    return matrix;
}

void xdmfree(DMATRIX matrix)
{
    int i;

    if (matrix != NULL) {
	if (matrix->data != NULL) {
            for (i=0; i<matrix->row; i++)
                wfree(matrix->data[i]);
            wfree(matrix->data);
	}
	if (matrix->imag != NULL) {
            for (i=0; i<matrix->row; i++)
                wfree(matrix->imag[i]);
            wfree(matrix->imag);
	}
	wfree(matrix);
    }

    return;
}


/* from voperate.cc */
static double dvmax(DVECTOR x, long *index)
{
    long k;
    long ind;
    double max;

    ind = 0;
    max = x->data[ind];
    for (k = 1; k < x->length; k++) {
	if (max < x->data[k]) {
	    ind = k;
	    max = x->data[k];
	}
    }

    if (index != NULL) {
	*index = ind;
    }

    return max;
}

static double dvmin(DVECTOR x, long *index)
{
    long k;
    long ind;
    double min;

    ind = 0;
    min = x->data[ind];
    for (k = 1; k < x->length; k++) {
	if (min > x->data[k]) {
	    ind = k;
	    min = x->data[k];
	}
    }

    if (index != NULL) {
	*index = ind;
    }

    return min;
}
