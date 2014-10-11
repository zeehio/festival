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

#ifndef HTS211_ENGINE_H
#define HTS211_ENGINE_H

#ifdef __cplusplus
#define HTS211_ENGINE_H_START extern "C" {
#define HTS211_ENGINE_H_END   }
#else
#define HTS211_ENGINE_H_START
#define HTS211_ENGINE_H_END
#endif                          /* __CPLUSPLUS */

HTS211_ENGINE_H_START;

#include <stdio.h>

/*  ------------------------ copyright ----------------------------  */

#ifdef PACKAGE_VERSION
#define HTS211_VERSION   PACKAGE_VERSION
#else
#define HTS211_VERSION   "1.04"
#endif
#define HTS211_URL       "http://hts-engine.sourceforge.net/"
#define HTS211_COPYRIGHT "2001-2010  Nagoya Institute of Technology", \
                      "2001-2008  Tokyo Institute of Technology"
#define HTS211_NCOPYRIGHT 2

/* HTS211_show_copyright: write copyright to file pointer */
void HTS211_show_copyright(FILE * fp);

/* HTS211_get_copyright: write copyright to string */
void HTS211_get_copyright(char *str);

/*  -------------------------- common -----------------------------  */

typedef int HTS211_Boolean;
#ifndef TRUE
#define TRUE  1
#endif                          /* !TRUE */
#ifndef FALSE
#define FALSE 0
#endif                          /* !FALSE */

#define ZERO  1.0e-10           /* ~(0) */
#define LZERO (-1.0e+10)        /* ~log(0) */
#define LTPI  1.83787706640935  /* log(2*PI) */

/*  -------------------------- model ------------------------------  */

/* HTS211_Window: Window coefficients to calculate dynamic features. */
typedef struct _HTS211_Window {
   int size;                    /* number of windows (static + deltas) */
   int *l_width;                /* left width of windows */
   int *r_width;                /* right width of windows */
   double **coefficient;        /* window coefficient */
   int max_width;               /* maximum width of windows */
} HTS211_Window;

/* HTS211_Pattern: List of patterns in a question and a tree. */
typedef struct _HTS211_Pattern {
   char *string;                /* pattern string */
   struct _HTS211_Pattern *next;   /* pointer to the next pattern */
} HTS211_Pattern;

/* HTS211_Question: List of questions in a tree. */
typedef struct _HTS211_Question {
   char *string;                /* name of this question */
   HTS211_Pattern *head;           /* pointer to the head of pattern list */
   struct _HTS211_Question *next;  /* pointer to the next question */
} HTS211_Question;

/* HTS211_Node: List of tree nodes in a tree. */
typedef struct _HTS211_Node {
   int index;                   /* index of this node */
   int pdf;                     /* index of PDF for this node  ( leaf node only ) */
   struct _HTS211_Node *yes;       /* pointer to its child node (yes) */
   struct _HTS211_Node *no;        /* pointer to its child node (no) */
   struct _HTS211_Node *next;      /* pointer to the next node */
   HTS211_Question *quest;         /* question applied at this node */
} HTS211_Node;

/* HTS211_Tree: List of decision trees in a model. */
typedef struct _HTS211_Tree {
   HTS211_Pattern *head;           /* pointer to the head of pattern list for this tree */
   struct _HTS211_Tree *next;      /* pointer to next tree */
   HTS211_Node *root;              /* root node of this tree */
   int state;                   /* state index of this tree */
} HTS211_Tree;

/* HTS211_Model: Set of PDFs, decision trees and questions. */
typedef struct _HTS211_Model {
   int vector_length;           /* vector length (include static and dynamic features) */
   int ntree;                   /* # of trees */
   int *npdf;                   /* # of PDFs at each tree */
   double ***pdf;               /* PDFs */
   HTS211_Tree *tree;              /* pointer to the list of trees */
   HTS211_Question *question;      /* pointer to the list of questions */
} HTS211_Model;

/* HTS211_Stream: Set of models and a window. */
typedef struct _HTS211_Stream {
   int vector_length;           /* vector_length (include static and dynamic features) */
   HTS211_Model *model;            /* models */
   HTS211_Window window;           /* window coefficients */
   HTS211_Boolean msd_flag;        /* flag for MSD */
   int interpolation_size;      /* # of models for interpolation */
} HTS211_Stream;

/* HTS211_ModelSet: Set of duration models, HMMs and GV models. */
typedef struct _HTS211_ModelSet {
   HTS211_Stream duration;         /* duration PDFs and trees */
   HTS211_Stream *stream;          /* parameter PDFs, trees and windows */
   HTS211_Stream *gv;              /* GV PDFs */
   HTS211_Model gv_switch;         /* GV switch */
   int nstate;                  /* # of HMM states */
   int nstream;                 /* # of stream */
} HTS211_ModelSet;

/*  ----------------------- model method --------------------------  */

/* HTS211_ModelSet_initialize: initialize model set */
void HTS211_ModelSet_initialize(HTS211_ModelSet * ms, int nstream);

/* HTS211_ModelSet_load_duration: load duration model and number of state */
void HTS211_ModelSet_load_duration(HTS211_ModelSet * ms, FILE ** pdf_fp,
                                FILE ** tree_fp, int interpolation_size);

/* HTS211_ModelSet_load_parameter: load parameter model */
void HTS211_ModelSet_load_parameter(HTS211_ModelSet * ms, FILE ** pdf_fp,
                                 FILE ** tree_fp, FILE ** win_fp,
                                 int stream_index, HTS211_Boolean msd_flag,
                                 int window_size, int interpolation_size);

/* HTS211_ModelSet_load_gv: load GV model */
void HTS211_ModelSet_load_gv(HTS211_ModelSet * ms, FILE ** pdf_fp, FILE ** tree_fp,
                          int stream_index, int interpolation_size);

/* HTS211_ModelSet_have_gv_tree: if context-dependent GV is used, return true */
HTS211_Boolean HTS211_ModelSet_have_gv_tree(HTS211_ModelSet * ms, int stream_index);

/* HTS211_ModelSet_load_gv_switch: load GV switch */
void HTS211_ModelSet_load_gv_switch(HTS211_ModelSet * ms, FILE * fp);

/* HTS211_ModelSet_have_gv_switch: if GV switch is used, return true */
HTS211_Boolean HTS211_ModelSet_have_gv_switch(HTS211_ModelSet * ms);

/* HTS211_ModelSet_get_nstate: get number of state */
int HTS211_ModelSet_get_nstate(HTS211_ModelSet * ms);

/* HTS211_ModelSet_get_nstream: get number of stream */
int HTS211_ModelSet_get_nstream(HTS211_ModelSet * ms);

/* HTS211_ModelSet_get_vector_length: get vector length */
int HTS211_ModelSet_get_vector_length(HTS211_ModelSet * ms, int stream_index);

/* HTS211_ModelSet_is_msd: get MSD flag */
HTS211_Boolean HTS211_ModelSet_is_msd(HTS211_ModelSet * ms, int stream_index);

/* HTS211_ModelSet_get_window_size: get dynamic window size */
int HTS211_ModelSet_get_window_size(HTS211_ModelSet * ms, int stream_index);

/* HTS211_ModelSet_get_window_left_width: get left width of dynamic window */
int HTS211_ModelSet_get_window_left_width(HTS211_ModelSet * ms, int stream_index,
                                       int window_index);

/* HTS211_ModelSet_get_window_right_width: get right width of dynamic window */
int HTS211_ModelSet_get_window_right_width(HTS211_ModelSet * ms, int stream_index,
                                        int window_index);

/* HTS211_ModelSet_get_window_coefficient: get coefficient of dynamic window */
double HTS211_ModelSet_get_window_coefficient(HTS211_ModelSet * ms, int stream_index,
                                           int window_index,
                                           int coefficient_index);

/* HTS211_ModelSet_get_window_max_width: get max width of dynamic window */
int HTS211_ModelSet_get_window_max_width(HTS211_ModelSet * ms, int stream_index);

/* HTS211_ModelSet_get_duration_interpolation_size: get interpolation size (duration model) */
int HTS211_ModelSet_get_duration_interpolation_size(HTS211_ModelSet * ms);

/* HTS211_ModelSet_get_parameter_interpolation_size: get interpolation size (parameter model) */
int HTS211_ModelSet_get_parameter_interpolation_size(HTS211_ModelSet * ms,
                                                  int stream_index);

/* HTS211_ModelSet_get_gv_interpolation_size: get interpolation size (GV model) */
int HTS211_ModelSet_get_gv_interpolation_size(HTS211_ModelSet * ms, int stream_index);

/* HTS211_ModelSet_use_gv: get GV flag */
HTS211_Boolean HTS211_ModelSet_use_gv(HTS211_ModelSet * ms, int index);

/* HTS211_ModelSet_get_duration_index: get index of duration tree and PDF */
void HTS211_ModelSet_get_duration_index(HTS211_ModelSet * ms, char *string,
                                     int *tree_index, int *pdf_index,
                                     int interpolation_index);

/* HTS211_ModelSet_get_duration: get duration using interpolation weight */
void HTS211_ModelSet_get_duration(HTS211_ModelSet * ms, char *string, double *mean,
                               double *vari, double *iw);

/* HTS211_ModelSet_get_parameter_index: get index of parameter tree and PDF */
void HTS211_ModelSet_get_parameter_index(HTS211_ModelSet * ms, char *string,
                                      int *tree_index, int *pdf_index,
                                      int stream_index, int state_index,
                                      int interpolation_index);

/* HTS211_ModelSet_get_parameter: get parameter using interpolation weight */
void HTS211_ModelSet_get_parameter(HTS211_ModelSet * ms, char *string, double *mean,
                                double *vari, double *msd, int stream_index,
                                int state_index, double *iw);

/* HTS211_ModelSet_get_gv: get GV using interpolation weight */
void HTS211_ModelSet_get_gv(HTS211_ModelSet * ms, char *string, double *mean,
                         double *vari, int stream_index, double *iw);

/* HTS211_ModelSet_get_gv_switch: get GV switch */
HTS211_Boolean HTS211_ModelSet_get_gv_switch(HTS211_ModelSet * ms, char *string);

/* HTS211_ModelSet_clear: free model set */
void HTS211_ModelSet_clear(HTS211_ModelSet * ms);

/*  -------------------------- label ------------------------------  */

/* HTS211_LabelString: individual label string with time information */
typedef struct _HTS211_LabelString {
   struct _HTS211_LabelString *next;       /* pointer to next label string */
   char *name;                  /* label string */
   double start;                /* start frame specified in the given label */
   double end;                  /* end frame specified in the given label */
} HTS211_LabelString;

/* HTS211_Label: list of label strings */
typedef struct _HTS211_Label {
   HTS211_LabelString *head;       /* pointer to the head of label string */
   int size;                    /* # of label strings */
   HTS211_Boolean frame_flag;      /* flag for frame length modification */
   double speech_speed;         /* speech speed rate */
} HTS211_Label;

/*  ----------------------- label method --------------------------  */

/* HTS211_Label_initialize: initialize label */
void HTS211_Label_initialize(HTS211_Label * label);

/* HTS211_Label_load_from_fn: load label from file name */
void HTS211_Label_load_from_fn(HTS211_Label * label, int sampling_rate, int fperiod,
                            char *fn);

/* HTS211_Label_load_from_fp: load label list from file pointer */
void HTS211_Label_load_from_fp(HTS211_Label * label, int sampling_rate, int fperiod,
                            FILE * fp);

/* HTS211_Label_load_from_string: load label from string */
void HTS211_Label_load_from_string(HTS211_Label * label, int sampling_rate,
                                int fperiod, char *data);

/* HTS211_Label_load_from_string_list: load label list from string list */
void HTS211_Label_load_from_string_list(HTS211_Label * label, int sampling_rate,
                                     int fperiod, const char **data, int size);

/* HTS211_Label_set_speech_speed: set speech speed rate */
void HTS211_Label_set_speech_speed(HTS211_Label * label, double f);

/* HTS211_Label_set_frame_specified_flag: set frame specified flag */
void HTS211_Label_set_frame_specified_flag(HTS211_Label * label, HTS211_Boolean i);

/* HTS211_Label_get_size: get number of label string */
int HTS211_Label_get_size(HTS211_Label * label);

/* HTS211_Label_get_string: get label string */
char *HTS211_Label_get_string(HTS211_Label * label, int string_index);

/* HTS211_Label_get_frame_specified_flag: get frame specified flag */
HTS211_Boolean HTS211_Label_get_frame_specified_flag(HTS211_Label * label);

/* HTS211_Label_get_start_frame: get start frame */
double HTS211_Label_get_start_frame(HTS211_Label * label, int string_index);

/* HTS211_Label_get_end_frame: get end frame */
double HTS211_Label_get_end_frame(HTS211_Label * label, int string_index);

/* HTS211_Label_get_speech_speed: get speech speed rate */
double HTS211_Label_get_speech_speed(HTS211_Label * label);

/* HTS211_Label_clear: free label */
void HTS211_Label_clear(HTS211_Label * label);

/*  -------------------------- sstream ----------------------------  */

/* HTS211_SStream: individual state stream */
typedef struct _HTS211_SStream {
   int vector_length;           /* vector length (include static and dynamic features) */
   double **mean;               /* mean vector sequence */
   double **vari;               /* variance vector sequence */
   double *msd;                 /* MSD parameter sequence */
   int win_size;                /* # of windows (static + deltas) */
   int *win_l_width;            /* left width of windows */
   int *win_r_width;            /* right width of windows */
   double **win_coefficient;    /* window cofficients */
   int win_max_width;           /* maximum width of windows */
   double *gv_mean;             /* mean vector of GV */
   double *gv_vari;             /* variance vector of GV */
   HTS211_Boolean *gv_switch;      /* GV flag sequence */
} HTS211_SStream;

/* HTS211_SStreamSet: set of state stream */
typedef struct _HTS211_SStreamSet {
   HTS211_SStream *sstream;        /* state streams */
   int nstream;                 /* # of streams */
   int nstate;                  /* # of states */
   int *duration;               /* duration sequence */
   int total_state;             /* total state */
   int total_frame;             /* total frame */
} HTS211_SStreamSet;

/*  ----------------------- sstream method ------------------------  */

/* HTS211_SStreamSet_initialize: initialize state stream set */
void HTS211_SStreamSet_initialize(HTS211_SStreamSet * sss);

/* HTS211_SStreamSet_create: parse label and determine state duration */
void HTS211_SStreamSet_create(HTS211_SStreamSet * sss, HTS211_ModelSet * ms,
                           HTS211_Label * label, double *duration_iw,
                           double **parameter_iw, double **gv_iw);

/* HTS211_SStreamSet_get_nstream: get number of stream */
int HTS211_SStreamSet_get_nstream(HTS211_SStreamSet * sss);

/* HTS211_SStreamSet_get_vector_length: get vector length */
int HTS211_SStreamSet_get_vector_length(HTS211_SStreamSet * sss, int stream_index);

/* HTS211_SStreamSet_is_msd: get MSD flag */
HTS211_Boolean HTS211_SStreamSet_is_msd(HTS211_SStreamSet * sss, int stream_index);

/* HTS211_SStreamSet_get_total_state: get total number of state */
int HTS211_SStreamSet_get_total_state(HTS211_SStreamSet * sss);

/* HTS211_SStreamSet_get_total_frame: get total number of frame */
int HTS211_SStreamSet_get_total_frame(HTS211_SStreamSet * sss);

/* HTS211_SStreamSet_get_msd: get msd parameter */
double HTS211_SStreamSet_get_msd(HTS211_SStreamSet * sss, int stream_index,
                              int state_index);

/* HTS211_SStreamSet_window_size: get dynamic window size */
int HTS211_SStreamSet_get_window_size(HTS211_SStreamSet * sss, int stream_index);

/* HTS211_SStreamSet_get_window_left_width: get left width of dynamic window */
int HTS211_SStreamSet_get_window_left_width(HTS211_SStreamSet * sss, int stream_index,
                                         int window_index);

/* HTS211_SStreamSet_get_window_right_width: get right width of dynamic window */
int HTS211_SStreamSet_get_window_right_width(HTS211_SStreamSet * sss,
                                          int stream_index, int window_index);

/* HTS211_SStreamSet_get_window_coefficient: get coefficient of dynamic window */
double HTS211_SStreamSet_get_window_coefficient(HTS211_SStreamSet * sss,
                                             int stream_index, int window_index,
                                             int coefficient_index);

/* HTS211_SStreamSet_get_window_max_width: get max width of dynamic window */
int HTS211_SStreamSet_get_window_max_width(HTS211_SStreamSet * sss, int stream_index);

/* HTS211_SStreamSet_use_gv: get GV flag */
HTS211_Boolean HTS211_SStreamSet_use_gv(HTS211_SStreamSet * sss, int stream_index);

/* HTS211_SStreamSet_get_duration: get state duration */
int HTS211_SStreamSet_get_duration(HTS211_SStreamSet * sss, int state_index);

/* HTS211_SStreamSet_get_mean: get mean parameter */
double HTS211_SStreamSet_get_mean(HTS211_SStreamSet * sss, int stream_index,
                               int state_index, int vector_index);

/* HTS211_SStreamSet_set_mean: set mean parameter */
void HTS211_SStreamSet_set_mean(HTS211_SStreamSet * sss, int stream_index,
                             int state_index, int vector_index, double f);

/* HTS211_SStreamSet_get_vari: get variance parameter */
double HTS211_SStreamSet_get_vari(HTS211_SStreamSet * sss, int stream_index,
                               int state_index, int vector_index);

/* HTS211_SStreamSet_set_vari: set variance parameter */
void HTS211_SStreamSet_set_vari(HTS211_SStreamSet * sss, int stream_index,
                             int state_index, int vector_index, double f);

/* HTS211_SStreamSet_get_gv_mean: get GV mean parameter */
double HTS211_SStreamSet_get_gv_mean(HTS211_SStreamSet * sss, int stream_index,
                                  int vector_index);

/* HTS211_SStreamSet_get_gv_mean: get GV variance parameter */
double HTS211_SStreamSet_get_gv_vari(HTS211_SStreamSet * sss, int stream_index,
                                  int vector_index);

/* HTS211_SStreamSet_set_gv_switch: set GV switch */
void HTS211_SStreamSet_set_gv_switch(HTS211_SStreamSet * sss, int stream_index,
                                  int state_index, HTS211_Boolean i);

/* HTS211_SStreamSet_get_gv_switch: get GV switch */
HTS211_Boolean HTS211_SStreamSet_get_gv_switch(HTS211_SStreamSet * sss, int stream_index,
                                         int state_index);

/* HTS211_SStreamSet_clear: free state stream set */
void HTS211_SStreamSet_clear(HTS211_SStreamSet * sss);

/*  -------------------------- pstream ----------------------------  */

/* HTS211_SMatrices: Matrices/Vectors used in the speech parameter generation algorithm. */
typedef struct _HTS211_SMatrices {
   double **mean;               /* mean vector sequence */
   double **ivar;               /* inverse diag variance sequence */
   double *g;                   /* vector used in the forward substitution */
   double **wuw;                /* W' U^-1 W  */
   double *wum;                 /* W' U^-1 mu */
} HTS211_SMatrices;

/* HTS211_PStream: Individual PDF stream. */
typedef struct _HTS211_PStream {
   int vector_length;           /* vector length (include static and dynamic features) */
   int static_length;           /* static features length */
   int length;                  /* stream length */
   int width;                   /* width of dynamic window */
   double **par;                /* output parameter vector */
   HTS211_SMatrices sm;            /* matrices for parameter generation */
   int win_size;                /* # of windows (static + deltas) */
   int *win_l_width;            /* left width of windows */
   int *win_r_width;            /* right width of windows */
   double **win_coefficient;    /* window coefficients */
   HTS211_Boolean *msd_flag;       /* Boolean sequence for MSD */
   double *gv_mean;             /* mean vector of GV */
   double *gv_vari;             /* variance vector of GV */
   HTS211_Boolean *gv_switch;      /* GV flag sequence */
   int gv_length;               /* frame length for GV calculation */
} HTS211_PStream;

/* HTS211_PStreamSet: Set of PDF streams. */
typedef struct _HTS211_PStreamSet {
   HTS211_PStream *pstream;        /* PDF streams */
   int nstream;                 /* # of PDF streams */
   int total_frame;             /* total frame */
} HTS211_PStreamSet;

/*  ----------------------- pstream method ------------------------  */

/* HTS211_PStreamSet_initialize: initialize parameter stream set */
void HTS211_PStreamSet_initialize(HTS211_PStreamSet * pss);

/* HTS211_PStreamSet_create: parameter generation using GV weight */
void HTS211_PStreamSet_create(HTS211_PStreamSet * pss, HTS211_SStreamSet * sss,
                           double *msd_threshold, double *gv_weight);

/* HTS211_PStreamSet_get_nstream: get number of stream */
int HTS211_PStreamSet_get_nstream(HTS211_PStreamSet * pss);

/* HTS211_PStreamSet_get_static_length: get static features length */
int HTS211_PStreamSet_get_static_length(HTS211_PStreamSet * pss, int stream_index);

/* HTS211_PStreamSet_get_total_frame: get total number of frame */
int HTS211_PStreamSet_get_total_frame(HTS211_PStreamSet * pss);

/* HTS211_PStreamSet_get_parameter: get parameter */
double HTS211_PStreamSet_get_parameter(HTS211_PStreamSet * pss, int stream_index,
                                    int frame_index, int vector_index);

/* HTS211_PStreamSet_get_parameter_vector: get parameter vector */
double *HTS211_PStreamSet_get_parameter_vector(HTS211_PStreamSet * pss,
                                            int stream_index, int frame_index);

/* HTS211_PStreamSet_get_msd_flag: get generated MSD flag per frame */
HTS211_Boolean HTS211_PStreamSet_get_msd_flag(HTS211_PStreamSet * pss, int stream_index,
                                        int frame_index);

/* HTS211_PStreamSet_is_msd: get MSD flag */
HTS211_Boolean HTS211_PStreamSet_is_msd(HTS211_PStreamSet * pss, int stream_index);

/* HTS211_PStreamSet_clear: free parameter stream set */
void HTS211_PStreamSet_clear(HTS211_PStreamSet * pss);

/*  -------------------------- gstream ----------------------------  */

#ifndef HTS211_EMBEDDED
/* HTS211_GStream: Generated parameter stream. */
typedef struct _HTS211_GStream {
   int static_length;           /* static features length */
   double **par;                /* generated parameter */
} HTS211_GStream;
#endif                          /* !HTS211_EMBEDDED */

/* HTS211_GStreamSet: Set of generated parameter stream. */
typedef struct _HTS211_GStreamSet {
   int total_nsample;           /* total sample */
   int total_frame;             /* total frame */
   int nstream;                 /* # of streams */
#ifndef HTS211_EMBEDDED
   HTS211_GStream *gstream;        /* generated parameter streams */
#endif                          /* !HTS211_EMBEDDED */
   short *gspeech;              /* generated speech */
} HTS211_GStreamSet;

/*  ----------------------- gstream method ------------------------  */

/* HTS211_GStreamSet_initialize: initialize generated parameter stream set */
void HTS211_GStreamSet_initialize(HTS211_GStreamSet * gss);

/* HTS211_GStreamSet_create: generate speech */
void HTS211_GStreamSet_create(HTS211_GStreamSet * gss, HTS211_PStreamSet * pss,
                           int stage, HTS211_Boolean use_log_gain,
                           int sampling_rate, int fperiod, double alpha,
                           double beta, HTS211_Boolean * stop, double volume,
                           int audio_buff_size);

/* HTS211_GStreamSet_get_total_nsample: get total number of sample */
int HTS211_GStreamSet_get_total_nsample(HTS211_GStreamSet * gss);

/* HTS211_GStreamSet_get_total_frame: get total number of frame */
int HTS211_GStreamSet_get_total_frame(HTS211_GStreamSet * gss);

#ifndef HTS211_EMBEDDED
/* HTS211_GStreamSet_get_static_length: get static features length */
int HTS211_GStreamSet_get_static_length(HTS211_GStreamSet * gss, int stream_index);
#endif                          /* !HTS211_EMBEDDED */

/* HTS211_GStreamSet_get_speech: get synthesized speech parameter */
short HTS211_GStreamSet_get_speech(HTS211_GStreamSet * gss, int sample_index);

#ifndef HTS211_EMBEDDED
/* HTS211_GStreamSet_get_parameter: get generated parameter */
double HTS211_GStreamSet_get_parameter(HTS211_GStreamSet * gss, int stream_index,
                                    int frame_index, int vector_index);
#endif                          /* !HTS211_EMBEDDED */

/* HTS211_GStreamSet_clear: free generated parameter stream set */
void HTS211_GStreamSet_clear(HTS211_GStreamSet * gss);

/*  -------------------------- engine -----------------------------  */

/* HTS211_Global: Global settings. */
typedef struct _HTS211_Global {
   int stage;                   /* Gamma=-1/stage: if stage=0 then Gamma=0 */
   HTS211_Boolean use_log_gain;    /* log gain flag (for LSP) */
   int sampling_rate;           /* sampling rate */
   int fperiod;                 /* frame period */
   double alpha;                /* all-pass constant */
   double beta;                 /* postfiltering coefficient */
   int audio_buff_size;         /* audio buffer size (for audio device) */
   double *msd_threshold;       /* MSD thresholds */
   double *duration_iw;         /* weights for duration interpolation */
   double **parameter_iw;       /* weights for parameter interpolation */
   double **gv_iw;              /* weights for GV interpolation */
   double *gv_weight;           /* GV weights */
   HTS211_Boolean stop;            /* stop flag */
   double volume;               /* volume */
} HTS211_Global;

/* HTS211_Engine: Engine itself. */
typedef struct _HTS211_Engine {
   HTS211_Global global;           /* global settings */
   HTS211_ModelSet ms;             /* set of duration models, HMMs and GV models */
   HTS211_Label label;             /* label */
   HTS211_SStreamSet sss;          /* set of state streams */
   HTS211_PStreamSet pss;          /* set of PDF streams */
   HTS211_GStreamSet gss;          /* set of generated parameter streams */
} HTS211_Engine;

/*  ----------------------- engine method -------------------------  */

/* HTS211_Engine_initialize: initialize engine */
void HTS211_Engine_initialize(HTS211_Engine * engine, int nstream);

/* HTS211_engine_load_duration_from_fn: load duration pdfs ,trees and number of state from file names */
void HTS211_Engine_load_duration_from_fn(HTS211_Engine * engine, char **pdf_fn,
                                      char **tree_fn, int interpolation_size);

/* HTS211_Engine_load_duration_from_fp: load duration pdfs, trees and number of state from file pointers */
void HTS211_Engine_load_duration_from_fp(HTS211_Engine * engine, FILE ** pdf_fp,
                                      FILE ** tree_fp, int interpolation_size);

/* HTS211_Engine_load_parameter_from_fn: load parameter pdfs, trees and windows from file names */
void HTS211_Engine_load_parameter_from_fn(HTS211_Engine * engine, char **pdf_fn,
                                       char **tree_fn, char **win_fn,
                                       int stream_index, HTS211_Boolean msd_flag,
                                       int window_size, int interpolation_size);

/* HTS211_Engine_load_parameter_from_fp: load parameter pdfs, trees and windows from file pointers */
void HTS211_Engine_load_parameter_from_fp(HTS211_Engine * engine, FILE ** pdf_fp,
                                       FILE ** tree_fp, FILE ** win_fp,
                                       int stream_index, HTS211_Boolean msd_flag,
                                       int window_size, int interpolation_size);

/* HTS211_Engine_load_gv_from_fn: load GV pdfs and trees from file names */
void HTS211_Engine_load_gv_from_fn(HTS211_Engine * engine, char **pdf_fn,
                                char **tree_fn, int stream_index,
                                int interpolation_size);

/* HTS211_Engine_load_gv_from_fp: load GV pdfs and trees from file pointers */
void HTS211_Engine_load_gv_from_fp(HTS211_Engine * engine, FILE ** pdf_fp,
                                FILE ** tree_fp, int stream_index,
                                int interpolation_size);

/* HTS211_Engine_load_gv_switch_from_fn: load GV switch from file names */
void HTS211_Engine_load_gv_switch_from_fn(HTS211_Engine * engine, char *fn);

/* HTS211_Engine_load_gv_switch_from_fp: load GV switch from file pointers */
void HTS211_Engine_load_gv_switch_from_fp(HTS211_Engine * engine, FILE * fp);

/* HTS211_Engine_set_sampling_rate: set sampling rate */
void HTS211_Engine_set_sampling_rate(HTS211_Engine * engine, int i);

/* HTS211_Engine_get_sampling_rate: get sampling rate */
int HTS211_Engine_get_sampling_rate(HTS211_Engine * engine);

/* HTS211_Engine_set_fperiod: set frame shift */
void HTS211_Engine_set_fperiod(HTS211_Engine * engine, int i);

/* HTS211_Engine_get_fperiod: get frame shift */
int HTS211_Engine_get_fperiod(HTS211_Engine * engine);

/* HTS211_Engine_set_alpha: set alpha */
void HTS211_Engine_set_alpha(HTS211_Engine * engine, double f);

/* HTS211_Engine_set_gamma: set gamma (Gamma=-1/i: if i=0 then Gamma=0) */
void HTS211_Engine_set_gamma(HTS211_Engine * engine, int i);

/* HTS211_Engine_set_log_gain: set log gain flag (for LSP) */
void HTS211_Engine_set_log_gain(HTS211_Engine * engine, HTS211_Boolean i);

/* HTS211_Engine_set_beta: set beta */
void HTS211_Engine_set_beta(HTS211_Engine * engine, double f);

/* HTS211_Engine_set_audio_buff_size: set audio buffer size */
void HTS211_Engine_set_audio_buff_size(HTS211_Engine * engine, int i);

/* HTS211_Engine_get_audio_buff_size: get audio buffer size */
int HTS211_Engine_get_audio_buff_size(HTS211_Engine * engine);

/* HTS211_Egnine_set_msd_threshold: set MSD threshold */
void HTS211_Engine_set_msd_threshold(HTS211_Engine * engine, int stream_index,
                                  double f);

/* HTS211_Engine_set_duration_interpolation_weight: set interpolation weight for duration */
void HTS211_Engine_set_duration_interpolation_weight(HTS211_Engine * engine,
                                                  int interpolation_index,
                                                  double f);

/* HTS211_Engine_set_parameter_interpolation_weight: set interpolation weight for parameter */
void HTS211_Engine_set_parameter_interpolation_weight(HTS211_Engine * engine,
                                                   int stream_index,
                                                   int interpolation_index,
                                                   double f);

/* HTS211_Engine_set_gv_interpolation_weight: set interpolation weight for GV */
void HTS211_Engine_set_gv_interpolation_weight(HTS211_Engine * engine,
                                            int stream_index,
                                            int interpolation_index, double f);

/* HTS211_Engine_set_gv_weight: set GV weight */
void HTS211_Engine_set_gv_weight(HTS211_Engine * engine, int stream_index, double f);

/* HTS211_Engine_set_stop_flag: set stop flag */
void HTS211_Engine_set_stop_flag(HTS211_Engine * engine, HTS211_Boolean b);

/* HTS211_Engine_set_volume: set volume */
void HTS211_Engine_set_volume(HTS211_Engine * engine, double f);

/* HTS211_Engine_get_total_state: get total number of state */
int HTS211_Engine_get_total_state(HTS211_Engine * engine);

/* HTS211_Engine_set_state_mean: set mean value of state */
void HTS211_Engine_set_state_mean(HTS211_Engine * engine, int stream_index,
                               int state_index, int vector_index, double f);

/* HTS211_Engine_get_state_mean: get mean value of state */
double HTS211_Engine_get_state_mean(HTS211_Engine * engine, int stream_index,
                                 int state_index, int vector_index);

/* HTS211_Engine_get_state_duration: get state duration */
int HTS211_Engine_get_state_duration(HTS211_Engine * engine, int state_index);

/* HTS211_Engine_get_nstate: get number of state */
int HTS211_Engine_get_nstate(HTS211_Engine * engine);

/* HTS211_Engine_load_label_from_fn: load label from file pointer */
void HTS211_Engine_load_label_from_fn(HTS211_Engine * engine, char *fn);

/* HTS211_Engine_load_label_from_fp: load label from file name */
void HTS211_Engine_load_label_from_fp(HTS211_Engine * engine, FILE * fp);

/* HTS211_Engine_load_label_from_string: load label from string */
void HTS211_Engine_load_label_from_string(HTS211_Engine * engine, char *data);

/* HTS211_Engine_load_label_from_string_list: load label from string list */
void HTS211_Engine_load_label_from_string_list(HTS211_Engine * engine, const char **data,
                                            int size);

/* HTS211_Engine_create_sstream: parse label and determine state duration */
void HTS211_Engine_create_sstream(HTS211_Engine * engine);

/* HTS211_Engine_create_pstream: generate speech parameter vector sequence */
void HTS211_Engine_create_pstream(HTS211_Engine * engine);

/* HTS211_Engine_create_gstream: synthesis speech */
void HTS211_Engine_create_gstream(HTS211_Engine * engine);

/* HTS211_Engine_save_information: output trace information */
void HTS211_Engine_save_information(HTS211_Engine * engine, FILE * fp);

/* HTS211_Engine_save_label: output label with time */
void HTS211_Engine_save_label(HTS211_Engine * engine, FILE * fp);

#ifndef HTS211_EMBEDDED
/* HTS211_Engine_save_generated_parameter: output generated parameter */
void HTS211_Engine_save_generated_parameter(HTS211_Engine * engine, FILE * fp,
                                         int stream_index);
#endif                          /* !HTS211_EMBEDDED */

/* HTS211_Engine_save_generated_speech: output generated speech */
void HTS211_Engine_save_generated_speech(HTS211_Engine * engine, FILE * fp);

/* HTS211_Engine_save_riff: output RIFF format file */
void HTS211_Engine_save_riff(HTS211_Engine * engine, FILE * wavfp);

/* HTS211_Engine_refresh: free memory per one time synthesis */
void HTS211_Engine_refresh(HTS211_Engine * engine);

/* HTS211_Engine_clear: free engine */
void HTS211_Engine_clear(HTS211_Engine * engine);

/*  -------------------------- audio ------------------------------  */

#if !defined(AUDIO_PLAY_WINCE) && !defined(AUDIO_PLAY_WIN32) && !defined(AUDIO_PLAY_NONE)
#if defined(__WINCE__) || defined(_WINCE) || defined(_WINCE) || defined(__WINCE)
#define AUDIO_PLAY_WINCE
#elif defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define AUDIO_PLAY_WIN32
#else
#define AUDIO_PLAY_NONE
#endif                          /* WINCE || WIN32 */
#endif                          /* !AUDIO_PLAY_WINCE && !AUDIO_PLAY_WIN32 && !AUDIO_PLAY_NONE */

/* HTS211_Audio: For MS Windows (Windows Mobile) audio output device. */
#if defined (AUDIO_PLAY_WIN32) || defined(AUDIO_PLAY_WINCE)
#include<windows.h>
#include<mmsystem.h>
typedef struct _HTS211_Audio {
   HWAVEOUT hwaveout;           /* audio device handle */
   WAVEFORMATEX waveformatex;   /* wave formatex */
   short *buff;                 /* current buffer */
   int buff_size;               /* current buffer size */
   int which_buff;              /* double buffering flag */
   HTS211_Boolean now_buff_1;      /* double buffering flag */
   HTS211_Boolean now_buff_2;      /* double buffering flag */
   WAVEHDR buff_1;              /* buffer */
   WAVEHDR buff_2;              /* buffer */
   int max_buff_size;           /* buffer size of audio output device */
} HTS211_Audio;
#endif                          /* AUDIO_PLAY_WIN32 || AUDIO_PLAY_WINCE */

/* HTS211_Audio: For Linux, etc. */
#ifdef AUDIO_PLAY_NONE
typedef struct _HTS211_Audio {
   int i;                       /* make compiler happy */
} HTS211_Audio;
#endif                          /* AUDIO_PLAY_NONE */

/*  -------------------------- vocoder ----------------------------  */

/* HTS211_Vocoder: structure for setting of vocoder */
typedef struct _HTS211_Vocoder {
   int stage;                   /* Gamma=-1/stage: if stage=0 then Gamma=0 */
   double gamma;                /* Gamma */
   HTS211_Boolean use_log_gain;    /* log gain flag (for LSP) */
   int fprd;                    /* frame shift */
   int iprd;                    /* interpolation period */
   int seed;                    /* seed of random generator */
   unsigned long next;          /* temporary variable for random generator */
   HTS211_Boolean gauss;           /* flag to use Gaussian noise */
   double rate;                 /* sampling rate */
   double p1;                   /* used in excitation generation */
   double pc;                   /* used in excitation generation */
   double p;                    /* used in excitation generation */
   double inc;                  /* used in excitation generation */
   int sw;                      /* switch used in random generator */
   int x;                       /* excitation signal */
   HTS211_Audio *audio;            /* pointer for audio device */
   double *freqt_buff;          /* used in freqt */
   int freqt_size;              /* buffer size for freqt */
   double *spectrum2en_buff;    /* used in spectrum2en */
   int spectrum2en_size;        /* buffer size for spectrum2en */
   double r1, r2, s;            /* used in random generator */
   double *postfilter_buff;     /* used in postfiltering */
   int postfilter_size;         /* buffer size for postfiltering */
   double *c, *cc, *cinc, *d1;  /* used in the MLSA/MGLSA filter */
   double *pade;                /* used in mlsadf */
   double *lsp2lpc_buff;        /* used in lsp2lpc */
   int lsp2lpc_size;            /* buffer size of lsp2lpc */
   double *gc2gc_buff;          /* used in gc2gc */
   int gc2gc_size;              /* buffer size for gc2gc */
} HTS211_Vocoder;

/*  ----------------------- vocoder method ------------------------  */

/* HTS211_Vocoder_initialize: initialize vocoder */
void HTS211_Vocoder_initialize(HTS211_Vocoder * v, const int m, const int stage,
                            HTS211_Boolean use_log_gain, const int rate,
                            const int fperiod, int buff_size);

/* HTS211_Vocoder_synthesize: pulse/noise excitation and MLSA/MGLSA filster based waveform synthesis */
void HTS211_Vocoder_synthesize(HTS211_Vocoder * v, const int m, double lf0,
                            double *spectrum, double alpha, double beta,
                            double volume, short *rawdata);

/* HTS211_Vocoder_postfilter_mcp: postfilter for MCP */
void HTS211_Vocoder_postfilter_mcp(HTS211_Vocoder * v, double *mcp, const int m,
                                double alpha, double beta);

/* HTS211_Vocoder_clear: clear vocoder */
void HTS211_Vocoder_clear(HTS211_Vocoder * v);

HTS211_ENGINE_H_END;

#endif                          /* !HTS211_ENGINE_H */
