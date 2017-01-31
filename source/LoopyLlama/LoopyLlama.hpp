//-------------------------------------------------------------------------------------------------------
//Loopy Llama by Chris Kline
//-------------------------------------------------------------------------------------------------------
//#pragma warning (disable : 4996)


#ifndef __LoopyLlama_H
#define __LoopyLlama_H

#define STEREO

#ifndef __SimpleLowPass__
#include "SimpleLowpass.h"
#endif

#include "audioeffectx.h"

//constants 
const unsigned int noiseLoopSize = 2565547;
const unsigned int maxSeconds = 60;
const unsigned int maxTaps = 8; // will actually be 2 more than this
/*
typedef struct {
   float  *Imp;
   float  *ImpD;
   float   LpScl;
   unsigned int   Nmult;
   unsigned int   Nwing;
   double  minFactor;
   double  maxFactor;
   unsigned int   XSize;
   float  *X;
   unsigned int   Xp; // Current "now"-sample pointer for input 
   unsigned int   Xread; // Position to put new samples 
   unsigned int   Xoff;
   unsigned int   YSize;
   float  *Y;
   unsigned int   Yp;
   double  Time;
} rsdata;

#define Npc 4096

#ifndef MAX
#define MAX(x,y) ((x)>(y) ?(x):(y))
#endif
#define PI 3.14159263
*/
/*
#define WIDTH 32                
#define DELAY_SIZE 64

#define USE_TABLE 0          
#define USE_INTERP 0        

#define SAMPLES_PER_ZERO_CROSSING 32    
*/


enum
{
	// Global
	kNumPrograms = 5,

	// Parameters Tags
	kBeats = 0,
	kBPM,
	kRecord,
	kReset,
	kPlay,
	kPaused,
	kSpeed,
	kIn,
	kLoopVol,
	kDry,
	kFeedBack,
	kFadeTime,
	kEQFreq,
	kTapeHiss,
	kDirection,
	kRecordMode,
	kOneLoop,
	kLoopSetsBeats,
	kReverseMode,
	kRecordAt,
//	kRecordButtonMode,
	kNoteCC,
	kMIDIChannel,
	kTapTempoCC,
	kRecordButtonCC,
	kResetButtonCC,
	kPlayButtonCC,
	kPausedButtonCC,
	kDirectionCC,
	kFeedbackCC,
	kLpfCC,
	kMetVolCC,
	kTaps,
	kMetronomeNote,
	kMetronomeVolume,
	kMetronomeInMains,
//	kAddProgram,
	kUnpausePlay,

	kNumParams,

	kIsRecording,//monitor vars
	kIsLoopRunning,
	kIsTempoOn,
	kCurrentBar,
	kCurrentPos,
	kVUlevel,
	kTapCount,
	kTapHappened,
	kLoad,
	kSave,
	kAbout,
};

//class LoopyLlama;

//------------------------------------------------------------------------
class LoopyLlamaProgram
{
friend class LoopyLlama;
public:
	LoopyLlamaProgram ();
	~LoopyLlamaProgram () {}

private:	
	float fBeats;
	float fBPM;
	float fRecord;
	float fReset;
	float fPlay;
	float fPaused;
	float fSpeed;
	float fIn;
	float fLoopVol;
	float fDry;
	float fFeedBack;
	float fFadeTime;
	float fEQFreq;
	float fTapeHiss;
	float fDirection;
	float fReverseMode;
	float fRecordMode;
	float fOneLoop;
	float fLoopSetsBeats;
	float fRecordAt;
//	float fRecordButtonMode;
	float fNoteCC;
	float fTapTempoCC;
	float fRecordButtonCC;
	float fResetButtonCC;
	float fPlayButtonCC;
	float fPausedButtonCC;
	float fDirectionCC;
	float fFeedbackCC;
	float fLpfCC;
	float fMetVolCC;
	float fMIDIChannel;
	float fTaps;
	float fMetronomeNote;
	float fMetronomeVolume;
	float fMetronomeInMains;
//	float fAddProgram;
	float fUnpausePlay;
	char name[24];
};

//-------------------------------------------------------------------------------------------------------
class LoopyLlama : public AudioEffectX
{
public:
	LoopyLlama (audioMasterCallback audioMaster);
	~LoopyLlama ();

	// Processes
	virtual void process (float **inputs, float **outputs, long sampleFrames);
	virtual void processReplacing (float **inputs, float **outputs, long sampleFrames);

	// Program
	virtual void setProgram (long program);
	virtual void setProgramName (char *name);
	virtual void getProgramName (char *name);

	virtual void resume ();

	// Parameters
	virtual void setParameter (long index, float value);
	virtual float getParameter (long index);
	virtual void getParameterLabel (long index, char *label);
	virtual void getParameterDisplay (long index, char *text);
	virtual void getParameterName (long index, char *text);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual long getVendorVersion () { return 1000; }
	virtual long onKeyDown (VstKeyCode &keyCode);
	
	virtual VstPlugCategory getPlugCategory () { return kPlugCategEffect; }
	virtual long canDo (char* text);

	virtual void tapHappened (float tapType);
	virtual long processEvents (VstEvents* events);
	LoopyLlamaProgram *programs;







protected:
	virtual void writeDebug(char text[64],int type = 0);
	
	VstTimeInfo _masterTimeInfo;
	virtual void setTimeInfo(VstTimeInfo * info);

	float makeNoise(int exponentMask);
	virtual void resetLoop (int resetType);
	virtual void setRecording (float value);
	virtual void sendMIDIData (char midiData0,char midiData1,char midiData2,char midiData3);
//	virtual void setBeatLength (unsigned long gapToMeasure, int numberOfBeats);
	float crossFade (float signalA,float signalB, float precision, float position);
	virtual void loadFile (char* WavePath);
	virtual void saveFile (char* WavePath);

	/*
double linear_interp(double first_number,double second_number,double fraction);
double t_sinc(double x);
double sinc(double x);
void make_sinc();
*/


	float *buffer, *preBuffer, *denormalNoiseLoop, *tapeNoiseLoop, *metronomeL, *metronomeR, *beatBuffer, *beatBuffer2, *buffer2, *preBuffer2;
/*	float fBeats, fFadeTime, fBPM, fRecord,fReset, fPlay, fPaused, fSpeed, fIn, fLoopVol, fDry, fTapTempoCC, fTaps, fNoteCC,
		fDirection, fFeedBack, fEQFreq, fTapeHiss, fAddProgram, fUnpausePlay, fRecordMode, fReverseMode, fMetronomeVolume, fRecordButtonCC, 
			fResetButtonCC, fMetronomeNote, fMIDIChannel;
*/
    float peakVU;//, speedRatio;
	unsigned long  loopSize, fadeTime,  countSamples, fadeIn, fadeOut, fadeLoop, lastRecordMove, 
		lastResetMove, lastMetronome, *tapTimes, uniqueTime, loopCursor, cursor, preCursor, resetAt, metLength, metCursor,
		beatBufferCopy,beatBufferCursor,beatBufferFade,beatBufferOffset;
	int tapCount,copyPreBuffer, useBuffer, usePreBuffer, recordWaiting, bufferDirection, preBufferDirection, 
		dontCopyThisLoop, resetWaiting, continueRecording, firstLoop, loopRunning, isRecording, tempoLightOn, beatOffset, beatBufferDirection;
//	double beatLength;
	double bpm;
	double sampleRate;
//	double formerPos;
	unsigned int noiseLoopCursor, beats, rand_state;

//	float outputEQX[5],outputEQY[5], inputEQX[5], inputEQY[5];
//	float sinc_table[WIDTH * SAMPLES_PER_ZERO_CROSSING];
//	int delay_buffer[3*WIDTH];
//int gimme_data(long j);

//void new_data(int data);

	bool init;
	SimpleLowpass LPL,LPR;




};
#define sliderTo7Bit(i)     (int)((i) * 127 + 1)
#define MIDIToSlider(i)     (float)((i) / 127.f)
#define sliderTo16(i)     (int)((i) * 15 + 1)

#endif
