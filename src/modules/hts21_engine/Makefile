###########################################################################
##                                                                       ##
##  ---------------------------------------------------------------      ##
##      The HMM-Based Speech Synthesis System (HTS): version 1.1b        ##
##                        HTS Working Group                              ##
##                                                                       ##
##                   Department of Computer Science                      ##
##                   Nagoya Institute of Technology                      ##
##                                and                                    ##
##    Interdisciplinary Graduate School of Science and Engineering       ##
##                   Tokyo Institute of Technology                       ##
##                      Copyright (c) 2001-2003                          ##
##                        All Rights Reserved.                           ##
##                                                                       ##
##  Permission is hereby granted, free of charge, to use and             ##
##  distribute this software and its documentation without               ##
##  restriction, including without limitation the rights to use,         ##
##  copy, modify, merge, publish, distribute, sublicense, and/or         ##
##  sell copies of this work, and to permit persons to whom this         ##
##  work is furnished to do so, subject to the following conditions:     ##
##                                                                       ##
##    1. The code must retain the above copyright notice, this list      ##
##       of conditions and the following disclaimer.                     ##
##                                                                       ##
##    2. Any modifications must be clearly marked as such.               ##
##                                                                       ##
##  NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF TECHNOLOGY,      ##
##  HTS WORKING GROUP, AND THE CONTRIBUTORS TO THIS WORK DISCLAIM        ##
##  ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL           ##
##  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT       ##
##  SHALL NAGOYA INSTITUTE OF TECHNOLOGY, TOKYO INSITITUTE OF            ##
##  TECHNOLOGY, HTS WORKING GROUP, NOR THE CONTRIBUTORS BE LIABLE        ##
##  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY            ##
##  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      ##
##  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS       ##
##  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR              ##
##  PERFORMANCE OF THIS SOFTWARE.                                        ##
##                                                                       ##
###########################################################################
##  Nagoya Institute of Technology's HTS Engine                          ##
##  Integrated in the Festival tree to allow it to be more available     ##
##  Alan W Black (awb@cs.cmu.edu)
###########################################################################
TOP=../../..
DIRNAME=src/modules/hts_engine
H = parser.h

H = defaults.h global.h misc.h mlpg.h model.h tree.h vocoder.h \
    hts21_mlsa_resynthesis.h
CPPSRCS = hts_engine.cc misc.cc mlpg.cc model.cc tree.cc vocoder.cc \
    hts21_mlsa_resynthesis.cc
SRCS = $(CPPSRCS)

OBJS = $(CPPSRCS:.cc=.o) 

FILES=Makefile $(SRCS) $(H)

LOCAL_INCLUDES = -I../include

INLIB = $(TOP)/src/lib/libFestival.a

ALL = .buildlib

include $(TOP)/config/common_make_rules


