/* ***** BEGIN LICENSE BLOCK *****
 * Version: MIT/X11 License
 * 
 * Copyright (c) 2005 Diego Casorran
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 * Contributor(s):
 *   Diego Casorran <dcasorran@gmail.com> (Original Author)
 * 
 * ***** END LICENSE BLOCK ***** */

#ifndef SPEEXY_AMUISTARTUP_H
#define SPEEXY_AMUISTARTUP_H

#define PROGRAM_NAME	"Speexy"
#define PROGRAM_VERZ	"0.1"
#define PROGRAM_AUTH	"Diego Casorran <diegocr@users.sf.net>"
#define PROGRAM_COPY	"Copyright (C) 2004 " PROGRAM_AUTH " - All Rights Reserved"
#define PROGRAM_DESC	"Speex Audio Codec Manager"
#define PROGRAM_TITL	PROGRAM_NAME " v" PROGRAM_VERZ " - " PROGRAM_DESC
#define PROGRAM_DATE	"06.12.2004"

#define NOSTDLIB_STACK		16384
#define NOSTDLIB_WBENCHMSG	1
#define NOSTDLIB_OMIT_CASOLIB	1
#define NOSTDLIB_UTILLIB	1
#define NOSTDLIB_MUILIB		1
#include <casorran/nostdlib.h>
#include <casorran/debug.h>

#include <proto/speex.h>
#include <proto/ogg.h>

#include <SDI_hook.h>
#include <macrosmui.h>

typedef enum {
	AAT_ENCODER, AAT_DECODER, AAT_PLAYER, AAT_INFO
} AccionATomar;

extern AccionATomar AAT;
extern Object *app, *win;
LONG xget( Object *obj, ULONG attribute);
ULONG filesize(char *filename);

LONG MYRequest( char *title, char *gadgets, const char *fmt, APTR args);
#define tell( fmt, args...)						\
({									\
	ULONG _tags[] = { args };					\
	MYRequest( PROGRAM_NAME, "_ok",					\
			(const char *)(fmt), (APTR)_tags);		\
})
#define ask( fmt, args...)						\
({									\
	ULONG _tags[] = { args };					\
	MYRequest( PROGRAM_NAME, "_Yes|_No",				\
			(const char *)(fmt), (APTR)_tags);		\
})


/** Simple sprintf() replacement
 **/
void SPrintF( char *dst, const char *fmt, APTR args);
#define sprintf( dst, fmt, args...)					\
({									\
	ULONG _tags[] = { args };     SPrintF( dst, fmt, (APTR)_tags);	\
})

extern struct MsgPort *msgport;


#endif /* SPEEXY_AMUISTARTUP_H */
