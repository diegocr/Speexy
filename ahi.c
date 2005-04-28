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


#include "amuistartup.h"	/* :ts=4	*/
#include "ahi.h"

struct ahi_modes *AHIModes	= NULL;
struct AHIRequest *AHIio	= NULL;
struct MsgPort *AHImp		= NULL;
struct Library *AHIBase		= NULL;
BYTE AHIDevice				= -1;


INLINE void AHIMClose( void )
{
	if(!AHIDevice)
	{
		CloseDevice((struct IORequest *)AHIio);	AHIDevice = -1;
	}
	if(AHIio)
	{
		DeleteIORequest((struct IORequest *)AHIio);	AHIio = NULL;
	}
	if(AHImp)
	{
		DeleteMsgPort(AHImp);	AHImp = NULL;
	}
}

INLINE BOOL AHIMOpen( void )
{
	if((AHImp = CreateMsgPort()))
	{
		if((AHIio = (struct AHIRequest *) CreateIORequest(AHImp,sizeof(struct AHIRequest))))
		{
			AHIio->ahir_Version = 4;
			AHIDevice = OpenDevice( AHINAME, AHI_NO_UNIT, (struct IORequest *)AHIio, NULL);
		}
		if(!AHIDevice)
		{
			AHIBase    = (struct Library *) AHIio->ahir_Std.io_Device;
			
			return TRUE;
		}
	}
	
	return FALSE;
}

void clear_ahiModes( void )
{
	int x;
	
	if(!AHIModes)
		return;
	
	for( x = 0; AHIModes->name[x]; x++)
	{
		free( AHIModes->name[x] );	AHIModes->name[x] = NULL;
	}
	
	free( AHIModes );	AHIModes = NULL;
}

BOOL fill_ahiModes( void )
{
#if 0
	ULONG id = AHI_INVALID_ID;	int x;
	
	if(!AHIMOpen())
		return FALSE;
	
	if(!(AHIModes = malloc(sizeof(struct ahi_modes))))
		return FALSE;
	
	AHIModes->selected = -1;
	
	for( x = 0; ((id = AHI_NextAudioID(id)) != AHI_INVALID_ID); x++)
	{
		LONG minmix = 0, maxmix = 0;
		UBYTE name[AHI_NAMELEN];	*name = 0;
		
		if( x == MAX_AHI_MODES-1 )
		{
			tell("too few space to fill all audio modes !?"); break;
		}
		
		AHI_GetAudioAttrs(id, NULL,
			AHIDB_BufferLen,	AHI_NAMELEN-1,
			AHIDB_Name,			(ULONG) &name,
			AHIDB_MinMixFreq,	(ULONG) &minmix,
			AHIDB_MaxMixFreq,	(ULONG) &maxmix,
		TAG_DONE);
		
		if(!(AHIModes->name[x] = malloc( AHI_NAMELEN )))
		{
			clear_ahiModes();	return FALSE;
		}
		
		CopyMem( name, AHIModes->name[x], AHI_NAMELEN);
		
		AHIModes->id[x]		= id;
		AHIModes->min_rate[x]	= minmix;
		AHIModes->max_rate[x]	= maxmix;
		
		DB("MODE = 0x%08lx, NAME = \"%s\", RATES = %ld->%ld\n", AHIModes->id[x], AHIModes->name[x], AHIModes->min_rate[x], AHIModes->max_rate[x])
	}
	
	AHIModes->name[x] = NULL;	AHIMClose();
	
#endif	
	return TRUE;
}
