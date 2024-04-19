
#pragma once
#include <stdexcept>
#include <cstring> // strncpy for Error
#include <algorithm> // fill
#include <string>
#include <fmt/core.h>

// Messages
#define MSG_NULL     0    // No message
#define MSG_CLOSE    1    // Close window
#define MSG_ICONIFY  2    // Iconify window
#define MSG_MAXIMISE 3    // Maximise window
#define MSG_BUTTON   4    // Button pressed
#define MSG_TOGGLE   5    // Toggle state change
#define MSG_TEXT     6    // Return pressed 
#define MSG_TEXT_UPD 7    // Text change
#define MSG_SCROLL   8    // Scroll change
#define MSG_LIST     9    // List select
#define MSG_LIST_DBL 10   // List DBL click
#define MSG_LIST_COL 11   // List column move
#define MSG_COMBO    12   // Combo select
#define MSG_MENU     13   // Menu select
#define MSG_TAB      14   // Tab window change
#define MSG_SPINNER  15   // Spinner change
#define MSG_PANELR   16   // Panel open/close

#define MSG_USER     256

// SDL extensions

/*
#define SDLK_CTRLC 3
#define SDLK_CTRLX 24
#define SDLK_CTRLV 22
*/

#define SDL_MOUSEBUTTONDBLCLICK SDL_USEREVENT + 0

// Macros

#define DELETE_TEX(t) if(t) { glDeleteTextures(1,&t);t=0; }
template<typename T>
constexpr void SAFE_DELETE(T& x) {if(x) {delete x; x=nullptr;}}
template<typename T>
constexpr void SAFE_FREE(T& x) {if(x) {free(x); x=nullptr;}}
#define SAFE_CLEAR(vect) if(vect) {vect.clear();}
template<typename T>
constexpr void IVALIDATE_DLG(T& dlg){ if(dlg && !dlg->IsVisible()) dlg->InvalidateDeviceObjects();}
template<typename T>
constexpr void RVALIDATE_DLG(T& dlg){ if(dlg && !dlg->IsVisible()) dlg->RestoreDeviceObjects();}
template<typename T>
constexpr void ZEROVECTOR(T& _vector){ std::fill(_vector.begin(),_vector.end(),0);}
#define WRITEBUFFER(_value,_type) *((_type *)buffer)=_value;buffer += sizeof(_type)
#define READBUFFER(_type) *(_type*)buffer;buffer+=sizeof(_type)

// Constants

#define GL_OK   1
#define GL_FAIL 0

// Type definitions
typedef unsigned char BYTE;

struct Error : public std::runtime_error {
    explicit Error( const std::string& what_ ) : std::runtime_error(what_) {}
    template<typename... P>
    explicit Error(const std::string& fmtstring, const P&... fmtargs) : std::runtime_error(fmt::format(fmtstring, fmtargs...)) {}
    explicit Error( const char * what_ ) : std::runtime_error(what_) {}
};

struct GLCOLOR {

  float r;
  float g;
  float b;
  float a;

} ;

struct GLMATERIAL {

  GLCOLOR   Diffuse;
  GLCOLOR   Ambient;
  GLCOLOR   Specular;
  GLCOLOR   Emissive;
  float     Shininess;

} ;

struct GLVIEWPORT {

  int x;
  int y;
  int width;
  int height;

};

class LockWrapper {
public:
    [[nodiscard]] LockWrapper(size_t& _lockCount);
    ~LockWrapper();
    bool IsLocked();
    bool IsOwner();
private:
    bool owner = false;
    size_t& lockCount;
};