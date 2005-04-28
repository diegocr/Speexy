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


#define _PROTO_OGG_H 1
#include <libraries/ogg.h>
#include "amuistartup.h"	/* :ts=3	*/
#include "ahi.h"
#include "speexdec.h"
#include "flags.h"
#include "mui.h"

#include <math.h>
#include <string.h>
#include "wav_io.h"

#define readint(buf, base) (((buf[base+3]<<24)&0xff000000)| \
                           ((buf[base+2]<<16)&0xff0000)| \
                           ((buf[base+1]<<8)&0xff00)| \
  	           	    (buf[base]&0xff))
BOOL playing=FALSE;

/* minimun size to play a mono sound in narrowband mode */
ULONG ahibuffer_length		= 32768 + MAX_FRAME_SIZE;

static	struct Library *AHIBase		= NULL;
static	struct AHIRequest *AHIio	= NULL;
static	struct AHIRequest *AHIio2	= NULL;
static	struct MsgPort *AHImp		= NULL;
static	signed short *ahibuffer1	= NULL;
static	signed short *ahibuffer2	= NULL;
static	BYTE AHIDevice	= -1;

#include "write_wav_header.h"

BOOL player_task = FALSE, decoder_task = FALSE;
	
static void print_comments(char *comments, int length)
{
	   char *c=comments, *end;
	   int len, i, nb_fields;
	   
	   static char string[4096];	*string = 0;
	   
	   if (length<8)
	   {
	      info("Invalid/corrupted comments\n");
	      return;
	   }
	   end = c+length;
	   len=readint(c, 0);
	   c+=4;
	   if (c+len>end)
	   {
	      info("Invalid/corrupted comments\n");
	      return;
	   }
	   CopyMem( c, &string[strlen(string)], len);
	   c+=len;
	   if (c+4>end)
	   {
	      info("Invalid/corrupted comments\n");
	      return;
	   }
	   nb_fields=readint(c, 0);
	   c+=4;
	   for (i=0;i<nb_fields;i++)
	   {
	      if (c+4>end)	/* TODO: those return by breaks ? */
	      {
	         info("Invalid/corrupted comments\n");
	         return;
	      }
	      len=readint(c, 0);
	      c+=4;
	      if (c+len>end)
	      {
	         info("Invalid/corrupted comments\n");
	         return;
	      }
	      CopyMem( c, &string[strlen(string)], len);
	      c+=len;
	   }
	   
	   if(*string)
	   	info(string);
}

static void th_AHIClose( void )
{
		if(ahibuffer1)
			free( ahibuffer1 );
		if(ahibuffer2)
			free( ahibuffer2 );
	#if 0
		if(CheckIO((struct IORequest *) AHIio ))
		{
			AbortIO((struct IORequest *) AHIio);
			WaitIO ((struct IORequest *) AHIio);
		}
	#endif
		if(!AHIDevice)
			CloseDevice((struct IORequest *)AHIio);
		if(AHIio)
			DeleteIORequest((struct IORequest *)AHIio);
		if(AHIio2)
			DeleteIORequest((struct IORequest *)AHIio2);
		if(AHImp)
			DeleteMsgPort(AHImp);
}

BPTR out_file_open(char *outFile, int rate, int *channels)
{
	   BPTR fout = 0;		DB("Entering\n")
	   
	   if(playing)
	   {
			/* increase memory needed aprox.. */
			ahibuffer_length *= (ULONG) (( rate / 8000 ) + (*channels));
			
			DB("Opening AHI Device from Task 0x%08lX\n", (long) FindTask(0))
			
			if((AHImp = CreateMsgPort()))
			{
				if((AHIio = (struct AHIRequest *) CreateIORequest(AHImp,sizeof(struct AHIRequest))))
				{
					AHIio->ahir_Version = 4;
					AHIDevice = OpenDevice( AHINAME, AHI_DEFAULT_UNIT, (struct IORequest *)AHIio, NULL);
				}
				if(!AHIDevice)
				{
					AHIBase = (struct Library *) AHIio->ahir_Std.io_Device;
					
					DB("AHIBase == 0x%08lx\n", (long) AHIBase)
					
					if((AHIio2 = AllocMem(sizeof(struct AHIRequest), MEMF_PUBLIC)))
					{
						CopyMem( AHIio, AHIio2, sizeof(struct AHIRequest));
						
						if((ahibuffer1 = malloc( ahibuffer_length )))
							ahibuffer2 = malloc( ahibuffer_length );
					}
				}
			}
			
			if(! ahibuffer2 )
			{
				//putMsg(IPCA_ERROR, "Couldn't Init AHI Device!");
				return -1;
			}
	   } else {
	         fout = Open( outFile, MODE_NEWFILE);
	         if (!fout)
	         {
	            tell("Couldn't write to %s!", (ULONG)outFile);
	            return -1;
	         }
   	      if (Stricmp(outFile+strlen(outFile)-4,".wav")==0)
   	         write_wav_header(fout, rate, *channels, 0, 0);
   	}
   	
   	DB("Done\n")
   	
   	return fout;
}
	
static int _speex_std_stereo_request_handler(SpeexBits *bits, void *state, void *data) 
{
	return speex_std_stereo_request_handler( bits, state, data );
}
	
static void *process_header(ogg_packet *op, int enh_enabled, int *frame_size, int *rate, int *nframes, int forceMode, int *channels, SpeexStereoState *stereo, int *extra_headers)
{
	   void *st;
	   const SpeexMode *mode;
	   SpeexHeader *header;
	   int modeID;
	   SpeexCallback callback;		DB("Entering\n")
      
	   header = speex_packet_to_header((char*)op->packet, op->bytes);
	   if (!header)
	   {
	      tell("Cannot read header\n");
	      return NULL;
	   }
	   if (header->mode >= SPEEX_NB_MODES)
	   {
	      tell("Mode number %ld does not (yet/any longer) exist in this version\n", 
	               header->mode);
	      return NULL;
	   }
	   
	   modeID = header->mode;
	   if (forceMode!=-1)
	      modeID = forceMode;
	   mode = speex_mode_list[modeID];
	   
	   if (header->speex_version_id > 1)
	   {
	      tell("This file was encoded with Speex bit-stream version %ld,\nwhich I don't know how to decode\n", header->speex_version_id);
	      return NULL;
	   }
	
	   if (mode->bitstream_version < header->mode_bitstream_version)
	   {
	      tell("The file was encoded with a newer version of Speex.\nYou need to upgrade in order to play it.\n");
	      return NULL;
	   }
	   if (mode->bitstream_version > header->mode_bitstream_version) 
	   {
	      tell("The file was encoded with an older version of Speex.\nYou would need to downgrade the version in order to play it.\n");
	      return NULL;
	   }
	   
	   st = speex_decoder_init(mode);
	   if (!st)
	   {
	      tell("Decoder initialization failed.\n");
	      return NULL;
	   }
	   speex_decoder_ctl(st, SPEEX_SET_ENH, &enh_enabled);
	   speex_decoder_ctl(st, SPEEX_GET_FRAME_SIZE, frame_size);
	
	   if (!(*channels==1))
	   {
	      callback.callback_id = SPEEX_INBAND_STEREO;
	      callback.func = _speex_std_stereo_request_handler;
	      callback.data = stereo;
	      speex_decoder_ctl(st, SPEEX_SET_HANDLER, &callback);
	   }
	   if (!*rate)
	      *rate = header->rate;
	   /* Adjust rate if --force-* options are used */
	   if (forceMode!=-1)
	   {
	      if (header->mode < forceMode)
	         *rate <<= (forceMode - header->mode);
	      if (header->mode > forceMode)
	         *rate >>= (header->mode - forceMode);
	   }
		
	   speex_decoder_ctl(st, SPEEX_SET_SAMPLING_RATE, rate);
		
	   *nframes = header->frames_per_packet;
		
	   if (*channels==-1)
	      *channels = header->nb_channels;
/*		
	//      putMsg( IPCA_STATUS_INFO, (const char *) 
	      set( status_info, MUIA_Textinput_Contents, (ULONG)
	      	"Decoding %ld Hz audio using %s %smode", 
	               (ULONG)(*rate), (ULONG)mode->modeName, (ULONG)((header->vbr) ? "VBR ":""));
*/
#warning fixme
#if 0	
	      if (*channels==1)
	         info(" (mono");
	      else
	         info(" (stereo");
	      
	      if (header->vbr)
	         info(", VBR)\n");
	      else
	         fprintf(stderr, ")\n");
	      /*info("Decoding %d Hz audio at %d bps using %s mode\n", 
	       *rate, mode->bitrate, mode->modeName);*/
#endif
	   *extra_headers = header->extra_headers;
	
	   free(header);		DB("Done\n")
	   return st;
}
	
	/************************************************************************/
	/************************************************************************/
	/****************************  M A I N  *********************************/
	/************************************************************************/
	/************************************************************************/

void speexdec(struct speexdec *sdata)
{
   char *inFile, *outFile;
   BPTR fin = 0, fout = 0;
   short out[MAX_FRAME_SIZE];
   short output[MAX_FRAME_SIZE];
   int frame_size=0;
   void *st=NULL;
   SpeexBits bits;
   int packet_count=0;
   int stream_init = 0;
   
   ogg_sync_state oy;
   ogg_page       og;
   ogg_packet     op;
   ogg_stream_state os;
   int enh_enabled;
   int nframes=2;
   int close_in=0;
   int eos=0;
   int forceMode=-1;
   int audio_size=0;
   int start_time = 0, stop_time = 0;
   int frames_counter = 0;
   SpeexStereoState stereo = SPEEX_STEREO_STATE_INIT;
   int channels=-1;
   int rate=0;
   int extra_headers;
   int wav_format=0;
   long tsig, volumen, balance;
   ULONG file_length, remaining, n;
   
   struct AHIRequest *link = NULL;
   ULONG sigs, AHIStep = 0;
   APTR tp;
   
   DB("Initiliazing decoder/player\n")
   
   player_task = decoder_task = TRUE;
   
	outFile	= sdata->output_file;
   playing = (BOOL) (strlen(outFile)==0);
   
   enh_enabled = 1;
   
   if(isFlagSet( sdata->attrs, SPEEXDEC_NOENH))
   	enh_enabled=0;
   if(isFlagSet( sdata->attrs, SPEEXDEC_FORCENB))
   	forceMode = 0;
   else if(isFlagSet( sdata->attrs, SPEEXDEC_FORCEWB))
   	forceMode = 1;
   else if(isFlagSet( sdata->attrs, SPEEXDEC_FORCEUWB))
   	forceMode = 2;
   if(isFlagSet( sdata->attrs, SPEEXDEC_FORCEMONO))
   	channels = 1;
   else if(isFlagSet( sdata->attrs, SPEEXDEC_FORCESTEREO))
   	channels = 2;
	
	if(sdata->rate)
		rate = sdata->rate;
	
	if(sdata->start)
		start_time = sdata->start;
	
	if(sdata->stop)
		stop_time = sdata->stop;
	
	volumen	= sdata->volumen;
	balance	= sdata->balance;
	
   inFile	= sdata->input_file;
			DB("Input = \"%s\"\n", inFile)
   		DB("Output = \"%s\"\n", outFile)
   
   wav_format = strlen(outFile)>=4 && (!Stricmp(outFile+strlen(outFile)-4,".wav"));
   
   DB("player?, %s\n", (playing ? "YES":"NO"))
   
   if(!(inFile && *inFile))
   {
   	tell("Select a input filename !");	goto quit;
   }
   
   /*Open input file*/
   fin = Open( inFile, MODE_OLDFILE);
   if (!fin)
   {
      tell("Couldn't open %s!", (ULONG)inFile);
      goto quit;
   }
   close_in=1;
   
   file_length = remaining = filesize( inFile );
   
   info("%s \"%s\"", (ULONG)(playing ? "Playing":"Decoding"), (ULONG)inFile);
   
   /*Init Ogg data struct*/
   ogg_sync_init(&oy);
   
   speex_bits_init(&bits);
   
   #define MUI_EVENTS	(DoMethod(app, MUIM_Application_NewInput, &sigs)\
										  != (ULONG) MUIV_Application_ReturnID_Quit)
   
   /*Main decoding loop*/
   //while (1)
   while(MUI_EVENTS)
   {
      char *data;
      int i, j, nb_read;
      unsigned int ahibufferCnt = 0;
      
      /*Get the ogg buffer for writing*/
      data = ogg_sync_buffer(&oy, 200);
      
      /*Read bitstream from input file*/
      nb_read = FRead(fin, data, sizeof(char), 200);      
      ogg_sync_wrote(&oy, nb_read);

      /*Loop for all complete pages we got (most likely only one)*/
      while(ogg_sync_pageout(&oy, &og)==1 && MUI_EVENTS)
      {
         if (stream_init == 0) {
            ogg_stream_init(&os, ogg_page_serialno(&og));
            stream_init = 1;
         }
         /*Add page to the bitstream*/
         ogg_stream_pagein(&os, &og);
         /*Extract all available packets*/
         while (!eos && ogg_stream_packetout(&os, &op)==1 && MUI_EVENTS)
         {
            /*If first packet, process as Speex header*/
            if (packet_count==0)
            {
               st = process_header(&op, enh_enabled, &frame_size, &rate, &nframes, forceMode, &channels, &stereo, &extra_headers);
               if (!nframes)
                  nframes=1;
               if (!st)
                  goto quit;
               if((fout = out_file_open(outFile, rate, &channels)) == -1)
               {
               	tell("something went wromg (AHI?)...");	goto quit;
               }

            } else if (packet_count==1)
            {
            		info("****** %s ***********************", (ULONG)"COMMENTS START");
               print_comments((char*)op.packet, op.bytes);
            		info("****** %s ***********************", (ULONG)"COMMENTS END");
            } else if (packet_count<=1+extra_headers)
            {
               /* Ignore extra headers */
            } else {
               
               frames_counter += nframes * frame_size;
               
               /*End of stream condition*/
               if (op.e_o_s)
                  eos=1;
               
               if(start_time)
               {
                  	audio_size += sizeof(short)*frame_size*channels;
                  	
                  	if((int)( (float)frames_counter / (float)rate ) >= start_time)
                  		start_time = 0;
                  	
               		continue;
               }
               
               /*Copy Ogg packet to Speex bitstream*/
               speex_bits_read_from(&bits, (char*)op.packet, op.bytes);
               
               for (j=0;j!=nframes;j++)
               {
                  int ret;
                  /*Decode frame*/
                  ret = speex_decode(st, NULL, output);

                  if (ret==-1)
                     break;
                  if (ret==-2)
                  {
                     tell("Decoding error: corrupted stream?\n");
                     break;
                  }
                  if (speex_bits_remaining(&bits)<0)
                  {
                     tell("Decoding overflow: corrupted stream?\n");
                     break;
                  }
                  if (channels==2)
                     speex_decode_stereo(output, frame_size, &stereo);
                  
                  /*Convert to short and save to output file*/
		  				if (!playing)
                  {
                     for (i=0;i<frame_size*channels;i++)
                        out[i]=le_short(output[i]);
                     
                     FWrite(fout, out, sizeof(short), frame_size*channels);
                     
		  				} else
		  				{
		     				LONG *memptr = (LONG*)ahibuffer1, ahibufLen = ((*(memptr-1)) - (sizeof(ULONG)*2));
                     for (i=0;i<frame_size*channels;i++)
                     {
                        ahibuffer1[ahibufferCnt++] = output[i];
                        
                        if(ahibufferCnt == ( ahibufLen / sizeof(short)))
                        {
                        	#define AHI_REALLOC_SIZE	4096
                        	
                        	if(!(ahibuffer1 = realloc( ahibuffer1, (ahibufLen + AHI_REALLOC_SIZE))))
                        	{
                        		tell("out of memory...");	goto quit;
                        	}
                        	
                        	ahibufLen += AHI_REALLOC_SIZE;
                        }
                     }
		  			   }
                  
                  audio_size+=sizeof(short)*frame_size*channels;
               }
            }
            packet_count++;
         }
      }
      
      if (ahibufferCnt && playing)
      {
//      	putUMsg( IPCA_STATUS_TIME, ((ULONG)( (float)frames_counter / (float)rate )));
#warning fixme, timing
      	
      	AHIio->ahir_Std.io_Message.mn_Node.ln_Pri = 0;
      	AHIio->ahir_Std.io_Command  = CMD_WRITE;
      	AHIio->ahir_Std.io_Data     = ahibuffer1;
      	AHIio->ahir_Std.io_Length   = (ahibufferCnt-1) * sizeof(short);
      	AHIio->ahir_Std.io_Offset   = 0;
      	AHIio->ahir_Frequency       = rate;
      	AHIio->ahir_Type            = ((channels == 1) ? AHIST_M16S : AHIST_S16S);
      	AHIio->ahir_Volume          = 0x10000;
      	AHIio->ahir_Position        = 0x8000;
      	AHIio->ahir_Link            = link;
      	SendIO((struct IORequest *) AHIio);			AHIStep++;
      	
      	if(link)
      	{
      		sigs = Wait( (1L << AHImp->mp_SigBit) | SIGBREAKF_CTRL_C);
      		if(sigs & SIGBREAKF_CTRL_C)
      		{
      			AbortIO((struct IORequest *) AHIio);
      			WaitIO((struct IORequest *) AHIio);
      			
      			if(AHIStep > 1)
      			{
      				AbortIO((struct IORequest *) AHIio2);
      				WaitIO((struct IORequest *) AHIio2);
      			}
      			
      			info("Player interrupted");
      			
      			break;
      		}
      		if(WaitIO((struct IORequest *) link))
      		{
      			tell("AHI I/O Error!"); break;
      		}
      	}
      	
      	link		= AHIio;
      	
      	/* switch buffer for double buffering */
      	tp				= ahibuffer1;
      	ahibuffer1	= ahibuffer2;
      	ahibuffer2	= tp;
      	
      	tp		= AHIio;
      	AHIio		= AHIio2;
      	AHIio2		= tp;
      }
      if(nb_read == 0)
         break;
   }

   if (wav_format)
   {
      if (Seek(fout,4,OFFSET_BEGINNING) != -1)
      {
         int tmp;
         tmp = le_int(audio_size+36);
         FWrite(fout, &tmp,4,1);
         if (Seek(fout,32,OFFSET_CURRENT) != -1)
         {
            tmp = le_int(audio_size);
            FWrite(fout, &tmp,4,1);
         } else
         {
            tell("First seek worked, second didn't\n");
         }
      } else {
         tell("Cannot seek on wave file, size will be incorrect\n");
      }
   }

quit:
	
   if (st)
      speex_decoder_destroy(st);
   else if(fin)
   {
      tell("This doesn't look like a Speex file");
   }
   speex_bits_destroy(&bits);
   if (stream_init)
      ogg_stream_clear(&os);
   ogg_sync_clear(&oy);

	if (AHIStep)
		WaitIO((struct IORequest *) AHIio2);

   if (close_in)
      Close(fin);
   if (fout)
      Close(fout);   

   info("Player finished");
   
   player_task = decoder_task = FALSE;
   
   th_AHIClose();
}
