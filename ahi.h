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

#ifndef SPEEXY_AHI_H
#define SPEEXY_AHI_H

#include <proto/ahi.h>

#define AHI_NAMELEN	128
#define MAX_AHI_MODES	99

struct ahi_modes
{
	char *name [MAX_AHI_MODES];
	ULONG  id  [MAX_AHI_MODES];
	
	int selected;
	
	ULONG min_rate[MAX_AHI_MODES], max_rate[MAX_AHI_MODES];
};

extern struct ahi_modes *AHIModes;

//void AHIClose( void );
//BOOL AHIOpen( BOOL GettingModes );
void clear_ahiModes( void );
BOOL fill_ahiModes( void );

#define MAX_FRAME_SIZE	2000


#endif /* SPEEXY_AHI_H */
