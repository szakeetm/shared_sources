
#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

#include "GLApp.h"
#include "Helper/MathTools.h" //Min, max
#include "Helper/ConsoleLogger.h" //Min, max
#include "GLToolkit.h"
#include "GLWindowManager.h"
#include "GLComponent.h"
#include "GLWindow.h"
#include <math.h>
#include <stdlib.h>
#include <cstring> //strcpy, etc.
#include <imgui_impl_sdl2.h>
#include <imgui_internal.h>
#include "ImguiWindow.h"

#ifndef _WIN32
#include <unistd.h> //_exit()
#endif

GLApplication *theApp=NULL;

/*#ifdef _WIN32
LARGE_INTEGER perfTickStart; // Fisrt tick
double perfTicksPerSec;      // Performance counter (number of tick per second)
#endif*/

GLApplication::GLApplication() : m_strFrameStats{"\0"}, m_strEventStats{"\0"}, m_strModifierStates{"\0"}, errMsg{"\0"} {

  m_bWindowed = true;
  m_strWindowTitle = "GL application";
  strcpy(m_strFrameStats,"");
  strcpy(m_strEventStats,"");
  strcpy(m_strModifierStates,"");
  m_screenWidth = 640;
  m_screenHeight = 480;
  m_minScreenWidth = 640;
  m_minScreenHeight = 480;
  m_bResizable = false;
  masterWindowPtr = new GLWindow();
  masterWindowPtr->SetMaster(true);
  masterWindowPtr->SetBorder(false);
  masterWindowPtr->SetBackgroundColor(0,0,0);
  masterWindowPtr->SetBounds(0,0,m_screenWidth,m_screenHeight);
  masterWindowPtr->SetVisible(true); // Make top level shell


#if defined(DEBUG)
  nbRestore = 0;
  fPaintTime = 0.0;
  fMoveTime = 0.0;
#endif

#ifdef _WIN32
  m_fullScreenWidth = GetSystemMetrics(SM_CXSCREEN);
  m_fullScreenHeight = GetSystemMetrics(SM_CYSCREEN);

/*  LARGE_INTEGER qwTicksPerSec;
  QueryPerformanceFrequency( &qwTicksPerSec );
  QueryPerformanceCounter( &perfTickStart );
  perfTicksPerSec = (double)qwTicksPerSec.QuadPart;*/

#else
  // TODO
  m_fullScreenWidth=1280;
  m_fullScreenHeight=1024;
#endif

    nbMouse=nbWheel=nbMouseMotion=nbJoystic=nbKey=nbSystem=nbActive=nbResize=nbExpose=nbOther=0;
    m_fFPS = 0.0f;
    m_bitsPerPixel = 0;

    m_fTime = 0.0f;
    fMoveTime=0.0f;
    fPaintTime=0.0f;
    mainScreen=nullptr;
    mainContext={};
    nbFrame=0;
    nbEvent=0;
    firstTick=0;
    quit=false;
    wereEvents = false;
    nbPoly=0;
    nbLine=0;
    nbRestore=0;
}

GLApplication::~GLApplication(){
    //SAFE_DELETE(wnd); //Program terminates, no need to spend time with manual release of resources
}

double GLApplication::GetTick() {

	return (double)SDL_GetTicks() / 1000.0;

}

int GLApplication::setUpSDL(bool doFirstInit) {
	int errCode;
	if (doFirstInit) {

		Uint32 flags;
		flags = SDL_WINDOW_OPENGL;
		flags |= (m_bWindowed ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
		flags |= (m_bResizable ? SDL_WINDOW_RESIZABLE : 0);

		mainScreen = SDL_CreateWindow("My Game Window",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			m_screenWidth, m_screenHeight,
			flags);

		if (mainScreen == NULL)
		{
			GLToolkit::Log("GLApplication::SDL_CreateWindow() failed.");
			return GL_FAIL;
		}
		mainContext = SDL_GL_CreateContext(mainScreen);
		if (mainContext == NULL) {
			GLToolkit::Log("GLApplication::SDL_GL_CreateContext() failed.");
			return GL_FAIL;
		}

		m_bitsPerPixel = SDL_BITSPERPIXEL(SDL_GetWindowPixelFormat(mainScreen));

        SDL_EnableScreenSaver();
	}

	errCode = GLToolkit::RestoreDeviceObjects(m_screenWidth, m_screenHeight);
	if (!errCode) {
		GLToolkit::Log("GLApplication::setUpSDL GLToolkit::RestoreDeviceObjects() failed.");
		return GL_FAIL;
	}
	if (doFirstInit) OneTimeSceneInit();

  GLWindowManager::RestoreDeviceObjects();
#if defined(DEBUG)
  nbRestore++;
#endif
  masterWindowPtr->SetBounds(0,0,m_screenWidth,m_screenHeight);
  errCode = RestoreDeviceObjects();
  if( !errCode ) {
    GLToolkit::Log("GLApplication::setUpSDL GLApplication::RestoreDeviceObjects() failed.");
    return GL_FAIL;
  }

  return GL_OK;

}

int GLApplication::ToggleFullscreen() {

  GLToolkit::InvalidateDeviceObjects();
  GLWindowManager::InvalidateDeviceObjects();
  InvalidateDeviceObjects();

  m_bWindowed = !m_bWindowed;

  Uint32 flags = SDL_GetWindowFlags(mainScreen);
  if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
  {
      //Restore windowed size
      m_screenWidth = m_windowedScreenWidth;
      m_screenHeight = m_windowedScreenHeight;
      //Set to windowed mode
      SDL_SetWindowFullscreen(mainScreen, 0); 
      Resize(m_screenWidth, m_screenHeight);
  }
  else
  {
      //Remember windowed size
      m_windowedScreenWidth = m_screenWidth;
      m_windowedScreenHeight = m_screenHeight;
      //Set to full screen size
      m_screenWidth = m_fullScreenWidth;
      m_screenHeight = m_fullScreenHeight;
      //Set to fullscreen mode
      SDL_SetWindowFullscreen(mainScreen, SDL_WINDOW_FULLSCREEN_DESKTOP);
      Resize(m_screenWidth, m_screenHeight);
  }

  return setUpSDL();
}

void GLApplication::SetTitle(std::string title) {

  m_strWindowTitle = title;
  SDL_SetWindowTitle(mainScreen, title.c_str());

}

int GLApplication::Create(int width, int height, bool bFullScreen ) {

	theApp=this;
  m_screenWidth = width;
  m_screenHeight = height;
  m_bWindowed = !bFullScreen;

  //Initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  { 
      std::cout << (fmt::format("GLApplication::Create SDL_Init() failed:\n{}" , SDL_GetError()));
      return GL_FAIL;
  }

  //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
  //SDL_EnableUNICODE( 1 );
  //SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
    
  //SDL_WM_SetCaption( m_strWindowTitle.c_str(), NULL ); //To replace

  return setUpSDL(true);

}

void GLApplication::Pause(bool bPause) {
}

int GLApplication::Resize( size_t nWidth, size_t nHeight, bool forceWindowed ) {

  int width  = std::max((int)nWidth,m_minScreenWidth);
  int height = std::max((int)nHeight,m_minScreenHeight);

  m_screenWidth = width;
  m_screenHeight = height;

  SDL_SetWindowSize(mainScreen, width,height); //Enlarge too small window if needed
  
  GLToolkit::InvalidateDeviceObjects();
  GLWindowManager::InvalidateDeviceObjects();
  InvalidateDeviceObjects();
  
  if( forceWindowed ) m_bWindowed = true;

  if( setUpSDL() == GL_OK ) {
    GLWindowManager::Resize();
    return GL_OK;
  } else {
    return GL_FAIL;
  }
  return GL_OK;
}

void GLApplication::Add(GLComponent *comp) {
  masterWindowPtr->Add(comp);
}

void GLApplication::Exit() {
    BeforeExit(); //While GUI still exists
  char *logs = GLToolkit::GetLogs();
#ifdef _WIN32
  if(logs) {
    strcat(logs,"\nDo you want to exit ?");
    if( MessageBox(NULL,logs,"[Unexpected error]",MB_YESNO)==IDNO ) {
      GLToolkit::ClearLogs();
      return;
    }
  }
#else
  if(logs) {
    Log::console_error("[Unexpected error]\n");
    Log::console_error("{}",logs);
  }
#endif
  SAFE_FREE(logs);

  if(imWnd) {
      imWnd->destruct();
      delete imWnd;
      imWnd = nullptr;
  }
  
  GLToolkit::InvalidateDeviceObjects();
  masterWindowPtr->InvalidateDeviceObjects();
  InvalidateDeviceObjects();
  
  AfterExit(); //After GUI destroyed

  SDL_GL_DeleteContext(mainContext);
  SDL_DestroyWindow(mainScreen);
  SDL_Quit();
  this->quit = true;
}

void GLApplication::UpdateStats() {

  time_type fTick = clock_type::now();
  float eps;

  // Update timing
  nbFrame++;
  //float fTime = (float)(fTick - lastTick) * 0.001f;
  std::chrono::duration<double> duration = std::chrono::duration_cast<time_ratio>(fTick - lastTick);

  if( duration.count() >= 1.0 ) { // more than 1.0 sec
     //int t0 = fTick;
     //int t = t0 - lastTick;
     m_fFPS = (float)(nbFrame) / (float)duration.count();
     eps = (float)(nbEvent) / (float)duration.count();
     nbFrame = 0;
     nbEvent = 0;
     lastTick = fTick;
     sprintf(m_strFrameStats,"%.2f fps (%dx%dx%d)   ",m_fFPS,m_screenWidth,m_screenHeight,m_bitsPerPixel);
     sprintf(m_strEventStats,"%.2f eps C:%d W:%d M:%d J:%d K:%d S:%d A:%d R:%d E:%d O:%d   ",
             eps,nbMouse,nbWheel,nbMouseMotion,nbJoystic,nbKey,nbSystem,nbActive,nbResize,nbExpose,nbOther);

#if defined(__MACOSX__) || defined(__APPLE__)
	static char ctrlText[] = "CMD";
#else
  static char ctrlText[] = "CTRL";
#endif

     sprintf(m_strModifierStates,"SHIFT:%d %s:%d ALT:%d CAPS:%d TAB:%d SPACE:%d D:%d Z:%d",
             masterWindowPtr->IsShiftDown(),ctrlText,masterWindowPtr->IsCtrlDown(),masterWindowPtr->IsAltDown(),masterWindowPtr->IsCapsLockOn(),
             masterWindowPtr->IsTabDown(),masterWindowPtr->IsSpaceDown(),masterWindowPtr->IsDkeyDown(),masterWindowPtr->IsZkeyDown());        
  }

  m_fTime = (float) m_Timer.Elapsed();
  //m_fElapsedTime = (fTick - lastFrTick) * 0.001f;
  //lastFrTick = fTick;

#if defined(DEBUG)
  nbPoly=0;
  nbLine=0;
#endif

}

void GLApplication::UpdateEventCount(SDL_Event *evt) {

  switch(evt->type) {		

      case SDL_KEYDOWN:
      case SDL_KEYUP:
        nbKey++;
        break;

      case SDL_MOUSEMOTION:
        nbMouseMotion++;
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
          nbMouse++;
        break;

	  case SDL_MOUSEWHEEL:
		nbWheel++;
		break;

      case SDL_JOYAXISMOTION:
      case SDL_JOYBALLMOTION:
      case SDL_JOYHATMOTION:
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        nbJoystic++;
        break;

      case SDL_QUIT:
      case SDL_SYSWMEVENT:
        nbSystem++;
        break;

	  case SDL_WINDOWEVENT:
		  switch (evt->window.event) {
		  case SDL_WINDOWEVENT_RESIZED:
			  nbResize++;
			  break;

		  case SDL_WINDOWEVENT_EXPOSED:
			  nbExpose++;
			  break;
		  case SDL_WINDOWEVENT_ENTER:
		  case SDL_WINDOWEVENT_FOCUS_GAINED:
			  nbActive++;
			  break;
		  }
		  break;

      default:
        nbOther++;
        break;

  }

  if(evt->type) nbEvent++;

}

void GLApplication::Run() {
	#if defined(MOLFLOW)
	extern MolFlow *mApp;
	#endif

	#if defined(SYNRAD)
	extern SynRad*mApp;
	#endif
  SDL_Event sdlEvent;

  quit = false;
  int    ok;
  GLenum glError;
//#if defined(DEBUG)
  double t1,t0;
//#endif

  // Stats
  m_Timer.ReInit();
  m_Timer.Start();
  m_fTime        = 0.0f;
  //m_fElapsedTime = 0.0f;
  m_fFPS         = 0.0f;
  fMoveTime =			
	  fPaintTime = 0.0;
  nbFrame        = 0;
  nbEvent        = 0;
  nbMouse        = 0;
  nbMouseMotion  = 0;
  nbKey          = 0;
  nbSystem       = 0;
  nbActive       = 0;
  nbResize       = 0;
  nbJoystic      = 0;
  nbOther        = 0;
  nbExpose       = 0;
  nbWheel        = 0;

  //lastTick = lastFrTick = firstTick = SDL_GetTicks();
  lastTick  = clock_type::now();

  mApp->CheckForRecovery();
  wereEvents = false;
  wereEvents_imgui = 2;

  // TODO: Activate imgui directly on launch here
#ifdef ENABLE_IMGUI_TESTS
  if(mApp->argv.size()>=2 && mApp->argv[1]=="--ImTest" && !imWnd) {
      std::cout<<"Launching ImGui test sequence...\n";
      imWnd = new ImguiWindow(this);
      imWnd->init();
      imWnd->show_app_main_menu_bar = true;
      imWnd->testEngine.RunTests();
  }
#endif
  //Wait for user exit
  while( !quit )
  {
        
     //While there are events to handle
      while (!quit && (SDL_PollEvent(&sdlEvent) || (imWnd && imWnd->forceDrawNextFrame)))
     {
         bool forceSkipEvents = false;
         bool activeImGuiEvent = false;
         if(imWnd) {
             if (imWnd->forceDrawNextFrame) {
                 imWnd->forceDrawNextFrame = false;
             }
            auto ctx = ImGui::GetCurrentContext();
            activeImGuiEvent = (ImGui::GetIO().WantCaptureKeyboard || ctx->WantCaptureKeyboardNextFrame != -1)
                                    || (ImGui::GetIO().WantCaptureMouse || ctx->WantCaptureMouseNextFrame != -1)
                                    || (ImGui::GetIO().WantTextInput || ctx->WantTextInputNextFrame != -1)
                                    || ImGui::IsAnyItemHovered();
            if (!activeImGuiEvent) {
                // workaround for some mouse events getting triggered on old implementation first, results e.g. in selection-rectangle when clicking on ImGui window
                // ImGui_ImplSDL2_NewFrame updates the mouse position, but is only called on ImGui render cycle
                // Check for mouse events in imgui windows manually
                // FIXME: Can be removed when GUI has been fully moved to ImGui
                if(sdlEvent.type == SDL_MOUSEBUTTONDOWN){
                    for(auto win : ctx->Windows){
                        if(win->Active) {
                            // Mouse position
                            //auto mouse_pos = ImGui::GetIO().MousePos;
                            auto& mouse_pos = sdlEvent.button;
                            if (win->OuterRectClipped.Min.x < mouse_pos.x &&
                                win->OuterRectClipped.Max.x > mouse_pos.x
                                &&
                                win->OuterRectClipped.Min.y < mouse_pos.y &&
                                win->OuterRectClipped.Max.y > mouse_pos.y) {
                                forceSkipEvents = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (activeImGuiEvent) {
                wereEvents_imgui = 3;
                if(ImGui_ImplSDL2_ProcessEvent(&sdlEvent)){
                    //Handle input events caught by ImGui
                }
                continue;
            }
         }
		//if (sdlEvent.type!=SDL_MOUSEMOTION || sdlEvent.motion.state!=0) {
            wereEvents = true;
            wereEvents_imgui = 2;
        //}
       if (forceSkipEvents) wereEvents = false;

       UpdateEventCount(&sdlEvent);
       switch( sdlEvent.type ) {

         case SDL_QUIT:
           if (mApp->AskToSave()) quit = true;
           break;

           case (SDL_DROPFILE): {      // In case if dropped file
               char* dropped_filedir;                  // Pointer for directory of dropped file
               dropped_filedir = sdlEvent.drop.file;
               // Shows directory of dropped file
               mApp->DropEvent(dropped_filedir);
               SDL_free(dropped_filedir);    // Free dropped_filedir memory

               break;
           }

         case SDL_WINDOWEVENT:
           if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED) Resize(sdlEvent.window.data1,sdlEvent.window.data2);
           break;

         case SDL_SYSWMEVENT:
           break;

         default:

           if(!forceSkipEvents && GLWindowManager::ManageEvent(&sdlEvent)) {
             // Relay to GLApp EventProc
             EventProc(&sdlEvent);
           }

       }
     }

     if( quit ) {
		 Exit();
       return;
     }

     glError = glGetError();
     if( glError!=GL_NO_ERROR ) {
       GLToolkit::Log("GLApplication::ManageEvent() failed.");
       GLToolkit::printGlError(glError); 
       Exit();
     }

     UpdateStats();

	 Uint32 flags = SDL_GetWindowFlags(mainScreen);
     if (flags && (SDL_WINDOW_SHOWN & flags)) { //Application visible


       t0 = GetTick();

       // Call FrameMove
       ok = FrameMove();

       t1 = GetTick();
       fMoveTime = 0.9*fMoveTime + 0.1*(t1 - t0); //Moving average over 10 FrameMoves

       glError = glGetError();
       if( !ok || glError!=GL_NO_ERROR ) {
         GLToolkit::Log("GLApplication::FrameMove() failed.");
         GLToolkit::printGlError(glError); 
         Exit();
       }

       // Repaint
       if (wereEvents || wereEvents_imgui > 0) {
           wereEvents_imgui -= 1; // allow to queue multiple imgui passes
		   GLWindowManager::Repaint();
		   wereEvents = false;
	   }

	   GLToolkit::CheckGLErrors("GLApplication::Paint()");
     
     } else {
       SDL_Delay(100);
     }
      
  }
  
  //Clean up and exit
  Exit();
  
}

void GLApplication::RequestExit()
{
    quit = true; //will be executed in next Run loop
}

bool GLApplication::Get_m_bWindowed() {
    return m_bWindowed;
}