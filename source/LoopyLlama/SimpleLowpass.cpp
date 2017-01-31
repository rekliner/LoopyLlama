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

#include "SimpleLowpass.h"
#define pi 3.141592f

#ifndef __SimpleLowPass__
#define __SimpleLowPass__
#endif

SimpleLowpass::SimpleLowpass()
{
 //samplerate=44100.0;
 c0=0.0;
 t0=0.0;
 init();
}

SimpleLowpass::~SimpleLowpass()
{
}

void SimpleLowpass::init()
{
 // initialize values
 yold1=0.0;
 y=0;
 calc();
};

void SimpleLowpass::calc()
{
 // calculate temporary variables
 t0=0.98f*c0*c0+0.02f;
 // calculate coefficients
 a0=t0;
 b1=t0-1;
};

float SimpleLowpass::process(float x)
{
 // process input
 yold1=y;
 y= a0*x  -b1*yold1;
 return y;
}

float SimpleLowpass::getC0()
{ return c0; }

void SimpleLowpass::setC0(float c)
{ c0=c;calc(); }

