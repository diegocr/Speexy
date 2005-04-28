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


#include "amuistartup.h"
#include "mui.h"
#include <proto/asl.h>
#include "ahi.h"
#include <MUI/NListview_mcc.h>
#include "classes.h"

extern Object *ahifreq, *info_list;

void InfoA( const char *fmt, APTR args)
{
	static char buf[4096];
	char *ptr = (char *) buf;
	
	CopyMem("\0338·\0332 ", ptr, 6);
	
	SPrintF( &ptr[6], fmt, args);
	
	DoMethod( info_list, MUIM_NList_InsertSingleWrap, buf
		, MUIV_NList_Insert_Bottom, WRAPCOL1, ALIGN_LEFT);
	DoMethod( info_list, MUIM_NList_Jump, MUIV_NList_Jump_Bottom);
}

Object *String( STRPTR Contents, ULONG MaxLen )
{
	return StringObject, StringFrame,
		MUIA_CycleChain,        	1,
		MUIA_String_AdvanceOnCR,	1,
		MUIA_String_MaxLen,		MaxLen,
		MUIA_String_Contents,		Contents,
		MUIA_ObjectID, 			MUIObjectIDCount++,
	End;
}

Object *PopString( STRPTR Contents, ULONG MaxLen, STRPTR ASLText)
{
   return (Object*) PopaslObject,
	MUIA_Popstring_String, String( Contents, MaxLen ),
	MUIA_Popstring_Button, PopButton(MUII_PopFile),
	ASLFR_TitleText, ASLText,
	ASLFR_DrawersOnly, FALSE,
   End;
}

Object *Cycle( STRPTR *array, STRPTR help)
{
	Object *obj = MUI_MakeObject(MUIO_Cycle, NULL, array);
	if (obj) SetAttrs(obj,
		MUIA_CycleChain, 1,
		MUIA_ShortHelp, (ULONG)help,
		MUIA_ObjectID, MUIObjectIDCount++,
	TAG_DONE);
 return(obj);
}

Object *Slider( LONG min, LONG max, LONG CurrentValue, STRPTR Format, STRPTR help, BOOL MYClass)
{
#if 0
	Object *obj = MUI_MakeObject(MUIO_Slider, NULL, min, max);
	if (obj) SetAttrs(obj,
		MUIA_CycleChain, 	1,
		MUIA_ShortHelp, 	(ULONG)help,
		MUIA_Slider_Level,	CurrentValue,
		MUIA_ObjectID,		MUIObjectIDCount++,
		(Format ? MUIA_Numeric_Format	: TAG_IGNORE),
		(Format ? (ULONG) Format	: 0),
	TAG_DONE);
#else
	Object *obj;
	
	if(MYClass)
		obj = SLiderObject,
			MUIA_Numeric_Min  , min,
			MUIA_Numeric_Max  , max,
			MUIA_Numeric_Value, CurrentValue,
			MUIA_CycleChain   , 1,
			MUIA_ShortHelp    , (ULONG)help,
			MUIA_ObjectID     , MUIObjectIDCount++,
		TAG_DONE);
	else	obj = SliderObject,
			MUIA_Numeric_Min  , min,
			MUIA_Numeric_Max  , max,
			MUIA_Numeric_Value, CurrentValue,
			MUIA_CycleChain   , 1,
			MUIA_ShortHelp    , (ULONG)help,
			MUIA_ObjectID     , MUIObjectIDCount++,
		End;
	
	if(!MYClass && obj && Format)
		set(obj, MUIA_Numeric_Format, (ULONG) Format);
	
#endif
 return(obj);
}

Object *CheckMark(STRPTR help)
{
	Object *obj = MUI_MakeObject(MUIO_Checkmark, 0L);
	if (obj) SetAttrs(obj,
		MUIA_CycleChain, 1,
		MUIA_ShortHelp, (ULONG)help,
		MUIA_ObjectID, MUIObjectIDCount++,
	TAG_DONE);
 return(obj);
}

Object *Button( STRPTR txt, LONG width,  STRPTR help)
{
	Object *obj = MUI_MakeObject(MUIO_Button, txt);
	if(obj) SetAttrs(obj,
		 MUIA_CycleChain, 1,
		 MUIA_FixWidth, width,
		 MUIA_ShortHelp, (ULONG)help,
	        TAG_DONE);
 return(obj);
}




HOOKPROTONH(PO_HandleXPKFunc, void, APTR pop, APTR text)
{
   char *entry;
   
   if(!AHIModes)
   	return;
   
   DoMethod(pop, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &entry);
   if (entry)
   {
   	//AHIModes->selected = xget( pop, MUIA_NList_Active);
   	// OPS, the list is sorted...
   	
   	for( AHIModes->selected = 0; AHIModes->name[AHIModes->selected]; AHIModes->selected++)
   	{
   		if(!Stricmp( entry, AHIModes->name[AHIModes->selected])) break;
   	}
   	
   	if(AHIModes->name[AHIModes->selected] == NULL)
   	{
   		//tell("internal error getting mode !?!"); 
   		AHIModes->selected = -1;	/* USE Unit 0 */
   	}
   	else
   	{
   		SetAttrs( ahifreq, 
   			MUIA_Slider_Min,	AHIModes->min_rate[AHIModes->selected],
   			MUIA_Slider_Max,	AHIModes->max_rate[AHIModes->selected],
   		TAG_DONE);
   	}
   	
   	DB("ModeSelected = %ld, \"%s\"\n", AHIModes->selected, AHIModes->name[AHIModes->selected])
   	
   	set(text, MUIA_Text_Contents, (ULONG)entry);
   }
}
MakeStaticHook(PO_HandleXPKHook, PO_HandleXPKFunc);

// Creates a popup list of available AHI modes
APTR MakeAHIModesPop( APTR *text )
{
   APTR lv, po;
   
   if ((po = PopobjectObject,
      MUIA_Popstring_String, *text = TextObject, TextFrame,
         MUIA_Background, MUII_TextBack,
//         MUIA_FixWidthTxt, "MMMM",
//         MUIA_ObjectID, MUIObjectIDCount++,
      End,
      MUIA_Popstring_Button, PopButton(MUII_PopUp),
      MUIA_Popobject_ObjStrHook, &PO_HandleXPKHook,
//      MUIA_Popobject_WindowHook, &PO_WindowHook,
      MUIA_Popobject_Object, lv = NListviewObject,
         MUIA_NListview_NList, NListObject,
            InputListFrame,
            MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
            MUIA_NList_DestructHook, MUIV_NList_DestructHook_String,
         End,
      End,
   End))
   {
      int x;
      
      DoMethod(lv, MUIM_List_InsertSingle, ":: Default (Unit 0) ::", MUIV_List_Insert_Sorted);
      
      if(AHIModes) for( x = 0; AHIModes->name[x]; x++)
      {
      	DoMethod(lv, MUIM_List_InsertSingle, AHIModes->name[x], MUIV_List_Insert_Sorted);
      }
//      else DoMethod(lv, MUIM_List_InsertSingle, "couldn't get modes!", MUIV_List_Insert_Sorted);
      
      DoMethod(lv,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE,po,2,MUIM_Popstring_Close,TRUE);
   }
   return po;
}






