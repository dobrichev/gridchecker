//Code inspired by Glenn Fowler's sudoku solver/generator
//The original is published under the following license
		/***********************************************************************
		*               This software is part of the ast package               *
		*          Copyright (c) 2005-2009 AT&T Intellectual Property          *
		*                      and is licensed under the                       *
		*                  Common Public License, Version 1.0                  *
		*                    by AT&T Intellectual Property                     *
		*                                                                      *
		*                A copy of the License is available at                 *
		*            http://www.opensource.org/licenses/cpl1.0.txt             *
		*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
		*                                                                      *
		*              Information and Software Systems Research               *
		*                            AT&T Research                             *
		*                           Florham Park NJ                            *
		*                                                                      *
		*                 Glenn Fowler <gsf@research.att.com>                  *
		***********************************************************************/
//This is a MODIFIED version of the original code

#ifndef ANCHOR5_H_INCLUDED

#define ANCHOR5_H_INCLUDED

#include <memory.h>
#include "rowminlex.h"

extern void anchor5(const char *in, char *out);

struct transformer5 : transformer
{
	void byGrid(const char* sol);
	//void transform5(const char *in, char *out);
	//void transformer5::reverseTransform(const char *in, char *out);
};

#endif // ANCHOR5_H_INCLUDED
