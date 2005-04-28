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
#include "mui.h"
#include <MUI/textinput_mcc.h>
#include <MUI/NListview_mcc.h>
#include "ahi.h"
#include "classes.h"
#include "speexdec.h"
#include "flags.h"

Object *app, *win;			LONG MUIObjectIDCount = 1;
static void MUI_Interface( void );
void speexdec(struct speexdec *sdata);

HDEC( HerzSelector );
HDEC( AAT );
HDEC( Launch );

struct SpeexBase *SpeexBase = NULL;
struct Library *OggBase = NULL;

AccionATomar AAT = AAT_ENCODER;
static Object 
	*quality, *complex, *frames, *ABRObj, *ABRTog, *max_bitrate,
	*title, *author, *comments, 
	*input_file, *output_file, *input_type,
	*VBR, *VAD, *DTX, *DEN, *AGC,
	*RawRate, *RawEndian, *RawFreq, *RawFreq, *RawStereo,
	
	*output_type, *output_mode, *decrate, *decrateAct, *enh, *time_start, *time_stop,
	
	*ahimode, *ReGroupObj, *Start, *Stop,
	
	*dummy UNUSED
;
Object *ahifreq, *ahivol, *ahibal, *status_info, *status_time, *info_list
;
NOSTDLIB_INIT_PROGRAM
	
	if((SpeexBase = (struct SpeexBase *) OpenLibrary("speex.library", 2)))
	{
		if((OggBase = OpenLibrary("ogg.library", 2)))
		{
			if(SetupClasses())
			{
				if(!fill_ahiModes())
					tell("warning: Couldn't parse AHI modes!,\nAHI isn't installed here or you are running out of mem (?)");
				
				MUI_Interface();
				
				clear_ahiModes();
				
				ClearClasses();
			}
			else tell("can't setup classes !?");
			
			CloseLibrary((struct Library *) OggBase );
		}
		else tell("Couldn't open %s", (ULONG)"ogg.library");
		
		CloseLibrary((struct Library *) SpeexBase );
	}
	else tell("Couldn't open %s", (ULONG)"speex.library");
	
NOSTDLIB_FREE_PROGRAM

STATIC UBYTE __version[] __attribute__((unused)) = 
	"$VER:" PROGRAM_NAME " v" PROGRAM_VERZ " (" PROGRAM_DATE ") " PROGRAM_COPY ;


INLINE void MUI_Loop( Object **Window )
{
	Object *win = *Window, *app = _app( win );
	
	ULONG sigs = 0;		DB("MUI Looping task 0x%08lx\n", (long) FindTask(0))
	
	while(1)
	{
		while(DoMethod(app, MUIM_Application_NewInput, &sigs)
			!= (ULONG) MUIV_Application_ReturnID_Quit )
		{
			if( sigs )
			{
				sigs = Wait(sigs | SIGBREAKF_CTRL_C);
				if (sigs & SIGBREAKF_CTRL_C) {
					break;
				}
			}
		}
#if 0
		Forbid();
		if( decoder_task || player_task )
		{
			Permit();
			
			tell("I can't quit, some subprocess is still running...");
			
			continue;
		}
		Permit();
#endif

		break;
	}
}

static void MUI_Interface( void )
{
	STATIC CONST STRPTR ReGroupTitles[] =
	{
		"Encoder", "Decoder", "Player", "Debug/Info", NULL
	};
	STATIC CONST STRPTR EncoderPages[] =
	{
		"Main", "RAW Input Options", "\"Tag Editor\"", NULL
	};
	STATIC CONST STRPTR InputTypes[] =
	{
		"dunno", "Narrowband (8 kHz)", "Wideband (16 kHz)", "Ultra-wideband (32 kHz)", NULL
	};
	STATIC CONST STRPTR OutputTypes[] =
	{
		"default", "Narrowband (8 kHz)", "Wideband (16 kHz)", "Ultra-wideband (32 kHz)", NULL
	};
	STATIC CONST STRPTR RawEndianType[] =
	{
		"dunno", "little-endian", "big-endian", NULL
	};
	STATIC CONST STRPTR RawFreqType[] =
	{
		"dunno", "8-bit unsigned", "16-bit signed", NULL
	};
	STATIC CONST STRPTR OutputMode[] =
	{
		"Mono", "Stereo", NULL
	};
	app = ApplicationObject,
		MUIA_Application_Title,			PROGRAM_NAME,
		MUIA_Application_Version,		PROGRAM_VERZ,
		MUIA_Application_Copyright,		PROGRAM_COPY,
		MUIA_Application_Author,		PROGRAM_AUTH,
		MUIA_Application_Description,	PROGRAM_DESC,
		MUIA_Application_Base,			PROGRAM_NAME,
		SubWindow,				win  =  WindowObject,
			MUIA_Window_Title,			PROGRAM_TITL,
			MUIA_Window_ID,				MAKE_ID('M','A','I','N'),
/*			MUIA_Window_Menustrip,		MUI_MakeObject(MUIO_MenustripNM,MenuData1,0),*/
			WindowContents, 			VGroup,
				Child, HGroup, GroupFrameT(0),
					Child, ColGroup(2), GroupFrame,
						Child, Label2("Input:"),
						Child, input_file = PopString( NULL, 512, "Select input stream"),
						Child, Label2("Output:"),
						Child, output_file = PopString( NULL, 512, "Select output stream"),
					End,
					Child, VGroup, GroupFrame,
						Child, Start = Button( "start", 44, 0),
						Child, Stop  = Button( "stop" , 44, 0),
					End,
				End,
				Child, ReGroupObj = RegisterGroup( ReGroupTitles ),
	/*	Frame */	Child, RegisterGroup( EncoderPages ),			/* ENCODER */
						Child, VGroup,
							Child, RectangleObject, End,
							Child, ColGroup(2), GroupFrame,
								Child, Label1("Input type"),
								Child, input_type = Cycle( (STRPTR *)InputTypes, 0),
							End,
							Child, RectangleObject, End,
							Child, HGroup,
								Child, VGroup,
									Child, ColGroup(2), GroupFrame,
										Child, Label2("Quality"),
										Child, quality = Slider( 0, 10, 8, NULL, 0, FALSE),
										Child, Label2("Complex"),
										Child, complex = Slider( 0, 10, 3, 0, "Set encoding complexity", FALSE),
										Child, Label2("nFrames"),
										Child, frames = Slider( 0, 10, 1, 0, "Number of frames per Ogg packet", FALSE),
										Child, Label2("Max BitRate"),
										Child, max_bitrate = Slider( 8000, 48000, 22050, "%ld Hz", "Maximun encoding bit-rate", FALSE),
										Child, Label2("ABR"),
										Child, HGroup,
											Child, ABRObj = Slider( 8000, 48000, 22050, "%ld Hz", "Enable average bit-rate at rate bps", FALSE),
											Child, ABRTog = CheckMark( "de-active ABR"),
										End,
									End,
								End,
								Child, VGroup,
									Child, ColGroup(2), GroupFrame,
										Child, Label1("VBR"),
										Child, VBR = CheckMark( "Enable variable bit-rate" ),
										Child, Label1("VAD"),
										Child, VAD = CheckMark( "Enable voice activity detection" ),
										Child, Label1("DTX"),
										Child, DTX = CheckMark( "Enable file-based discontinuous transmission" ),
										Child, Label1("DEN"),
										Child, DEN = CheckMark( "Denoise the input before encoding" ),
										Child, Label1("AGC"),
										Child, AGC = CheckMark( "Apply adaptive gain control before encoding" ),
									End,
								End,
							End,
							Child, RectangleObject, End,
						End,			/* Encoder Frame */
						Child, VGroup, GroupFrame,
							Child, RectangleObject, End,
							Child, ColGroup(2),
								Child, Label2("BitRate"),
								Child, RawRate = Slider( 8000, 48000, 22050, "%ld Hz", "Sampling rate for raw input", FALSE),
								Child, VSpace(1),
								Child, RectangleObject, End,
								Child, Label("Endian"),
								Child, RawEndian = Cycle( (STRPTR *)RawEndianType, "Raw input type"),
								Child, Label("Bits"),
								Child, RawFreq = Cycle( (STRPTR *)RawFreqType, "Raw input type"),
								Child, VSpace(1),
								Child, RectangleObject, End,
								Child, Label1("Stereo"),
								Child, HGroup,
									Child, RawStereo = CheckMark("Consider raw input as stereo"),
									Child, RectangleObject, End,
								End,
							End,
							Child, RectangleObject, End,
						End,									/* Encoder Frame */
						Child, VGroup, GroupFrame,
							Child, ColGroup(2),
								Child, Label2("Title"),
								Child, title = String( 0, 1024 ),
								Child, Label2("Author"),
								Child, author = String( "Speexy - Speex Manager for AmigaOS :-)", 1024 ),
							End,
							Child, VGroup, GroupFrameT("Comments"),
								Child, comments = TextinputscrollObject, StringFrame, MUIA_CycleChain, 1,
									MUIA_Textinput_Multiline, TRUE, 
									MUIA_ObjectID,	MUIObjectIDCount++,
									MUIA_ControlChar, 'c',
								//	MUIA_Textinput_Contents, C2,
								End,
							End,
						End,
/*		*/			End,
	/*	Frame */	Child, VGroup,				/* DECODER */
						Child, RectangleObject, End,
						Child, ColGroup(2), GroupFrame,
							Child, Label1("Output type"),
							Child, output_type = Cycle( (STRPTR *)OutputTypes, "will force decoding on the given type"),
							Child, Label1("Output Mode"),
							Child, output_mode = Cycle( (STRPTR *)OutputMode, 0 ),
							Child, Label2("Rate"),
							Child, HGroup,
								Child, decrate = Slider( 8000, 48000, 22050, "%ld Hz", "Force decoding at given sampling rate", FALSE),
								Child, decrateAct = CheckMark( "Force decoding at given sampling rate" ),
							End,
							Child, VSpace(0),
							Child, HGroup, GroupFrameT( 0 ),
								Child, RectangleObject, End,
								Child, Label1("perceptual enhancement"),
								Child, enh = CheckMark( "Enable/Disable perceptual enhancement\n... ON by default!" ),
							End,
						End,
						Child, RectangleObject, End,
						Child, ColGroup(2), GroupFrameT("Start/Stop Timer Range"),
							Child, Label2("Start"),
							Child, time_start = Slider( 0, 18000 /*5 horas */, 0, 0, 0, TRUE),
							Child, Label2("Stop"),
							Child, time_stop = Slider( 0, 18000 /*5 horas */, 0, 0, 0, TRUE),
						End,
						Child, RectangleObject, End,
/*		*/			End,
	/*	Frame */	Child, VGroup,				/* PLAYER */
						Child, RectangleObject, End,
						Child, ColGroup(2), GroupFrameT("Audio Settings"),
							MUIA_Disabled,				TRUE,
							Child, Label2("AHI Mode"),
							Child, MakeAHIModesPop( (APTR) &ahimode ),
							Child, Label2("Frequency"),
							Child, ahifreq = Slider( 8000, 48000, 22050, "%ld Hz", "AHI Frquency", FALSE),
							Child, HSpace(2),
							Child, RectangleObject, End,
							Child, Label2("Volume"),
							Child, ahivol = Slider( 0, 100, 100, "%ld %%", 0, FALSE),
							Child, Label2("Balance"),
							Child, ahibal = Slider( -10, 10, 0, 0, 0, TRUE),
						End,
						Child, RectangleObject, End,
						Child, VGroup, GroupFrameT("Status"),
							Child, status_info = TextinputObject, TextFrame, 
								MUIA_Background, MUII_TextBack,
								MUIA_Textinput_NoInput, TRUE,
							End,
							Child, status_time = Slider( 0, 0, 0, 0, "Current sample time position", TRUE),
						End,
						Child, RectangleObject, End,
					End,
					Child, NListviewObject,
						MUIA_NListview_NList, info_list = NListObject,
							MUIA_ObjectID, MAKE_ID('I','N','F','O'),
							MUIA_NList_ConstructHook, 	MUIV_NList_ConstructHook_String,
							MUIA_NList_DestructHook,	MUIV_NList_DestructHook_String,
							MUIA_NList_AutoVisible,		TRUE,
							MUIA_NList_TypeSelect,		MUIV_NList_TypeSelect_Char,
						End,
					End,
/*	Fr END */	End,
			End,
		End,
	End;
	
	
	if ( app )
	{
		DoMethod( win, MUIM_Notify,MUIA_Window_CloseRequest, TRUE, 
			app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);
		
		DoMethod(app, MUIM_Application_Load, MUIV_Application_Load_ENVARC);
		
		set( ABRObj, MUIA_Disabled, TRUE);
		set( decrate, MUIA_Disabled, TRUE);
		set( enh, MUIA_Selected, TRUE);
		
		set( status_info, MUIA_Textinput_Contents, 
			(ULONG) "\033cUse the decoder options to more accurate audio output");
		
		set( win, MUIA_Window_Open, TRUE);
		
		if(xget( win, MUIA_Window_Open))
		{
			((struct Process *)FindTask(0))->pr_WindowPtr = 
				(struct Window *) xget( win, MUIA_Window_Window);
			
			DoMethod( max_bitrate, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, MUIV_Notify_Self, 
				3, MUIM_CallHook, &HerzSelectorHook, MUIV_TriggerValue);
			DoMethod( ABRObj, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, MUIV_Notify_Self, 
				3, MUIM_CallHook, &HerzSelectorHook, MUIV_TriggerValue);
			DoMethod( RawRate, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, MUIV_Notify_Self, 
				3, MUIM_CallHook, &HerzSelectorHook, MUIV_TriggerValue);
			DoMethod( decrate, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, MUIV_Notify_Self, 
				3, MUIM_CallHook, &HerzSelectorHook, MUIV_TriggerValue);
			DoMethod( ahifreq, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, MUIV_Notify_Self, 
				3, MUIM_CallHook, &HerzSelectorHook, MUIV_TriggerValue);
			
			DoMethod( ABRTog, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
				ABRObj, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
			DoMethod( decrateAct, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
				decrate, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
			
			DoMethod( ReGroupObj, MUIM_Notify, MUIA_Group_ActivePage, MUIV_EveryTime, 
				MUIV_Notify_Self, 3, MUIM_CallHook, &AATHook, MUIV_TriggerValue);
			
			DoMethod( Start, MUIM_Notify, MUIA_Pressed, FALSE,
				MUIV_Notify_Self, 3, MUIM_CallHook, &LaunchHook, TRUE);
			DoMethod( Stop, MUIM_Notify, MUIA_Pressed, FALSE,
				MUIV_Notify_Self, 3, MUIM_CallHook, &LaunchHook, FALSE);
			
			MUI_Loop( &win );
		}
		else tell("Couldn't open window!");
		
		DB("Cleaning mui app\n")
		
		set( win, MUIA_Window_Open, FALSE);
		
//		DoMethod(app, MUIM_Application_Save, MUIV_Application_Save_ENVARC);
#warning not saving to envarc
		
		MUI_DisposeObject( app );
	}
	else tell("Couldn't create mui app!");
}


#define _GCyc(obj)		(xget(obj,MUIA_Cycle_Active))
#define _GBool(obj)		((BOOL)(xget(obj,MUIA_Selected)))
#define _GSli(obj)		(xget(obj,MUIA_Slider_Level))
#define _GStr(obj)		((STRPTR)(xget(obj,MUIA_String_Contents)))

HOOKP( AAT, LONG *value, AAT = (AccionATomar) (*value););

extern signed short *ahibuffer1, *ahibuffer2;
extern struct AHIRequest *AHIio, *AHIio2;
extern struct MsgPort *AHImp;

#define DBHEX( xx )		DBL("\t%s = 0x%08lx\n", (ULONG) #xx, (long) xx)

static struct speexdec *decoder_data( struct speexdec *s)
{
	LONG val;
	
	bzero( (APTR)s, sizeof(struct speexdec));
	
	DB("DECODER OPTIONS:\n")
	
	if((val = _GCyc( output_type)))
	{
		switch( val )
		{
			case 1:	SET_FLAG( s->attrs, SPEEXDEC_FORCENB); break;
			case 2:	SET_FLAG( s->attrs, SPEEXDEC_FORCEWB); break;
			case 3:	SET_FLAG( s->attrs, SPEEXDEC_FORCEUWB);break;
		}
		DBL("\tOutputType = %ld\n", val)
	}
	
	switch( (val = _GCyc(output_mode)) )
	{
		case 0:	SET_FLAG( s->attrs, SPEEXDEC_FORCEMONO); break;
		case 1:	SET_FLAG( s->attrs, SPEEXDEC_FORCESTEREO); break;
	}
	DBL("\tStereo = \"%s\"\n", (val ? "YES":"NO"))
	
	if(_GBool( decrateAct ))
		s->rate = _GSli( decrate );		DBL("\tRATE = %ld\n", s->rate)
	
	if(!_GBool( enh ))
	{
		SET_FLAG( s->attrs, SPEEXDEC_NOENH);	DBL("\tPerceptual enhacement DISABLED\n")
	}
	
	DBL("\tATTR = 0x%08lx\n", s->attrs)
	
	if((val = _GSli( time_start )))
		s->start = val;
	
	if((val = _GSli( time_stop )))
		s->stop = val;
	
	DBL("\tSTART = %ld, STOP = %ld\n", s->start, s->stop)
	
	s->input_file	= (char *)_GStr( input_file );
	s->output_file	= (char *)_GStr( output_file );
	
	DBL("\tInput = \"%s\"\n", s->input_file)
	DBL("\tOutput = \"%s\"\n\n", s->output_file)
	
	return (struct speexdec *) s;
}

HOOKP( Launch, LONG *mode,
{
	if((*mode)== TRUE)
	{
		switch( AAT )
		{
			case AAT_ENCODER:
			{
				tell("Sorry, the Encoder is unimplemented on this version");
			} break;
			case AAT_DECODER:
			{
				BOOL allow;
				
				Forbid();
				allow = (decoder_task ? FALSE:TRUE);
				Permit();
				
				if(!allow)
				{
					tell("Currently only a simultaneous %s is allowed.", (ULONG)"decoder");
				}
				else
				{
					static struct speexdec s;
					decoder_data( &s );
					
					//th_spawn(NULL, PROGRAM_NAME " decoder", &speexdec, 0, (APTR)&s);
					speexdec( &s );
				}
			} break;
			case AAT_PLAYER:
			{
				BOOL allow;
				
				Forbid();
				allow = (player_task ? FALSE:TRUE);
				Permit();
				
				if(!allow)
				{
					tell("real-time mixing not allowed :-P");
				}
				else
				{
					static struct speexdec s;
					decoder_data( &s );		s.output_file = "";
					
					//th_spawn(NULL, PROGRAM_NAME " player", &speexdec, 0, (APTR)&s);
					speexdec( &s );
				}
			} break;
			default:
			{
				tell("I can't do anything on that field!");
			} break;
		}
	}
	else
	{
		switch( AAT )
		{
			case AAT_ENCODER:
			{
				tell("Sorry, the Encoder is unimplemented on this version");
			} break;
			case AAT_DECODER:
			{
/*
				Forbid();
				if(decoder_task)
				{
					Signal( decoder_task, SIGBREAKF_CTRL_C);
				}
				else
				{
					tell("%s ISN't Active!", (ULONG)"decoder");
				}
				Permit();
*/
			} break;
			case AAT_PLAYER:
			{
/*
				Forbid();
				if(player_task)
				{
					Signal( player_task, SIGBREAKF_CTRL_C);
				}
				else
				{
					tell("%s ISN't Active!", (ULONG)"player");
				}
				Permit();
*/
			} break;
			default:
			{
				tell("I can't do anything on that field!");
			} break;
		}
	}
});

HOOKP( HerzSelector, ULONG *val,
{
	ULONG fake;
	ULONG value = (*val);
	
		 if( (value >  7000) && (value <  9000) )	fake =  8000;
	else if( (value > 10000) && (value < 11500) )	fake = 11025;
	else if( (value > 11500) && (value < 13000) )	fake = 12000;
	else if( (value > 14000) && (value < 18000) )	fake = 16000;
	else if( (value > 21000) && (value < 23000) )	fake = 22050;
	else if( (value > 23000) && (value < 25000) )	fake = 24000;
	else if( (value > 30000) && (value < 34000) )	fake = 32000;
	else if( (value > 42000) && (value < 46000) )	fake = 44100;
	else if( (value > 46000) && (value < 49000) )	fake = 48000;
	else fake = value;
	
	set( obj, MUIA_Slider_Level, fake );
});




LONG xget( Object *obj, ULONG attribute)	{
	LONG x;		get( obj, attribute, &x);		return(x);
}

/** Intuition/MUI Request passport...
 **/
LONG MYRequest( char *title, char *gadgets, const char *fmt, APTR args)
{
	if(!app && IntuitionBase)
	{
		struct EasyStruct es = {
			sizeof (struct EasyStruct), 0, title, (char *)fmt, gadgets
		};
		
		return EasyRequestArgs( NULL, &es, NULL, args);
	}
	
	if(!MUIMasterBase)
		return -1;
	
	return MUI_RequestA( app, win, 0, title, gadgets, (char *)fmt, args);
}


/**	Stolen from libmui ...
 **/
Object *MUI_NewObject(char *classname, Tag tag1, ...)
{
	return(MUI_NewObjectA(classname, (struct TagItem *) &tag1));
}
Object *MUI_MakeObject(LONG type, ...)
{
	return(MUI_MakeObjectA(type, (ULONG *)(((ULONG)&type)+4)));
}


/** Replacemnet Fucntions. We assume to compile using -nostdlib, so there are
 ** some common functions used almost all the time in mormal cases...
 **/
unsigned int strlen( const char *string)
{ const char *s=string;
  while(*s++); return ((unsigned int) ( ~(string-s) ));
}
void bzero(char * p, int len)
{
	while (len--) *p++ = '\0';
}
#if 0
char *strstr( const char *a, const char *b)	/* CASE INSensITiVE ! */
{
   int l = strlen(b);
   for (; *a; a++) if(Strnicmp(a, b, l) == 0) return (char *)a;
   return NULL;
}
#endif

#if 1
static UWORD struffChar[] = { 
  0x16c0,     // moveb %d0,%a3@+
  0x4e75      // rts
};

void SPrintF( char *dst, const char *fmt, APTR args)
{
	RawDoFmt( (UBYTE *) fmt, args, (void(*)(void)) &struffChar, dst );
}
#endif

ULONG filesize(char *filename)
{
	struct FileInfoBlock fib;
	register BPTR ccd_lock;
	
	ULONG file_size = 0;
	
	if((ccd_lock = Lock(filename, SHARED_LOCK)) && Examine(ccd_lock, &fib))
		file_size = fib.fib_Size;
	
	if(ccd_lock)
		UnLock( ccd_lock );
	
	return file_size;
}

int main()	{	return start();}
