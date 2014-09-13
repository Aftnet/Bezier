#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <windows.h>		// Header File For Windows - has structures for BMP format
#include <stdio.h>	    	// Header File For Standard Input/Output
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "Glext.h"

bool NeHeLoadBitmap(LPTSTR szFileName, GLuint &texid, bool wrap);

#endif 