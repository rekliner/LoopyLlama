// This filter source code was created by FilterExplorer version 1.7
// (C)opyright 2002-2004 by Tobias Fleischer alias Tobybear
// This is a computer-generated file and no guarantee is given that it
// works as expected! - Use at your own risk!
// Author contact:
// web: www.tobybear.de   mail: tobybear@web.de

// simple lowpass
// c0: cutoff frequency (0..1), 1 is Nyquist
// c1: resonance
// 
// created by Tobybear alias Tobias Fleischer
// www.tobybear.de
// tobybear@web.de

// Filter type: FIR, 0th order

class SimpleLowpass
{
public:
 SimpleLowpass();
 void init();
 void calc();
 float process(float x);
 ~SimpleLowpass();
 float getC0();
 void setC0(float c);
protected:
 float a0;
 float b1;
 float t0;
 float c0;
 float yold1;
 float y;
 float samplerate;
};
