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


#include "amuistartup.h"	/* :ts=4	*/
#include "classes.h"

struct MUI_CustomClass *SLider;
MUI_DISPATCH_DECL(SLider_dispatch);

extern Object *ahibal;

struct SLider_classData
{
	UBYTE time[32];
	UBYTE balance[32];
};

BOOL SetupClasses( void )
{
	SLider = MUI_CreateCustomClass(NULL, MUIC_Slider, NULL,
		sizeof(struct SLider_classData), ENTRY(SLider_dispatch));
	
	if(SLider)
		return TRUE;
	
	ClearClasses();
	return FALSE;
}

VOID ClearClasses( void )
{
	if(SLider)
	{
		MUI_DeleteCustomClass(SLider); SLider = NULL;
	}
}

MUI_DISPATCH(SLider_dispatch)
{
	if (msg->MethodID == MUIM_Numeric_Stringify)
	{
		struct SLider_classData *data = INST_DATA(cl,obj);
		struct MUIP_Numeric_Stringify *m = (APTR)msg;
		
		if(obj == ahibal)
		{
			CONST STRPTR fmt = 
				((m->value == 0) ? "%ld" : ((m->value < 0) ? "left %ld" : "%ld right"));
			
			LONG value = 
				((m->value == 0) ? m->value : ((m->value < 0) ? -(m->value) : m->value));
			
			sprintf( data->balance, fmt, value);
			
			return((ULONG) data->balance);
		}
		else
		{
			sprintf( data->time, ((m->value == 0) ? "\0338disabled":"%02ld:%02ld:%02ld"),
				m->value/3600, (m->value%3600)/60, (m->value%3600)%60);
			
			return((ULONG)data->time);
		}
	}
	return(DoSuperMethodA(cl,obj,msg));
}
