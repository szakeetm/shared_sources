// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLAPPH_
#define _GLAPPH_

//extern long long	  nbDesStart;
//extern long long	  nbHitStart;

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "GLTypes.h"  //GL_OK
#include "Helper/Chronometer.h"
#include <omp.h>
class GLComponent;
class GLWindow;
#include <string>
#include <fmt/core.h>
#include <mutex>

class ImguiWindow;
class GLApplication {

protected:

    // Internal variables for the state of the app
    bool      m_bWindowed;
    std::string     m_strWindowTitle;
    int       m_minScreenWidth;
    int       m_minScreenHeight;
    int       m_screenWidth;
    int       m_screenHeight;
    int       m_windowedScreenWidth;
    int       m_windowedScreenHeight;
    int       m_fullScreenWidth;
    int       m_fullScreenHeight;
    bool      m_bResizable;
    GLWindow  *wnd;


    // Overridable variables for the app

    virtual int OneTimeSceneInit()                         { return GL_OK; }
    virtual int RestoreDeviceObjects()                     { return GL_OK; }
	virtual int FrameMove() { return GL_OK; }
    virtual int InvalidateDeviceObjects()                  { return GL_OK; }
    virtual int AfterExit()                                { return GL_OK; } //GUI already destroyed
    virtual void BeforeExit() {} //GUI still present

public:
    // Top level window methods
    int ToggleFullscreen();
    void SetTitle(std::string title);
	virtual int EventProc(SDL_Event *event)                { return GL_OK; }
    // Functions to create, run, pause, and clean up the application
    virtual int  Create(int width, int height, bool bFullScreen);
    virtual void Pause(bool bPause);
    virtual int  Resize(size_t width, size_t height, bool forceWindowed=false);
    void  Run();
    void  RequestExit();
    void  Exit();

    // Statistics management (expert usage)
    void UpdateStats();
    void UpdateEventCount(SDL_Event *evt);

    // Internal constructor
    GLApplication();
    virtual ~GLApplication();

    // Components management
    void Add(GLComponent *comp);
    virtual void ProcessMessage(GLComponent *src,int message) {};

    // Variables for timing
    char              m_strFrameStats[64]; 
    char              m_strEventStats[128]; 
  
    //float             m_fElapsedTime;      // Time elapsed since last frame
    float             m_fFPS;              // Instanteous frame rate
	float			  m_fTime;             // Number of second since app startup (WIN32 only)
	Chronometer       m_Timer;
    double            GetTick();           // Number of millisecond since app startup (WIN32 only)

    bool wereEvents; //Repaint in next cycle (instead of immediately by calling GLWindowManager::Repaint() )
    int wereEvents_imgui{ 2 }; // tracks the number of passes for ImGui rendering (deafult allows queue of 2, some events increase to 3)
    size_t imguiRenderLock = 0;

    //getters

    bool Get_m_bWindowed();

    // Debugging stuff
    int  nbPoly;
    int  nbLine;
    int  nbRestore;
    double fMoveTime;
    double fPaintTime;

	SDL_Window *mainScreen;
	SDL_GLContext mainContext;
    ImguiWindow  *imWnd;

private:

   int setUpSDL(bool doFirstInit=false);

   int m_bitsPerPixel;
   char errMsg[512];
   time_type  lastTick;
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

   bool quit;
};

#endif /* _GLAPPH_ */

