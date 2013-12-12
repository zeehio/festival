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
/*   tree.c : decision trees handling functions                      */
/*                                                                   */ 
/*                                   2003/06/11 by Heiga Zen         */
/*  ---------------------------------------------------------------  */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include "festival.h"

#include "misc.h"
#include "tree.h"

HTS_Boolean DPMatch (char *str, char *pat, int pos, int max)
{
   if (pos > max) return 0;
   if (*str == '\0' && *pat == '\0') return 1;
   
   if (*pat == '*') {
      if ( DPMatch(str+1, pat, pos+1, max)==1 )
         return 1;
      else
         return DPMatch(str+1, pat+1, pos+1, max);
   }
   if (*str == *pat || *pat == '?') {
      if ( DPMatch(str+1, pat+1, pos+1, max+1)==1 )
         return 1;
      
      else 
         if (*(pat + 1) == '*')
            return DPMatch(str+1, pat+2, pos+1, max+1);
   }
   
   return 0;
}

HTS_Boolean PMatch (char *str, char *pat)
{
   int i, max = 0;
   for(i=0; i < (int)strlen(pat); i++)
      if (pat[i] != '*') max++;
         
   return DPMatch(str, pat, 0, strlen(str)-max);
}

HTS_Boolean QMatch (char *str, Question *q)
{
   HTS_Boolean flag = 0;
   Pattern *p;
  
   for (p=q->phead; p!=q->ptail; p=p->next) {
      flag = PMatch(str, p->pat);
      if (flag)
         return 1;
   }
   
   return 0;
}

int SearchTree (char *str, Node *node)
{
   HTS_Boolean answer = QMatch(str, node->quest);

   if (answer) {
      if (node->yes->pdf>0) 
         return node->yes->pdf;
      else 
         return SearchTree(str, node->yes);
   }
   else {
      if (node->no->pdf>0) 
         return node->no->pdf;
      else
        return SearchTree (str, node->no);
   }
   
   return -1;
}

void LoadQuestions(FILE *fp, Question *q, Mtype type)
{
   char buf[1024];

   GetToken(fp, buf);
   q->qName = wstrdup(buf);
   q->phead = q->ptail = walloc(Pattern,1);

   GetToken(fp,buf);
   if (strcmp(buf, "{")==0) {
      while (strcmp(buf,"}")!=0) {
          GetToken (fp, buf);
          q->ptail->pat = wstrdup(buf);
          q->ptail->next = walloc(Pattern,1);
          q->ptail = q->ptail->next;
          GetToken (fp, buf);
      }
   }
}

HTS_Boolean IsTree (Tree *tree, char *buf)
{
   char *s,*l,*r;

   s = buf;
   if ( ((l = strchr(s, '[')) == NULL) || ((r = strrchr(s, ']'))==NULL) ) {
      return 0;
   }
   else {
      *r = '\0';
      s = l+1;
      tree->state = atoi(s);
   }
   
   return 1;
}

HTS_Boolean IsNum (char *buf)
{
   int i;

   for (i=0; i<(int)strlen(buf); i++)
      if (! (isdigit(buf[i]) || (buf[i] == '-'))) 
         return 0;
      
   return 1;
}

Question *FindQuestion(TreeSet *ts, Mtype type, char *buf)
{
   Question *q;
   
   for (q=ts->qhead[type];q!=ts->qtail[type];q=q->next)
      if (strcmp(buf, q->qName)==0)
         return q;
      
   printf(" Error ! Cannot find question %s ! \n",buf);
   festival_error();

   return 0;
}

int name2num(char *buf)
{
   return (atoi(strrchr(buf,'_')+1));
}

Node *FindNode (Node *node, int num)
{
   Node *dest;
   
   if (node->idx==num) return node;
   else {
      if (node->yes != NULL) {
         dest = FindNode(node->yes, num);
         if (dest) return dest;
      }
      if (node->no != NULL) {
         dest = FindNode(node->no, num);
         if (dest) return dest;
      }
   }
   return NULL;
}
         
void LoadTree (TreeSet *ts, FILE *fp, Tree *tree, Mtype type)
{
   char buf[1024];
   Node *node;
   
   GetToken(fp, buf);
   node = walloc(Node,1);
   tree->root = node;
   
   if ( strcmp(buf,"{") == 0 ) {
      while ( GetToken(fp,buf),strcmp(buf,"}")!= 0 ) {
         node = FindNode(tree->root, atoi(buf));
         GetToken (fp, buf);     /* load a question applied at this node */
         
         node->quest = FindQuestion(ts, type, buf);
         node->yes = walloc(Node,1);
         node->no  = walloc(Node,1);

         GetToken (fp, buf);
         if (IsNum(buf)) {
            node->no->idx = atoi(buf);
         }
         else {
            node->no->pdf = name2num(buf);
         }
         
         GetToken(fp, buf);
         if (IsNum(buf)) {
            node->yes->idx = atoi(buf);
         }
         else {
            node->yes->pdf = name2num(buf);
         }
      }
   }
   else {
      node->pdf = name2num(buf);
   }
}
   
void LoadTreesFile(TreeSet *ts, Mtype type)
{
   char buf[1024];
   Question *q;
   Tree *t;
   FILE *fp = ts->fp[type];
   
   q = walloc(Question,1);
   ts->qhead[type] = q;  ts->qtail[type] = NULL;

   t = walloc(Tree,1);
   ts->thead[type] = t;  ts->ttail[type] = NULL;
   
   while (!feof(fp)) {
      GetToken(fp, buf);
      if (strcmp(buf, "QS") == 0) {
         LoadQuestions(fp, q, type);
         q->next = walloc(Question,1);
         q = ts->qtail[type] = q->next;
         q->next = NULL;
      }
      if (IsTree(t, buf)) {
         LoadTree(ts, fp, t, type);
         t->next = walloc(Tree,1);
         t = ts->ttail[type] = t->next;
         t->next = NULL;
      }
   }
}

void InitTreeSet(TreeSet *ts) 
{
   ts->fp[DUR] = NULL;
   ts->fp[LF0] = NULL;
   ts->fp[MCP] = NULL;
   
   return; 
} 

static void delete_tree_nodes(Node *node)
{
    if (!node)
	return;
    if (node->yes)
	delete_tree_nodes(node->yes);
    if (node->no)
	delete_tree_nodes(node->no);
    wfree(node);
}

void FreeTrees(TreeSet *ts, Mtype type)
{
    Question *nq, *qq;
    Pattern *pp, *np;
    Tree *tt, *nt;

    for (qq = ts->qhead[type]; qq; qq = nq)
    {
	nq = qq->next;

	wfree(qq->qName);
	for (pp = qq->phead; pp; pp = np)
	{
	    np = pp->next;
	    wfree(pp->pat);
	    wfree(pp);
	}
	wfree(qq);
    }

    for (tt = ts->thead[type]; tt; tt = nt)
    {
	nt = tt->next;

	delete_tree_nodes(tt->root);

	wfree(tt);
    }


}

/* -------------------- End of "tree.c" -------------------- */
