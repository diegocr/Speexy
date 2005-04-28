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

#ifndef SPEEXY_SPEEXDEC_H
#define SPEEXY_SPEEXDEC_H


struct speexdec
{
	char *input_file;	/* regular Speex file */
	char *output_file;	/* NULL to play to AHI unit 0 */
	
	unsigned long attrs;	/* See flags.h */
	
	unsigned int rate;	/* Force decoding at sampling rate (Hz) */
	
	unsigned long start, stop;
	
	unsigned long volumen, balance;
#if 0	
	struct SpeexBase **SpeexBase;
	struct DosLibrary **DOSBase;
	struct UtilityBase **UtilityBase;
	
	signed short **ahibuffer1, **ahibuffer2;
	struct AHIRequest **AHIio, **AHIio2;
	struct MsgPort **AHImp;
#endif
};

extern BOOL player_task, decoder_task; // FIXME, no longer struct Task

#endif /* SPEEXY_SPEEXDEC_H */
