//-------------------------------------------------------------------------------------------------------
//Loopy Llama by Chris Kline
//-------------------------------------------------------------------------------------------------------


#include "LoopyLlama.hpp"


#ifndef __LoopyLlamaEdit__
#include "LoopyLlamaEditor.h"
#endif

bool oome = false;

#if MAC
#pragma export on
#endif

//------------------------------------------------------------------------
// Prototype of the export function main
//------------------------------------------------------------------------
#if BEOS
#define main main_plugin
extern "C" __declspec(dllexport) AEffect *main_plugin (audioMasterCallback audioMaster);

#elif MACX
#define main main_macho
extern "C" AEffect *main_macho (audioMasterCallback audioMaster);

#else
AEffect *pluginmain (audioMasterCallback audioMaster);
#endif

//------------------------------------------------------------------------
AEffect *pluginmain (audioMasterCallback audioMaster)
{
	// Get VST Version
	if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version

	// Create the AudioEffect
	LoopyLlama* effect = new LoopyLlama (audioMaster);
	if (!effect)
		return 0;

	// Check if no problem in constructor of LoopyLlama
	if (oome)
	{
		delete effect;
		return 0;
	}
	return effect->getAeffect ();
}

#if MAC
#pragma export off
#endif

//------------------------------------------------------------------------
#if WIN32
#include <windows.h>
void* hInstance;
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;

	char tmp[MAX_PATH];

/*
if
(GetModuleFileName((HMODULE)hInstance,tmp,MAX_PATH-1))
{ 
// find the last path delimiter
char *lastDelimiter=strrchr(tmp,'\\');
// and truncate the DLL name
if (lastDelimiter) {
lastDelimiter[1]=0x0;
}
//myPath = tmp;
}
else
{
// do something
}

*/


	return 1;
}
#endif
