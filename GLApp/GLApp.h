/*
  File:        GLApp.h
  Description: SDL/OpenGL OpenGL application framework
  Author:      J-L PONS (2007)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

extern float      m_fTime;             // Current time in seconds
extern long long	  nbDesStart;
extern long long	  nbHitStart;

#include <SDL.h>
#include <SDL_opengl.h>
#include "GLTypes.h"
#include "GLWindow.h"
#include "GLComponent.h"
#include "GLFont.h"
#include <string>

#ifndef _GLAPPH_
#define _GLAPPH_

class GLApplication {

protected:

    // Internal variables for the state of the app
    BOOL      m_bWindowed;
    std::string     m_strWindowTitle;
    int       m_minScreenWidth;
    int       m_minScreenHeight;
    int       m_screenWidth;
    int       m_screenHeight;
    int       m_fscreenWidth;
    int       m_fscreenHeight;
    BOOL      m_bResizable;
    GLWindow  *wnd;

    // Top level window methods
    int ToggleFullscreen();
    void SetTitle(std::string title);

    // Overridable variables for the app

    virtual int OneTimeSceneInit()                         { return GL_OK; }
    virtual int RestoreDeviceObjects()                     { return GL_OK; }
	virtual int FrameMove() { return GL_OK; }
    virtual int InvalidateDeviceObjects()                  { return GL_OK; }
    virtual int OnExit()                                   { return GL_OK; }
    

public:
	virtual int EventProc(SDL_Event *event)                { return GL_OK; }
    // Functions to create, run, pause, and clean up the application
    virtual int  Create(int width, int height, BOOL bFullScreen);
    virtual void Pause(BOOL bPause);
    virtual int  Resize(DWORD width, DWORD height, BOOL forceWindowed=FALSE);
    void  Run();
    void  Exit();

    // Statistics management (expert usage)
    void UpdateStats();
    void UpdateEventCount(SDL_Event *evt);

    // Internal constructor
    GLApplication();

    // Components management
    void Add(GLComponent *comp);
    virtual void ProcessMessage(GLComponent *src,int message) {};

    // Variables for timing
    char              m_strFrameStats[64]; 
    char              m_strEventStats[128]; 
  
    //float             m_fElapsedTime;      // Time elapsed since last frame
    float             m_fFPS;              // Instanteous frame rate
    double            GetTick();           // Number of second since app startup (WIN32 only)

#ifdef _DEBUG
    // Debugging stuff
    int  nbPoly;
    int  nbLine;
    int  nbRestore;
    double fMoveTime;
    double fPaintTime;
#endif

private:

   int setUpSDL(BOOL doFirstInit=FALSE);

   int m_bitsPerPixel;
   char errMsg[512];
   int  lastTick;
   //int  lastFrTick;
   int  nbFrame;
   int  nbEvent;
   int  nbMouse;
   int  nbWheel;
   int  nbMouseMotion;
   int  nbJoystic;
   int  nbKey;
   int  nbSystem;
   int  nbActive;
   int  nbResize;
   int  nbOther;
   int  nbExpose;
   int  firstTick;

};

#endif /* _GLAPPH_ */
