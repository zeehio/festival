/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis System (HTS)             */
/*           hts_engine API developed by HTS Working Group           */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2001-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef HTS211_HIDDEN_H
#define HTS211_HIDDEN_H

#ifdef __cplusplus
#define HTS211_HIDDEN_H_START extern "C" {
#define HTS211_HIDDEN_H_END   }
#else
#define HTS211_HIDDEN_H_START
#define HTS211_HIDDEN_H_END
#endif                          /* __CPLUSPLUS */

HTS211_HIDDEN_H_START;

/* hts_engine libraries */
#include "HTS211_engine.h"

/*  -------------------------- misc -------------------------------  */

#if !defined(WORDS_BIGENDIAN) && !defined(WORDS_LITTLEENDIAN)
#define WORDS_LITTLEENDIAN
#endif                          /* !WORDS_BIGENDIAN && !WORDS_LITTLEENDIAN */
#if defined(WORDS_BIGENDIAN) && defined(WORDS_LITTLEENDIAN)
#undef WORDS_BIGENDIAN
#endif                          /* WORDS_BIGENDIAN && WORDS_LITTLEENDIAN */

#define HTS211_MAXBUFLEN 1024

/* HTS211_error: output error message */
void HTS211_error(const int error, char *message, ...);

/* HTS211_get_fp: wrapper for fopen */
FILE *HTS211_get_fp(const char *name, const char *opt);

/* HTS211_get_pattern_token: get pattern token */
void HTS211_get_pattern_token(FILE * fp, char *buff);

/* HTS211_get_token: get token (separator are space,tab,line break) */
HTS211_Boolean HTS211_get_token(FILE * fp, char *buff);

/* HTS211_get_token_from_string: get token from string (separator are space,tab,line break) */
HTS211_Boolean HTS211_get_token_from_string(const char *string, int *index, char *buff);

/* HTS211_fwrite_little_endian: fwrite with byteswap */
int HTS211_fwrite_little_endian(void *p, const int size, const int num, FILE * fp);

/* HTS211_fread_big_endiana: fread with byteswap */
int HTS211_fread_big_endian(void *p, const int size, const int num, FILE * fp);

/* HTS211_calloc: wrapper for calloc */
char *HTS211_calloc(const size_t num, const size_t size);

/* HTS211_strdup: wrapper for strdup */
char *HTS211_strdup(const char *string);

/* HTS211_calloc_matrix: allocate double matrix */
double **HTS211_alloc_matrix(const int x, const int y);

/* HTS211_free_matrix: free double matrix */
void HTS211_free_matrix(double **p, const int x);

/* HTS211_Free: wrapper for free */
void HTS211_free(void *p);

/*  -------------------------- pstream ----------------------------  */

/* check variance in finv() */
#define INFTY   ((double) 1.0e+38)
#define INFTY2  ((double) 1.0e+19)
#define INVINF  ((double) 1.0e-38)
#define INVINF2 ((double) 1.0e-19)

/* GV */
#define STEPINIT 0.1
#define STEPDEC  0.5
#define STEPINC  1.2
#define W1       1.0
#define W2       1.0
#define GV_MAX_ITERATION 5

/*  -------------------------- audio ------------------------------  */

/* HTS211_Audio_open: open audio device */
void HTS211_Audio_open(HTS211_Audio * as, int sampling_rate, int max_buff_size);

/* HTS211_Audio_write: send data to audio device */
void HTS211_Audio_write(HTS211_Audio * as, short data);

/* HTS211_Audio_close: close audio device */
void HTS211_Audio_close(HTS211_Audio * as);

/*  -------------------------- vocoder ----------------------------  */

#ifndef PI
#define PI  3.14159265358979323846
#endif                          /* !PI */
#ifndef PI2
#define PI2 6.28318530717958647692
#endif                          /* !PI2 */

#define RANDMAX 32767

#define IPERIOD 1
#define SEED    1
#define B0      0x00000001
#define B28     0x10000000
#define B31     0x80000000
#define B31_    0x7fffffff
#define Z       0x00000000

#ifdef HTS211_EMBEDDED
#define GAUSS     FALSE
#define PADEORDER 4             /* pade order (for MLSA filter) */
#define IRLENG    64            /* length of impulse response */
#else
#define GAUSS     TRUE
#define PADEORDER 5
#define IRLENG    96
#endif                          /* HTS211_EMBEDDED */

/* for MGLSA filter */
#define NORMFLG1 TRUE
#define NORMFLG2 FALSE
#define MULGFLG1 TRUE
#define MULGFLG2 FALSE
#define NGAIN    FALSE

HTS211_HIDDEN_H_END;

#endif                          /* !HTS_HIDDEN_H */
