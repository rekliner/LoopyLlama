//-------------------------------------------------------------------------------------------------------
//Loopy Llama by Chris Kline
//-------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <math.h>

#include <time.h>
 
#ifndef __LoopyLlama_H
#include "LoopyLlama.hpp"
#endif

//#if MAC
//#include "macpaths.cpp"
//#endif

//  exclude these for non-editor 
#ifndef __LoopyLlamaEdit__
#include "LoopyLlamaEditor.h"
#endif

#ifndef __AEffEditor__
#include "AEffEditor.hpp" 
#endif

//-------------------------------------------------------------------------------------------------------
LoopyLlama::LoopyLlama (audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, kNumPrograms, kNumParams)	// 1 program, 1 parameter only
{
	canMono ();				// makes sense to feed both inputs with the same signal
	canProcessReplacing (true);	// supports both accumulating and replacing output

	sampleRate = getSampleRate ();

//	char* test = "";
//extern HINSTANCE hModule;



	setNumInputs (2);		
	setNumOutputs (3);		
	setUniqueID ('LpLa');	// identify
//	beatLength = sampleRate / 2;
	LPL.setC0(1.f);
	LPR.setC0(1.f);
	bpm = 100;
	loopSize = 88200;
	fadeTime = long(sampleRate / 64);
	bufferDirection = preBufferDirection = 1;
	resetAt = 0;
	tempoLightOn = 0;
	tapCount = 0;
	lastMetronome = 0; 
	beatOffset = 0;
	countSamples = 0;
	beatBufferCopy = 0;
	firstLoop=1;
	metronomeL = metronomeR = NULL;
	init = false;
//	speedRatio = 1;
//	lastInSample = 0;
//	oldPpq = 0;

    _masterTimeInfo.timeSigDenominator = 4;
    _masterTimeInfo.timeSigNumerator   = 4;
	_masterTimeInfo.sampleRate = 44100;
	_masterTimeInfo.barStartPos = 0;
	_masterTimeInfo.cycleStartPos = 0;
	_masterTimeInfo.cycleEndPos = 0;
	_masterTimeInfo.samplesToNextClock = 0;

	tapTimes = new unsigned long [maxTaps];
	programs = new LoopyLlamaProgram [kNumPrograms];
	buffer = new float[long(sampleRate * maxSeconds)];
	preBuffer = new float[long(sampleRate * maxSeconds)]; //60 seconds of loop time...do you need more?
	buffer2 = new float[long(sampleRate * maxSeconds)];
	preBuffer2 = new float[long(sampleRate * maxSeconds)]; //60 seconds of loop time...do you need more?
	beatBuffer = new float[long(sampleRate * (60 / (50 / 2)))]; //2 beats at 50bpm just to be safe 
	beatBuffer2 = new float[long(sampleRate * (60 / (50 / 2)))]; //2 beats at 50bpm just to be safe 

//	memset(&tapTimes,0,sizeof(tapTimes);
	tapCount = 0;

	//denormal code courtesy of musicdsp.org - References : Hal Chamberlain, "Musical Applications of Microprocessors"
	denormalNoiseLoop = new float[noiseLoopSize];
	tapeNoiseLoop = new float[noiseLoopSize];
	rand_state = 1; // random seed
	for (noiseLoopCursor = 0;noiseLoopCursor < noiseLoopSize;noiseLoopCursor++) {
		denormalNoiseLoop[noiseLoopCursor] = makeNoise(0); //fill the noise loop with noise
		tapeNoiseLoop[noiseLoopCursor] = makeNoise(1); //fill the noise loop with noise
	}
	noiseLoopCursor = 0;
	
	if (programs) { 
	int cp = 1;
	//DEFAULT PROGRAMS

	strcpy (programs[cp].name, "DLl-4");
		programs[cp].fRecordMode = 0.25f;  //endpoints
	cp++;

	strcpy (programs[cp].name, "FripperLlama");
		programs[cp].fFeedBack = .8f;  //reduced feedback - loop will decay
		programs[cp].fEQFreq = .7f;  //eq out high end
		programs[cp].fFadeTime = 0.1f;	//slower fade in
		programs[cp].fTapeHiss = .25f;	//bit o tape hiss
	cp++;

	strcpy (programs[cp].name, "Moonwalking Llama");
		programs[cp].fBeats = 0.0305f;	//4 bars
		programs[cp].fDirection = 0.0f; //reverse
		programs[cp].fFeedBack = 0.0f; //no feedback - 1 loop only
	cp++;

	strcpy (programs[cp].name, "16 Bar Llooper");
		programs[cp].fBeats = 0.122f;	//16 bars
		programs[cp].fRecordButtonCC = 0.388f; //CC50
		programs[cp].fResetButtonCC = 0.4f;	//CC51
	cp++;

/*	strcpy (programs[cp].name, "Reverse Slave");
		programs[cp].fDry = 0.0f;  //no dry volume for parallel loopers
		programs[cp].fFadeTime = 0.1f;
		programs[cp].fDirection = 0.0f;
	cp++;

	strcpy (programs[cp].name, "Slave Llama");
		programs[cp].fDry = 0.0f;  //no dry volume for parallel loopers
		programs[cp].fFadeTime = 0.1f;	//slower fade time 500 samples or so
	cp++;

	strcpy (programs[cp].name, "16 Bar Slave");
		programs[cp].fBeats = 0.122f;	//16 bars
		programs[cp].fDry = 0.0f;  //no dry volume for parallel loopers
		programs[cp].fFadeTime = 0.1f;
	cp++;

	strcpy (programs[cp].name, "16 Bar Reverse Slave");
		programs[cp].fBeats = 0.122f;	//16 bars
		programs[cp].fDirection = 0.0f;
		programs[cp].fDry = 0.0f;  //no dry volume for parallel loopers
		programs[cp].fFadeTime = 0.1f;
	cp++;*/
	}


	setProgram(0);
	isRecording = 0;
	resetLoop(1); 
	editor = new LoopyLlamaEditor (this);  //disclude for non-editor

	
}

//-------------------------------------------------------------------------------------------------------
LoopyLlama::~LoopyLlama ()
{
	if (buffer)	{
		delete[] buffer;
		buffer = 0;
	}
	if (buffer2)	{
		delete[] buffer2;
		buffer2 = 0;
	}
	if (preBuffer) {
		delete[] preBuffer;
		preBuffer = 0;
	}
	if (preBuffer2) {
		delete[] preBuffer2;
		preBuffer2 = 0;
	}
	if (beatBuffer) {
		delete[] beatBuffer;
		beatBuffer = 0;
	}
	if (beatBuffer2) {
		delete[] beatBuffer2;
		beatBuffer2 = 0;
	}
	if (programs) {
		delete[] programs;
		programs = 0;
	}
	if (denormalNoiseLoop) {
		delete[] denormalNoiseLoop;
		denormalNoiseLoop = 0;
	}
	if (tapTimes) {
		delete[] tapTimes;
		tapTimes = 0;
	}
	if (metronomeL) {
		delete[] metronomeL;
		metronomeL = 0;
	}
	if (metronomeR) {
		delete[] metronomeR;
		metronomeR = 0;
	}
}

//------------------------------------------------------------------------
void LoopyLlama::resume ()
{
	if (init == false) {
		loadFile(NULL);
		init = true;
	}
}

//------------------------------------------------------------------------
float LoopyLlama::makeNoise (int noiseType)
{
	int mantissa, flt_rnd;
	rand_state = rand_state * 1234567UL + 890123UL;  //random 32 bits
	if (noiseType == 0) { //0 is denormal noise
		mantissa = rand_state & 0x807F0000; // Keeps sign and first 6 bits of mantissa...zeros out exponent
		flt_rnd = mantissa | 0x1E000000; // Set exponent to 2^-67 or about 10^-20
	} else { //1 or whatever is for white noise		
		mantissa = rand_state & 0x807FFFFF; // Keeps sign and zeros out exponent
		flt_rnd = mantissa |	0x3E800000; // Set exponent to 2^-3
	//	return int(rand_state) / 2147483648.f; //this is probably expensive i give up...
	}
	return *reinterpret_cast <const float *> (&flt_rnd);
}

//-------------------------------------------------------------------------------------------------------

long LoopyLlama::canDo (char* text){

	if (!strcmp (text, "sendVstMidiEvent"))    return 1;
	if (!strcmp (text, "sendVstEvents"))       return 1;

	if (!strcmp (text, "receiveVstEvents"))    return 1;
	if (!strcmp (text, "receiveVstMidiEvent")) return 1;

    if (!strcmp (text, "receiveVstTimeInfo"))  return 1;

    if (!strcmp (text, "sendVstTimeInfo"))     return 1;


	return -1;	// explicitly can't do; 0 => don't know
}
//program stuff----------------------------------------------------------------------------- 
LoopyLlamaProgram::LoopyLlamaProgram ()
{
	// default Program Values
	fBeats = 0.061f;	//16 bars
	fBPM = 0.210f; //136 bpm
	fRecord = 0.0f;	
	fReset = 0.0f;
	fPlay = 0.0f;		
	fPaused = 0.0f;	
	fSpeed = 0.5f;
	fIn = 1.0f;		//Full In volume
	fLoopVol = 1.0f;	//Full Loop Volume
	fDry = 1.0f;	//Full Dry Volume
	fFeedBack = 1.0f;	//Infinite Looping
	fFadeTime = 0.05f;	//~256sample fades
	fEQFreq = 1.0f;		//LPF off
	fTapeHiss = 0.0f;	//Tape Hiss off
	fDirection = 1.0f;	//playing forward
	fRecordMode = 0.0f;	//tap tempo mode
	fOneLoop = 0.0f;	//one loop off 
	fLoopSetsBeats = 0.0f;	//first loop sets number of beats
	fReverseMode = 0.0f;	//immediate reverse
	fRecordAt = 0.0f;		//next sample
//	fRecordButtonMode = 0.0f;//momentary		
	fNoteCC = 0.0f;		//controller taps
	fTapTempoCC = 0.192f; //cc or note 25 for no particular reason
	fRecordButtonCC = 0.314f; //cc or note 40 for no particular reason
	fResetButtonCC = 0.325f; //cc or note 42 for no particular reason
	fPlayButtonCC = 0.318f; //cc or note 41 for no particular reason
	fPausedButtonCC = 0.334f; //cc or note 43 for no particular reason
	fDirectionCC = 0.342f; //cc or note 44 for no particular reason
	fFeedbackCC = 0.349f; //cc or note 45 for no particular reason
	fLpfCC = 0.355f; //cc or note 46 for no particular reason
	fMetVolCC = 0.365f; //cc or note 47 for no particular reason
	fMIDIChannel = 0.f; //channel 1 for no particular reason
	fTaps = 0.36f;  //4 taps 
	fMetronomeNote = 0.65625f;	//i don't even know
	fMetronomeVolume = 0.65625f;	//i don't even know
	fMetronomeInMains = 0.f; // metronome off
	//	fAddProgram = 0.0f;		//don't add 1 to incoming program changes
	fUnpausePlay = 0.0f;		//unpausing doesn't start the loop over
	strcpy (name, "8 Bar Looper");

}

//------------------------------------------------------------------------
void LoopyLlama::setProgram (long program)
{
	LoopyLlamaProgram* cp = &programs[program];



	curProgram = program;
	setParameter (kBeats, cp->fBeats);	
	setParameter (kBPM, cp->fBPM);	
	setParameter (kRecord, cp->fRecord);
	setParameter (kReset, cp->fReset);
	setParameter (kPlay, cp->fPlay);
	setParameter (kPaused, cp->fPaused);
	setParameter (kSpeed, cp->fSpeed);
	setParameter (kIn, cp->fIn);
	setParameter (kLoopVol, cp->fLoopVol);
	setParameter (kDry, cp->fDry);
	setParameter (kFeedBack, cp->fFeedBack);
	setParameter (kFadeTime, cp->fFadeTime);
	setParameter (kEQFreq, cp->fEQFreq);
	setParameter (kDirection, cp->fDirection);
	setParameter (kRecordMode, cp->fRecordMode);
	setParameter (kOneLoop, cp->fOneLoop);
	setParameter (kLoopSetsBeats, cp->fLoopSetsBeats);
	setParameter (kReverseMode, cp->fReverseMode);
	setParameter (kTapeHiss, cp->fTapeHiss);
	setParameter (kRecordAt, cp->fRecordAt);
//	setParameter (kRecordButtonMode, cp->fRecordButtonMode);
	setParameter (kNoteCC, cp->fNoteCC);
	setParameter (kTapTempoCC, cp->fTapTempoCC);
	setParameter (kRecordButtonCC, cp->fRecordButtonCC);
	setParameter (kResetButtonCC, cp->fResetButtonCC);
	setParameter (kPlayButtonCC, cp->fPlayButtonCC);
	setParameter (kPausedButtonCC, cp->fPausedButtonCC);
	setParameter (kDirectionCC, cp->fDirectionCC);
	setParameter (kFeedbackCC, cp->fFeedbackCC);
	setParameter (kLpfCC, cp->fLpfCC);
	setParameter (kMetVolCC, cp->fMetVolCC);
	setParameter (kMIDIChannel, cp->fMIDIChannel);
	setParameter (kTaps, cp->fTaps);
	setParameter (kMetronomeNote, cp->fMetronomeNote);
	setParameter (kMetronomeVolume, cp->fMetronomeVolume);
	setParameter (kMetronomeInMains, cp->fMetronomeInMains);
//	setParameter (kAddProgram, cp->fAddProgram);
	setParameter (kUnpausePlay, cp->fUnpausePlay);

}



//-------------------------------------------------------------------------------------------------------
void LoopyLlama::setProgramName (char *name)
{
	LoopyLlamaProgram* cp = &programs[curProgram];
	strcpy (cp->name, name);
}

//-----------------------------------------------------------------------------------------
void LoopyLlama::getProgramName (char *name)
{
	LoopyLlamaProgram* cp = &programs[curProgram];
	strcpy (name,cp->name );
}




//-----------------------------------------------------------------------------
long LoopyLlama::onKeyDown (VstKeyCode &keyCode)  ///why doesn't this work in some hosts?
{
//	LoopyLlamaProgram* cp = &programs[curProgram];
	switch (keyCode.character) {
	case 't' :
	case 'T' :
	case ' ' :
		setParameterAutomated(kTapHappened,1.f);
		//	setParameter(kBPM,0.f);
		return 1;
	case 'r' :
	case 'R' :
		setParameterAutomated(kRecord,1.f);
//		if (cp->fRecordButtonMode < 0.5f)
//			setParameterAutomated(kRecord,0.f);
		return 1;
	case 'e' :
	case 'E' :
		setParameterAutomated(kReset,1.f);
		return 1;
	case 'q' :
	case 'Q' :
	case 'p' :
	case 'P' :
		setParameterAutomated(kPlay,1.f);
		return 1;
	case 'w' :
	case 'W' :
		setParameterAutomated(kPaused,1.f - getParameter(kPaused));
		return 1;
	default:
		return -1;
	}
}



//-----------------------------------------------------------------------------------------


void LoopyLlama::tapHappened (float tapType) {
	LoopyLlamaProgram* cp = &programs[curProgram];
	switch (int(cp->fRecordMode * 4)) {
	case 0: // in tap-tempo mode
		if ((countSamples - lastMetronome) > (sampleRate / 6)) {  //if its been more than 1/6 sec since the last metronome pulse send one
			sendMIDIData(char(cp->fMIDIChannel * 15) | 0x90,sliderTo7Bit(cp->fMetronomeNote),char(cp->fMetronomeVolume * 127),0); //send a metronome pulse
			metCursor = 0;
		}
		if (tapCount < (int(cp->fTaps * maxTaps + 1))) { //this is a buildup tap
			tapCount++;
			tapTimes[tapCount] = countSamples;// + deltaFrames;
		} else {  //if tapcount has just reached its goal
			if (tapCount < (int(cp->fTaps * maxTaps + 2))) {
				tapCount++;
			} else { //if its was already there and we kept tapping
				for (int i = 1; i <= tapCount; i++) 
					tapTimes[i - 1] = tapTimes[i];  //shift every entry over 1
			}
			tapTimes[tapCount] = countSamples;// + deltaFrames;
	//		setBeatLength(tapTimes[tapCount] - tapTimes[1],tapCount - 1);

		//	beatLength = double((tapTimes[tapCount] - tapTimes[1]) / (tapCount - 1));
		//	loopSize = (unsigned long)(beats * beatLength);

				//		bpm = value * 250 + 50;
				//	    loopSize = long((60.f * sampleRate * beats) / bpm);

		//	cp->fBPM = float(double(loopSize) / beats - (sampleRate / 10.f))/ float(sampleRate * 1.2f - (sampleRate / 10.f));
			bpm = (60.f * sampleRate) / ((double(tapTimes[tapCount] - tapTimes[1]) / (tapCount - 1)));
			while (bpm > 250 || bpm < 50) {  //make sure the tempo doesn't get all whacky
				if (bpm > 250) {	//halve the number of bars if it is too high
					bpm /= 2;
				}
				if (bpm < 50) {		//double the bars if it is too slow
					bpm *= 2;
				}
//				bpm = (60 * sampleRate * beats) / loopSize;
			}
			resetLoop(1);  //reset the loop and the click start point

			setParameterAutomated(kBPM,float((bpm - 50) / 250.f));  //update the slider

			if (editor)
				editor->update();
		
		}
		lastMetronome = countSamples;


		if (editor && (tapType == 1.f)) 
			editor->update();
			
			//((AEffGUIEditor*)editor)->setParameter(kTapHappened,1.f);  //turn the metronome light on
		break;
	case 1: //treat it as an endpoint tap if in endpoints mode
		setRecording(1.f); //need to update this to respond to sticky mode
		break;
	default: //otherwise ignore
		break;
	}

}
//****************  end of responses to parameters

//-----------------------------------------------------------------------------------------
float LoopyLlama::getParameter (long index)
{
	float v = 0;
	float temp = 0;
	LoopyLlamaProgram* cp = &programs[curProgram];

	switch (index)
	{
		case kBeats :			
			v = cp->fBeats;			
			break;
		case kBPM :				
			v = cp->fBPM;	
			break;
		case kRecord :			
			v = cp->fRecord;		
			break;
		case kReset :			
			v = cp->fReset;			
			break;
		case kPlay :			
			v = cp->fPlay;			
			break;
		case kPaused :			
			v = cp->fPaused;		
			break;
		case kSpeed :			
			v = cp->fSpeed;			
			break;
		case kIn :				
			v = cp->fIn;			
			break;
		case kLoopVol :			
			v = cp->fLoopVol;			
			break;
		case kDry :				
			v = cp->fDry;			
			break;
		case kFeedBack :		
			v = cp->fFeedBack;		
			break;
		case kFadeTime :		
			v = cp->fFadeTime;		
			break;
		case kEQFreq :			
			v = cp->fEQFreq;		
			break;
		case kTapeHiss :			
			v = cp->fTapeHiss;		
			break;
		case kDirection :		
			v = cp->fDirection;		
			break;
		case kRecordMode :		
			v = cp->fRecordMode;		
			break;
		case kOneLoop :			
			v = cp->fOneLoop;		
			break;
		case kLoopSetsBeats :			
			v = cp->fLoopSetsBeats;		
			break;
		case kReverseMode :		
			v = cp->fReverseMode;		
			break;
		case kNoteCC :		
			v = cp->fNoteCC;		
			break;
		case kRecordAt :		
			v = cp->fRecordAt;		
			break;
//		case kRecordButtonMode :		
//			v = cp->fRecordButtonMode;		
//			break;
		case kTapTempoCC :		
			v = cp->fTapTempoCC;	
			break;
		case kRecordButtonCC :	
			v = cp->fRecordButtonCC;	
			break;
		case kResetButtonCC :	
			v = cp->fResetButtonCC;	
			break;
		case kPlayButtonCC :	
			v = cp->fPlayButtonCC;	
			break;
		case kPausedButtonCC :	
			v = cp->fPausedButtonCC;	
			break;
		case kDirectionCC :	
			v = cp->fDirectionCC;	
			break;
		case kFeedbackCC :	
			v = cp->fFeedbackCC;	
			break;
		case kLpfCC :	
			v = cp->fLpfCC;	
			break;
		case kMetVolCC :	
			v = cp->fMetVolCC;	
			break;
		case kMIDIChannel :		
			v = cp->fMIDIChannel;	
			break;
		case kTaps :			
			v = cp->fTaps;	
			break;
		case kMetronomeNote :	
			v = cp->fMetronomeNote;		
			break;
		case kMetronomeVolume :	
			v = cp->fMetronomeVolume;		
			break;
		case kMetronomeInMains :	
			v = cp->fMetronomeInMains;		
			break;
//		case kAddProgram :		
//			v = cp->fAddProgram;	
//			break;
		case kUnpausePlay :		
			v = cp->fUnpausePlay;	
			break;
		case kIsRecording :		
			v = float(isRecording && !dontCopyThisLoop);	
			break;
		case kIsLoopRunning :	
			v = float(loopRunning && !firstLoop);	
			break;
		case kIsTempoOn :		
			v = float(tempoLightOn);	
			break;
		case kCurrentBar :
			temp = getParameter(kCurrentPos);
			v = float(int(temp * beats  //an integer from 0 to the number of bars (x)
				+ .1f * bufferDirection //it's a bit ahead to compensate for float precision and since the gui updates only twice a beat
				//- (beats * (temp * beats >= beats)) // makes it go to beginning instead of reaching highest point
				) + 1);  //makes it go from 1 to x+1 instead of 0 to x
			if (v > beats) v -= beats; //if the rounding makes it too high, wrap it back around
			if (v < 1) v += beats;		//if it rounds too low do the same
			break;

		case kVUlevel :			
			v = float(peakVU);peakVU = 0;	
			break;
		case kCurrentPos :	
			v = (float(loopCursor) / loopSize * ((int(cp->fRecordMode * 4) != 1) || (loopRunning))) * !firstLoop;	
			if (bufferDirection < 0 && !firstLoop) v = 1.f - v;
			break;
		case kTapCount :	
			v = float(tapCount);	
			break;
	}
	return v;
}

//-----------------------------------------------------------------------------------------
void LoopyLlama::getParameterName (long index, char *label)
{
	switch (index)
	{
		case kBeats :			strcpy (label, "Bars");				break;
		case kBPM :				strcpy (label, "Length of a beat");	break;
		case kRecord :			strcpy (label, "Record");			break;
		case kReset :			strcpy (label, "Reset");			break;
		case kPlay :			strcpy (label, "Play");				break;
		case kPaused :			strcpy (label, "Pause");			break;
		case kSpeed :			strcpy (label, "Speed");			break;
		case kIn :				strcpy (label, "In Volume");		break;
		case kLoopVol :			strcpy (label, "Loop Volume");		break;
		case kDry :				strcpy (label, "Dry Volume");		break;
		case kFeedBack :		strcpy (label, "FeedBack");			break;
		case kFadeTime :		strcpy (label, "Fade Time");		break;
		case kEQFreq :			strcpy (label, "Low Pass EQ Freq");	break;
		case kTapeHiss :		strcpy (label, "Low Pass EQ Slope");break;
		case kDirection :		strcpy (label, "Playback Direction");break;
		case kRecordMode :		strcpy (label, "Record Mode");		break;
		case kOneLoop :			strcpy (label, "One Loop Record");	break;
		case kLoopSetsBeats :	strcpy (label, "1st Loop sets #'o'beats");	break;
		case kReverseMode :		strcpy (label, "Reverse Mode");		break;
		case kRecordAt :		strcpy (label, "Start Recording At");		break;
//		case kRecordButtonMode :strcpy (label, "Record button mode");		break;
		case kNoteCC :			strcpy (label, "MIDI Notes or CCs");		break;
		case kTapTempoCC :		strcpy (label, "Tap Tempo Note/CC #");			break;
		case kRecordButtonCC:	strcpy (label, "Record Button Note/CC #");		break;
		case kResetButtonCC :	strcpy (label, "Reset Button Note/CC #");		break;
		case kPlayButtonCC :	strcpy (label, "Play Button Note/CC #");		break;
		case kPausedButtonCC :	strcpy (label, "Pause Button Note/CC #");		break;
		case kDirectionCC :		strcpy (label, "Direction Button Note/CC #");		break;
		case kFeedbackCC :		strcpy (label, "Feedback CC #");		break;
		case kLpfCC :			strcpy (label, "Lpf CC #");		break;
		case kMetVolCC :		strcpy (label, "Metronome Volume CC #");		break;
		case kMIDIChannel :		strcpy (label, "MIDI Channel");		break;
		case kTaps :			strcpy (label, "Minimum taps to average");		break;
		case kMetronomeNote :	strcpy (label, "Metronome Note#");	break;
		case kMetronomeVolume :	strcpy (label, "Metronome Volume");	break;
		case kMetronomeInMains :strcpy (label, "Metronome In Mains");	break;
//		case kAddProgram :		strcpy (label, "Add 1 to Program Changes");break;
		case kUnpausePlay :		strcpy (label, "Unpause starts from beginning of loop");break;
		default :				strcpy (label, "");					break;
	}
}

//-----------------------------------------------------------------------------------------
void LoopyLlama::getParameterDisplay (long index, char *text)
{
	LoopyLlamaProgram* cp = &programs[curProgram];
	//char text2[8];
	//text2 = 0;
	switch (index)
	{
		case kBeats ://   float2string (float(bpm), text2);					
			sprintf(text,"%d",beats);	
			break;
		case kBPM :
			sprintf (text, "%1.0f", bpm);
			break;
				
		case kRecord :
			if (isRecording)
				strcpy (text, "Yes");				
			else
				strcpy (text, "No");	
			if (continueRecording > 0) 	strcat(text, " (next loop too)");
			//long2string(((fRecordMode * 4 < 3) || (loopRunning == 1)),text);
			break;
		case kReset :	
			if (dontCopyThisLoop < 0.5)
				strcpy (text, "No");	
			else
				strcpy (text, "Not recording this loop");				
			break;
		case kIn :       
			dB2string (cp->fIn, text);					
			break;
		case kLoopVol :      
			dB2string (cp->fLoopVol, text);				
			break;
		case kDry :      
			dB2string (cp->fDry, text);				
			break;
		case kFeedBack :	
			//long2string (long(cp->fFeedBack * 100), text);	
			sprintf(text,"%u",int(cp->fFeedBack * 100));
			break;
		case kFadeTime :		
//			long2string (fadeTime, text);		
			sprintf(text,"%lu",fadeTime);
//			sprintf(text,"%2.3f",double(loopSize) / beats);//TEST CODE  !!!!!!!
			break;
		case kEQFreq :		
			if (cp->fEQFreq < 0.9f)	
				float2string ((cp->fEQFreq * .99f + .01f) * (float)sampleRate/3.14152f, text);
			else
				strcpy (text, "Bypassed");							
			break;
		case kTapeHiss :			
			dB2string (cp->fTapeHiss * .099f + .001f, text);									
			break;
		case kDirection : 
			if (cp->fDirection < 0.5)
				 strcpy (text, "Reverse");
			else 
				 strcpy (text, "Forward");		
			break;
		case kRecordMode : 
			switch (int(cp->fRecordMode * 4)) {
				case 0: strcpy (text, "Tap Tempo"); break ;
				case 1: strcpy (text, "Endpoints"); break ;
				case 2: strcpy (text, "Tempo Sync"); break ;
				default: strcpy (text, "Transport Lock"); break ;
			}
			break;
		case kRecordAt : 
			switch (int(cp->fRecordAt * 3)) {
				case 0: strcpy (text, "Next Sample"); break ;
				case 1: strcpy (text, "Next Beat"); break ;
				default: strcpy (text, "Next Loop"); break ;
			}
			break;
//		case kRecordButtonMode : 
//			if (cp->fRecordButtonMode >=.5f) 
//				 strcpy (text, "Toggle");
//			else
//				 strcpy (text, "Momentary");
//			break;
		case kReverseMode : 
			if (cp->fReverseMode < 0.5)
				strcpy (text, "Immediate");
			else 
				strcpy (text, "Relative");		
			break;
		case kNoteCC : 
			if (cp->fNoteCC  < .5) {
				strcpy (text, "Controller Taps");
			} else {
				strcpy (text, "Note Taps");
			}
			break;
																
		case kTapTempoCC :		 
			long2string(sliderTo7Bit(cp->fTapTempoCC), text);	
			break;
		case kRecordButtonCC :		 
			long2string(sliderTo7Bit(cp->fRecordButtonCC), text);	
			break;
		case kResetButtonCC :		 
			long2string(sliderTo7Bit(cp->fResetButtonCC), text);	
			break;
		case kPlayButtonCC :		 
			long2string(sliderTo7Bit(cp->fPlayButtonCC), text);	
			break;
		case kPausedButtonCC :		 
			long2string(sliderTo7Bit(cp->fPausedButtonCC), text);	
			break;
		case kDirectionCC :		 
			long2string(sliderTo7Bit(cp->fDirectionCC), text);	
			break;
		case kFeedbackCC :		 
			long2string(sliderTo7Bit(cp->fFeedbackCC), text);	
			break;
		case kLpfCC :		 
			long2string(sliderTo7Bit(cp->fLpfCC), text);	
			break;
		case kMetVolCC :		 
			long2string(sliderTo7Bit(cp->fMetVolCC), text);	
			break;
		case kMIDIChannel :		 
			long2string(sliderTo16(cp->fMIDIChannel), text);	
			break;
		case kTaps :		
			sprintf(text,"%d",int(cp->fTaps * maxTaps) + 2);		
			break;
		case kMetronomeNote :	
			long2string(sliderTo7Bit(cp->fMetronomeNote), text);	
			break;
		case kMetronomeVolume :       
			dB2string (cp->fMetronomeVolume, text);					
			break;
		case kMetronomeInMains : 
			if (cp->fMetronomeInMains < 0.5)
				strcpy (text, "No");
			else 
				strcpy (text, "Yes");		
			break;
//		case kAddProgram : 
//			if (cp->fAddProgram < 0.5)
//				strcpy (text, "No");
//			else 
//				strcpy (text, "Yes");		
//			break;
		case kUnpausePlay : 
			if (cp->fUnpausePlay < 0.5)
				strcpy (text, "No");
			else 
				strcpy (text, "Yes");		
			break;
		case kOneLoop : 
			if (cp->fOneLoop < 0.5)
				strcpy (text, "No");
			else 
				strcpy (text, "Yes");		
			break;		
		case kLoopSetsBeats : 
			if (cp->fLoopSetsBeats < 0.5)
				strcpy (text, "No");
			else 
				strcpy (text, "Yes");		
			break;		
		
		default : 
			strcpy (text, "");
			break;
																
	}
}
//-----------------------------------------------------------------------------------------
void LoopyLlama::getParameterLabel(long index, char *label)
{
	LoopyLlamaProgram* cp = &programs[curProgram];
	switch (index)
	{
		case kBeats :    
			strcpy (label, "Bars in Lloop");							
			break;
		case kBPM : 
			strcpy (label, "BPM");
			//if (fNoteCC >= 0.5) strcat(label," (This slider disabled)");	
			break;
		case kRecord : 
			if (isRecording)
				strcpy (label, "Yes");	
			else
				strcpy (label, "No");									
			break;
		case kReset :	 
			strcpy (label, "Tap to reset");							
			break;
		case kIn :		 
			strcpy (label, "dB");										
			break;
		case kLoopVol :      
			strcpy (label, "dB");										
			break;
		case kDry :		 
			strcpy (label, "dB");										
			break;
		case kFeedBack : 
			strcpy (label, "%");										
			break;
		case kFadeTime : 
			if (cp->fFadeTime > 0)
				strcpy (label, " spl's");										
			else
				strcpy (label, " sample");										
			break;
		case kEQFreq :		 
			strcpy (label, "Hz");									
			break;
		case kTapeHiss :      
			strcpy (label, "dB/octave");							
			break;
		case kDirection :		 
			strcpy (label, "Direction");						
			break;
		case kRecordMode :		 
			strcpy (label, "Mode")		;						
			break;
		case kReverseMode :		 
			strcpy (label, "Mode");							
			break;
		case kNoteCC : 
			strcpy (label, "Incoming MIDI");							
			break;
		case kTapTempoCC :		
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;			
		case kRecordButtonCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;											
		case kResetButtonCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;												
		case kPlayButtonCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;												
		case kPausedButtonCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;												
		case kDirectionCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;												
		case kFeedbackCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;												
		case kLpfCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;												
		case kMetVolCC :
			if (cp->fNoteCC < .5) {
				strcpy (label, "Controller #");
			} else {
				strcpy (label, "Note #");
			}
			break;												
		case kMIDIChannel :		 
			strcpy (label, "Channel")		;						
			break;
		case kTaps :		 
			strcpy (label, "Taps")		;						
			break;
		case kMetronomeNote :		 
			strcpy (label, "Note #")		;						
			break;
		case kMetronomeVolume :		 
			strcpy (label, "dB")		;						
			break;
//		case kAddProgram :		 
//			strcpy (label, "Program=bars");			
//			break;
		case kUnpausePlay :		 
			strcpy (label, "Unpause=Play");			
			break;
		default : 
			strcpy (label, "");
			break;
	}
}

//------------------------------------------------------------------------
bool LoopyLlama::getEffectName (char* name)
{
	strcpy (name, "Loopy Llama");
	return true;
}

//------------------------------------------------------------------------
bool LoopyLlama::getProductString (char* text)
{
	strcpy (text, "Loopy Llama");
	return true;
}

//------------------------------------------------------------------------
bool LoopyLlama::getVendorString (char* text)
{
	strcpy (text, "Rekliner.com");
	return true;
}

//------------------------------------------------------------------------
int LittleEndianIntRead(int num_bytes,FILE* pfile){
	int i,s=0;
	for(i=0;i<num_bytes;i++){

		s+=(1<<i*8)*getc(pfile);

	}
	return(s);
}


//------------------------------------------------------------------------
void LoopyLlama::saveFile (char* WavePath)
{
	
	unsigned int i,j;
	//char* WaveBuffer;
	FILE* pfile;
	int bits_per_sample = 16;//...i guess force it to 16 
	int block_align = 2 * (bits_per_sample / 8); ////num_channels*(bits per sample /8 = bytes per sample).
	int bytes_per_second = int(sampleRate) * block_align;//samples per second * block align
	int second_chunk_size = block_align * loopSize; //bytes per sample * loopSize
	int total_chunk_size = 44 + second_chunk_size; //header size (44) + bytes per sample * loopSize
	pfile=fopen(WavePath,"wb");
	if(pfile==NULL){
		return;
	}
	char* WaveBuffer=(char*)malloc(sizeof(char)*total_chunk_size);
	memset(WaveBuffer,0,sizeof(char)*total_chunk_size);
	

	WaveBuffer[0]=82;  //RIFF header
	WaveBuffer[1]=73;
	WaveBuffer[2]=70;
	WaveBuffer[3]=70;

	WaveBuffer[4]=(total_chunk_size>>32);   //total chunk size
	WaveBuffer[5]=(total_chunk_size>>8);
	WaveBuffer[6]=(total_chunk_size>>16);
	WaveBuffer[7]=(total_chunk_size>>24);

	WaveBuffer[8]=87;  //WAVE header
	WaveBuffer[9]=65;
	WaveBuffer[10]=86;
	WaveBuffer[11]=69;
	
	WaveBuffer[12]=102;  //fmt_ header
	WaveBuffer[13]=109;
	WaveBuffer[14]=116;
	WaveBuffer[15]=32;

	WaveBuffer[16]=16;  //Size of first chunk 16 bytes
	WaveBuffer[17]=0;
	WaveBuffer[18]=0;
	WaveBuffer[19]=0;
	
	WaveBuffer[20]=1;   //no compression
	WaveBuffer[21]=0;
	
	WaveBuffer[22]=2;   //2 channels
	WaveBuffer[23]=0;
	
	WaveBuffer[24]=(int(sampleRate)>>32);   //samples per second
	WaveBuffer[25]=(int(sampleRate)>>8);
	WaveBuffer[26]=(int(sampleRate)>>16);
	WaveBuffer[27]=(int(sampleRate)>>24);
	
	WaveBuffer[28]=(bytes_per_second>>32);	//bytes per second	
	WaveBuffer[29]=(bytes_per_second>>8);
	WaveBuffer[30]=(bytes_per_second>>16);
	WaveBuffer[31]=(bytes_per_second>>24);
	
	WaveBuffer[32]=block_align;   //Block align
	WaveBuffer[33]=0;

	WaveBuffer[34]=bits_per_sample;   //bits per sample
	WaveBuffer[35]=0;

	WaveBuffer[36]=100;	//data header
	WaveBuffer[37]=97;
	WaveBuffer[38]=116;		
	WaveBuffer[39]=97;
	
	WaveBuffer[40]=(second_chunk_size>>32);   //second_chunk_size
	WaveBuffer[41]=(second_chunk_size>>8);
	WaveBuffer[42]=(second_chunk_size>>16);
	WaveBuffer[43]=(second_chunk_size>>24);
	
	
	j=0;

	for(i=0;i<loopSize;i++){

			WaveBuffer[44+j]=((int)((buffer[i] + preBuffer[i])*32768)>>32);
			WaveBuffer[45+j]=((int)((buffer[i] + preBuffer[i])*32768)>>8);
			WaveBuffer[46+j]=((int)((buffer2[i] + preBuffer2[i])*32768)>>32);
			WaveBuffer[47+j]=((int)((buffer2[i] + preBuffer2[i])*32768)>>8);

			j+=4;
	}	
	
	
	rewind(pfile);
  	fwrite (WaveBuffer,1,(total_chunk_size),pfile);


	fclose(pfile);
	free(WaveBuffer);


}

//------------------------------------------------------------------------
void LoopyLlama::loadFile (char* WavePath)
{
	if (init == false) {

	unsigned int i;
	i = 0;
	FILE* pfile;
	signed short int temp[1];
	signed short int* ptemp;
	ptemp=&temp[0];
//	char errOut[64];
	if (WavePath == NULL) {
		i = 1;
		#if MAC
		CFBundleRef myBundle = CFBundleGetBundleWithIdentifier(CFSTR("com.ReklinerRecords.LoopyLlama"));
		if ( myBundle ) {
		    CFRetain( myBundle );
		    CFURLRef myBundleURL = CFBundleCopyBundleURL( myBundle ); //get the url of the bundle - could probably get fsref directly
		    FSRef myBundleFSRef,parentFSRef; //some temp fsref vars
		    CFURLGetFSRef(myBundleURL,&myBundleFSRef); //get a fsref from the url of the bundle
		    FSGetCatalogInfo( &myBundleFSRef, kFSCatInfoNone, NULL, NULL, NULL, &parentFSRef ); //get parent directory fsref
		    myBundleURL = CFURLCreateFromFSRef( kCFAllocatorDefault, &parentFSRef ); //get url of parent directory

		    if ( myBundleURL )  {
			char name[1024];
			name[0] = 0;
			CFStringRef tmpPath = CFURLCopyPath( myBundleURL );  //get path of url
			CFStringGetCString( tmpPath, name, 1024, kCFStringEncodingASCII); //format path to c string
			WavePath = name;  //point to the c string path
			CFRelease( tmpPath );
			CFRelease( myBundleURL );
		    }
		    CFRelease( myBundle );
		}
		strcat(WavePath,"llamabeat.wav"); //add the name of the wave file to the directory path
		pfile=fopen(WavePath,"rb"); //open it up!
	//	free(WavePath); 
		//path2fss(
		 //   fss2path(WavePath,getDirectory());
		 //   char * pch=strrchr(WavePath,'\\') + 1;
		 //   if (pch) *pch = '\0';		
		 #else
			char* metPath = (char *)malloc(1024 * sizeof(char));  //allocate some space up for the path
			GetModuleFileName(GetModuleHandle ("LoopyLlama.dll"),metPath,1024); //get the path of the dll file
			WavePath = metPath; //point towards this string
			char * pch=strrchr(WavePath,'\\') + 1; //make a pointer to just after the last slash in the path
			if (pch) *pch = '\0';  //end the string there (cut off the filename)
			strcat(WavePath,"llamabeat.wav"); //add on our wave file name
			pfile=fopen(WavePath,"rb");
			free(WavePath);  //free the space
		#endif
	} else {
//		sprintf(errOut,"open file : %s \n",WavePath);	
//		writeDebug(errOut,1);
		pfile=fopen(WavePath,"rb"); //not the metronome, so open the path as sent to us by setParameter 
	}
	
	if(pfile==NULL){
	//	sprintf(errOut,"Could not open file : %s \n",WavePath);	
	//	writeDebug(errOut,1);
		return;
	}

	rewind(pfile);


	fseek (pfile , 4 , SEEK_CUR );
	
	int total_chunk_size=LittleEndianIntRead(4,pfile);
 
	fseek (pfile , 4 , SEEK_CUR );
	fseek (pfile , 4 , SEEK_CUR );
	fseek (pfile , 4 , SEEK_CUR );
	fseek (pfile , 2 , SEEK_CUR );

	int number_of_channel=LittleEndianIntRead(2,pfile);
	
	int samples_per_second=LittleEndianIntRead(4,pfile);

	int bytes_per_second=LittleEndianIntRead(4,pfile);
	
	int block_align=LittleEndianIntRead(2,pfile);
//		sprintf(errOut,"block_align  : %s \n",block_align);	
//		writeDebug(errOut,1);
	
	int bits_per_sample=LittleEndianIntRead(2,pfile);
	int chunkSearch = 0;
	int second_chunk_size= 0;//LittleEndianIntRead(4,pfile);
//	fseek (pfile , 4 , SEEK_CUR ); //"data"
		while ((second_chunk_size != 0x64617461) && (chunkSearch < total_chunk_size)) {
//		while (second_chunk_size != 0x64617461) {
			second_chunk_size = second_chunk_size * 256 + getc(pfile);
			chunkSearch++;
//		sprintf(errOut,"%c",tempC);	
//		writeDebug(errOut,1);

		}

	second_chunk_size=LittleEndianIntRead(4,pfile);
//		sprintf(errOut,"second_chunk_size  : %u \n",second_chunk_size);	
//		writeDebug(errOut,1);

	unsigned int num_blocks=(second_chunk_size)/(block_align); //number of blocks;
	
	if (i != 0) {
		//it's reading in the metronome...
		metronomeL = new float[num_blocks];
		metronomeR = new float[num_blocks];
		if (metronomeL && metronomeR) {
			for(metCursor=0;metCursor < num_blocks;metCursor++){
			    #if MAC
			    temp[0] = LittleEndianIntRead((bits_per_sample)/8,pfile);
			    #else
			    fread (&temp,1,(bits_per_sample)/8,pfile);
			    #endif
//				fread (&temp,1,(bits_per_sample)/8,pfile);
				metronomeL[metCursor]=(float)temp[0]/32768.0f;

				if (number_of_channel==2)				//if it's a stereo file
	//				fread (&temp,1,(bits_per_sample)/8,pfile); //load the right channel into the buffer
				    #if MAC
				    temp[0] = LittleEndianIntRead((bits_per_sample)/8,pfile);
				    #else
				    fread (&temp,1,(bits_per_sample)/8,pfile);
				    #endif
				metronomeR[metCursor]=(float)temp[0]/32768.0f;
				
			}
	//		sprintf(errOut,"opening metronome : %s (%u) \n",WavePath,num_blocks);	
	//		writeDebug(errOut);
			metCursor--;
			metLength = metCursor;
		}
	} else {
		
		//it's loading a loop file
		loopRunning = usePreBuffer = useBuffer = 1;   //a loop is now running...copy it to the prebuffer
		isRecording = firstLoop = 0;
		for(loopSize=i=0;i < num_blocks && (loopSize < maxSeconds*sampleRate);i++){
			#if MAC
			temp[0] = LittleEndianIntRead((bits_per_sample)/8,pfile);
			#else
			fread (&temp,1,(bits_per_sample)/8,pfile);
			#endif
			buffer[loopSize]=(float)temp[0]/32768.0f;

			if(number_of_channel==2)
			    #if MAC
			    temp[0] = LittleEndianIntRead((bits_per_sample)/8,pfile);
			    #else
			    fread (&temp,1,(bits_per_sample)/8,pfile);
			    #endif
			buffer2[loopSize]=(float)temp[0]/32768.0f;			
			loopSize++;
		}
	//	loopSize--;

		///adjust tempo and beats
		useBuffer = 1;
		LoopyLlamaProgram* cp = &programs[curProgram];
		loopCursor=cursor=preCursor=0;
		bpm = (60 * sampleRate * beats) / loopSize;
		while (bpm > 250 || bpm < 50) {  //make sure the tempo doesn't get all whacky
			if (bpm > 250) {	//halve the number of bars if it is too high
				beats /= 2;
			}
			if (bpm < 50) {		//double the bars if it is too slow
				beats *= 2;
			}
			bpm = (60 * sampleRate * beats) / loopSize;
		}

		double beatLength = double(loopSize) / double(beats);
		//cp->fBPM = float(beatLength - (sampleRate / 10.f))/ float(sampleRate * 1.2f - (sampleRate / 10.f));
		//if (int(cp->fRecordMode * 4) != 2) resetLoop(1);  //reset the loop and the click start point
		cp->fBeats = float(beats - .5) / 127.f;
		cp->fBPM = float((bpm - 50.f) / 250.f);
		setParameterAutomated(kBeats,0.f);  //update the beats box
		setParameterAutomated(kBPM,0.f);  //update the slider
		loopCursor=cursor=preCursor=0;
		recordWaiting = resetWaiting = 0;
	}
	
	


	fclose(pfile);
	}


  
}
#if MAC
void FSSpecToPOSIXPath (const FSSpec *inSpec, char *ioPath, unsigned long inPathLength)
{
    OSStatus err = noErr;
    FSRef ref;
    FSSpec spec;
    CFStringRef nameString = NULL;
    CFStringRef pathString = NULL;
    CFURLRef pathURL = NULL;
    CFURLRef parentURL = NULL;
    int i;
    
  //  dprintf ("FSSpecToPOSIXPath called on volID %d, parID %d and name '", inSpec->vRefNum, inSpec->parID);
 //   for (i = 0; i < inSpec->name[0]; i++) { dprintf ("%c", inSpec->name[i+1]); }
  //  dprintf ("'\n");

    // First, try to create an FSRef for the FSSpec
    if (err == noErr) {
        err = FSpMakeFSRef (inSpec, &ref);
    }
    
    if (err == noErr) {
        // It's a directory or a file that exists; convert directly into a path
        err = FSRefMakePath (&ref, (UInt8 *)ioPath, inPathLength);
    } else {
        // The suck case.  It's a file that doesn't exist.
        err = noErr;
    	
        // Build an FSSpec for the parent directory, which must exist
        if (err == noErr) {
            Str31 name;
            name[0] = 0;
            
            err = FSMakeFSSpec (inSpec->vRefNum, inSpec->parID, name, &spec);
        }
    
        // Build an FSRef for the parent directory
        if (err == noErr) {
            err = FSpMakeFSRef (&spec, &ref);
        }
    
        // Now make a CFURL for the parent
        if (err == noErr) {
            parentURL = CFURLCreateFromFSRef(CFAllocatorGetDefault (), &ref);
            if (parentURL == NULL) { err = memFullErr; }
        }
    
        if (err == noErr) {
            nameString = CFStringCreateWithPascalString (CFAllocatorGetDefault (), inSpec->name, 
                                                        kCFStringEncodingMacRoman);
            if (nameString == NULL) { err = memFullErr; }
        }
    
        // Now we just add the filename back onto the path
        if (err == noErr) {
            pathURL = CFURLCreateCopyAppendingPathComponent (CFAllocatorGetDefault (), 
                                                            parentURL, nameString, 
                                                            false /* Not a directory */);
            if (pathURL == NULL) { err = memFullErr; }
        }
    
        if (err == noErr) {
            pathString = CFURLCopyFileSystemPath (pathURL, kCFURLPOSIXPathStyle);
            if (pathString == NULL) { err = memFullErr; }
        }
    
        if (err == noErr) {	
            Boolean converted = CFStringGetCString (pathString, ioPath, inPathLength, CFStringGetSystemEncoding ());
            if (!converted) { err = fnfErr; }
        }
    }    
    
    // Free allocated memory
    if (parentURL != NULL)  { CFRelease (parentURL);  }
    if (nameString != NULL) { CFRelease (nameString); }
    if (pathURL != NULL)    { CFRelease (pathURL);    }
    if (pathString != NULL) { CFRelease (pathString); }
    
//    if (err == noErr) {
  //      dprintf ("FSSpecToPOSIXPath returned path '%s'\n", ioPath);
 //   } else {
 //       dprintf ("FSSpecToPOSIXPath returned error %d (%s)\n", err, error_message (err));
 //   }
    
 //   return err;
}
#endif

//*******************responses to parameters
//-----------------------------------------------------------------------------------------
void LoopyLlama::setParameter (long index, float value)
{
	long tempCursor;
	LoopyLlamaProgram* cp = &programs[curProgram];
	switch (index)
	{
		case kBeats :		
			if (value > 0) {
				cp->fBeats = value;
				if ((int(cp->fRecordMode * 4) != 1) || loopRunning)	{
					beats = int(value * 127) + 1;
					loopSize = long((60.f * sampleRate * beats) / bpm);
				}
			}
			break;
		case kBPM :
			if ((int(cp->fRecordMode * 4) == 0))	{
			    if (value <= 1)	{
				    if (value > 0) {
					    //the knob goes from 1.1 seconds to 1/10 of a second
					    bpm = value * 250 + 50;
					    loopSize = long((60.f * sampleRate * beats) / bpm);
						cp->fBPM = value;
				    }
			    } else {
				    bpm = value;
				    loopSize = long((60.f * sampleRate * beats) / bpm);
					cp->fBPM = (value - 50.f) / 250.f;
			    }
	//		    resetLoop(1);  //if it's not in endpoints reset the loop and the click start point
			}
			break;
		case kRecord :			
			setRecording(value);						
			break;				
		case kReset :			
			cp->fReset = value;
			if ((value > 0) && countSamples - lastResetMove > sampleRate / 8)	{				
				if (!isRecording || dontCopyThisLoop || resetWaiting) {		//if it wasn't recording or reset was already hit once then reset
					if (int(cp->fRecordMode * 4) != 1 && (cp->fPaused < 0.5f) && !resetWaiting)	{ //if not endpoints mode, not paused, and not already reset
						resetLoop(2);// at the next beat or
					}	else	{		
						resetLoop(1);// immediately
					}
				} else { //it is recording and reset hasn't been hit once already
					if (!loopRunning || firstLoop) //if the first loop wasn't finished
						resetLoop(1); //immediate reset of loop contents and cursors
					else
						if (!resetWaiting) 
							dontCopyThisLoop = 1;  //otherwise skip what was just played
						else
							resetLoop(1);// reset
				}
				lastResetMove = countSamples;
			//	if (cp->fRecordButtonMode < .5f) //if it's not toggle mode
				//	setParameter(kReset,0.f);  //kick the reset button back up
			}
			break;
		case kPlay :
			cp->fPlay = value;
			if (value > 0) { 
					//if value is >1 or we're in toggle mode
				//cp->fPlay = 0;

				if (isRecording && firstLoop) {
					recordWaiting = 3; //turn recording off at next beat if it's still recording
					setRecording(1.f); //signal the end of the loop
				//	setRecording(1.f); //turn off recording - being ignored because of lastrecordmove
				} else {
					if (int(cp->fRecordMode * 4) < 3) {

						//restart the playback from the beginning of the loop immediately
						tempCursor = cursor + loopSize - loopCursor * bufferDirection;
						while (tempCursor > long(loopSize)) tempCursor -= loopSize;
						cursor = tempCursor;
						tempCursor = preCursor + loopSize - loopCursor * bufferDirection;
						while (tempCursor > long(loopSize)) tempCursor -= loopSize;
						preCursor = tempCursor;
						loopCursor =0;
			
						if (cp->fPaused >= .5f) setParameterAutomated(kPaused,0.f);  //unpause it if it was paused
						//if (cp->fRecordButtonMode < .5f) //if it's not toggle mode
					} else {

						VstTimeInfo * timeInfo = NULL;//(VstTimeInfo *) malloc(sizeof(VstTimeInfo));
						timeInfo = getTimeInfo(0xffff); //ALL FLAGS
						if (timeInfo) {
							beatOffset = int(fmod(timeInfo->ppqPos,beats)) + 1;
						}
					}
					//setParameter(kPlay,0.f);  //kick the play button back up
				}
			}
			break;
		case kPaused:
//			if ((cp->fRecordButtonMode < .5f) && (value > 0.f)) //if it's in momentary mode and the value is greater than 0
//				cp->fPaused = 1.f - cp->fPaused; //invert the current value
//			else
				cp->fPaused = value; //otherwise follow the button value
			if (value == 0.f && cp->fUnpausePlay == 1.f) {  //if unpause = play
				setParameterAutomated(kPlay,1.f); //kick the play button down
//				if (cp->fRecordButtonMode < .5f) //and if it's in momentary mode
//					setParameterAutomated(kPlay,0.f);	//play button back up
			}
			break;
		case kIn :				
			cp->fIn = value;						
			break;
		case kLoopVol :			
			cp->fLoopVol = value;					
			break;
		case kDry :				
			cp->fDry = value;					
			break;
		case kFeedBack :		
			cp->fFeedBack = value;			
			break;
		case kFadeTime :		
			cp->fFadeTime = value; 
			fadeTime = (unsigned long)(cp->fFadeTime * sampleRate / 8) + 1;
			beatBufferFade = fadeTime;
			break;
		case kEQFreq :			
			cp->fEQFreq = value;
			LPL.setC0(value);
			LPR.setC0(value);
			break;
		case kTapeHiss :			
			cp->fTapeHiss = value;
			break;
		case kDirection :		
			cp->fDirection = value;		
			break;
		case kRecordMode :		
			cp->fRecordMode = value; 
			isRecording = 0;
			if (int(cp->fRecordMode * 4) == 1) { //if its endpoint mode reset the loop
			    resetLoop(1);
			    lastResetMove = countSamples;
			}	
			break;
		case kOneLoop :		
			cp->fOneLoop = value;			
			break;
		case kLoopSetsBeats :		
			cp->fLoopSetsBeats = value;			
			break;
		case kReverseMode :		
			cp->fReverseMode = value;			
			break;
		case kRecordAt :		
			cp->fRecordAt = value;			
			break;
//		case kRecordButtonMode :		
//			cp->fRecordButtonMode = value;			
//			break;
		case kNoteCC :		
			cp->fNoteCC = value;			
			break;
		case kTapTempoCC :		
			cp->fTapTempoCC = value;		
			break;
		case kRecordButtonCC :	
			cp->fRecordButtonCC = value;		
			break;
		case kResetButtonCC :	
			cp->fResetButtonCC = value;		
			break;
		case kPlayButtonCC :	
			cp->fPlayButtonCC = value;		
			break;
		case kPausedButtonCC :	
			cp->fPausedButtonCC = value;		
			break;
		case kDirectionCC :	
			cp->fDirectionCC = value;		
			break;
		case kFeedbackCC :	
			cp->fFeedbackCC = value;		
			break;
		case kLpfCC :	
			cp->fLpfCC = value;		
			break;
		case kMetVolCC :	
			cp->fMetVolCC = value;		
			break;
		case kMIDIChannel :		
			cp->fMIDIChannel = value;		
			break;
		case kTaps :			
			cp->fTaps = value;		
			break;
		case kMetronomeNote :	
			cp->fMetronomeNote = value;			
			break;
		case kMetronomeVolume :	
			cp->fMetronomeVolume = value;			
			break;
		case kMetronomeInMains :	
			cp->fMetronomeInMains = value;			
			break;
//		case kAddProgram :		
//			cp->fAddProgram = value;		
//			break;
		case kUnpausePlay :		
			cp->fUnpausePlay = value;		
			break;
		case kTapHappened :		
			tapHappened(value);	
			break;


		case kLoad :
		{
			if (value > .5)	{
				VstFileType waveType ("Wave File", ".WAV", "wav", "wav",  "audio/wav", "audio/x-wav");
				VstFileSelect vstFileSelect;
				memset (&vstFileSelect, 0, sizeof (VstFileType));
				vstFileSelect.command     = kVstFileLoad;
				vstFileSelect.type        = kVstFileType;
				strcpy (vstFileSelect.title, "Load a loop from a .wav file");
				vstFileSelect.nbFileTypes = 1;
				vstFileSelect.fileTypes   = &waveType;
				vstFileSelect.returnPath  = new char[1024];
				vstFileSelect.initialPath = 0;
				CFileSelector * myFileSelect = new CFileSelector(this);
				myFileSelect->run(&vstFileSelect);
				if (myFileSelect) {
				    #if MAC
				    FSRef tmpRef;
				    FSpMakeFSRef((FSSpec*)vstFileSelect.returnPath,&tmpRef);  //get a FSRef from the FSSpec
				    //CFStringRef tmpPath;
				    FSRefMakePath (&tmpRef, (UInt8 *)vstFileSelect.returnPath, 1024); //make a path from the ref
				    
				    // CFURLRef tempURL = CFURLCreateFromFSRef(0, &tmpRef); //get a url from the FSRef
				   // if(tempURL){
					//CFURLGetFileSystemRepresentation(tempURL, true, (UInt8*)vstFileSelect.returnPath, 1023);
				//	CFStringRef tmpPath = CFURLCopyPath( tempURL );  //get path of url
					//CFStringGetCString( tmpPath, vstFileSelect.returnPath, 1024, kCFStringEncodingASCII); //format path to c string					
					//vstFileSelect.returnPath = (char *)tempURL;
				//	CFRelease( tmpPath );
				//	CFRelease(tempURL);
				 //   }
				    #endif
				    //char errOut[128];
				    //sprintf(errOut,"open file : %s \n",vstFileSelect.returnPath);	
				    //writeDebug(errOut,1);

				    loadFile(vstFileSelect.returnPath);
				}
				delete []vstFileSelect.returnPath;
				if (vstFileSelect.initialPath)
				    delete []vstFileSelect.initialPath;
			}
		} break;

		case kSave :
		{
			if  (value > .5){

				VstFileType waveType ("Wave File", ".WAV", "wav", "wav",  "audio/wav", "audio/x-wav");
				VstFileSelect vstFileSelect;
				memset (&vstFileSelect, 0, sizeof (VstFileType));
				vstFileSelect.command     = kVstFileSave;
				vstFileSelect.type        = kVstFileType;
				strcpy (vstFileSelect.title, "Save this loop to a .wav file");
				vstFileSelect.nbFileTypes = 1;
				vstFileSelect.fileTypes   = &waveType;
				vstFileSelect.returnPath  = new char[1024];
				vstFileSelect.initialPath = 0;
				//if (openFileSelector (&vstFileSelect))
				CFileSelector * myFileSelect = new CFileSelector(this);
				myFileSelect->run(&vstFileSelect);
				if (myFileSelect) {
				    #if MAC
				    //FSRef tmpRef;
				    //FSpMakeFSRef((FSSpec*)vstFileSelect.returnPath,&tmpRef);  //get a FSRef from the FSSpec
				   // FSRefMakePath (&tmpRef, (UInt8 *)vstFileSelect.returnPath, 1024); //make a path from the ref
				    FSSpecToPOSIXPath((FSSpec*)vstFileSelect.returnPath,vstFileSelect.returnPath,1024);
				    #endif
				    saveFile(vstFileSelect.returnPath);
				}
				delete []vstFileSelect.returnPath;
				if (vstFileSelect.initialPath)
					delete []vstFileSelect.initialPath;
			}
		} break;

	}
	if (editor) {
		((AEffGUIEditor*)editor)->setParameter (index, value);  //is this mac compatible?
	}
}




//------------------------------------------------------------------------
void LoopyLlama::resetLoop (int resetType)
{
	LoopyLlamaProgram* cp = &programs[curProgram];
	//1 is an immediate full reset of the loop
	//2 starts ignoring the buffers but doesn't reset the loop counters yet
	//memset is too expensive to use in real time...thus the cascading buffers
	if (resetType == 2) {//start clearing the buffers and we'll reset the loop again at a downbeat later
		useBuffer=usePreBuffer=dontCopyThisLoop=0;
		resetWaiting=1;

	} else { //full immediate reset
		resetWaiting=recordWaiting=useBuffer=usePreBuffer=  //preCursor=cursor=loopCursor=
			firstLoop = dontCopyThisLoop=fadeLoop=fadeIn=fadeOut=beatOffset=0;
		if (cp->fRecordMode * 4 < 3) {	// if it's not in transport lock mode
			preCursor=cursor=loopCursor=resetAt=beatBufferCursor=0;
		} else {
			resetAt=loopCursor;
		}
		bufferDirection = preBufferDirection = 1; //this will keep the counters from going haywire while waiting for the next tempo
		//if (!isRecording) loopRunning=0;
		isRecording=loopRunning=0;
		tempoLightOn = 0;
		cp->fPaused = 0.f;  //unpause

	}
}

//------------------------------------------------------------------------
void LoopyLlama::setRecording (float value)
{
	LoopyLlamaProgram* cp = &programs[curProgram];
	double beatLength = ((60.f * sampleRate) / bpm);
	if (int(cp->fRecordMode * 4) == 1 && !loopRunning) {//it's starting out and in endpoints mode
		if ( (value >= .5f) && countSamples - lastRecordMove > sampleRate / 8) { //ignore record exceptions
			//this ignores 0 values so the slider can be recursively set and adds a 1/8 second minimum tap time so a slider can be wiggled
			 //this is an endpoint tap 


				if (!firstLoop) {  //*********this is the first tap
					resetLoop(1); //immediate reset of loop contents and cursors
					//cursor = preCursor = loopCursor = recordWaiting = 0;
					isRecording = 1;
					fadeIn = fadeTime; //start the fade in
				//	cp->fRecord = 1.0f;
				//	tempoLightOn = loopRunning = useBuffer = 0;
					loopSize = (unsigned long)((maxSeconds - 2) * sampleRate);
					firstLoop = 1;


				} else {  //***************this is the second tap
					//recordWaiting = isRecording = 0;  //stop recording ... possible preference here
					loopSize = cursor;
					bpm = (60 * sampleRate * beats) / loopSize;
					while (bpm > 250 || bpm < 50) {  //make sure the tempo doesn't get all whacky
						if (bpm > 250) {	//halve the number of bars if it is too high
							beats /= 2;
						}
						if (bpm < 50) {		//double the bars if it is too slow
							beats *= 2;
						}
						bpm = (60 * sampleRate * beats) / loopSize;
					}
				//	beatLength = double(loopSize) / double(beats);
					//cp->fBPM = float(beatLength - (sampleRate / 10.f))/ float(sampleRate * 1.2f - (sampleRate / 10.f));
					//if (int(cp->fRecordMode * 4) != 2) resetLoop(1);  //reset the loop and the click start point
					cp->fBeats = float(beats - .5f) / 127.f;
					setParameterAutomated(kBeats,cp->fBeats);  //update the beats box
					setParameterAutomated(kBPM,float((bpm - 50.f) / 250.f));  //update the slider
					loopRunning = usePreBuffer = useBuffer = 1;   //a loop is now running...copy it to the prebuffer
					if (recordWaiting == 3) isRecording = 0; //if the play button was the 2nd hit
					recordWaiting = resetWaiting = firstLoop = 0;
				}
			lastRecordMove = countSamples;
		} //end record exception ignoring



	} else { //******************it's not in endpoints mode or a loop was running
		if ( (value > .5f ) && (countSamples - lastRecordMove > sampleRate / 8)) { // && ((cp->fRecButton >= .5f) || (value > 0)) //
			//this ignores lower values in momentary so the slider can be recursively set and adds a 1/8 second minimum tap time so a slider can be wiggled

			if (!isRecording) {//*****record on or 1st point******it's not already recording
				

				if (!loopRunning) {//*****if it's first loop
					firstLoop = 1;
					if (cp->fLoopSetsBeats > 0.5f) {  //if first loop size sets beats
						//set loopsize and beats to be the maximum loop time to get it out of the way
						beats = int(maxSeconds * sampleRate / beatLength);
						loopSize = (unsigned long)(beats * beatLength);
					}

					VstTimeInfo * timeInfo = NULL;//(VstTimeInfo *) malloc(sizeof(VstTimeInfo));
					if (int(cp->fRecordMode * 4) >= 3)  //it it's not transport locked
						timeInfo = getTimeInfo(0xffff); //ALL FLAGS
						//if it's not transport locked jump it to the last beat of the loop - so the next beat will be the 1
						loopCursor = (unsigned long)(fmod(loopCursor,beatLength));  //set the loop cursor to be relative to zero
						if (loopCursor > (beatLength / 2)) { //it's an early hit so add beats
							loopCursor += (unsigned long)((beatLength * (beats - 1)));  //start counting the loop from the next beat
							//************* this needs to change - this is ganky and you lose the stuff before the beat
							//****************do the current beat buffer thingy here!
							//***feed the already recorded crap into the prebuffer as a one shot with an offset
					//		recordWaiting = 1;  //since it will get truncated to the next beat we might as well fade it in...ganky
							recordWaiting = 2;  //makes sure firstLoop doesnt get unset
							if (timeInfo) {
								//float ppq = float(loopCursor) / loopSize * beats;
								beatOffset = int(fmod(timeInfo->ppqPos,beats)) + 1;
							}
						} else { // it was a late hit for the last beat so leave it just after zero
							isRecording = loopRunning = usePreBuffer = 1;
							recordWaiting = resetWaiting = 0;
							dontCopyThisLoop = 0;
							//leave it to start recording during bar 1
							//fadeIn = fadeTime; //start the fade in to prevent a click
							//****add in the audio you missed with the current beat buffer
							beatBufferFade = fadeTime;
							beatBufferCopy = loopCursor; //it doesn't copy sample 0 so this will work out
							beatBufferOffset = 0;
							beatBufferCursor = 0;
							beatBufferDirection = 1;
							if (beatBufferCopy < fadeTime) fadeIn = fadeTime - beatBufferCopy; 
							//if the fade is longer than the beatbuffer will be filling in make sure the fade resumes at the right spot with non buffered audio

							if (timeInfo) {
								//float ppq = float(loopCursor) / loopSize * beats;
								beatOffset = int(fmod(timeInfo->ppqPos,beats));
							}

						}
						resetAt = cursor = preCursor = loopCursor; //will this work in reverse?
						

	//				} else {
						//****set the beat offset
	//					isRecording = loopRunning = 1; //DONT FORGET DELAYED RECORD
	//					recordWaiting = 2;
	//				}
					} else { //***not the first loop
						firstLoop = 0;

						switch (int(cp->fRecordAt * 3))	{ //when is it set to turn on...
							case 0:		//record immediately
								isRecording = loopRunning = 1; 
								//recordWaiting = 2;  //makes sure firstLoop doesnt get set
								fadeIn = fadeTime; //start the fade in
							break;
							case 1:		//record at next beat
								recordWaiting = 1;//record at next beat
								break;
							default:	//record at next loop
								recordWaiting =  2;//record at next loop
								break;
							}

						if (cp->fOneLoop > .5f)  //if it's one loop only mode
							continueRecording = 1; //record the loop after this one since this will be a partial loop
							//end one loop only

					//	if (cp->fRecordButtonMode >= .5f)  //if it's in sticky record button mode
					//		cp->fRecord = 1.0f; //depress the record button

					//	if	((fDirection < .5 && fReverseMode < .5))  //if we are reversing and its immediate
					//	if ((!loopRunning || firstLoop) && (int(cp->fRecordMode * 4) < 3)) { //if this is the first time to record & not starting at next loop
							//resetWaiting = 2;
					//	}
					}//***end first loop stuff

				} else { 	//****record off or 2nd point**** it was recording or 2nd tap


					if ((cp->fLoopSetsBeats > 0.5f) && firstLoop) {  //if the first loop sets the number of beats and it's first loop
						float floatBeats = (float(loopCursor) / loopSize) * beats; //get the current beat with remainder
							//(samples so far divided by the maximum number of samples possible (quantified to beats) times maximum beats 
						beats = (unsigned int)(floatBeats);

						//****set the beat offset
						VstTimeInfo * timeInfo = NULL;//(VstTimeInfo *) malloc(sizeof(VstTimeInfo));
						timeInfo = getTimeInfo(0xffff); //ALL FLAGS

						if ((floatBeats - beats) > .5f) {//if it was more than halfway through this beat
							beats += 1;	//assume you were going for the next one
							if (recordWaiting != 3)
								recordWaiting = 3;  //ummm legacy?
							else //it must've been the play button
								recordWaiting = 4;  //turn recording off at next beat
							if (timeInfo)
								beatOffset = int(fmod(timeInfo->ppqPos,beats)) + 1;
						} else {
							//jump us back to beat one since we're "over" a little
							if (timeInfo)
								beatOffset = int(fmod(timeInfo->ppqPos,beats));
							loopCursor -= (unsigned long)(beats * beatLength);
							cursor -= (unsigned long)(beats * beatLength);
							preCursor -= (unsigned long)(beats * beatLength);
							// Do the beatbuffer here ... fade in the regular buffer?
							//beatBufferFade = 0;
							beatBufferCopy = cursor; //it doesn't copy sample 0 so this will work out
							beatBufferOffset = 0;
							beatBufferDirection = 1;
							beatBufferCursor = 0;
							beatBufferFade = 0;


							dontCopyThisLoop = 0;
							loopRunning = 1;
							useBuffer = usePreBuffer = 1;
							firstLoop = 0;
							recordWaiting = resetWaiting = 0;
							continueRecording = 0;
							

							if (recordWaiting == 3) {//if it was the play button hit
								isRecording = recordWaiting = 0;  //turn off recording
								fadeOut = fadeTime; //and fade out
							}
						}
						loopSize = (unsigned long)(beats * beatLength); //set the loop size to the end of this beat in samples
						cp->fBeats = float(beats - .5f) / 127.f;
						setParameterAutomated(kBeats,cp->fBeats);  //update the beats box



					} else { //it's not the first loop or beats are pre-determined
						if (cp->fOneLoop > .5f) { //if it's one loop only mode
							continueRecording = 1; //record the loop after this one also
					//		if (cp->fRecordButtonMode >= .5f)  //if it's in toggle record button mode
					//			cp->fRecord = 1.0f;	//keep the record button lit

						} else {  //it's in continuous record mode
							if (isRecording) //don't fade out if it was already off (midi problem only)
								fadeOut = fadeTime;//start the fade out
							isRecording = recordWaiting = 0;//stop recording
							cp->fRecord = 0.0f; //turn off the record button
					//		if (cp->fRecordButtonMode < .5f) //if it's in momentary record button mode
								setParameterAutomated(kRecord,0.f);  //update the button to push back up
						} // end continuous/one loop mode check
					} //end if loop sets beats







				} // end if recording
//			}//end if value == 0
			lastRecordMove = countSamples;
		} //end record exception ignoring
	} // end not endpoints mode
}


//-----------------------------------------------------------------------------------------
void LoopyLlama::process (float **inputs, float **outputs, long sampleFrames)
{
	processReplacing (inputs, outputs,sampleFrames);
}

//-----------------------------------------------------------------------------------------
void LoopyLlama::processReplacing (float **inputs, float **outputs, long sampleFrames)
{
	
	LoopyLlamaProgram* cp = &programs[curProgram];

	if (int(cp->fRecordMode * 4) < 2) { //***********send time if endpoints or tap mode
		_masterTimeInfo.samplePos = loopCursor;
		_masterTimeInfo.ppqPos = double(getParameter(kCurrentPos) * beats);	
		_masterTimeInfo.tempo = bpm;
	    _masterTimeInfo.flags = 0
        | (kVstTransportPlaying * !cp->fPaused)
        | kVstTempoValid
        | kVstBarsValid
        ;
	    //sending VstTimeInfo to host
		setTimeInfo(&_masterTimeInfo);

	} else {  //**********************receive time
		VstTimeInfo * timeInfo = NULL;//(VstTimeInfo *) malloc(sizeof(VstTimeInfo));
	    timeInfo = getTimeInfo(0xffff); //ALL FLAGS
        if (timeInfo) {
			if ((kVstTempoValid & timeInfo->flags)) {
				if (fabs(bpm - timeInfo->tempo) > .01) {  //This makes sure the tempo still matches the host, accounts for float precision error				
					bpm = timeInfo->tempo;
					if (bpm <1 || bpm > 300) bpm = 120; //for buggy hosts
					loopSize = long((60 * sampleRate * beats) / bpm);
					cp->fBPM = float(((60 * sampleRate) / bpm) - (sampleRate / 10))/ float(sampleRate * 1.2 - (sampleRate / 10));
				}
				if (cp->fRecordMode * 4 >= 3 && (kVstPpqPosValid & timeInfo->flags) && (timeInfo->ppqPos > 0)) {
					// if it's in exact sync mode and incoming sync is valid
					float ppq = float(loopCursor) / loopSize * beats;
					double ppqDiff = fmod(timeInfo->ppqPos,beats) - ppq - beatOffset;
					//if it's recording the first loop ignore this?
					//*****factor in beat offset here...im tired right now
					double fppqDiff = fabs(ppqDiff);
					if (fppqDiff > .02 && fabs(beats - fppqDiff) > .02) { // if it doesn't match the external ppq and the loop hasn't JUST restarted && ppq > .02
						//jump ahead to the proper ppq position in the loop
						if (ppqDiff < 0) ppqDiff+= beats;
						long jumpSamples = long(ppqDiff * (loopSize / beats));

//						char tempText[150];
//						sprintf(tempText,"epq=%f \n ipq=%f \n ppqDiff=%f \n jumpSamples=%i \n", timeInfo->ppqPos, ppq,ppqDiff,jumpSamples);
//						writeDebug(tempText);
						

						//jump all the buffers forward
						cursor += jumpSamples;
						preCursor += jumpSamples;
						loopCursor += jumpSamples;

						//if they went past the loop boundary wrap them back around
						if (cursor >= (loopSize - 1)) cursor -= (loopSize - 1);
						if (preCursor >= (loopSize - 1)) preCursor -= (loopSize - 1);
						if (loopCursor >= (loopSize - 1)) loopCursor -= (loopSize - 1);

						//turn off recording if it was in the midst of the first loop
						//if (isRecording && firstLoop)
						//	setRecording(1.f);
						
					}
					if (((kVstTransportPlaying & timeInfo->flags) > 0) != (cp->fPaused < .5f)) {
						setParameter(kPaused,float(!(kVstTransportPlaying & timeInfo->flags)));
					}
				}
			}
        }
	}
			
	float* in = inputs[0];
	float* out = outputs[0];
	float* in2 = inputs[1];
	float* out2 = outputs[1];
	float* out3 = outputs[2];

	unsigned long beatLength = long((60 * sampleRate) / bpm);  //calculate this stuff now so it's not doing it every sample
	int isEndpoints = bool(int(cp->fRecordMode * 4) == 1);
	if (isEndpoints && !loopRunning) beatLength = loopSize; //set this really high so there are no metronomes during endpoints
	//******************************* metronome & lights *********************************** 
	if (lastMetronome && (  (countSamples - lastMetronome) > (60.f * sampleRate / bpm / 2))) {// (countSamples - lastMetronome > (sampleRate /8)) ||
		//if there was recently a metronome - 1/2 beat length
		sendMIDIData(char(cp->fMIDIChannel * 15) | 0x90,sliderTo7Bit(cp->fMetronomeNote),0x00,0); //send a metronome pulse note off so as to allow time for a sample to play
		lastMetronome = 0;
		tempoLightOn = 0;
	}

	if ((cp->fPaused > 0.5f) || (isEndpoints && !isRecording && !loopRunning)) { //if it's paused or endpoints standby
		while (sampleFrames-- > 0) {  //route these samples directly out
			*out++ = *in++ * cp->fDry; 
			*out2++ = *in2++ * cp->fDry;
			countSamples++;
		}
		
	}

//	if (!useBuffer && (((loopCursor < resetAt) && (loopCursor + sampleFrames >= resetAt)) ||  (loopCursor + sampleFrames > loopSize + resetAt)))
		//if it has been reset but it's reaching a full loop since it was reset
//		resetWaiting += 2;

	
	while (sampleFrames-- > 0) { //otherwise continue as planned
		countSamples++;


		//********************degrade the buffer and add the prebuffer to the buffer if it's in use
		buffer[cursor] = (LPL.process(buffer[cursor]) * cp->fFeedBack //diminish the buffer with EQ and fade if they are in use
			+ preBuffer[preCursor] * usePreBuffer)	//add the prebuffer contents to the buffer is it's not being ignored
			 * (loopRunning * !firstLoop); //clear the buffer if it's the first loop or a loop is not running
		buffer2[cursor] = (LPR.process(buffer2[cursor]) * cp->fFeedBack 
			+ preBuffer2[preCursor] * usePreBuffer)
			 * (loopRunning * !firstLoop);

//		*in *= cp->fIn + denormalNoiseLoop[noiseLoopCursor] //economical denormal
//			+ tapeNoiseLoop[noiseLoopCursor] * cp->fTapeHiss * .0125f * fadePoint; //tape hiss;
//		preBuffer[preCursor] = *in * fadePoint;
		//right channel audio to prebuffers....
//		*in2 *= cp->fIn + denormalNoiseLoop[noiseLoopCursor] //economical denormal
//			+ tapeNoiseLoop[noiseLoopSize - 1 - noiseLoopCursor] * cp->fTapeHiss * .0125f * fadePoint; 
//		preBuffer2[preCursor] = *in2 * fadePoint;
				
		
		//loop fading
		float fadePoint = 1 * crossFade(0.f,1.0f,float(fadeTime),float(fadeLoop + fadeTime * (!fadeLoop)))  
			* crossFade(1.0f,0.0f,float(fadeTime),float(fadeIn))		//this fades in after recording has started
			* crossFade(float(isRecording),1.0f,float(fadeTime),float(fadeOut));   //this fades out after recording has ended
		
		//********************beat buffer
		if (!isEndpoints) {
			if (beatBufferCopy > 0)	{  //if it's transferring the buffer
				preBuffer[beatBufferOffset] += beatBuffer[beatBufferCursor] * crossFade(1.0f,0.0f,float(fadeTime),float(beatBufferFade));
				preBuffer2[beatBufferOffset] += beatBuffer2[beatBufferCursor++] * crossFade(1.0f,0.0f,float(fadeTime),float(beatBufferFade));
				//add in missing audio from the beatbuffer to the prebuffer with possible fade
				beatBufferOffset += beatBufferDirection;
				if (beatBufferFade > 0) beatBufferFade--;
				beatBufferCopy--;
			} else { //it's capturing the current bar to the buffer
				beatBuffer[beatBufferCursor] = *in * cp->fIn + tapeNoiseLoop[noiseLoopCursor] * cp->fTapeHiss * .0125f;
				beatBuffer2[beatBufferCursor] = *in2 * cp->fIn + tapeNoiseLoop[noiseLoopSize - 1 - noiseLoopCursor] * cp->fTapeHiss * .0125f;
				beatBufferCursor++;
			}
		}

		//****************************overwrite prebuffers with new audio
		preBuffer[preCursor] = *in * cp->fIn * fadePoint
			+ denormalNoiseLoop[noiseLoopCursor] //economical denormal
			+ tapeNoiseLoop[noiseLoopCursor] * cp->fTapeHiss * .0125f * fadePoint; //tape hiss;
		preBuffer2[preCursor] = *in2 * cp->fIn * fadePoint
			+ denormalNoiseLoop[noiseLoopCursor] //economical denormal
			+ tapeNoiseLoop[noiseLoopSize - 1 - noiseLoopCursor] * cp->fTapeHiss * .0125f * fadePoint; //tape hiss;


		//******************************output the mix of dry and loop - for some reason this must come after the input stuff
		//left channel
		*out = *in * cp->fDry + buffer[cursor] * cp->fLoopVol;

		//right channel .. output and setup for prebuffer
		*out2 = *in2 * cp->fDry + buffer2[cursor]  * cp->fLoopVol;


//		if (metCursor < metLength)	{
//			if (metronomeL && metronomeR) {
//				if (cp->fMetronomeInMains > 0.5f) {
//					*out += metronomeL[metCursor] * cp->fMetronomeVolume;
//					*out2 += metronomeR[metCursor] * cp->fMetronomeVolume;
//				}
//				*out3 = metronomeL[metCursor++] * cp->fMetronomeVolume;
//			} else {
//				*out3 = 0;
//			}
//		} else {
//			*out3 = 0;
//		}

	
	  
		
		out++;
		out2++;
		out3++;
		in++;
		in2++;


		//**********************  end of beat  ******************************* 
		if (( (loopCursor / beatLength) != 
			 ((loopCursor - 1) / beatLength)
			 || (loopCursor == 0))
			 && ((!isEndpoints) || loopRunning)) { //if it's been a whole bar (and not starting endpoints)
			if (resetWaiting == 1 || resetWaiting == 3) resetLoop(1); //reset completely since its on a downbeat
			switch (recordWaiting) {
			case 1:
				if (!isRecording)
					fadeIn = fadeTime; //start the fade in
				isRecording = loopRunning = 1;  //start recording if not already recording
				recordWaiting = 0;
				break;
			case 3:
				//if it's the first loop we wanted to record out until the end of the bar
			//	setRecording(1.0f);// - cp->fRecordButtonMode);	//so we can turn off recording now
				recordWaiting = resetWaiting = 0;
				loopRunning = 1;
				firstLoop = 0;
				break;
			case 4:
				//it's the first loop and we wanted to stop recording at the next bar
				isRecording = firstLoop = 0;
				loopRunning = 1;
				recordWaiting = 0;
				fadeOut = fadeTime; //and fade out
				break;
			}
//			char test[64]; sprintf(test,"Beatlength: %u \n%s\n",beatLength,(char *)getDirectory());
//			writeDebug(test);
			if (((tapCount >= int(cp->fTaps * (maxTaps) + 2)) || !tapCount) && ((countSamples - lastMetronome) > (sampleRate / 6))) {//if there wasn't just a controller tap
				sendMIDIData(char(cp->fMIDIChannel * 15) | 0x90,sliderTo7Bit(cp->fMetronomeNote),char(cp->fMetronomeVolume * 127),0); //send a metronome pulse
				lastMetronome = countSamples;
				tempoLightOn = 1;
				metCursor = 0;
			}
			if (tapCount && ((countSamples  - tapTimes[tapCount]) > (sampleRate * 2.5))) //if it has been too long since the last tap
				tapCount=0;
			peakVU = 0; //use this as a small history of peaks for VU	
			beatBufferCursor = 0;
		}


		//increment the cursors
//		*in++;
		loopCursor++;





		//***********************  end of loop  ********************************
		if (loopCursor > (loopSize - 1)) //if it's been a whole loop----- so not starting endpoints mode
		{
			loopCursor = 0;  //start counting the loop again
			if (dontCopyThisLoop) { //+++++++++++this could be done with resetwaiting
				usePreBuffer = 0;  //if the reset button was hit then this pass will not be copied
			} else {
				usePreBuffer = 1;  //make sure prebuffer is on - if clear was hit last pass then start using it again			
				if (loopRunning && !recordWaiting) {
					firstLoop = 0;
					useBuffer = 1;
					//mark this as the second or greater pass through the loop since recording started (the first pass should've output nothing).
				}
			}

			if (recordWaiting == 2) {  //if it was set to record at the next loop
				if (!isRecording)
					fadeIn = fadeTime; //start the fade in
				isRecording = loopRunning = 1;  //start recording if not already recording
				recordWaiting = 0;
			}

			if (cp->fDirection < .5 * useBuffer) {  //if reverse is on
				bufferDirection = -1;  //reverse the buffer direction if it hasn't happened yet
				if (cp->fReverseMode < .5) {
					preBufferDirection *= -1; //if we are in immediate reverse mode then flip the prebuffer direction...not if you just hit record though
				}
			} else {
				bufferDirection = preBufferDirection = 1;  //make sure the buffer is going forward
			}

			//this will stop recording or record another loop if a button was hit while recording
			if ((cp->fOneLoop > .5f) && !dontCopyThisLoop && !continueRecording && isRecording) { //+++++continuerecording could be replaced with recordwaiting
				//if it was set to record only one loop...and the loop has already run...logically it must have if we are here
				isRecording = 0;//stop recording...this would be the easy way to do it.
				fadeOut = fadeTime;//start the fade out
//				if (cp->fRecordButtonMode < .5f) 
//					setParameterAutomated(kRecord,0); //update the slider, but this doesn't re-trigger recording
			}
			dontCopyThisLoop = continueRecording = 0; //reset the loop level monitor vars
		}
		if (buffer[cursor] + preBuffer[preCursor] > peakVU) 
			peakVU = (buffer[cursor] * useBuffer + preBuffer[preCursor] * usePreBuffer); //cling to the highest peak for VU




		//*********************************** cursors ********************************** 
//		if (!isEndpoints || loopRunning) { //if it's not endpoints mode or a loop is runnning
			//buffer maintenance - wrap it back around
			if (bufferDirection > 0 && cursor >= (loopSize)) //if it's the end of the buffer, restart it
			    cursor = 0;
			else if (bufferDirection < 0 && cursor == 0) //if it's the beginning of the buffer and we are in reverse mode go to the end
			    cursor = loopSize;
			else
			    cursor += bufferDirection;
			
			if (preBufferDirection > 0 && preCursor >= (loopSize)) //if it's the end of the prebuffer, reverse/restart it
			    preCursor = 0;
			else if (preBufferDirection < 0 && preCursor == 0) //if it's the beginning of the buffer and we are in reverse mode go to the end
			    preCursor = loopSize;
			else
			    preCursor += preBufferDirection;
//		} else {
//			cursor += bufferDirection;
//		    preCursor += preBufferDirection;
//		}
		
		if (fadeIn > 0) fadeIn--;
		if (fadeOut > 0) {
			if (fadeOut == 1)	{
				//if it was waiting for a fadeout, do something here
			}
			fadeOut--;
		}
		if (cp->fDirection < .5 && cp->fReverseMode < .5)  {//if we are reversing and its immediate
			if ((loopSize - loopCursor) < fadeTime) { //if its at the end of the loop do a loop fade
				fadeLoop = loopSize - loopCursor;  //as it approaches 0 it fades out
			} else if (loopCursor + 2 < fadeTime) {  //if its at the beginning of a loop
				fadeLoop = loopCursor + 3;  //as loopcursor gets away from 0 it fades in
			} else {  //otherwise don't turn on the loop fading
				fadeLoop = 0;
			}
		}

		noiseLoopCursor++;
		if (noiseLoopCursor >= noiseLoopSize) 
			noiseLoopCursor = 0;  //loop the denormal noise

//		if ((cursor >= sampleRate * 60 - 1) || (preCursor >= sampleRate * 60 - 1)) //jackass proofery
//			resetLoop(1);   //dont let it record past the maximum 60 second buffer

    } //END WHILE sampleFrames--

	
}

//-----------------------------------------------------------------------------------------
float LoopyLlama::crossFade (float signalA,float signalB, float precision, float position)
{
	if (position <= 0 || precision <= 0) return signalA;
		else
	if (position >= precision) return signalB;
		else

	return (position / precision) * signalB + (1.0f - position / precision) * signalA;  //linear for now
}

//------------------------------------------------------------------------
void LoopyLlama::setTimeInfo(VstTimeInfo * info){
    if (audioMaster)
		audioMaster (&cEffect, audioMasterSetTime, 0, 0, info, 0);
}

//------------------------------------------------------------------------
void LoopyLlama::sendMIDIData (char midiData0,char midiData1,char midiData2,char midiData3)
{
	VstMidiEvent vstMidiEventToHost;
    vstMidiEventToHost.type            = kVstMidiType;
	vstMidiEventToHost.byteSize        = 24;
	vstMidiEventToHost.deltaFrames     = 0;
	vstMidiEventToHost.flags           = 0;
	vstMidiEventToHost.noteLength      = 0;
	vstMidiEventToHost.noteOffset      = 0;
	vstMidiEventToHost.midiData[0]     = midiData0;
    vstMidiEventToHost.midiData[1]     = midiData1;
	vstMidiEventToHost.midiData[2]     = midiData2;
	vstMidiEventToHost.midiData[3]     = midiData3;
    vstMidiEventToHost.detune          = 0;

	VstEvents vstEventsToHost;
	vstEventsToHost.numEvents = 1;
	vstEventsToHost.events[0] = (VstEvent*) &vstMidiEventToHost;
	vstEventsToHost.reserved = 0;

	sendVstEventsToHost(&vstEventsToHost);
}

//-----------------------------------------------------------------------------------------
long LoopyLlama::processEvents (VstEvents* ev)
{
	LoopyLlamaProgram* cp = &programs[curProgram];
	for (long i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;
		VstMidiEvent midiEvent = *((VstMidiEvent*)ev->events[i]);
		long status = midiEvent.midiData[0] & 0xf0;		// ignoring channel
		long channel = midiEvent.midiData[0] & 0x0f;		// ignoring status
		long data1 = midiEvent.midiData[1] & 0x7f;
		long data2 = midiEvent.midiData[2] & 0x7f;
		if ((channel == int(cp->fMIDIChannel * 15))) {
			if (((cp->fNoteCC < .5f && status == 0xB0) || 
				//if we are looking for controllers, status is a controller
				(cp->fNoteCC > .5f && status == 0x90 && data2 > 0))) {
				//or we are looking for notes, status is a note, and note value is not 0 (in case its some crazy ass note-off)
				if (data1 == sliderTo7Bit(cp->fTapTempoCC) && (data2 > 0)) {//make sure it's the right note or controller number
					tapHappened (1.f);
				}
				//allow for duplicate assignments
				if (data1 == sliderTo7Bit(cp->fRecordButtonCC) && (data2 > 0)) { //if it was the recording controller/note
					setRecording(1.f);					
				}
				if (data1 == sliderTo7Bit(cp->fResetButtonCC) && (data2 > 0)) { //if it was the reset controller/note 
					setParameter(kReset,1.f);
				}  
				if (data1 == sliderTo7Bit(cp->fPlayButtonCC) && ( data2 > 0)) { //if it was the play controller/note 
					setParameter(kPlay,1.f);
					setParameter(kPlay,0.f);
				} 
				if (data1 == sliderTo7Bit(cp->fPausedButtonCC) && (data2 > 0)) { //if it was the pause controller/note
					setParameter(kPaused,float(1.f - cp->fPaused));
				}
				if (data1 == sliderTo7Bit(cp->fDirectionCC) && ( data2 > 0)) { //if it was the direction controller/note 
					setParameter(kDirection,float(data2 > 63));
				}
				if (data1 == sliderTo7Bit(cp->fFeedbackCC) && ( data2 > 0)) { //if it was the feedback controller/note 
					setParameter(kFeedBack,float(data2) / 127.f);
				}
				if (data1 == sliderTo7Bit(cp->fLpfCC) && ( data2 > 0)) { //if it was the LPF controller/note 
					setParameter(kEQFreq,float(data2) / 127.f);
				}
				if (data1 == sliderTo7Bit(cp->fMetVolCC) && ( data2 > 0)) { //if it was the metronome volume controller/note 
					setParameter(kMetronomeVolume,float(data2) / 127.f);
				}
			} else if (status == 0xC0 && data1 >= 0 && data1 < 128) { //it's program data
		//		data1 += (cp->fAddProgram >= 0.5f); //this is for devices that display program 1 when they transmit program 0 and so on... add 1 for these older keybords
				if (data1 == 0) data1 = 1;  //just making absolute sure than nobody sent program 0....JackAssery I tell you

				cp->fBeats = (float)data1 / 127.f;
				setParameterAutomated(kBeats,cp->fBeats); 
				resetWaiting = 1;
			}   //end if it's a program change adjust the number of bars to equal it
				
		}  //end if MIDI channel
	}
	return 1;	// want more
}



















void LoopyLlama::writeDebug(char text[64], int type) {



			// ******debug stuff
	FILE* verbose;
	char fileName[32] = "loopyLlamaDebug.txt";
	sprintf(fileName,"loopyLlamaDebug%u.txt",uniqueTime);
	verbose = fopen ( fileName, "a" );


//	time_t rawtime;

//	  time ( &rawtime );

//	  fprintf (verbose,  " %s", ctime (&rawtime) );
	  fprintf (verbose,  "%s", text );

	  if (!type) {
	fprintf ( verbose, "TIME = %u \n", countSamples );
	getParameterDisplay(kRecordMode,text);
	  fprintf (verbose,  "Rec Mode: %s \n", text );
	fprintf ( verbose, "metsize = %u \n", metLength );

	fprintf ( verbose, "bpm = %f \n", bpm );
	fprintf ( verbose, "beats = %u \n", beats );
//	fprintf ( verbose, "current bar = %f / %i\n", float(double(loopCursor) / beatLength) + 1.f, loopCursor / beatLength + (long(loopCursor / beatLength) < beats + 1));
	fprintf ( verbose, " cursor = %u \n", cursor );
	fprintf ( verbose, " preCursor = %u \n", preCursor );
	fprintf ( verbose, " loopCursor = %u \n", loopCursor );
	fprintf ( verbose, " loopSize = %u \n", loopSize );
	fprintf ( verbose, " buffer = %.4f \n", buffer[cursor] );
	fprintf ( verbose, " prebuffer = %.4f \n", preBuffer[preCursor] );
	fprintf ( verbose, " fadeLoop = %u \n", (unsigned int)fadeLoop );

	fprintf ( verbose, "  bufferDirection = %i \n", bufferDirection );
	fprintf ( verbose, "  preBufferDirection = %i \n", preBufferDirection );
	fprintf ( verbose, "  useBuffer = %i \n", useBuffer );
	fprintf ( verbose, "  usePreBuffer = %i \n", usePreBuffer );
	fprintf ( verbose, "  loopRunning = %i \n", loopRunning );
	fprintf ( verbose, "  isRecording = %i \n", isRecording );
	fprintf ( verbose, "  recordWaiting = %i \n", recordWaiting );
	fprintf ( verbose, "  resetWaiting = %i \n \n", resetWaiting );

	  }
	fclose ( verbose );



	//********end debug
}