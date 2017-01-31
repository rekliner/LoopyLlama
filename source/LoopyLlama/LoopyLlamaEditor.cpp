//------------------------------------------------------------------------
//-
//- Project     : Use different Controls of VSTGUI
//- Filename    : LoopyLlamaEditor.cpp
//- Created by  : Yvan Grabit
//- Description :
//-
//- © 2000-1999, Steinberg Soft und Hardware GmbH, All Rights Reserved
//------------------------------------------------------------------------

#ifndef __LoopyLlamaEdit__
#include "LoopyLlamaEditor.h"
#endif

#ifndef __LoopyLlama__
#include "LoopyLlama.hpp"
#endif

#include <math.h>
#include <time.h>
#include <stdlib.h>	
#include <stdio.h>

#if WINDOWS
#include <windows.h>
#include <mmsystem.h>
#endif

enum
{
	// bitmaps
	kBackgroundBitmap = 10001,
	
	kSliderVBgBitmap,
	kSliderHandleBitmap,

	kRecordButtonBitmap,
	kResetButtonBitmap,

	kDirectionButtonBitmap,

	kReverseButtonBitmap,
	kTapButtonBitmap,

	kVuOnBitmap,
	kVuOffBitmap,

	kVSwitchBitmap,



	kCreditsBitmap, 

	kRedFlasherBitmap,
	kYellowFlasherBitmap,
	kGreenFlasherBitmap,

	kCheckboxBitmap,
	kKnobBodyBitmap,
	kKnobHandleBitmap,

	kPositionOnBitmap,
	kPositionOffBitmap,
 
	kPlayButtonBitmap,
	kPausedButtonBitmap,
	kTwoPositionVSwitchBitmap,
	kRecordAtBitmap,
	kSaveBitmap,
	kLoadBitmap,



	// others
	kBackgroundW = 400,
	kBackgroundH = 249

//	kAbout = 20000
};
const int kPositionResolution = 137;

//-----------------------------------------------------------------------------
// CLabel declaration
//-----------------------------------------------------------------------------
class CLabel : public CParamDisplay
{
public:
	CLabel (CRect &size, char *text);

	void draw (CDrawContext *pContext);

	void setLabel (char *text);
//	char * getLabel ();
	bool onDrop (void **ptrItems, long nbItems, long type, CPoint &where);

protected:
	char label[256];
};




//-----------------------------------------------------------------------------
// CLabel implementation
//-----------------------------------------------------------------------------
CLabel::CLabel (CRect &size, char *text)
: CParamDisplay (size)
{
	strcpy (label, "");
	setLabel (text);
}

//------------------------------------------------------------------------
void CLabel::setLabel (char *text)
{
	if (text)
		strcpy (label, text);
	setDirty ();
}

//-----------------------------------------------------------------------------
bool CLabel::onDrop (void **ptrItems, long nbItems, long type, CPoint &where)
{
	if (nbItems > 0 && type == kDropFiles)
	{
		char text[1024];
		long pos = where.h - size.left;
		sprintf (text, "%d : %s at %d", int(nbItems), (char*)ptrItems[0], int(pos));
		setLabel (text);
	}
	return true;
}

//------------------------------------------------------------------------
void CLabel::draw (CDrawContext *pContext)
{
	pContext->setFillColor (backColor);
	pContext->fillRect (size);
	pContext->setFrameColor (fontColor);
	pContext->drawRect (size);

	pContext->setFont (fontID);  //change font size here
	pContext->setFontColor (fontColor);
	pContext->drawString (label, size, false, kCenterText);
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
LoopyLlamaEditor::LoopyLlamaEditor (AudioEffect *effect) 
	:	AEffGUIEditor (effect)
{
	frame = 0;
//	oldTicks = getTicks();

	rect.left   = 0;
	rect.top    = 0;
	rect.right  = kBackgroundW;
	rect.bottom = kBackgroundH;

	// we decide in this plugin to open all bitmaps in the open function
//	effect->setUniqueID ('JaL2');

}


bool LoopyLlamaEditor::getEffectName (char* name)
{
	strcpy (name, "Loopy Llama");
//	strcpy (name, "LoopyLlama");
	return true;
}
//------------------------------------------------------------------------
bool LoopyLlamaEditor::getProductString (char* text)
{
	strcpy (text, "Loopy Llama");
//	strcpy (text, "LoopyLlama");
	return true;
}
//------------------------------------------------------------------------
bool LoopyLlamaEditor::getVendorString (char* text)
{
	strcpy (text, "Rekliner.com");
	return true;
}



//-----------------------------------------------------------------------------
LoopyLlamaEditor::~LoopyLlamaEditor ()
{}

//-----------------------------------------------------------------------------
long LoopyLlamaEditor::open (void *ptr)
{
	// always call this !!!
	AEffGUIEditor::open (ptr);
expired = false;
/*
//set the expire time
time_t Ttime;
struct tm * timeinfo;
timeinfo = localtime ( &Ttime );
timeinfo->tm_year = 2007      - 1900;
timeinfo->tm_mon  = 06         - 1;
timeinfo->tm_mday = 1           ;
Ttime = mktime ( timeinfo );



	expired = (time (NULL) > Ttime) ? true : false;

*/
	// init the background bitmap
	CBitmap *background = new CBitmap (kBackgroundBitmap);

	CRect size (0, 0, background->getWidth (), background->getHeight ());
	frame = new CFrame (size, ptr, this);

	//--CFrame-----------------------------------------------
	frame->setBackground (background);
	background->forget ();

	CPoint point (0, 0);





	//-------LIGHTS -----------------
	//--record light-------------------------------------
  	CBitmap *movieBitmap = new CBitmap (kRedFlasherBitmap);

	size (0, 0, movieBitmap->getWidth (), movieBitmap->getHeight () / 2);
	size.offset (5, 0);
	point (0, 0);
	cRecordLight = new CMovieBitmap (size, this, 0, 2, movieBitmap->getHeight () / 2, movieBitmap, point);
	frame->addView (cRecordLight);
	movieBitmap->forget ();

  	CBitmap *movieBitmap2 = new CBitmap (kYellowFlasherBitmap);

	//looping light
	size (0, 0, movieBitmap2->getWidth (), movieBitmap2->getHeight () / 2);
	size.offset (5, 40);
	point (0, 0);
	cLoopLight = new CMovieBitmap (size, this, 0, 2, movieBitmap2->getHeight () / 2, movieBitmap2, point);
	frame->addView (cLoopLight);
	movieBitmap2->forget ();

  	CBitmap *movieBitmap3 = new CBitmap (kGreenFlasherBitmap);
	//tempo light
	size (0, 0, movieBitmap3->getWidth (), movieBitmap3->getHeight () / 2);
	size.offset (5, 80);
	point (0, 0);
	cTempoLight = new CMovieBitmap (size, this, 0, 2, movieBitmap3->getHeight () / 2, movieBitmap3, point);
	frame->addView (cTempoLight);
	movieBitmap3->forget ();






	//--------------BUTTONS ------------------------//
	//--RecordButton-----------------------------------------------
	CBitmap *recordButton = new CBitmap (kRecordButtonBitmap);
	size (0, 0, recordButton->getWidth (), recordButton->getHeight () / 2);
 	size.offset (52, 8);
	cRecordButton = new CKickButton (size, this, kRecord, recordButton->getHeight() / 2, recordButton, point);
	frame->addView (cRecordButton);
	recordButton->forget ();

	//--ResetButton-----------------------------------------------
	CBitmap *resetButton = new CBitmap (kResetButtonBitmap);
	size (0, 0, resetButton->getWidth (), resetButton->getHeight () / 2);
 	size.offset (52, 44);
	point (0, 0);
	cResetButton = new CKickButton (size, this, kReset, resetButton->getHeight() / 2, resetButton, point);
	frame->addView (cResetButton);
	resetButton->forget ();


	//--PlayButton-----------------------------------------------
	CBitmap *playButton = new CBitmap (kPlayButtonBitmap);
	size (0, 0, playButton->getWidth (), playButton->getHeight () / 2);
 	size.offset (84, 8);
	point (0, 0);
	cPlayButton = new CKickButton (size, this, kPlay, playButton->getHeight() / 2, playButton, point);
	frame->addView (cPlayButton);
	playButton->forget ();

	//--PauseButton-----------------------------------------------
	CBitmap *pauseButton = new CBitmap (kPausedButtonBitmap);
	size (0, 0, pauseButton->getWidth (), pauseButton->getHeight () / 2);
 	size.offset (84, 44);
	point (0, 0);
	cPauseButton = new COnOffButton (size, this, kPaused, pauseButton);
	frame->addView (cPauseButton);
	pauseButton->forget ();


	//--tapButton-----------------------------------------------
	CBitmap *tapButton = new CBitmap (kTapButtonBitmap);
	size (0, 0, tapButton->getWidth (), tapButton->getHeight () / 2);
 	size.offset (52, 80);
	point (0, 0);
	cTapButton = new CKickButton (size, this, kTapHappened, tapButton->getHeight() / 2, tapButton, point);
	frame->addView (cTapButton);
	tapButton->forget ();

	//--DirectionButton-----------------------------------------------
	CBitmap *directionButton = new CBitmap (kDirectionButtonBitmap);
	size (0, 0, directionButton->getWidth (), directionButton->getHeight () / 2);
 	size.offset (52, 116);
	cDirectionButton = new COnOffButton (size, this, kDirection, directionButton);
	frame->addView (cDirectionButton);
	directionButton->forget ();

	//--Reverse mode Button checkboxes-----------------------------------------------
	CBitmap *reverseButton = new CBitmap (kReverseButtonBitmap);
	size (0, 0, reverseButton->getWidth (), reverseButton->getHeight () / 2);
 	size.offset (84, 116);
	cReverseButton = new COnOffButton (size, this, kReverseMode, reverseButton);
	frame->addView (cReverseButton);
	reverseButton->forget ();

	//--Note or CC button--------------------------------------
	CBitmap *twoPositionVSwitch = new CBitmap (kTwoPositionVSwitchBitmap);
	size (0, 0, twoPositionVSwitch->getWidth (), twoPositionVSwitch->getHeight () / 2);
 	size.offset (354, 214);
	cNoteCCButton = new COnOffButton (size, this, kNoteCC, twoPositionVSwitch);
	frame->addView (cNoteCCButton);

	//--Record Button Mode switch--------------------------------------
//	size (0, 0, twoPositionVSwitch->getWidth (), twoPositionVSwitch->getHeight () / 2);
// 	size.offset (248, 158);
//	cRecordButtonMode = new COnOffButton (size, this, kRecordButtonMode, twoPositionVSwitch);
//	frame->addView (cRecordButtonMode);
	
	
	twoPositionVSwitch->forget (); 


	//--save-----------------------------------------------
	CBitmap *saveButton = new CBitmap (kSaveBitmap);
	size (0, 0, saveButton->getWidth (), saveButton->getHeight () / 2);
 	size.offset (346, 5);
	point (0, 0);
	cSave = new CKickButton (size, this, kSave, saveButton->getHeight() / 2, saveButton, point);
	frame->addView (cSave);
	saveButton->forget (); 

	//--load-----------------------------------------------
	CBitmap *loadButton = new CBitmap (kLoadBitmap);
	size (0, 0, loadButton->getWidth (), loadButton->getHeight () / 2);
 	size.offset (367, 5);
	point (0, 0);
	cLoad = new CKickButton (size, this, kLoad, loadButton->getHeight() / 2, loadButton, point);
	frame->addView (cLoad);
	loadButton->forget ();




	//--CHECKBOXES-------------------------------------------------------
	CBitmap *checkbox = new CBitmap (kCheckboxBitmap);

	//--One loop only box-----------------------------------------------
	size (0, 0, checkbox->getWidth (), checkbox->getHeight () / 2);
  	size.offset (223, 183);
	cOneLoop = new COnOffButton (size, this, kOneLoop, checkbox);
	frame->addView (cOneLoop);

	//--unpause triggers play-----------------------------------------------
	size (0, 0, checkbox->getWidth (), checkbox->getHeight () / 2);
  	size.offset (24, 219);
	cUnpausePlay = new COnOffButton (size, this, kUnpausePlay, checkbox);
	frame->addView (cUnpausePlay);

	//--first loop sets beats checkbox-----------------------------------------------
	size (0, 0, checkbox->getWidth (), checkbox->getHeight () / 2);
  	size.offset (6, 187);
	cLoopSetsBeats = new COnOffButton (size, this, kLoopSetsBeats, checkbox);
	frame->addView (cLoopSetsBeats);

	//--metronome in mains checkbox-----------------------------------------------
	size (0, 0, checkbox->getWidth (), checkbox->getHeight () / 2);
 	size.offset (254, 137);
	cMetronomeInMains = new COnOffButton (size, this, kMetronomeInMains, checkbox);
	frame->addView (cMetronomeInMains);



	//--Add 1 to progams Button checkbox-----------------------------------------------
//	size (0, 0, checkbox->getWidth (), checkbox->getHeight () / 2);
// 	size.offset (313, 217);
//	cAddProgram = new COnOffButton (size, this, kAddProgram, checkbox);
//	frame->addView (cAddProgram);


	checkbox->forget ();




	//--FADERS--------------------------------------
	CBitmap *sliderHandleBitmap = new CBitmap (kSliderHandleBitmap);
	CBitmap *sliderVBgBitmap = new CBitmap (kSliderVBgBitmap);
	
	//--In to Loop Volume--------------------------------------
	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
	size.offset (122, 43);
	point (0, 0);
	cIn = new CVerticalSlider (size, this, kIn, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
	point (2, 0);
	cIn->setOffsetHandle (point);
	frame->addView (cIn);

	//--Loop Volume--------------------------------------
	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
	size.offset (157, 43);
	point (0, 0);
	cLoopVol = new CVerticalSlider (size, this, kLoopVol, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
	point (2, 0);
	cLoopVol->setOffsetHandle (point);
	frame->addView (cLoopVol);

	//--Dry Volume--------------------------------------
	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
	size.offset (198, 43);
	point (0, 0);
	cDry = new CVerticalSlider (size, this, kDry, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
	point (2, 0);
	cDry->setOffsetHandle (point);
	frame->addView (cDry);

	//--Feedback --------------------------------------
	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
	size.offset (245, 42);
	point (0, 0);
	cFeedBack = new CVerticalSlider (size, this, kFeedBack, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
	point (2, 0);
	cFeedBack->setOffsetHandle (point);
	frame->addView (cFeedBack);
		//--LABEL: feedback percentage--------------------------------------
		size (0, 0, 23, 11); //22 for number only
		size.offset (262, 60);
		cFeedbackDisplay = new CLabel (size, "");
		if (cFeedbackDisplay)
		{
			cFeedbackDisplay->setFont (kNormalFontVerySmall);
			cFeedbackDisplay->setFontColor (kWhiteCColor);
			cFeedbackDisplay->setBackColor (kGreyCColor);
			frame->addView (cFeedbackDisplay);
		}

	//--EQ Frequency --------------------------------------
	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
	size.offset (310, 30);
	point (0, 0);
	cEQFreq = new CVerticalSlider (size, this, kEQFreq, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
	point (2, 0);
	cEQFreq->setOffsetHandle (point);
	frame->addView (cEQFreq);

	//--Tape Hiss Volume --------------------------------------
	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
	size.offset (357, 30);
	point (0, 0);
	cTapeHiss = new CVerticalSlider (size, this, kTapeHiss, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
	point (2, 0);
	cTapeHiss->setOffsetHandle (point);
	frame->addView (cTapeHiss);


	//--Speed of playback --------------------------------------
//	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
//	size.offset (371, 43);
//	point (0, 0);
//	cSpeed = new CVerticalSlider (size, this, kPaused, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
//	point (2, 0);
//	cSpeed->setOffsetHandle (point);
//	frame->addView (cSpeed);


	//--Fade Time --------------------------------------
	size (0, 0, sliderVBgBitmap->getWidth (), sliderVBgBitmap->getHeight ());
	size.offset (324, 131);
	point (0, 0);
	cFadeTime = new CVerticalSlider (size, this, kFadeTime, size.top + 2, size.top + sliderVBgBitmap->getHeight () - sliderHandleBitmap->getHeight () - 1, sliderHandleBitmap, sliderVBgBitmap, point, kBottom);
	point (2, 0);
	cFadeTime->setOffsetHandle (point);
	frame->addView (cFadeTime);

		//--LABEL: fade time in samples--------------------------------------
		size (0, 0, 48, 11); //22 for number only
		size.offset (341, 150);
		cFadeTimeDisplay = new CLabel (size, "");
		if (cFadeTimeDisplay)
		{
			cFadeTimeDisplay->setFont (kNormalFontVerySmall);
			cFadeTimeDisplay->setFontColor (kWhiteCColor);
			cFadeTimeDisplay->setBackColor (kGreyCColor);
			frame->addView (cFadeTimeDisplay);
		}



	sliderVBgBitmap->forget ();
	sliderHandleBitmap->forget ();





	//-----SWITCHES----------------------------------
	CBitmap *switchVBitmap = new CBitmap (kVSwitchBitmap);

	//-- record mode--------------------------------------
	size (0, 0, switchVBitmap->getWidth (), switchVBitmap->getHeight () / 4);
	size.offset (121, 131);
	cRecordMode = new CVerticalSwitch (size, this, kRecordMode, 4, switchVBitmap->getHeight () / 4, 4, switchVBitmap, point);
	frame->addView (cRecordMode);

	//--CVerticalSwitch set tempo mode--------------------------------------
//	size (0, 0, switchVBitmap->getWidth (), switchVBitmap->getHeight () / 4);
//	size.offset (214, 130);
//	cNoteCC = new CVerticalSwitch (size, this, kNoteCC, 4, switchVBitmap->getHeight () / 4, 4, switchVBitmap, point);
//	frame->addView (cNoteCC);

	switchVBitmap->forget ();

	CBitmap *recordAtBitmap = new CBitmap (kRecordAtBitmap);
	//-- record at next...--------------------------------------
	size (0, 0, recordAtBitmap->getWidth (), recordAtBitmap->getHeight () / 3);
	size.offset (217, 171);
	cRecordAt = new CHorizontalSwitch (size, this, kRecordAt, 3, recordAtBitmap->getHeight () / 3, 3, recordAtBitmap, point);
	frame->addView (cRecordAt);
	recordAtBitmap->forget ();





	//----TEXT EDITS------------------------
	//--cBars--------------------------------------
	size (0, 0, 39, 20);
	size.offset (5, 152);
//	char * textEditDefault = "";
//	effect->long2string(int(effect->getParameter (kBeats) * 127),textEditDefault);
	//float2string(char textEditDefault, effect->getParameter (kBeats));
	cBars = new CTextEdit (size, this, kBeats, 0, 0, k3DIn);
	if (cBars)
	{
		cBars->setFont (kNormalFontVeryBig);
		cBars->setFontColor (kWhiteCColor);
		cBars->setBackColor (kBlackCColor);
		cBars->setFrameColor (kWhiteCColor);
		cBars->setHoriAlign (kCenterText);
		frame->addView (cBars);
	}

	//--LABEL: current bar--------------------------------------
	size (0, 0, 30, 20);
	size.offset (64, 152);
	cCurrentBar = new CLabel (size, "Bar");
	if (cCurrentBar)
	{
		cCurrentBar->setFont (kNormalFontVeryBig);
		cCurrentBar->setFontColor (kWhiteCColor);
		cCurrentBar->setBackColor (kGreyCColor);
		frame->addView (cCurrentBar);
	}

	//--cTempo--------------------------------------
	size (0, 0, 39, 20);
	size.offset (5, 119);
	cTempo = new CTextEdit (size, this, kBPM, 0, 0, k3DIn);
	if (cTempo)
	{
//		int myMerlot[4] = {0,     0,   0, 0};
		cTempo->setFont (kNormalFontVeryBig);
		cTempo->setFontColor (kWhiteCColor);
		cTempo->setBackColor (kBlackCColor);
		cTempo->setFrameColor (kWhiteCColor);
		cTempo->setHoriAlign (kCenterText);
		frame->addView (cTempo);
	}


	//--cMetronome Note--------------------------------------
	size (0, 0, 20, 15);
	size.offset (285, 137);
	cMetronomeNote = new CTextEdit (size, this, kMetronomeNote, 0, 0, k3DIn);
	if (cMetronomeNote)
	{
		cMetronomeNote->setFont (kNormalFontBig);
		cMetronomeNote->setFontColor (kWhiteCColor);
		cMetronomeNote->setBackColor (kBlackCColor);
		cMetronomeNote->setFrameColor (kWhiteCColor);
		cMetronomeNote->setHoriAlign (kCenterText);
		frame->addView (cMetronomeNote);
	}

	//minutia---------------------------------------------

	//--cTaps--------------------------------------
	size (0, 0, 15, 15);
	size.offset (67, 217);
	cTaps = new CTextEdit (size, this, kTaps, 0, 0, k3DIn);
	if (cTaps)
	{
		cTaps->setFont (kNormalFontBig);
		cTaps->setFontColor (kWhiteCColor);
		cTaps->setBackColor (kBlackCColor);
		cTaps->setFrameColor (kWhiteCColor);
		cTaps->setHoriAlign (kCenterText);
		frame->addView (cTaps);
	}
	//--midiChannel--------------------------------------
	size (0, 0, 15, 15);
	size.offset (92, 217);
	cMIDIChannel = new CTextEdit (size, this, kMIDIChannel, 0, 0, k3DIn);
	if (cMIDIChannel)
	{
		cMIDIChannel->setFont (kNormalFontBig);
		cMIDIChannel->setFontColor (kWhiteCColor);
		cMIDIChannel->setBackColor (kBlackCColor);
		cMIDIChannel->setFrameColor (kWhiteCColor);
		cMIDIChannel->setHoriAlign (kCenterText);
		frame->addView (cMIDIChannel);
	}
	//--recordCC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (118, 217);
	cRecordButtonCC = new CTextEdit (size, this, kRecordButtonCC, 0, 0, k3DIn);
	if (cRecordButtonCC)
	{
		cRecordButtonCC->setFont (kNormalFontBig);
		cRecordButtonCC->setFontColor (kWhiteCColor);
		cRecordButtonCC->setBackColor (kBlackCColor);
		cRecordButtonCC->setFrameColor (kWhiteCColor);
		cRecordButtonCC->setHoriAlign (kCenterText);
		frame->addView (cRecordButtonCC);
	}
	//--playCC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (145, 217);
	cPlayButtonCC = new CTextEdit (size, this, kPlayButtonCC, 0, 0, k3DIn);
	if (cPlayButtonCC)
	{
		cPlayButtonCC->setFont (kNormalFontBig);
		cPlayButtonCC->setFontColor (kWhiteCColor);
		cPlayButtonCC->setBackColor (kBlackCColor);
		cPlayButtonCC->setFrameColor (kWhiteCColor);
		cPlayButtonCC->setHoriAlign (kCenterText);
		frame->addView (cPlayButtonCC);
	}
	//--resetCC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (172, 217);
	cResetButtonCC = new CTextEdit (size, this, kResetButtonCC, 0, 0, k3DIn);
	if (cResetButtonCC)
	{
		cResetButtonCC->setFont (kNormalFontBig);
		cResetButtonCC->setFontColor (kWhiteCColor);
		cResetButtonCC->setBackColor (kBlackCColor);
		cResetButtonCC->setFrameColor (kWhiteCColor);
		cResetButtonCC->setHoriAlign (kCenterText);
		frame->addView (cResetButtonCC);
	}
	//--pauseCC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (200, 217);
	cPauseButtonCC = new CTextEdit (size, this, kPausedButtonCC, 0, 0, k3DIn);
	if (cPauseButtonCC)
	{
		cPauseButtonCC->setFont (kNormalFontBig);
		cPauseButtonCC->setFontColor (kWhiteCColor);
		cPauseButtonCC->setBackColor (kBlackCColor);
		cPauseButtonCC->setFrameColor (kWhiteCColor);
		cPauseButtonCC->setHoriAlign (kCenterText);
		frame->addView (cPauseButtonCC);
	}
	//--tap tempo CC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (226, 217);
	cTapTempoCC = new CTextEdit (size, this, kTapTempoCC, 0, 0, k3DIn);
	if (cTapTempoCC)
	{
		cTapTempoCC->setFont (kNormalFontBig);
		cTapTempoCC->setFontColor (kWhiteCColor);
		cTapTempoCC->setBackColor (kBlackCColor);
		cTapTempoCC->setFrameColor (kWhiteCColor);
		cTapTempoCC->setHoriAlign (kCenterText);
		frame->addView (cTapTempoCC);
	}
	//--direction CC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (252, 217);
	cDirectionCC = new CTextEdit (size, this, kDirectionCC, 0, 0, k3DIn);
	if (cDirectionCC)
	{
		cDirectionCC->setFont (kNormalFontBig);
		cDirectionCC->setFontColor (kWhiteCColor);
		cDirectionCC->setBackColor (kBlackCColor);
		cDirectionCC->setFrameColor (kWhiteCColor);
		cDirectionCC->setHoriAlign (kCenterText);
		frame->addView (cDirectionCC);
	}
	//--feedback CC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (277, 217);
	cFeedbackCC = new CTextEdit (size, this, kFeedbackCC, 0, 0, k3DIn);
	if (cFeedbackCC)
	{
		cFeedbackCC->setFont (kNormalFontBig);
		cFeedbackCC->setFontColor (kWhiteCColor);
		cFeedbackCC->setBackColor (kBlackCColor);
		cFeedbackCC->setFrameColor (kWhiteCColor);
		cFeedbackCC->setHoriAlign (kCenterText);
		frame->addView (cFeedbackCC);
	}
	//--lpf CC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (302, 217);
	cLpfCC = new CTextEdit (size, this, kLpfCC, 0, 0, k3DIn);
	if (cLpfCC)
	{
		cLpfCC->setFont (kNormalFontBig);
		cLpfCC->setFontColor (kWhiteCColor);
		cLpfCC->setBackColor (kBlackCColor);
		cLpfCC->setFrameColor (kWhiteCColor);
		cLpfCC->setHoriAlign (kCenterText);
		frame->addView (cLpfCC);
	}
	//--Metronome Volume CC--------------------------------------
	size (0, 0, 20, 15);
	size.offset (328, 217);
	cMetVolCC = new CTextEdit (size, this, kMetVolCC, 0, 0, k3DIn);
	if (cMetVolCC)
	{
		cMetVolCC->setFont (kNormalFontBig);
		cMetVolCC->setFontColor (kWhiteCColor);
		cMetVolCC->setBackColor (kBlackCColor);
		cMetVolCC->setFrameColor (kWhiteCColor);
		cMetVolCC->setHoriAlign (kCenterText);
		frame->addView (cMetVolCC);
	}







	//--metronome volume--------------------------------------
	CBitmap *knob   = new CBitmap (kKnobHandleBitmap);
	CBitmap *bodyKnob = new CBitmap (kKnobBodyBitmap);

 	size (0, 0, bodyKnob->getWidth (), bodyKnob->getHeight ());
	size.offset (222, 137);
	point (0, 0);
	cMetronomeVolume = new CKnob (size, this, kMetronomeVolume, bodyKnob, knob, point);
	cMetronomeVolume->setInsetValue (3);
	frame->addView (cMetronomeVolume);
	knob->forget ();
	bodyKnob->forget ();



	//--CSplashScreen--------------------------------------
	CBitmap *creditsBitmap = new CBitmap (kCreditsBitmap);

	size (0, 0, 170, 25);
	size.offset (117, 0);
	point (0, 0);
	CRect toDisplay (0, 0, creditsBitmap->getWidth (), creditsBitmap->getHeight ());
	toDisplay.offset (0, 0);

	cCredits = new CSplashScreen (size, this, kAbout, creditsBitmap, toDisplay, point);
	frame->addView (cCredits);

/*
	//------expire date

	//	const long * tmpTime = (const long *)malloc(sizeof(long));
	//	tmpTime = (const long *)mktime(timeinfo);
		size (0, 0, 200, 11); //22 for number only
		size.offset (10, 186);
		//char * strInt = new char[255];
		char strExpire[60] = "";
		if (expired) {
			strcat(strExpire,"Plugin expired! Please download latest version!");
		} else {
			strcat(strExpire,"Beta expires on ");
			strcat(strExpire,ctime (&Ttime));
		}
		cExpireDate = new CLabel (size, strtok (strExpire,"\n"));
//		if (cExpireDate)
//		{
			cExpireDate->setFont (kNormalFontVerySmall);
			cExpireDate->setFontColor (kWhiteCColor);
			cExpireDate->setBackColor (kGreyCColor);
//			frame->addView (cExpireDate);
//			frame->removeView(cExpireDate);
			

//		}
*/


	//--CVuMeter--------------------------------------
	CBitmap* vuOnBitmap  = new CBitmap (kVuOnBitmap);
	CBitmap* vuOffBitmap = new CBitmap (kVuOffBitmap);

	size (0, 0, vuOnBitmap->getWidth (), vuOnBitmap->getHeight ());
	size.offset (173, 38);
	cVuMeter = new CVuMeter (size, vuOnBitmap, vuOffBitmap, 14);
	cVuMeter->setDecreaseStepValue (0.1f);
	frame->addView (cVuMeter);
	vuOnBitmap->forget ();
	vuOffBitmap->forget ();


	//--CPositionMeter--------------------------------------
	CBitmap* positionOnBitmap  = new CBitmap (kPositionOnBitmap);
	CBitmap* positionOffBitmap = new CBitmap (kPositionOffBitmap);

	size (0, 0, positionOnBitmap->getWidth (), positionOnBitmap->getHeight ());
	size.offset (117, 111);
	cPositionMeter = new CVuMeter (size, positionOnBitmap, positionOffBitmap, kPositionResolution,kHorizontal );
	cPositionMeter->setDecreaseStepValue(1.f);
	frame->addView (cPositionMeter); 
	positionOnBitmap->forget ();
	positionOffBitmap->forget ();


	getAllControls ();

/*
	//copy protection
	if (expired) {  
		close()	;
		

			size (0, 0, creditsBitmap->getWidth (), creditsBitmap->getHeight ());
		frame = new CFrame (size, ptr, this);
		frame->setBackground (creditsBitmap);
		frame->addView (cExpireDate);

	}
*/
	creditsBitmap->forget ();

	
//	frame->setSize(kBackgroundW,kBackgroundH);


#if MAC   //-------------------   begin mac OSX specific timer code ---------------

     InstallEventLoopTimer(
         GetCurrentEventLoop(),
         .5,
         .001,
         NewEventLoopTimerUPP( timerWatch ),
         this,
         &lightTimer );  

	return true;
}

//----mac OSX timer function-------------------------------------------------------------------------
void timerWatch (EventLoopTimerRef inTimer, void *inUserData)
{
	LoopyLlamaEditor * gui = NULL;
	gui = ((LoopyLlamaEditor*)inUserData);	
	if (gui) {
		GrafPtr oldPort;
		bool swapped = QDSwapPort(
		    (GrafPtr)GetWindowPort(
			(WindowRef)gui->getFrame()->getSystemWindow()),
			    &oldPort);

		gui->lightMonitor(gui);
		
		if ( swapped )
		    ::QDSwapPort( oldPort,NULL );         
	    
     } else {
	     RemoveEventLoopTimer(inTimer);
	    inTimer=0;
	 }
}


#else   //-------------------   end Mac OSX and begin windows specific timer code ---------------

    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    DWORD resolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    timeBeginPeriod(resolution);
    // create the timer
    m_idEvent = timeSetEvent(
        resolution,
        resolution,
        timerWatch,
        (DWORD)this,
        TIME_PERIODIC);

	return true;
}

//------windows timer function-----------------------------------------------------------------------


void CALLBACK timerWatch(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2) {
	LoopyLlamaEditor* gui = (LoopyLlamaEditor*) dwUser;
	if (gui) {
		gui->lightMonitor(gui);
	} else {
		// destroy the timer
	    timeKillEvent(wTimerID);
	}
}
#endif  //-----------    end windows specific code ----------------------


//-----MAC & PC ------if the timer light has changed update it, the bar number, and the position meter ------------------------------------------------------------------
void LoopyLlamaEditor::lightMonitor(LoopyLlamaEditor * gui) {	
	if (gui->cTempoLight && gui->cCredits && gui->cCurrentBar && gui->cVuMeter) {
		float isLightOn,currentPosition;
		isLightOn = gui->getEffect()->getParameter(kIsTempoOn);
		currentPosition = gui->getEffect()->getParameter(kCurrentPos);
//		if (floor(currentPosition * kPositionResolution) != floor(gui->cPositionMeter->getValue() * kPositionResolution)) {  //too expensive
//		}
		if (gui->cCredits->getValue() < 1.f) {  //if the credits aren't up
			gui->cPositionMeter->setValue(currentPosition);
			gui->cPositionMeter->redraw();
			if (isLightOn != gui->cTempoLight->getValue ()) {
				gui->cTempoLight->setValue (isLightOn);

				char strBar[8] = "";
				sprintf(strBar,"%1.0f",gui->getEffect()->getParameter(kCurrentBar));
				gui->cCurrentBar->setLabel(strBar); 
				gui->cTempoLight->redraw(); 
				gui->cCurrentBar->redraw();

//				if (gui->frame->isChild(gui->cExpireDate))
//					gui->frame->removeView(gui->cExpireDate); //dont want to do this...extra cpu load
			}
		} else {
			//if (!gui->frame->attached(gui->cExpireDate)) {
			//	CRect size (0, 0, 180, 11);
			//	size.offset (50, 160);
			//	gui->cExpireDate->setFont (kNormalFontVerySmall);
			//	gui->cExpireDate->setFontColor (kWhiteCColor);
			//	gui->cExpireDate->setBackColor (kGreyCColor);

			//}
	//		if (gui->frame->isChild(gui->cExpireDate))
		//		gui->frame->addView(gui->cExpireDate);
//			gui->cExpireDate->redraw();//setLabel(cExpireDate->getLabel());
		}
	} 
}

//-----------------------------------------------------------------------------
void LoopyLlamaEditor::getAllControls () { 

	//main
	setParameter(kIn,0.f);
	setParameter(kLoopVol,0.f);
	setParameter(kDry,0.f);

	//buttons  --  record, reset, play, & tap tempo are independant of plug
	setParameter(kDirection,0.f);
	setParameter(kReverseMode,0.f);
//	setParameter(kAddProgram,0.f);
	setParameter(kUnpausePlay,0.f);

	//options
	setParameter(kRecordMode,0.f);
	setParameter(kOneLoop,0.f);
	setParameter(kLoopSetsBeats,0.f);
//	setParameter(kRecordButtonMode,0.f);
	setParameter(kRecordAt,0.f);
	setParameter(kNoteCC,0.f);
	setParameter(kFadeTime,0.f);

	//llamatronics
	setParameter(kFeedBack,0.f);
	setParameter(kEQFreq,0.f);
	setParameter(kTapeHiss,0.f);
//	setParameter(kSpeed,0.f);

	//text edits
	setParameter(kBeats,0.f);
	setParameter(kBPM,0.f);
	setParameter(kRecordButtonCC,0.f);
	setParameter(kResetButtonCC,0.f);
	setParameter(kPlayButtonCC,0.f);
	setParameter(kPausedButtonCC,0.f);
	setParameter(kDirectionCC,0.f);
	setParameter(kFeedbackCC,0.f);
	setParameter(kLpfCC,0.f);
	setParameter(kMetVolCC,0.f);
	setParameter(kMIDIChannel,0.f);

	//minutia
	setParameter(kTaps,0.f);
	setParameter(kTapTempoCC,0.f);
	setParameter(kMetronomeNote,0.f);
	setParameter(kMetronomeVolume,0.f);
	setParameter(kMetronomeInMains,0.f);

}

//-----------------------------------------------------------------------------
long LoopyLlamaEditor::onKeyDown (VstKeyCode &keyCode)  ///why doesn't this work anymore?
{
	switch (keyCode.character) {
	case 't' :
	case 'T' :
	case ' ' :
		effect->setParameter(kTapHappened,1.f);
			setParameter(kBPM,0.f);
		return 1;
	case 'r' :
	case 'R' :
		effect->setParameter(kRecord,1.f);
		return 1;
	case 'e' :
	case 'E' :
		effect->setParameter(kReset,1.f);
		return 1;
	case 'q' :
	case 'Q' :
	case 'p' :
	case 'P' :
		effect->setParameter(kPlay,1.f);
		return 1;
	case 'w' :
	case 'W' :
		effect->setParameter(kPaused,1.f - effect->getParameter(kPaused));
		return 1;
	default:
		return -1;
	}
}


//-----------------------------------------------------------------------------
void LoopyLlamaEditor::resume ()
{
	// called when the plugin will be On
}

//-----------------------------------------------------------------------------
void LoopyLlamaEditor::suspend ()
{
	// called when the plugin will be Off
}

//-----------------------------------------------------------------------------
void LoopyLlamaEditor::close ()
{
	
#if MAC //mac timer removal code
	RemoveEventLoopTimer(lightTimer);
	lightTimer = 0;
#else  //PC Timer removal code
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    DWORD resolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
	timeKillEvent(m_idEvent);
	timeEndPeriod (resolution);  //this might be unnecessary
//	m_idEvent = 0;
#endif

	// don't forget to remove the frame !!
	if (frame)
		delete frame;
	frame = 0;

	// set to zero all pointer (security)
	cRecordButton    = 0;
	cResetButton    = 0;
	cRecordMode = 0;
	cRecordAt = 0;
	cRecordButtonMode = 0;
	cReverseButton   = 0;
	cTapButton   = 0;
	cPauseButton   = 0;
	cPlayButton   = 0;
	cPauseButton   = 0;
	cNoteCCButton   = 0;
	cDirectionButton   = 0;
	cIn   = 0;
	cDry   = 0;
	cLoad   = 0;
	cLoopVol   = 0;
	cFeedBack   = 0;
	cBars         = 0;
	cTempo         = 0;
	cTapeHiss = 0;
	cUnpausePlay         = 0;
	cEQFreq = 0;
	cCredits = 0;
//	cSpeed = 0;
	cRecordLight  = 0;
	cLoopLight  = 0;
	cLoopSetsBeats  = 0;
	cTempoLight  = 0;
	cVuMeter      = 0;
	cTapTempoCC  = 0;
	cMetronomeNote = 0;
	cMetronomeVolume = 0;
	cMetronomeInMains = 0;
	cFadeTime  = 0;
	cFadeTimeDisplay  = 0;
	cTaps      = 0;
	cAddProgram  = 0;
	cOneLoop  = 0;
	cRecordButtonCC  = 0;
	cResetButtonCC  = 0;
	cPlayButtonCC  = 0;
	cPauseButtonCC  = 0;
	cDirectionCC  = 0;
	cFeedbackCC  = 0;
	cLpfCC  = 0;
	cMetVolCC  = 0;
	cMIDIChannel  = 0;
	//cExpireDate  = 0;

	cViewContainer = 0;
	cCurrentBar = 0;
}

//-----------------------------------------------------------------------------
void LoopyLlamaEditor::idle ()
{

    if (!frame || !cRecordLight || !cLoopLight || !cTempo || !cCredits || !cVuMeter)
	return;

	if (!cCredits->getValue()) {
		cVuMeter->setValue(effect->getParameter(kVUlevel));
	//    cVuMeter->redraw();
		cRecordLight->setValue (effect->getParameter(kIsRecording));
		cLoopLight->setValue (effect->getParameter(kIsLoopRunning));
		char * text = new char[64];
		effect->getParameterDisplay(kBPM,text);
		if (atof(text) != monitorBPM) {
		    monitorBPM = atof(text);
		    cTempo->setText(text);
		}
		delete[] text;
	}


	AEffGUIEditor::idle ();		// always call this to ensure update	

}
	
	//-----------------------------------------------------------------------------
void LoopyLlamaEditor::setParameter (long index, float value)
{
	if (!frame || expired)
		return;



	
//	double tempSR;
	char * strInt = new char[64];
	// called from the Aeffect to update the control's value

	// test if the plug is opened
	switch (index)
	{
	case kRecord:
		if (cRecordButton)
			cRecordButton->setValue (value);
		break;

	case kPlay:
		if (cPlayButton)
			cPlayButton->setValue (value);
		break;

	case kPaused:
		if (cPauseButton)
			cPauseButton->setValue (value);
		break;

	case kRecordAt:
		if (cRecordAt)
			cRecordAt->setValue (value);
		break;

	case kRecordButtonCC:
		if (cRecordButtonCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cRecordButtonCC->setText(strInt);
		}
		break;

	case kResetButtonCC:
		if (cResetButtonCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cResetButtonCC->setText(strInt);
		}
		break;

	case kPlayButtonCC:
		if (cPlayButtonCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cPlayButtonCC->setText(strInt);
		}
		break;

	case kPausedButtonCC:
		if (cPauseButtonCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cPauseButtonCC->setText(strInt);
		}
		break;

	case kDirectionCC:
		if (cDirectionCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cDirectionCC->setText(strInt);
		}
		break;

	case kFeedbackCC:
		if (cFeedbackCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cFeedbackCC->setText(strInt);
		}
		break;

	case kLpfCC:
		if (cLpfCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cLpfCC->setText(strInt);
		}
		break;

	case kMetVolCC:
		if (cMetVolCC) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cMetVolCC->setText(strInt);
		}
		break;

	case kMIDIChannel:
		if (cMIDIChannel) {
			sprintf(strInt,"%i",int(effect->getParameter (index) * 15 + 1));
			cMIDIChannel->setText(strInt);
		}
		break;

//	case kAddProgram:
//		if (cAddProgram)
//			cAddProgram->setValue (value);
//		break;

	case kOneLoop:
		if (cOneLoop)
			cOneLoop->setValue (value);
		break;

	case kUnpausePlay:
		if (cUnpausePlay)
			cUnpausePlay->setValue (value);
		break;

	case kDry:
		if (cDry)
			cDry->setValue (effect->getParameter (index));
		break;

	case kMetronomeVolume:
		if (cMetronomeVolume)
			cMetronomeVolume->setValue (effect->getParameter (index));
		break;

	case kFeedBack:
		if (cFeedBack)
			cFeedBack->setValue (effect->getParameter (index));
			effect->getParameterDisplay(index,strInt);
			//sprintf(strInt,"%i%",int(effect->getParameterDisplay (index)));
			strcat(strInt, "%");
			if (effect->getParameter (index) == 1.f) strcpy(strInt,"oo");
			cFeedbackDisplay->setLabel(strInt); 
		break;

	case kFadeTime:
		if (cFadeTime)
			cFadeTime->setValue (effect->getParameter (index));
			sprintf(strInt,"%i",int(effect->getParameter (index) * effect->getSampleRate() / 8.f + 1));
			strcat(strInt, " spl");
			if (effect->getParameter (index) * effect->getSampleRate() / 8.f + 1 > 1)
				strcat(strInt, "'s");
//			sprintf(strInt,"%d",time (NULL));
			cFadeTimeDisplay->setLabel(strInt); 
		break;

	case kLoopVol:
		if (cLoopVol)
			cLoopVol->setValue (effect->getParameter (index));
 		break;

	case kLoopSetsBeats:
		if (cLoopSetsBeats)
			cLoopSetsBeats->setValue (effect->getParameter (index));
 		break;

	case kIn:
		if (cIn)
			cIn->setValue (effect->getParameter (index));
 		break;
 
	case kEQFreq:
		if (cEQFreq)
			cEQFreq->setValue (effect->getParameter (index));
 		break;

	case kTapeHiss:
		if (cTapeHiss)
			cTapeHiss->setValue (effect->getParameter (index));
 		break;

	case kSpeed:
//		if (cSpeed)
//			cSpeed->setValue (effect->getParameter (index));
 		break;

	case kBeats:
		if (cBars)	{
			sprintf(strInt,"%i",int(effect->getParameter (index) * 127 + 1));
			cBars->setText(strInt);
			cCurrentBar->setLabel(strInt); 
		}
 		break;

	case kTaps:
		if (cTaps)	{
			sprintf(strInt,"%i",int(effect->getParameter (index) * 8) + 2);
			cTaps->setText(strInt);
		}
 		break;

	case kTapTempoCC:
		if (cTapTempoCC)	{
			sprintf(strInt,"%i",int(effect->getParameter (kTapTempoCC) * 127 + 1));
			cTapTempoCC->setText(strInt);
		}
 		break;


	case kMetronomeNote:
		if (cMetronomeNote)	{
			sprintf(strInt,"%i",int(effect->getParameter (kMetronomeNote) * 127 + 1));
			cMetronomeNote->setText(strInt);
		}
 		break;

	case kBPM:
		if (cTempo)	{

			
//			tempSR = effect->getSampleRate ();
//			int tempBPM = int((60 * tempSR) / float(long(effect->getParameter (kBPM) * ((long)tempSR * 1.2 - ((long)tempSR / 10)) + ((long)tempSR / 10))));
			//if (int(fRecordMode * 4) < 3) resetLoop(1);  //reset the loop and the click start point
		//	bpm = (60 * sampleRate) / float(delay);
//			if (tempBPM > 240) 
//				tempBPM /= 2;
//			if (tempBPM < 60)
//				tempBPM *= 2;							;

//			sprintf(strInt,"%d",tempBPM);

			effect->getParameterDisplay (kBPM,strInt);
			cTempo->setText(strInt);
		}
 		break;

	case kRecordMode:
		if (cRecordMode)	{
			cRecordMode->setValue (effect->getParameter (index));
//			switch (int(value * 4))	{
//			case 0:
//				frame->addView(cTapButton);
//				break;
//			case 1:
//				frame->removeView(cTapButton);
//				break;
//			}	
		}
 		break;

		if (cRecordButtonMode)	{
			cRecordButtonMode->setValue (effect->getParameter (index));
		}
 		break;

	case kNoteCC:
		if (cNoteCCButton)
			cNoteCCButton->setValue (effect->getParameter (index));
 		break;

	case kReverseMode:
		if (cReverseButton)
			cReverseButton->setValue (effect->getParameter (index));
 		break;

	case kDirection:
		if (cDirectionButton)
			cDirectionButton->setValue (effect->getParameter (index));
 		break;
	case kMetronomeInMains:
		if (cMetronomeInMains)
			cMetronomeInMains->setValue (effect->getParameter (index));
 		break;




	}
	if (strInt)
		delete[] strInt;
	
	// call this to be sure that the graphic will be updated
	postUpdate ();


}


//-----------------------------------------------------------------------------
void LoopyLlamaEditor::valueChanged (CDrawContext* context, CControl* control)
{
	if (!frame || expired)
		return; 
//	double tempSR;
	char * text = new char[64];
	// called when something changes in the UI (mouse, key..)
	switch (control->getTag ())
	{
	case kBeats:
		cBars->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kTapTempoCC:
		cTapTempoCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kRecordButtonCC:
		cRecordButtonCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kResetButtonCC:
		cResetButtonCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kPlayButtonCC:
		cPlayButtonCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kPausedButtonCC:
		cPauseButtonCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kDirectionCC:
		cDirectionCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kFeedbackCC:
		cFeedbackCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kLpfCC:
		cLpfCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kMetVolCC:
		cMetVolCC->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;
		
	case kMIDIChannel:
		cMIDIChannel->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 15 + 1.f/30));
		setParameter(control->getTag (),0.f);
		break;
		
	case kMetronomeNote:
		cMetronomeNote->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 1.f) / 127 + 1.f/254));
		setParameter(control->getTag (),0.f);
		break;


	case kTaps:
		cTaps->getText (text);
		effect->setParameterAutomated(control->getTag (),float((atof(text) - 2.f) / 8 + 1.f/16));
		setParameter(control->getTag (),0.f);
		break;
		
		
	case kBPM:
		cTempo->getText (text);
		//tempSR = effect->getSampleRate ();
		effect->setParameterAutomated(kBPM,float((atof(text) - 50.f) / 250));
		setParameter(control->getTag (),0.f);
		break;
		

	case kRecordMode:
		effect->setParameterAutomated (control->getTag (), control->getValue ());
		switch (int(control->getValue () * 4))	{
			case 0:
				if (!frame->isChild(cTapButton))
					frame->addView(cTapButton);
				if (!frame->isChild(cPauseButton))
					frame->addView(cPauseButton);
				break;
			case 1:
			case 2:
				if (frame->isChild(cTapButton))
					frame->removeView(cTapButton);
				if (!frame->isChild(cPauseButton))
					frame->addView(cPauseButton);
				break;
			default:
				if (frame->isChild(cPauseButton))
					frame->removeView(cPauseButton);
				if (frame->isChild(cTapButton))
					frame->removeView(cTapButton);
				break;
			} 
		frame->setDirty();
		break;
		control->update(context); 
		
//	case kRecord:
//		if (effect->getParameter(kRecordButtonMode) < .5f) { //if its in momentary mode
//			if (control->getValue () > 0) {  //kick it on and off
//				effect->setParameterAutomated (control->getTag (), 1.f);
//				effect->setParameterAutomated (control->getTag (), 0.f);
//				setParameter(kRecord,0.f);
//			}
//		} else {
//			effect->setParameterAutomated (control->getTag (), control->getValue ());
//		}
//		break;

	case kTapHappened:  //tap tempo button
		if (control->getValue () > .5f)
			effect->setParameterAutomated (kTapHappened, 0.5f);
			//setParameter(kBPM,0.f);
		if (cTempo) {
		    effect->getParameterDisplay(kBPM,text);
			cTempo->setText(text);
			}
		break;

	case kFadeTime:
		effect->setParameterAutomated (control->getTag (), control->getValue ());
		control->update (context);	
			//setParameter(kBPM,0.f);
		if (cFadeTimeDisplay) {
				char * tempText2 = new char[64];
				effect->getParameterDisplay(control->getTag (),text);
				effect->getParameterLabel(control->getTag (),tempText2);
				strcat(text, tempText2);		
				cFadeTimeDisplay->setLabel(text);
				delete[] tempText2;
			}
		break;
	case kFeedBack:
		effect->setParameterAutomated (control->getTag (), control->getValue ());
		control->update (context);	
			//setParameter(kBPM,0.f);
		if (cFeedbackDisplay) {
				//char * tempText2 = new char[64];
				effect->getParameterDisplay(control->getTag (),text);
				//effect->getParameterLabel(control->getTag (),tempText2);
				strcat(text, "%");
				if (control->getValue () == 1.f) strcpy(text,"oo");
				cFeedbackDisplay->setLabel(text);
			}
		break;
//	case kNoteCC:
//		effect->setParameterAutomated (control->getTag (), control->getValue ());
//		control->update (context);	
//		if (cTempo) {
//		    effect->getParameterDisplay(kBPM,text);
//		    cTempo->setText(text);
//		}
//	break;

	case kAbout:
	/*	if (frame->isChild(cExpireDate)){
//			if (cExpireDate) 
				frame->removeView (cExpireDate);
				frame->setDirty();
		} else {
//			if (cExpireDate) 
				frame->addView (cExpireDate);
//				frame->setDirty();
		}
*/
//	break;

	default:
		effect->setParameterAutomated (control->getTag (), control->getValue ());
		control->update (context);	
	}
	if (text)
		delete[] text;
}
