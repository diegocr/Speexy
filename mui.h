/* ***** BEGIN LICENSE BLOCK *****
 * Version: MIT/X11 License
 * 
 * Copyright (c) 2004 Diego Casorran
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

#ifndef SPEEXY_MUI_H
#define SPEEXY_MUI_H

#undef Cycle
#undef Slider
#undef String
#undef CheckMark

extern LONG MUIObjectIDCount;

Object *String( STRPTR Contents, ULONG MaxLen );
Object *PopString( STRPTR Contents, ULONG MaxLen, STRPTR ASLText);
Object *Cycle( STRPTR *array, STRPTR help);
Object *Slider( LONG min, LONG max, LONG CurrentValue, STRPTR Format, STRPTR help, BOOL MYClass);
Object *CheckMark(STRPTR help);
Object *Button( STRPTR txt, LONG width,  STRPTR help);
APTR MakeAHIModesPop( APTR *text );

void InfoA( const char *fmt, APTR args);
#define info( fmt, args...)				\
({							\
	ULONG _tags[] = { args };			\
	InfoA( (const char *)fmt, (APTR)_tags);		\
})

#endif /* SPEEXY_MUI_H */
