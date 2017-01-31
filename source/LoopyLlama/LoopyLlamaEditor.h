//------------------------------------------------------------------------
//- Loopy Llama editor header
//------------------------------------------------------------------------

#ifndef __LoopyLlamaEdit__
#define __LoopyLlamaEdit__

#ifndef __vstgui__
#include "vstgui.h"
#endif

class CLabel;

#if MAC 
	void timerWatch(EventLoopTimerRef inTimer, void *inUserData);
#else
	void CALLBACK timerWatch(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);
#endif

//-----------------------------------------------------------------------------
class LoopyLlamaEditor : public AEffGUIEditor, public CControlListener
{
public:
	LoopyLlamaEditor (AudioEffect *effect);
	virtual ~LoopyLlamaEditor ();

	void suspend ();
	void resume ();
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	CMovieBitmap      *cRecordLight,*cLoopLight,*cTempoLight;
	CLabel            *cCurrentBar;
	CSplashScreen     *cCredits;
	CVuMeter          *cPositionMeter,*cVuMeter;

	static void lightMonitor(LoopyLlamaEditor* gui);
#if MAC 
	EventLoopTimerRef lightTimer;
#else
	MMRESULT m_idEvent;
#endif


protected:
	virtual void getAllControls ();
	virtual long onKeyDown (VstKeyCode &keyCode);
	//bool keysRequired ();
	virtual long open (void *ptr);
	virtual void idle ();
	void setParameter (long index, float value);
	virtual void close ();
	
private:
	void valueChanged (CDrawContext* context, CControl* control);

	COnOffButton      *cDirectionButton,*cReverseButton, *cAddProgram, *cPauseButton, 
		*cUnpausePlay, *cOneLoop, *cNoteCCButton, *cRecordButtonMode, *cLoopSetsBeats, *cMetronomeInMains;
	CKickButton       *cResetButton,*cRecordButton,*cTapButton, *cPlayButton, *cLoad, *cSave;
	CKnob             *cMetronomeVolume;
	CVerticalSwitch   *cRecordMode;
	CHorizontalSwitch *cRecordAt;
	CVerticalSlider   *cIn,*cLoopVol,*cDry,*cFeedBack, *cEQFreq, *cTapeHiss, *cFadeTime, *cSpeed;

	CTextEdit         *cBars,*cTempo, *cTapTempoCC, *cMetronomeNote, *cTaps,*cRecordButtonCC,*cResetButtonCC,
		*cPlayButtonCC,*cPauseButtonCC,*cMIDIChannel, *cDirectionCC, *cFeedbackCC, *cLpfCC, *cMetVolCC;


	CViewContainer    *cViewContainer;

	CLabel            *cFadeTimeDisplay,*cFeedbackDisplay,*cExpireDate;

	long              oldTicks;
	double monitorBPM;
	bool expired;

};

#endif
