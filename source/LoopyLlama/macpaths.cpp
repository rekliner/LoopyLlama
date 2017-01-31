// macpaths.cpp - converts between full paths and Mac FSSpec records
//
// This code is free software. You can redistribute it and/or modify
// it without limitation. Paul Kellett (@mda-vst.com) September 2002
//
// This code will probably give real Mac programmers nightmares, but
// it works, and sometimes you want to stick with the standard C/C++
// file I/O, and to do that you need paths not the FSSpec structures
// that are returned by VSTGUI functions like openFileSelector() and
// getDirectory().  I'm publishing this code because I couldn't find
// any decent documentation on this stuff, and all the code examples
// I found depended on a load of other files. Hopfully this "quick &
// dirty" version will save someone some time.

#if MAC
#include <Files.h> //Mac-specific stuff
#include <stdio.h>
#include <string.h>


//path2fss makes an FSSpec from a path with or without a filename
int path2fss(FSSpec *fss, char *path)
{
  char buf[256];
  char *p = &buf[1];
  strcpy(p, path); //convert to Str255 
  buf[0] = strlen(p);

  return(FSMakeFSSpec(0, 0, (unsigned char *)buf, fss)); //== noErr
}


//fss2path takes the FSSpec of a file, folder or volume and returns it's path 
void fss2path(char *path, FSSpec *fss)
{
  int l;             //fss->name contains name of last item in path
  for(l=0; l<(fss->name[0]); l++) path[l] = fss->name[l + 1]; 
  path[l] = 0;

  if(fss->parID != fsRtParID) //path is more than just a volume name
  { 
    int i, len;
    CInfoPBRec pb;
    
    pb.dirInfo.ioNamePtr = fss->name;
    pb.dirInfo.ioVRefNum = fss->vRefNum;
    pb.dirInfo.ioDrParID = fss->parID;
    do
    {
      pb.dirInfo.ioFDirIndex = -1;  //get parent directory name
      pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrParID;   
      if(PBGetCatInfoSync(&pb) != noErr) break;

      len = fss->name[0] + 1;
      for(i=l; i>=0;  i--) path[i + len] = path[i];
      for(i=1; i<len; i++) path[i - 1] = fss->name[i]; //add to start of path
      path[i - 1] = ':';
      l += len;
} while(pb.dirInfo.ioDrDirID != fsRtDirID); //while more directory levels
  }
}
#endif
