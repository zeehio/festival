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
/*    tree.h : decision tree definition                              */
/*                                                                   */ 
/*                                    2003/06/11 by Heiga Zen        */
/*  ---------------------------------------------------------------  */

typedef struct _Pattern{  /* pattern handler for question storage */
   char *pat;               /* pattern */
   struct _Pattern *next;   /* link to next pattern */
} Pattern;

typedef struct _Question { /* question storage */
   char *qName;              /* name of this question */
   Pattern *phead;           /* link to head of pattern list */
   Pattern *ptail;           /* link to tail of pattern list */
   struct _Question *next;  /* link to next question */
} Question;

typedef struct _Node {     /* node of decision tree */
   int idx;                 /* index of this node */
   int pdf;                 /* index of pdf for this node  ( leaf node only ) */
   struct _Node *yes;       /* link to child node (yes) */
   struct _Node *no;        /* link to child node (no)  */
   Question *quest;          /* question applied at this node */
} Node;
   
typedef struct _Tree {      
   int state;                 /* state position of this tree */
   struct _Tree *next;        /* link to next tree */
   Node *root;                 /* root node of this decision tree */
} Tree;

typedef struct _TreeSet {
   Question *qhead[3];
   Question *qtail[3];

   Tree *thead[3];
   Tree *ttail[3];
   
   FILE *fp[3];
   
} TreeSet;

void LoadTreesFile (TreeSet *, Mtype);
int SearchTree (char *, Node *);
void InitTreeSet(TreeSet *);
void FreeTrees(TreeSet *ts, Mtype type);

/* -------------------- End of "tree.h" -------------------- */

