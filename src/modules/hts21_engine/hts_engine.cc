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
/*    hts_engine.c : a compact HMM-based speech synthesis engine     */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */
/*  Modified by Alan W Black (awb@cs.cmu.edu)                        */
/*  April 2004                                                       */
/*  Make it compile with c++ and integrate as a Festival module      */
/*  ---------------------------------------------------------------  */

/*  Standard C Libraries  */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "festival.h"

#include "misc.h"
#include "tree.h"
#include "model.h"
#include "global.h"
#include "vocoder.h"
#include "mlpg.h"
#include "defaults.h"

using namespace std;


void HTS21_Process ( FILE *, FILE *, FILE *, FILE *, PStream *, PStream *, 
		   globalP *, ModelSet *, TreeSet *, VocoderSetup *);

/* OutLabel : output label with frame number or time */
void OutLabel (UttModel *um, HTS_Boolean XIMERA) 
{
   Model *m;
   char *tmp;
   int nframe = 0;
   
   for (m=um->mhead; m!=um->mtail; m=m->next) {
      if (XIMERA) {      /* in XIMERA format */
         tmp = wstrdup(m->name);
         tmp = strchr(tmp,'-')+1;
         *(strchr(tmp,'+')) = '\0';
         fprintf(stdout,"%1.3f  %s\n", (((float)nframe)*FPERIOD)/((float)RATE), tmp);   
      } 
      else               /* in HTK & HTS format */
         fprintf(stdout, "%d %d %s\n", nframe, nframe+m->totaldur,m->name);
      
      nframe += m->totaldur;
   }

   if (XIMERA)   /* in XIMERA format */
      fprintf(stdout,"%1.3f  __END__\n", (((float)nframe)*FPERIOD)/((float)RATE));

}

void HTS21_Process ( FILE *labfp, FILE *rawfp, FILE *lf0fp, FILE *mcepfp, 
		   PStream *mceppst, PStream *lf0pst, globalP *gp, 
		   ModelSet *ms, TreeSet *ts, VocoderSetup *vs )
{
   char buf[1024];
   Tree *tree;
   int state, diffdur=0;
   int start, end;
   int rate, nf;
   int i;
   float f, mean, var;
   HTS_Boolean hastime;
   Model *m, *mm, *nm;
   UttModel um;
   
   rate = FPERIOD * 10000000 / RATE;
   
   mean = var = 0.0;

   m = walloc(Model,1);
   um.mtail = um.mhead = m;  
   um.totalframe = um.nState = um.nModel = 0;
   start = 0;
   end = 0;
   
   while (!feof(labfp)) {
      GetToken (labfp,buf);
      if (!isalnum(buf[0])) break;
      if (isdigit(buf[0]))
         hastime = TRUE;
      else 
         hastime = FALSE;
      
      if (hastime) {
         if (gp->algnst) {
            start = atoi(buf);
            GetToken(labfp, buf);
            end = atoi(buf);
            GetToken(labfp, buf);
            GetToken(labfp, buf);
         }
         else if (gp->algnph) {
            start = atoi(buf);
            GetToken(labfp, buf);
            end = atoi(buf);
            GetToken(labfp, buf);
         }
         else {
            do {
               GetToken(labfp, buf);
            } while (isdigit(buf[0]));
         }
      }
      
      m->name = wstrdup(buf);
      
      if (hastime && gp->algnph) {
         m->durpdf = SearchTree(m->name, ts->thead[DUR]->root);
         FindDurPDF(m, ms, gp->RHO, diffdur);
         nf = 0;
         for (state=2; state<=ms->nstate+1; state++)
            nf += m->dur[state];
           
         fprintf(stderr, ">>>nf=%d %d\n", nf, (end-start)/rate);
         
         f = (float)(end-start)/(rate*nf);
         m->totaldur = 0;
         
         for (state=2; state<=ms->nstate+1; state++) {
            nf = (int)(f*m->dur[state]+0.5);
            if (nf<=0)  nf=1;
            fprintf(stderr, "%d: %d %f %d\n", state, m->dur[state], f, nf);
            m->dur[state] = nf;
            m->totaldur += m->dur[state];
         }
         um.totalframe += m->totaldur;
      }
      else if (hastime && gp->algnst) {
         m->dur = walloc(int,ms->nstate+2);
         m->dur[2] = (end-start)/rate;
         m->totaldur = m->dur[2];
         um.totalframe += m->dur[2];
         
         for (state=3; state<=ms->nstate+1; state++) {
            GetToken(labfp, buf);
            start = atoi(buf);
            GetToken(labfp, buf); 
            end = atoi(buf);
            GetToken(labfp, buf);
            m->dur[state] = (end-start)/rate;
            m->totaldur += m->dur[state];
            um.totalframe  += m->dur[state];
         }
      } 
      else {
         m->durpdf = SearchTree(m->name, ts->thead[DUR]->root);   
         if (gp->LENGTH==0) {
            FindDurPDF(m, ms, gp->RHO, diffdur);
            um.totalframe += m->totaldur;
         }
         else {   /* if total length of generated speech is specified */
            for (state=2; state<=ms->nstate+1; state++) {
               mean += ms->durpdf[m->durpdf][state];
               var  += ms->durpdf[m->durpdf][state+ms->nstate];
            }
         }
      }
      
      /* for excitation */
      m->lf0pdf      = walloc(int,ms->nstate+2);
      m->lf0mean     = walloc(float *,ms->nstate+2);
      m->lf0variance =  walloc(float *,ms->nstate+2);
      m->voiced      = walloc(HTS_Boolean, ms->nstate);
      
      for (tree=ts->thead[LF0],state=2; tree!=ts->ttail[LF0]; tree=tree->next,state++) {
         m->lf0pdf[state] = SearchTree(m->name, tree->root);
         FindLF0PDF(state, m, ms, gp->UV);
      }

      /* for spectrum */
      m->mceppdf      = walloc(int,ms->nstate+2);
      m->mcepmean     = walloc(float *,ms->nstate+2);
      m->mcepvariance = walloc(float *,ms->nstate+2);
      
/*      m->mceppdf -= 2;  m->mcepmean -= 2;  m->mcepvariance -= 2; */
      
      for (tree=ts->thead[MCP],state=2; tree!=ts->ttail[MCP]; tree=tree->next,state++) {
         m->mceppdf[state] = SearchTree(m->name, tree->root);
         FindMcpPDF(state, m, ms);
      }
      
      m->next = walloc(Model,1);
      m = um.mtail = m->next;
      
      um.nModel++;
      um.nState+=ms->nstate;
   }
   
   if (gp->LENGTH > 0 && gp->LENGTH < um.nState) {
      fprintf(stderr, "Specified length of generated speech is too short ! (this sentence HMM is composed from %d states)\n", um.nState);
      fprintf(stderr, "Please specify more than %.1f seconds.\n", (float)(um.nState*FPERIOD)/RATE);
      festival_error();
   }
   
   /* if total length of generated speech is specified */
   /* compute RHO */
   if (gp->LENGTH>0) {
      gp->RHO = (gp->LENGTH - mean)/var;
      /* compute state duration for each state */
      for (m=um.mhead; m!=um.mtail; m=m->next) {
         FindDurPDF(m, ms, gp->RHO, diffdur);
         um.totalframe += m->totaldur;
      }
   }
   
   /* Output label information */
   /* OutLabel(&um, gp->XIMERA); */
   
   pdf2speech(rawfp, lf0fp, mcepfp, mceppst, lf0pst, gp, ms, &um, vs);

   /* Tidy up memory */
   for (mm=um.mhead; mm; mm=nm)
   {
       nm = mm->next;
       for (i=0; i<ms->nstate+2; i++)
       {
	   if (mm->lf0mean) wfree(mm->lf0mean[i]);
	   if (mm->lf0variance) wfree(mm->lf0variance[i]);
       }
       wfree(mm->mcepvariance);
       wfree(mm->mcepmean);
       wfree(mm->mceppdf);
       wfree(mm->voiced);
       wfree(mm->lf0variance);
       wfree(mm->lf0mean);
       wfree(mm->lf0pdf);
       wfree(mm->dur);
       wfree(mm->name);
       wfree(mm);
   }
}

static FILE *do_fopen(const char *fname,const char *mode)
{
    FILE *fd;

    fd = fopen(fname,mode);
    if (fd == NULL)
    {
        cerr << "hts_engine: failed to open " << fname << endl;
	festival_error();
    }
    return fd;
}

LISP HTS21_Synthesize_Utt(LISP utt)
{
    EST_Utterance *u = get_c_utt(utt);
    EST_Item *item = 0;
    LISP hts_engine_params = NIL;
    LISP hts_output_params = NIL;
    FILE *labfp=NULL;
    FILE *lf0fp=NULL, *mcepfp=NULL, *rawfp=NULL;

    ModelSet     ms;
    TreeSet      ts;
    PStream      mceppst, lf0pst;
    globalP      gp;
    VocoderSetup vs;
   
    /* default value for control parameter */
    gp.RHO      = 0.0; 
    gp.ALPHA    = 0.42;
    gp.F0_STD   = 1.0;
    gp.F0_MEAN  = 0.0;
    gp.UV       = 0.5;
    gp.LENGTH   = 0;
    gp.algnst   = FALSE;
    gp.algnph   = FALSE;
    gp.XIMERA   = FALSE;

    /* Get voice specific params */
    hts_engine_params = siod_get_lval("hts_engine_params",
			"HTS_ENGINE: no parameters set for module");
    /* We should be internalize these ones more */
    hts_output_params = siod_get_lval("hts_output_params",
			"HTS_ENGINE: no output parameters set for module");
    
    /* initialise TreeSet and ModelSet */
    InitTreeSet (&ts);
    InitModelSet(&ms);
   
    /* delta window handler for log f0 */
    lf0pst.dw.fn = walloc(char *,20);
    lf0pst.dw.num = 1;

    /* delta window handler for mel-cepstrum */
    mceppst.dw.fn = walloc(char *,20);
    mceppst.dw.num = 1;

    /* Load parameters */
    mceppst.dw.fn[1] = (char *)get_param_str("-dm1",hts_engine_params,
				   "hts/mcep_dyn.win");
    mceppst.dw.fn[2] = (char *)get_param_str("-dm2",hts_engine_params,
				   "hts/mcep_acc.win");
    mceppst.dw.num = 3;

    lf0pst.dw.fn[1] = (char *)get_param_str("-df1",hts_engine_params,
				   "hts/lf0_dyn.win");
    lf0pst.dw.fn[2] = (char *)get_param_str("-df2",hts_engine_params,
				   "hts/lf0_acc.win");
    lf0pst.dw.num = 3;
    
    ts.fp[DUR]=do_fopen(get_param_str("-td",hts_engine_params,
				   "hts/trees-dur.inf"),"r");
    ts.fp[LF0]=do_fopen(get_param_str("-tf",hts_engine_params,
				   "hts/trees-lf0.inf"), "r");
    ts.fp[MCP]=do_fopen(get_param_str("-tm",hts_engine_params,
				   "hts/trees-mcep.inf"), "r");
    ms.fp[DUR]=do_fopen(get_param_str("-md",hts_engine_params,
				   "hts/duration.pdf"),"rb");
    ms.fp[LF0]=do_fopen(get_param_str("-mf",hts_engine_params,
				   "hts/lf0.pdf"), "rb");
    ms.fp[MCP]=do_fopen(get_param_str("-mm",hts_engine_params,
				   "hts/mcep.pdf"), "rb");

    rawfp = do_fopen(get_param_str("-or",hts_output_params,
				"tmp.raw"), "wb");
    lf0fp = do_fopen(get_param_str("-of",hts_output_params,
				"tmp.f0"), "wb");
    mcepfp = do_fopen(get_param_str("-om",hts_output_params,
				 "tmp.mcep"), "wb");
    labfp = do_fopen(get_param_str("-labelfile",hts_output_params,
				 "utt.feats"), "r");

    gp.RHO      = get_param_float("-r",hts_engine_params,0.0);
    gp.ALPHA    = get_param_float("-a",hts_engine_params,0.42);
    gp.F0_STD = get_param_float("-fs",hts_engine_params,1.0);
    gp.F0_MEAN = get_param_float("-fm",hts_engine_params,0.0);
    gp.UV       = get_param_float("-u",hts_engine_params,0.5);
    gp.LENGTH   = (int)get_param_float("-l",hts_engine_params,0.0);

    /* do what needs to be done */
    LoadTreesFile(&ts, DUR);
    LoadTreesFile(&ts, LF0);
    LoadTreesFile(&ts, MCP);
   
    /* load model files for duration, log f0 and mel-cepstrum */
    LoadModelFiles(&ms);
   
    /* if the name of output speech file is not specified, waveform generation won't be generated */
    if (rawfp!=NULL)
	init_vocoder(ms.mcepvsize-1, &vs);

    /* check the number of window */
    if (lf0pst.dw.num != ms.lf0stream) 
    {
	cerr << "Festival: HTS: dynamic window for f0 is illegal\n";
	festival_error();
    }
    if (ms.mcepvsize % mceppst.dw.num != 0 ) 
    {
	cerr << "Festival: HTS: dynamic window for mcep is illegal\n";
	festival_error();
    }
   
    /* generate speech */
    if (u->relation("Segment")->first())  /* only if there segments */
        HTS21_Process(labfp, rawfp, lf0fp, mcepfp, 
                    &mceppst, &lf0pst, &gp, &ms, &ts, &vs);

    /* Load back in the waveform */
    EST_Wave *w = new EST_Wave;

    fclose(ts.fp[DUR]);
    fclose(ts.fp[LF0]);
    fclose(ts.fp[MCP]);
    fclose(ms.fp[DUR]);
    fclose(ms.fp[LF0]);
    fclose(ms.fp[MCP]);
    fclose(rawfp);
    fclose(lf0fp);
    fclose(mcepfp);
    fclose(labfp);

    wfree(vs.c);
    wfree(lf0pst.dw.fn);
    wfree(mceppst.dw.fn);
    FreeTrees(&ts, DUR);
    FreeTrees(&ts, LF0);
    FreeTrees(&ts, MCP);
    DeleteModelSet(&ms);

    if (u->relation("Segment")->first())  /* only if there segments */
        w->load_file(get_param_str("-or",hts_output_params,"tmp.raw"),
                     "raw", 16000,
                     "short", str_to_bo("native"), 1);

    item = u->create_relation("Wave")->append();
    item->set_val("wave",est_val(w));

    return utt;
}

LISP hts21_mlsa_resynthesis(LISP ltrack);

void festival_hts21_engine_init(void)
{
    proclaim_module("hts21_engine");

    festival_def_utt_module("HTS21_Synthesize",HTS21_Synthesize_Utt,
    "(HTS21_Synthesis UTT)\n\
  Synthesize a waveform using the HTS 2.1 Engine and the current models");
    init_subr_1("hts21_mlsa_resynthesis",hts21_mlsa_resynthesis,
                "(hts21_mlsa_resynthesis TRACK)\n\
  Return a WAVE synthesized from the F0/MCEP TRACK.");
}

/* -------------------- End of "hts_engine.c" -------------------- */
