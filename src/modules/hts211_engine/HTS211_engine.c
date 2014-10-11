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

#ifndef HTS211_ENGINE_C
#define HTS211_ENGINE_C

#ifdef __cplusplus
#define HTS211_ENGINE_C_START extern "C" {
#define HTS211_ENGINE_C_END   }
#else
#define HTS211_ENGINE_C_START
#define HTS211_ENGINE_C_END
#endif                          /* __CPLUSPLUS */

HTS211_ENGINE_C_START;

#include <string.h>             /* for strcpy() */

/* hts_engine libraries */
#include "HTS211_hidden.h"

/* HTS211_Engine_initialize: initialize engine */
void HTS211_Engine_initialize(HTS211_Engine * engine, int nstream)
{
   int i;

   /* default value for control parameter */
   engine->global.stage = 0;
   engine->global.use_log_gain = FALSE;
   engine->global.sampling_rate = 16000;
   engine->global.fperiod = 80;
   engine->global.alpha = 0.42;
   engine->global.beta = 0.0;
   engine->global.audio_buff_size = 0;
   engine->global.msd_threshold =
       (double *) HTS211_calloc(nstream, sizeof(double));
   for (i = 0; i < nstream; i++)
      engine->global.msd_threshold[i] = 0.5;

   /* interpolation weight */
   engine->global.parameter_iw =
       (double **) HTS211_calloc(nstream, sizeof(double *));
   engine->global.gv_iw = (double **) HTS211_calloc(nstream, sizeof(double *));
   engine->global.duration_iw = NULL;
   for (i = 0; i < nstream; i++)
      engine->global.parameter_iw[i] = NULL;
   for (i = 0; i < nstream; i++)
      engine->global.gv_iw[i] = NULL;

   /* GV weight */
   engine->global.gv_weight = (double *) HTS211_calloc(nstream, sizeof(double));
   for (i = 0; i < nstream; i++)
      engine->global.gv_weight[i] = 1.0;

   /* stop flag */
   engine->global.stop = FALSE;
   /* volume */
   engine->global.volume = 1.0;

   /* initialize model set */
   HTS211_ModelSet_initialize(&engine->ms, nstream);
   /* initialize label list */
   HTS211_Label_initialize(&engine->label);
   /* initialize state sequence set */
   HTS211_SStreamSet_initialize(&engine->sss);
   /* initialize pstream set */
   HTS211_PStreamSet_initialize(&engine->pss);
   /* initialize gstream set */
   HTS211_GStreamSet_initialize(&engine->gss);
}

/* HTS211_Engine_load_duratin_from_fn: load duration pdfs, trees and number of state from file names */
void HTS211_Engine_load_duration_from_fn(HTS211_Engine * engine, char **pdf_fn,
                                      char **tree_fn, int interpolation_size)
{
   int i;
   FILE **pdf_fp, **tree_fp;

   pdf_fp = (FILE **) HTS211_calloc(interpolation_size, sizeof(FILE *));
   tree_fp = (FILE **) HTS211_calloc(interpolation_size, sizeof(FILE *));
   for (i = 0; i < interpolation_size; i++) {
      pdf_fp[i] = HTS211_get_fp(pdf_fn[i], "rb");
      tree_fp[i] = HTS211_get_fp(tree_fn[i], "r");
   }
   HTS211_Engine_load_duration_from_fp(engine, pdf_fp, tree_fp,
                                    interpolation_size);
   for (i = 0; i < interpolation_size; i++) {
      fclose(pdf_fp[i]);
      fclose(tree_fp[i]);
   }
   HTS211_free(pdf_fp);
   HTS211_free(tree_fp);
}

/* HTS211_Engine_load_duration_from_fp: load duration pdfs, trees and number of state from file pointers */
void HTS211_Engine_load_duration_from_fp(HTS211_Engine * engine, FILE ** pdf_fp,
                                      FILE ** tree_fp, int interpolation_size)
{
   int i;

   HTS211_ModelSet_load_duration(&engine->ms, pdf_fp, tree_fp, interpolation_size);
   engine->global.duration_iw =
       (double *) HTS211_calloc(interpolation_size, sizeof(double));
   for (i = 0; i < interpolation_size; i++)
      engine->global.duration_iw[i] = 1.0 / interpolation_size;
}

/* HTS211_Engine_load_parameter_from_fn: load parameter pdfs, trees and windows from file names */
void HTS211_Engine_load_parameter_from_fn(HTS211_Engine * engine, char **pdf_fn,
                                       char **tree_fn, char **win_fn,
                                       int stream_index, HTS211_Boolean msd_flag,
                                       int window_size, int interpolation_size)
{
   int i;
   FILE **pdf_fp, **tree_fp, **win_fp;

   pdf_fp = (FILE **) HTS211_calloc(interpolation_size, sizeof(FILE *));
   tree_fp = (FILE **) HTS211_calloc(interpolation_size, sizeof(FILE *));
   win_fp = (FILE **) HTS211_calloc(window_size, sizeof(FILE *));
   for (i = 0; i < interpolation_size; i++) {
      pdf_fp[i] = HTS211_get_fp(pdf_fn[i], "rb");
      tree_fp[i] = HTS211_get_fp(tree_fn[i], "r");
   }
   for (i = 0; i < window_size; i++)
      win_fp[i] = HTS211_get_fp(win_fn[i], "r");
   HTS211_Engine_load_parameter_from_fp(engine, pdf_fp, tree_fp, win_fp,
                                     stream_index, msd_flag,
                                     window_size, interpolation_size);
   for (i = 0; i < interpolation_size; i++) {
      fclose(pdf_fp[i]);
      fclose(tree_fp[i]);
   }
   for (i = 0; i < window_size; i++)
      fclose(win_fp[i]);
   HTS211_free(pdf_fp);
   HTS211_free(tree_fp);
   HTS211_free(win_fp);
}

/* HTS211_Engine_load_parameter_from_fp: load parameter pdfs, trees and windows from file pointers */
void HTS211_Engine_load_parameter_from_fp(HTS211_Engine * engine, FILE ** pdf_fp,
                                       FILE ** tree_fp, FILE ** win_fp,
                                       int stream_index, HTS211_Boolean msd_flag,
                                       int window_size, int interpolation_size)
{
   int i;

   HTS211_ModelSet_load_parameter(&engine->ms, pdf_fp, tree_fp, win_fp,
                               stream_index, msd_flag,
                               window_size, interpolation_size);
   engine->global.parameter_iw[stream_index] =
       (double *) HTS211_calloc(interpolation_size, sizeof(double));
   for (i = 0; i < interpolation_size; i++)
      engine->global.parameter_iw[stream_index][i] = 1.0 / interpolation_size;
}

/* HTS211_Engine_load_gv_from_fn: load GV pdfs and trees from file names */
void HTS211_Engine_load_gv_from_fn(HTS211_Engine * engine, char **pdf_fn,
                                char **tree_fn, int stream_index,
                                int interpolation_size)
{
   int i;
   FILE **pdf_fp, **tree_fp;

   pdf_fp = (FILE **) HTS211_calloc(interpolation_size, sizeof(FILE *));
   if (tree_fn)
      tree_fp = (FILE **) HTS211_calloc(interpolation_size, sizeof(FILE *));
   else
      tree_fp = NULL;
   for (i = 0; i < interpolation_size; i++) {
      pdf_fp[i] = HTS211_get_fp(pdf_fn[i], "rb");
      if (tree_fn) {
         if (tree_fn[i])
            tree_fp[i] = HTS211_get_fp(tree_fn[i], "r");
         else
            tree_fp[i] = NULL;
      }
   }
   HTS211_Engine_load_gv_from_fp(engine, pdf_fp, tree_fp, stream_index,
                              interpolation_size);
   for (i = 0; i < interpolation_size; i++) {
      fclose(pdf_fp[i]);
      if (tree_fp && tree_fp[i])
         fclose(tree_fp[i]);
   }
   HTS211_free(pdf_fp);
   if (tree_fp)
      HTS211_free(tree_fp);
}

/* HTS211_Engine_load_gv_from_fp: load GV pdfs and trees from file pointers */
void HTS211_Engine_load_gv_from_fp(HTS211_Engine * engine, FILE ** pdf_fp,
                                FILE ** tree_fp, int stream_index,
                                int interpolation_size)
{
   int i;

   HTS211_ModelSet_load_gv(&engine->ms, pdf_fp, tree_fp, stream_index,
                        interpolation_size);
   engine->global.gv_iw[stream_index] =
       (double *) HTS211_calloc(interpolation_size, sizeof(double));
   for (i = 0; i < interpolation_size; i++)
      engine->global.gv_iw[stream_index][i] = 1.0 / interpolation_size;
}

/* HTS211_Engine_load_gv_switch_from_fn: load GV switch from file name */
void HTS211_Engine_load_gv_switch_from_fn(HTS211_Engine * engine, char *fn)
{
   FILE *fp = HTS211_get_fp(fn, "r");

   HTS211_Engine_load_gv_switch_from_fp(engine, fp);
   fclose(fp);
}

/* HTS211_Engine_load_gv_switch_from_fp: load GV switch from file pointer */
void HTS211_Engine_load_gv_switch_from_fp(HTS211_Engine * engine, FILE * fp)
{
   HTS211_ModelSet_load_gv_switch(&engine->ms, fp);
}

/* HTS211_Engine_set_sampling_rate: set sampling rate */
void HTS211_Engine_set_sampling_rate(HTS211_Engine * engine, int i)
{
   if (i < 1)
      i = 1;
   if (i > 48000)
      i = 48000;
   engine->global.sampling_rate = i;
}

/* HTS211_Engine_get_sampling_rate: get sampling rate */
int HTS211_Engine_get_sampling_rate(HTS211_Engine * engine)
{
   return engine->global.sampling_rate;
}

/* HTS211_Engine_set_fperiod: set frame shift */
void HTS211_Engine_set_fperiod(HTS211_Engine * engine, int i)
{
   if (i < 1)
      i = 1;
   if (i > 48000)
      i = 48000;
   engine->global.fperiod = i;
}

/* HTS211_Engine_get_fperiod: get frame shift */
int HTS211_Engine_get_fperiod(HTS211_Engine * engine)
{
   return engine->global.fperiod;
}

/* HTS211_Engine_set_alpha: set alpha */
void HTS211_Engine_set_alpha(HTS211_Engine * engine, double f)
{
   if (f < 0.0)
      f = 0.0;
   if (f > 1.0)
      f = 1.0;
   engine->global.alpha = f;
}

/* HTS211_Engine_set_gamma: set gamma (Gamma = -1/i: if i=0 then Gamma=0) */
void HTS211_Engine_set_gamma(HTS211_Engine * engine, int i)
{
   if (i < 0)
      i = 0;
   engine->global.stage = i;
}

/* HTS211_Engine_set_log_gain: set log gain flag (for LSP) */
void HTS211_Engine_set_log_gain(HTS211_Engine * engine, HTS211_Boolean i)
{
   engine->global.use_log_gain = i;
}

/* HTS211_Engine_set_beta: set beta */
void HTS211_Engine_set_beta(HTS211_Engine * engine, double f)
{
   if (f < -0.8)
      f = -0.8;
   if (f > 0.8)
      f = 0.8;
   engine->global.beta = f;
}

/* HTS211_Engine_set_audio_buff_size: set audio buffer size */
void HTS211_Engine_set_audio_buff_size(HTS211_Engine * engine, int i)
{
   if (i < 0)
      i = 0;
   if (i > 48000)
      i = 48000;
   engine->global.audio_buff_size = i;
}

/* HTS211_Engine_get_audio_buff_size: get audio buffer size */
int HTS211_Engine_get_audio_buff_size(HTS211_Engine * engine)
{
   return engine->global.audio_buff_size;
}

/* HTS211_Egnine_set_msd_threshold: set MSD threshold */
void HTS211_Engine_set_msd_threshold(HTS211_Engine * engine, int stream_index,
                                  double f)
{
   if (f < 0.0)
      f = 0.0;
   if (f > 1.0)
      f = 1.0;
   engine->global.msd_threshold[stream_index] = f;
}

/* HTS211_Engine_set_duration_interpolation_weight: set interpolation weight for duration */
void HTS211_Engine_set_duration_interpolation_weight(HTS211_Engine * engine,
                                                  int interpolation_index,
                                                  double f)
{
   engine->global.duration_iw[interpolation_index] = f;
}

/* HTS211_Engine_set_parameter_interpolation_weight: set interpolation weight for parameter */
void HTS211_Engine_set_parameter_interpolation_weight(HTS211_Engine * engine,
                                                   int stream_index,
                                                   int interpolation_index,
                                                   double f)
{
   engine->global.parameter_iw[stream_index][interpolation_index] = f;
}

/* HTS211_Engine_set_gv_interpolation_weight: set interpolation weight for GV */
void HTS211_Engine_set_gv_interpolation_weight(HTS211_Engine * engine,
                                            int stream_index,
                                            int interpolation_index, double f)
{
   engine->global.gv_iw[stream_index][interpolation_index] = f;
}

/* HTS211_Engine_set_gv_weight: set GV weight */
void HTS211_Engine_set_gv_weight(HTS211_Engine * engine, int stream_index, double f)
{
   if (f < 0.0)
      f = 0.0;
   if (f > 2.0)
      f = 2.0;
   engine->global.gv_weight[stream_index] = f;
}

/* HTS211_Engine_set_stop_flag: set stop flag */
void HTS211_Engine_set_stop_flag(HTS211_Engine * engine, HTS211_Boolean b)
{
   engine->global.stop = b;
}

/* HTS211_Engine_set_volume: set volume */
void HTS211_Engine_set_volume(HTS211_Engine * engine, double f)
{
   if (f < 0.0)
      f = 0.0;
   engine->global.volume = f;
}

/* HTS211_Engine_get_total_state: get total number of state */
int HTS211_Engine_get_total_state(HTS211_Engine * engine)
{
   return HTS211_SStreamSet_get_total_state(&engine->sss);
}

/* HTS211_Engine_set_state_mean: set mean value of state */
void HTS211_Engine_set_state_mean(HTS211_Engine * engine, int stream_index,
                               int state_index, int vector_index, double f)
{
   HTS211_SStreamSet_set_mean(&engine->sss, stream_index, state_index,
                           vector_index, f);
}

/* HTS211_Engine_get_state_mean: get mean value of state */
double HTS211_Engine_get_state_mean(HTS211_Engine * engine, int stream_index,
                                 int state_index, int vector_index)
{
   return HTS211_SStreamSet_get_mean(&engine->sss, stream_index, state_index,
                                  vector_index);
}

/* HTS211_Engine_get_state_duration: get state duration */
int HTS211_Engine_get_state_duration(HTS211_Engine * engine, int state_index)
{
   return HTS211_SStreamSet_get_duration(&engine->sss, state_index);
}

/* HTS211_Engine_get_nstate: get number of state */
int HTS211_Engine_get_nstate(HTS211_Engine * engine)
{
   return HTS211_ModelSet_get_nstate(&engine->ms);
}

/* HTS211_Engine_load_label_from_fn: load label from file name */
void HTS211_Engine_load_label_from_fn(HTS211_Engine * engine, char *fn)
{
   HTS211_Label_load_from_fn(&engine->label, engine->global.sampling_rate,
                          engine->global.fperiod, fn);
}

/* HTS211_Engine_load_label_from_fp: load label from file pointer */
void HTS211_Engine_load_label_from_fp(HTS211_Engine * engine, FILE * fp)
{
   HTS211_Label_load_from_fp(&engine->label, engine->global.sampling_rate,
                          engine->global.fperiod, fp);
}

/* HTS211_Engine_load_label_from_string: load label from string */
void HTS211_Engine_load_label_from_string(HTS211_Engine * engine, char *data)
{
   HTS211_Label_load_from_string(&engine->label, engine->global.sampling_rate,
                              engine->global.fperiod, data);
}

/* HTS211_Engine_load_label_from_string_list: load label from string list */
void HTS211_Engine_load_label_from_string_list(HTS211_Engine * engine, const char **data,
                                            int size)
{
   HTS211_Label_load_from_string_list(&engine->label, engine->global.sampling_rate,
                                   engine->global.fperiod, data, size);
}

/* HTS211_Engine_create_sstream: parse label and determine state duration */
void HTS211_Engine_create_sstream(HTS211_Engine * engine)
{
   HTS211_SStreamSet_create(&engine->sss, &engine->ms, &engine->label,
                         engine->global.duration_iw,
                         engine->global.parameter_iw, engine->global.gv_iw);
}

/* HTS211_Engine_create_pstream: generate speech parameter vector sequence */
void HTS211_Engine_create_pstream(HTS211_Engine * engine)
{
   HTS211_PStreamSet_create(&engine->pss, &engine->sss,
                         engine->global.msd_threshold,
                         engine->global.gv_weight);
}

/* HTS211_Engine_create_gstream: synthesis speech */
void HTS211_Engine_create_gstream(HTS211_Engine * engine)
{
   HTS211_GStreamSet_create(&engine->gss, &engine->pss, engine->global.stage,
                         engine->global.use_log_gain,
                         engine->global.sampling_rate, engine->global.fperiod,
                         engine->global.alpha, engine->global.beta,
                         &engine->global.stop, engine->global.volume,
                         engine->global.audio_buff_size);
}

/* HTS211_Engine_save_information: output trace information */
void HTS211_Engine_save_information(HTS211_Engine * engine, FILE * fp)
{
   int i, j, k, l, m, n;
   double temp;
   HTS211_Global *global = &engine->global;
   HTS211_ModelSet *ms = &engine->ms;
   HTS211_Label *label = &engine->label;
   HTS211_SStreamSet *sss = &engine->sss;
   HTS211_PStreamSet *pss = &engine->pss;

   /* global parameter */
   fprintf(fp, "[Global parameter]\n");
   fprintf(fp, "Sampring frequency                     -> %8d(Hz)\n",
           global->sampling_rate);
   fprintf(fp, "Frame period                           -> %8d(point)\n",
           global->fperiod);
   fprintf(fp, "                                          %8.5f(msec)\n",
           1e+3 * global->fperiod / global->sampling_rate);
   fprintf(fp, "All-pass constant                      -> %8.5f\n",
           (float) global->alpha);
   fprintf(fp, "Gamma                                  -> %8.5f\n",
           (float) (global->stage == 0 ? 0.0 : -1.0 / global->stage));
   if (global->stage != 0)
      fprintf(fp, "Log gain flag                          -> %s\n",
              global->use_log_gain ? "TRUE" : "FALSE");
   fprintf(fp, "Postfiltering coefficient              -> %8.5f\n",
           (float) global->beta);
   fprintf(fp, "Audio buffer size                      -> %8d(sample)\n",
           global->audio_buff_size);
   fprintf(fp, "\n");

   /* duration parameter */
   fprintf(fp, "[Duration parameter]\n");
   fprintf(fp, "Number of states                       -> %8d\n",
           HTS211_ModelSet_get_nstate(ms));
   fprintf(fp, "         Interpolation                 -> %8d\n",
           HTS211_ModelSet_get_duration_interpolation_size(ms));
   /* check interpolation */
   for (i = 0, temp = 0.0;
        i < HTS211_ModelSet_get_duration_interpolation_size(ms); i++)
      temp += global->duration_iw[i];
   for (i = 0; i < HTS211_ModelSet_get_duration_interpolation_size(ms); i++)
      if (global->duration_iw[i] != 0.0)
         global->duration_iw[i] /= temp;
   for (i = 0; i < HTS211_ModelSet_get_duration_interpolation_size(ms); i++)
      fprintf(fp,
              "         Interpolation weight[%2d]      -> %8.0f(%%)\n", i,
              (float) (100 * global->duration_iw[i]));
   fprintf(fp, "\n");

   fprintf(fp, "[Stream parameter]\n");
   for (i = 0; i < HTS211_ModelSet_get_nstream(ms); i++) {
      /* stream parameter */
      fprintf(fp, "Stream[%2d] vector length               -> %8d\n", i,
              HTS211_ModelSet_get_vector_length(ms, i));
      fprintf(fp, "           Dynamic window size         -> %8d\n",
              HTS211_ModelSet_get_window_size(ms, i));
      /* interpolation */
      fprintf(fp, "           Interpolation               -> %8d\n",
              HTS211_ModelSet_get_parameter_interpolation_size(ms, i));
      for (j = 0, temp = 0.0;
           j < HTS211_ModelSet_get_parameter_interpolation_size(ms, i); j++)
         temp += global->parameter_iw[i][j];
      for (j = 0; j < HTS211_ModelSet_get_parameter_interpolation_size(ms, i); j++)
         if (global->parameter_iw[i][j] != 0.0)
            global->parameter_iw[i][j] /= temp;
      for (j = 0; j < HTS211_ModelSet_get_parameter_interpolation_size(ms, i); j++)
         fprintf(fp,
                 "           Interpolation weight[%2d]    -> %8.0f(%%)\n", j,
                 (float) (100 * global->parameter_iw[i][j]));
      /* MSD */
      if (HTS211_ModelSet_is_msd(ms, i)) { /* for MSD */
         fprintf(fp, "           MSD flag                    ->     TRUE\n");
         fprintf(fp, "           MSD threshold               -> %8.5f\n",
                 global->msd_threshold[i]);
      } else {                  /* for non MSD */
         fprintf(fp, "           MSD flag                    ->    FALSE\n");
      }
      /* GV */
      if (HTS211_ModelSet_use_gv(ms, i)) {
         fprintf(fp, "           GV flag                     ->     TRUE\n");
         if (HTS211_ModelSet_have_gv_switch(ms)) {
            if (HTS211_ModelSet_have_gv_tree(ms, i)) {
               fprintf(fp,
                       "           GV type                     ->     CDGV\n");
               fprintf(fp,
                       "                                       ->  +SWITCH\n");
            } else
               fprintf(fp,
                       "           GV type                     ->   SWITCH\n");
         } else {
            if (HTS211_ModelSet_have_gv_tree(ms, i))
               fprintf(fp,
                       "           GV type                     ->     CDGV\n");
            else
               fprintf(fp,
                       "           GV type                     ->   NORMAL\n");
         }
         fprintf(fp, "           GV weight                   -> %8.0f(%%)\n",
                 (float) (100 * global->gv_weight[i]));
         fprintf(fp, "           GV interpolation size       -> %8d\n",
                 HTS211_ModelSet_get_gv_interpolation_size(ms, i));
         /* interpolation */
         for (j = 0, temp = 0.0;
              j < HTS211_ModelSet_get_gv_interpolation_size(ms, i); j++)
            temp += global->gv_iw[i][j];
         for (j = 0; j < HTS211_ModelSet_get_gv_interpolation_size(ms, i); j++)
            if (global->gv_iw[i][j] != 0.0)
               global->gv_iw[i][j] /= temp;
         for (j = 0; j < HTS211_ModelSet_get_gv_interpolation_size(ms, i); j++)
            fprintf(fp,
                    "           GV interpolation weight[%2d] -> %8.0f(%%)\n", j,
                    (float) (100 * global->gv_iw[i][j]));
      } else {
         fprintf(fp, "           GV flag                     ->    FALSE\n");
      }
   }
   fprintf(fp, "\n");

   /* generated sequence */
   fprintf(fp, "[Generated sequence]\n");
   fprintf(fp, "Number of HMMs                         -> %8d\n",
           HTS211_Label_get_size(label));
   fprintf(fp, "Number of stats                        -> %8d\n",
           HTS211_Label_get_size(label) * HTS211_ModelSet_get_nstate(ms));
   fprintf(fp, "Length of this speech                  -> %8.3f(sec)\n",
           (float) ((double) HTS211_PStreamSet_get_total_frame(pss) *
                    global->fperiod / global->sampling_rate));
   fprintf(fp, "                                       -> %8.3d(frames)\n",
           HTS211_PStreamSet_get_total_frame(pss) * global->fperiod);

   for (i = 0; i < HTS211_Label_get_size(label); i++) {
      fprintf(fp, "HMM[%2d]\n", i);
      fprintf(fp, "  Name                                 -> %s\n",
              HTS211_Label_get_string(label, i));
      fprintf(fp, "  Duration\n");
      for (j = 0; j < HTS211_ModelSet_get_duration_interpolation_size(ms); j++) {
         fprintf(fp, "    Interpolation[%2d]\n", j);
         HTS211_ModelSet_get_duration_index(ms, HTS211_Label_get_string(label, i), &k,
                                         &l, j);
         fprintf(fp, "      Tree index                       -> %8d\n", k);
         fprintf(fp, "      PDF index                        -> %8d\n", l);
      }
      for (j = 0; j < HTS211_ModelSet_get_nstate(ms); j++) {
         fprintf(fp, "  State[%2d]\n", j + 2);
         fprintf(fp, "    Length                             -> %8d(frames)\n",
                 HTS211_SStreamSet_get_duration(sss,
                                             i * HTS211_ModelSet_get_nstate(ms) +
                                             j));
         for (k = 0; k < HTS211_ModelSet_get_nstream(ms); k++) {
            fprintf(fp, "    Stream[%2d]\n", k);
            if (HTS211_ModelSet_is_msd(ms, k)) {
               if (HTS211_SStreamSet_get_msd
                   (sss, k,
                    i * HTS211_ModelSet_get_nstate(ms) + j) >
                   global->msd_threshold[k])
                  fprintf(fp,
                          "      MSD flag                         ->     TRUE\n");
               else
                  fprintf(fp,
                          "      MSD flag                         ->    FALSE\n");
            }
            for (l = 0;
                 l < HTS211_ModelSet_get_parameter_interpolation_size(ms, k);
                 l++) {
               fprintf(fp, "      Interpolation[%2d]\n", l);
               HTS211_ModelSet_get_parameter_index(ms,
                                                HTS211_Label_get_string(label, i),
                                                &m, &n, k, j + 2, l);
               fprintf(fp, "        Tree index                     -> %8d\n",
                       m);
               fprintf(fp, "        PDF index                      -> %8d\n",
                       n);
            }
         }
      }
   }
}

/* HTS211_Engine_save_label: output label with time */
void HTS211_Engine_save_label(HTS211_Engine * engine, FILE * fp)
{
   int i, j;
   int frame, state, duration;

   HTS211_Label *label = &engine->label;
   HTS211_SStreamSet *sss = &engine->sss;
   const int nstate = HTS211_ModelSet_get_nstate(&engine->ms);
   const double rate =
       engine->global.fperiod * 1e+7 / engine->global.sampling_rate;

   for (i = 0, state = 0, frame = 0; i < HTS211_Label_get_size(label); i++) {
      for (j = 0, duration = 0; j < nstate; j++)
         duration += HTS211_SStreamSet_get_duration(sss, state++);
      /* in HTK & HTS format */
      fprintf(fp, "%d %d %s\n", (int) (frame * rate),
              (int) ((frame + duration) * rate),
              HTS211_Label_get_string(label, i));
      frame += duration;
   }
}

#ifndef HTS211_EMBEDDED
/* HTS211_Engine_save_generated_parameter: output generated parameter */
void HTS211_Engine_save_generated_parameter(HTS211_Engine * engine, FILE * fp,
                                         int stream_index)
{
   int i, j;
   float temp;
   HTS211_GStreamSet *gss = &engine->gss;

   for (i = 0; i < HTS211_GStreamSet_get_total_frame(gss); i++)
      for (j = 0; j < HTS211_GStreamSet_get_static_length(gss, stream_index); j++) {
         temp = (float) HTS211_GStreamSet_get_parameter(gss, stream_index, i, j);
         fwrite(&temp, sizeof(float), 1, fp);
      }
}
#endif                          /* !HTS211_EMBEDDED */

/* HTS211_Engine_save_generated_speech: output generated speech */
void HTS211_Engine_save_generated_speech(HTS211_Engine * engine, FILE * fp)
{
   int i;
   short temp;
   HTS211_GStreamSet *gss = &engine->gss;

   for (i = 0; i < HTS211_GStreamSet_get_total_nsample(gss); i++) {
      temp = HTS211_GStreamSet_get_speech(gss, i);
      fwrite(&temp, sizeof(short), 1, fp);
   }
}

/* HTS211_Engine_save_riff: output RIFF format file */
void HTS211_Engine_save_riff(HTS211_Engine * engine, FILE * fp)
{
   int i;
   short temp;

   HTS211_GStreamSet *gss = &engine->gss;
   char data_01_04[] = { 'R', 'I', 'F', 'F' };
   int data_05_08 = HTS211_GStreamSet_get_total_nsample(gss) * sizeof(short) + 36;
   char data_09_12[] = { 'W', 'A', 'V', 'E' };
   char data_13_16[] = { 'f', 'm', 't', ' ' };
   int data_17_20 = 16;
   short data_21_22 = 1;        /* PCM */
   short data_23_24 = 1;        /* monoral */
   int data_25_28 = engine->global.sampling_rate;
   int data_29_32 = engine->global.sampling_rate * sizeof(short);
   short data_33_34 = sizeof(short);
   short data_35_36 = (short) (sizeof(short) * 8);
   char data_37_40[] = { 'd', 'a', 't', 'a' };
   int data_41_44 = HTS211_GStreamSet_get_total_nsample(gss) * sizeof(short);

   /* write header */
   HTS211_fwrite_little_endian(data_01_04, sizeof(char), 4, fp);
   HTS211_fwrite_little_endian(&data_05_08, sizeof(int), 1, fp);
   HTS211_fwrite_little_endian(data_09_12, sizeof(char), 4, fp);
   HTS211_fwrite_little_endian(data_13_16, sizeof(char), 4, fp);
   HTS211_fwrite_little_endian(&data_17_20, sizeof(int), 1, fp);
   HTS211_fwrite_little_endian(&data_21_22, sizeof(short), 1, fp);
   HTS211_fwrite_little_endian(&data_23_24, sizeof(short), 1, fp);
   HTS211_fwrite_little_endian(&data_25_28, sizeof(int), 1, fp);
   HTS211_fwrite_little_endian(&data_29_32, sizeof(int), 1, fp);
   HTS211_fwrite_little_endian(&data_33_34, sizeof(short), 1, fp);
   HTS211_fwrite_little_endian(&data_35_36, sizeof(short), 1, fp);
   HTS211_fwrite_little_endian(data_37_40, sizeof(char), 4, fp);
   HTS211_fwrite_little_endian(&data_41_44, sizeof(int), 1, fp);
   /* write data */
   for (i = 0; i < HTS211_GStreamSet_get_total_nsample(gss); i++) {
      temp = HTS211_GStreamSet_get_speech(gss, i);
      HTS211_fwrite_little_endian(&temp, sizeof(short), 1, fp);
   }
}

/* HTS211_Engine_refresh: free model per one time synthesis */
void HTS211_Engine_refresh(HTS211_Engine * engine)
{
   /* free generated parameter stream set */
   HTS211_GStreamSet_clear(&engine->gss);
   /* free parameter stream set */
   HTS211_PStreamSet_clear(&engine->pss);
   /* free state stream set */
   HTS211_SStreamSet_clear(&engine->sss);
   /* free label list */
   HTS211_Label_clear(&engine->label);
   /* stop flag */
   engine->global.stop = FALSE;
}

/* HTS211_Engine_clear: free engine */
void HTS211_Engine_clear(HTS211_Engine * engine)
{
   int i;

   HTS211_free(engine->global.msd_threshold);
   HTS211_free(engine->global.duration_iw);
   for (i = 0; i < HTS211_ModelSet_get_nstream(&engine->ms); i++) {
      HTS211_free(engine->global.parameter_iw[i]);
      if (engine->global.gv_iw[i])
         HTS211_free(engine->global.gv_iw[i]);
   }
   HTS211_free(engine->global.parameter_iw);
   HTS211_free(engine->global.gv_iw);
   HTS211_free(engine->global.gv_weight);

   HTS211_ModelSet_clear(&engine->ms);
}

/* HTS211_get_copyright: write copyright to string */
void HTS211_get_copyright(char *str)
{
   int i, nCopyright = HTS211_NCOPYRIGHT;
   char url[] = HTS211_URL, version[] = HTS211_VERSION;
   char *copyright[] = { HTS211_COPYRIGHT };

   sprintf(str, "\nThe HMM-based speech synthesis system (HTS)\n");
   sprintf(str, "%shts_engine API version %s (%s)\n", str, version, url);
   for (i = 0; i < nCopyright; i++) {
      if (i == 0)
         sprintf(str, "%sCopyright (C) %s\n", str, copyright[i]);
      else
         sprintf(str, "%s              %s\n", str, copyright[i]);
   }
   sprintf(str, "%sAll rights reserved.\n", str);

   return;
}

/* HTS211_show_copyright: write copyright to file pointer */
void HTS211_show_copyright(FILE * fp)
{
   char buf[HTS211_MAXBUFLEN];

   HTS211_get_copyright(buf);
   fprintf(fp, "%s", buf);

   return;
}

HTS211_ENGINE_C_END;

#endif                          /* !HTS211_ENGINE_C */
