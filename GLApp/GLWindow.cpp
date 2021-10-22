// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLComponent.h"
#include "GLToolkit.h"
#include "Helper/MathTools.h" //Saturate
#include "GLWindowManager.h"
#include "GLApp.h"
#include "GLMenu.h"
#include <cstring> //strcpy, etc.

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

extern GLApplication *theApp;

#define DRAGG_NONE  0
#define DRAGG_POS   1
#define DRAGG_SIZE  2
#define DRAGG_SIZEH 3
#define DRAGG_SIZEV 4

#define CLOSE_SYSBTN 0
#define MAX_SYSBTN   1
#define ICON_SYSBTN  2

GLWindow::GLWindow() : _title{}, iconTitle{}, GLContainer() {
    mXOrg = mYOrg = 0;
    orgWidth = orgHeight = 0;
    posXSave = posYSave = 0;
    widthSave = heightSave = 0;

    draggMode = DRAGG_NONE;
    _width = 0;
    _height = 0;
    iconWidth = 0;
    posX = 0;
    posY = 0;
    strcpy(this->_title, "");
    strcpy(this->iconTitle, "");

    closeState = 0;
    maxState = 0;
    iconState = 0;
    rBack = 212;
    gBack = 208;
    bBack = 200;
    border = true;
    isMaster = false;
    visible = false;
    menuBar = nullptr;
    isResizable = false;
    minWidth = 30;
    minHeight = 0;
    menus = nullptr;
    isModal = false;
    SetWindow(this);
    _iconifiable = false;
    _iconified = false;
    maximized = false;
    lastClick = 0;
    animateFocus = true;

}

GLWindow::~GLWindow() {

    if (menuBar) {
        menuBar->InvalidateDeviceObjects();
        SAFE_DELETE(menuBar);
    }
    if (menus) {
        menus->Clear();
        SAFE_DELETE(menus);
    }

}

void GLWindow::SetAnimatedFocus(bool animate) {
    animateFocus = animate;
}

int GLWindow::GetHeight() const {
    return _height;
}

int GLWindow::GetWidth() const {
    return _width;
}

void GLWindow::SetMenuBar(GLComponent *bar, int hBar) {

    if (lastFocus == menuBar) lastFocus = nullptr;
    if (draggedComp == menuBar) draggedComp = nullptr;
    menuBar = bar;
    int m = (strlen(_title)) ? 20 : 2;
    if (menuBar) {
        menuBar->SetBounds(0, m, _width, hBar);
        menuBar->SetParent(this);
    }

}

void GLWindow::AddMenu(GLMenu *menu) {
    if (!menus) {
        menus = new GLContainer();
        menus->SetWindow(this);
        menus->RedirectMessage(this);
    }
    menus->Add(menu);
}

[[maybe_unused]] void GLWindow::RemoveMenu(GLMenu *menu) {
    if (menus) menus->PostDelete(menu);
}

void GLWindow::ReassignMenu(GLMenu *menu, GLContainer *newParent) {
    //unregisters and sets a new parent
    //used after GLMenu::Track()
    if (menus) menus->Remove(menu, newParent);
}

void GLWindow::CloseMenu() {

    if (menuBar) {
        if (lastFocus == menuBar) lastFocus = nullptr;
        if (draggedComp == menuBar) {
            draggedComp = nullptr;
            draggMode = DRAGG_NONE;
        }
        menuBar->SetFocus(false);
    }

}

void GLWindow::SetMaster(bool master) {
    isMaster = master;
}

void GLWindow::SetTitle(const std::string &title) {
    SetTitle(title.c_str());
}

void GLWindow::SetTitle(const char *title) {
    if (title) {

        strcpy(this->_title, title);
        minWidth = std::max(minWidth, GLToolkit::GetDialogFontBold()->GetTextWidth(title) + 30);

        // Compute a short height for icon state
        int w = 0;
        int lgth = (int) strlen(title);
        int i = 0;
        char c[2];
        c[1] = 0;
        while (w < 90 && i < lgth) {
            c[0] = title[i];
            w += GLToolkit::GetDialogFontBold()->GetTextWidth(c);
            if (w < 90) {
                iconTitle[i] = c[0];
                i++;
            }
        }
        iconTitle[i] = 0;
        if (i < lgth || i == 0) strcat(iconTitle, "...");
        iconWidth = GLToolkit::GetDialogFontBold()->GetTextWidth(iconTitle) + 63;

    } else {
        strcpy(this->_title, "");
        strcpy(this->iconTitle, "");
        minWidth = 30;
    }
    //GLWindowManager::Repaint();
}

void GLWindow::SetPosition(int x, int y) {
    posX = x;
    posY = y;
}

void GLWindow::SetClosable(bool c) {
    this->closable = c;
}

void GLWindow::SetBounds(int x, int y, int w, int h) {
    posX = x;
    posY = y;
    _width = w;
    _height = h;
}

void GLWindow::SetMinimumSize(int width, int height) {
    minWidth = width;
    minHeight = height;
}

void GLWindow::SetIconfiable(bool iconifiable) {
    this->_iconifiable = iconifiable;
}

void GLWindow::SetBorder(bool b) {
    border = b;
}

void GLWindow::SetResizable(bool sizable) {
    isResizable = sizable;
}

void GLWindow::SetBackgroundColor(int r, int g, int b) {
    rBack = r;
    gBack = g;
    bBack = b;
}

void GLWindow::GetBackgroundColor(int *r, int *g, int *b) const {
    *r = rBack;
    *g = gBack;
    *b = bBack;
}

void GLWindow::GetBounds(int *x, int *y, int *w, int *h) const {
    *x = posX;
    *y = posY;
    *h = _height;
    *w = _width;
}

bool GLWindow::IsCtrlDown() {
    SDL_Keymod mod = SDL_GetModState();
#if defined(__MACOSX__) || defined(__APPLE__)
    return mod & KMOD_GUI; //Mac command key
#else
    return mod & KMOD_CTRL;
#endif
}

bool GLWindow::IsShiftDown() {
    SDL_Keymod mod = SDL_GetModState();
    return mod & KMOD_SHIFT;
}

bool GLWindow::IsAltDown() {
    SDL_Keymod mod = SDL_GetModState();
    return mod & KMOD_ALT;
}

bool GLWindow::IsSpaceDown() {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
    return state[SDL_GetScancodeFromKey(SDLK_SPACE)];
}

bool GLWindow::IsDkeyDown() {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
    return state[SDL_GetScancodeFromKey(SDLK_d)];
}

bool GLWindow::IsZkeyDown() {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
    return state[SDL_GetScancodeFromKey(SDLK_z)];
}

bool GLWindow::IsCapsLockOn() {
    //return GLWindowManager::IsCapsLockOn();
    return SDL_GetModState() & KMOD_CAPS;
}

bool GLWindow::IsTabDown() {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
    return state[SDL_GetScancodeFromKey(SDLK_TAB)];
}

/*
int GLWindow::GetModState() {
	return GLWindowManager::GetModState();
}
*/

int GLWindow::GetUpMargin() {
    bool hasTitle = strlen(_title) != 0;
    int mUp = 0;
    if (hasTitle) mUp += 19;
    if (menuBar) {
        int xb, yb, wb, hb;
        menuBar->GetBounds(&xb, &yb, &wb, &hb);
        mUp += (hb + 2);
    }
    return mUp;
}

bool GLWindow::IsInWindow(int mx, int my) const {
    return (mx >= posX && mx <= posX + _width && my >= posY && my <= posY + _height);
}

bool GLWindow::IsInSysButton(SDL_Event *evt, int which_btn) {

    int mX = evt->button.x;
    int mY = evt->button.y;
    int width = (_iconified ? iconWidth : _width);

    switch (which_btn) {
        case CLOSE_SYSBTN:
            return (mX >= posX + width - 18 && mX < posX + width - 5 && mY >= posY + 4 && mY <= posY + 15);
        case MAX_SYSBTN:
            return (mX >= posX + width - 35 && mX < posX + width - 22 && mY >= posY + 4 && mY <= posY + 15);
        case ICON_SYSBTN:
            return (mX >= posX + width - 52 && mX < posX + width - 39 && mY >= posY + 4 && mY <= posY + 15);
        default:
            break;
    }

    return false;

}

bool GLWindow::IsInComp(GLComponent *src, int mx, int my) {
    int x, y, w, h;
    src->GetBounds(&x, &y, &w, &h);
    int u = GetUpMargin();
    if (src != menuBar) {
        x = x + posX;
        y = y + posY + u;
    } else {
        if (isMaster) y += u;
    }
    return (mx >= x && mx <= x + w && my >= y && my <= y + h);
}

[[maybe_unused]] void GLWindow::GetClientArea(int *x, int *y, int *w, int *h) {
    *x = 0;
    *y = 0;
    *h = _height - GetUpMargin();
    *w = _width;
}

void GLWindow::ProcessMessage(GLComponent *src, int message) {

    // Relay to master
    if (isMaster) theApp->ProcessMessage(src, message);

    // Handle window event
    switch (message) {
        case MSG_ICONIFY:
            Iconify(!_iconified);
            break;
        case MSG_MAXIMISE:
            Maximise(!maximized);
            break;
        case MSG_CLOSE:
            if (closable) SetVisible(false);
            break;
        default:
            break;
    }

}

void GLWindow::UpdateOnResize() {

    GLWindow* master = GLWindowManager::GetTopLevelWindow();
    int masterWidth = master->GetWidth();
    int masterHeight = master->GetHeight();

    if (maximized) { //Keep maximized
        int u = master->GetUpMargin();
        SetBounds(1, u, masterWidth - 2, masterHeight - u - 2);
    }
    else { //Keep on screen
        int childX, childY, childW, childH;
        GetBounds(&childX, &childY, &childW, &childH);
        SetBounds(std::max(0, std::min(childX, masterWidth - 100)), std::max(0, std::min(childY, masterHeight - 50)), childW, childH);
    }

}

void GLWindow::Maximise(bool max) {

    if (!isResizable) return;

    if (!maximized && max) {

        GLWindow *master = GLWindowManager::GetTopLevelWindow();
        int u = master->GetUpMargin();
        int nW = master->GetWidth() - 2;
        int nH = master->GetHeight() - u - 2;
        posXSave = posX;
        posYSave = posY;
        widthSave = _width;
        heightSave = _height;
        GLWindowManager::AnimateMaximize(this, 1, u, nW, nH);
        maximized = true;

    } else if (maximized && !max) {

        maximized = false;
        GLWindowManager::AnimateMaximize(this, posXSave, posYSave, widthSave, heightSave);

    }

}

void GLWindow::Iconify(bool iconify) {

    if (!_iconifiable) return;

    if (!iconify) {
        posX = posXSave;
        posY = posYSave;
        GLWindowManager::AnimateDeIconify(this);
        posX = posXSave;
        posY = posYSave;
        _iconified = iconify;
        GLWindowManager::BringToFront(this);
    } else {
        _iconified = iconify;
        posXSave = posX;
        posYSave = posY;
        GLWindowManager::AnimateIconify(this);
    }

}

bool GLWindow::IsIconic() const {
    return _iconified;
}

bool GLWindow::IsMaximized() const {
    return maximized;
}

int GLWindow::GetIconWidth() const {
    return iconWidth;
}

bool GLWindow::IsMoving() const {
    return draggMode != DRAGG_NONE;
}

int GLWindow::GetScreenX(GLComponent *src) const {
    int x, y, w, h;
    src->GetBounds(&x, &y, &w, &h);
    return x + posX;
}

int GLWindow::GetScreenY(GLComponent *src) {
    int x, y, w, h;
    src->GetBounds(&x, &y, &w, &h);
    return y + posY + GetUpMargin();
}

int GLWindow::GetX(GLComponent *src, SDL_Event *evt) const {

    int x = 0, y = 0, w = 0, h = 0;
    if (src)
        src->GetBounds(&x, &y, &w, &h);

    if (evt->type == SDL_MOUSEBUTTONUP || evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONDBLCLICK) {
        return evt->button.x - (posX + x);
    } else if (evt->type == SDL_MOUSEMOTION) {
        return evt->motion.x - (posX + x);
    }

    return 0;

}

int GLWindow::GetY(GLComponent *src, SDL_Event *evt) {

    int u = GetUpMargin();
    int x = 0, y = 0, w = 0, h = 0;
    if (src)
        src->GetBounds(&x, &y, &w, &h);

    if (evt->type == SDL_MOUSEBUTTONUP || evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONDBLCLICK) {
        return evt->button.y - (posY + u + y);
    } else if (evt->type == SDL_MOUSEMOTION) {
        return evt->motion.y - (posY + u + y);
    }

    return 0;

}

void GLWindow::SetVisible(bool v) {

    if (!visible) {
        if (v) {
            GLWindowManager::RegisterWindow(this);
            visible = true;
            _iconified = false;
            if (maximized) UpdateOnResize();

        }
    } else {
        if (!v) {
            visible = false;
            if (_iconified) Iconify(false);
            GLWindowManager::UnRegisterWindow(this);
            GLWindowManager::Repaint();
        } else {
            if (_iconified) Iconify(false);
            GLWindowManager::BringToFront(this);
        }
    }

}

bool GLWindow::IsVisible() const {
    return visible;
}

void GLWindow::DoModal() {
    GLWindowManager::FullRepaint();
    SetVisible(true);
    int nbRegistered = GLWindowManager::GetNbWindow();
    isModal = true;
    bool appActive = true;
    SetResizable(false);
    SetIconfiable(false);

    // Modal Loop
    SDL_Event evt;
    while (visible) {

        bool needRedraw = false;

        // Get activation
        Uint32 flags = SDL_GetWindowFlags(theApp->mainScreen);


        bool active = (flags && (SDL_WINDOW_SHOWN & flags)); //App visible
        if (!appActive && active) needRedraw = true;
        appActive = active;

        //While there are events to handle
        while (SDL_PollEvent(&evt)) {

            theApp->UpdateEventCount(&evt);
            GLWindowManager::SearchKeyboardShortcut(&evt, false);
            if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_RESIZED) {
                theApp->Resize(evt.window.data1, evt.window.data2);
                needRedraw = true;
            }
            if (evt.type != SDL_MOUSEWHEEL) ManageEvent(&evt); //Prevent closing combos
            if (!evtProcessed && evt.type == SDL_MOUSEBUTTONDOWN && animateFocus) GLWindowManager::AnimateFocus(this);
            if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_EXPOSED) {
                needRedraw = true;
            }
        }

        theApp->UpdateStats();
        //mApp->SendHeartBeat();


        if (visible) {

            if (needRedraw) {
                GLWindowManager::FullRepaint();
            } else {

                if (appActive) {
                    if (IsMoving()) {
                        GLWindowManager::Repaint();
                    } else {
                        int n = GLWindowManager::GetNbWindow();
                        // Modeless window on dialog
                        GLWindowManager::RepaintRange(nbRegistered - 1, n);
                    }
                }

            }

            if (!appActive) SDL_Delay(100);
            else SDL_Delay(30);

        }

    }

    SetVisible(false);
    GLWindowManager::FullRepaint();
    isModal = false;

}

bool GLWindow::IsDragging() {
    return (draggedComp != nullptr) || draggMode;
}

void GLWindow::CancelDrag(SDL_Event *evt) {

    if (draggMode) GLWindowManager::FullRepaint();
    draggMode = DRAGG_NONE;

    // System button
    if (closeState && !IsInSysButton(evt, CLOSE_SYSBTN)) closeState = 0;
    if (maxState && !IsInSysButton(evt, MAX_SYSBTN)) maxState = 0;
    if (iconState && !IsInSysButton(evt, ICON_SYSBTN)) iconState = 0;

    GLContainer::CancelDrag(evt);

}

void GLWindow::ManageMenu(SDL_Event *evt) {

    evtProcessed = false;
    if (menus) {
        menus->ManageEvent(evt);
        menus->RelayEvent(evt);
        evtProcessed = menus->IsEventProcessed();
    }

}

void GLWindow::UpdateSize(int newWidht, int newHeight, int cursor) {

    int srcW, srcH;
    GLToolkit::GetScreenSize(&srcW, &srcH);
    int uMargin = GetUpMargin();
    Saturate(newWidht, minWidth, srcW);
    Saturate(newHeight, minHeight + uMargin, srcW);
    if (newWidht != _width || newHeight != _height) {
        maximized = false;
        SetBounds(posX, posY, newWidht, newHeight);
        evtProcessed = true;
    }
    GLToolkit::SetCursor(cursor);

}

void GLWindow::ManageEvent(SDL_Event *evt) {

    GLContainer::ManageEvent(evt);

    // Window motion
    if (evt->type == SDL_MOUSEMOTION && !_iconified) {

        if (isResizable && !maximized && !draggMode) {
            int mX = evt->motion.x;
            int mY = evt->motion.y;
            if (mX >= posX + _width - 10 && mX <= posX + _width + 2 && mY >= posY + _height - 10 &&
                mY <= posY + _height + 2) {
                GLToolkit::SetCursor(CURSOR_SIZE);
                evtProcessed = true;
            } else if (mX >= posX + _width - 2 && mX <= posX + _width + 2 && mY >= posY && mY <= posY + _height) {
                GLToolkit::SetCursor(CURSOR_SIZEH);
                evtProcessed = true;
            } else if (mX >= posX && mX <= posX + _width && mY >= posY + _height - 2 && mY <= posY + _height + 2) {
                GLToolkit::SetCursor(CURSOR_SIZEV);
                evtProcessed = true;
            }

        }

        switch (draggMode) {
            case DRAGG_POS: {
                int mX = evt->motion.x;
                int mY = evt->motion.y;
                int diffX = (mX - mXOrg);
                int diffY = (mY - mYOrg);
                mXOrg = mX;
                mYOrg = mY;
                if ((diffX || diffY)) {
                    maximized = false;
                    SetBounds(posX + diffX, posY + diffY, _width, _height);
                    evtProcessed = true;
                    GLToolkit::SetCursor(CURSOR_DEFAULT);
                }
                return;
            }
            case DRAGG_SIZE: {
                int newWidht = orgWidth + (evt->motion.x - mXOrg);
                int newHeight = orgHeight + (evt->motion.y - mYOrg);
                UpdateSize(newWidht, newHeight, CURSOR_SIZE);
                return;
            }
            case DRAGG_SIZEH: {
                int newWidht = orgWidth + (evt->motion.x - mXOrg);
                UpdateSize(newWidht, _height, CURSOR_SIZEH);
                return;
            }
            case DRAGG_SIZEV: {
                int newHeight = orgHeight + (evt->motion.y - mYOrg);
                UpdateSize(_width, newHeight, CURSOR_SIZEV);
                return;
            }
        }
    }

    // Z order
    if (!isModal && !_iconified) {
        if (evt->type == SDL_MOUSEBUTTONDOWN && evt->button.button == SDL_BUTTON_LEFT) {
            int mX = evt->button.x;
            int mY = evt->button.y;
            if (mX >= posX && mX <= posX + _width && mY >= posY && mY <= posY + _height)
                GLWindowManager::BringToFront(this);
        }
    }

    // Title bar
    if (strlen(_title) != 0 || _iconified)
        if (evt->type == SDL_MOUSEBUTTONUP || evt->type == SDL_MOUSEBUTTONDOWN) {
            if (evt->button.button == SDL_BUTTON_LEFT) {

                int mX = evt->button.x;
                int mY = evt->button.y;

                int width = (_iconified ? iconWidth : _width);

                if (mX >= posX + 2 && mX <= posX + width - 2 && mY >= posY && mY <= posY + 17) {

                    evtProcessed = true;
                    GLToolkit::SetCursor(CURSOR_DEFAULT);

                    if (IsInSysButton(evt, CLOSE_SYSBTN)) {

                        // System button (close win)
                        if (evt->type == SDL_MOUSEBUTTONDOWN) {
                            closeState = 1;
                        }

                        if (evt->type == SDL_MOUSEBUTTONUP && closeState) {
                            ProcessMessage(nullptr, MSG_CLOSE);
                            closeState = 0;
                        }

                    } else if (isResizable && !_iconified && IsInSysButton(evt, MAX_SYSBTN)) {

                        // Maximise button (maximise win)
                        if (evt->type == SDL_MOUSEBUTTONDOWN) {
                            maxState = 1;
                        }

                        if (evt->type == SDL_MOUSEBUTTONUP && maxState) {
                            ProcessMessage(nullptr, MSG_MAXIMISE);
                            maxState = 0;
                        }

                    } else if (_iconifiable && IsInSysButton(evt, ICON_SYSBTN)) {

                        // Iconify button (iconify win)
                        if (evt->type == SDL_MOUSEBUTTONDOWN) {
                            iconState = 1;
                        }

                        if (evt->type == SDL_MOUSEBUTTONUP && iconState) {
                            ProcessMessage(nullptr, MSG_ICONIFY);
                            iconState = 0;
                        }

                    } else {

                        if (evt->type == SDL_MOUSEBUTTONDOWN) {
                            size_t t = SDL_GetTicks();
                            if ((t - lastClick) < 250) {

                                // Maximize/Minimize (iconify non resizable window) on double click
                                if (isResizable && !_iconified) ProcessMessage(nullptr, MSG_MAXIMISE);
                                else ProcessMessage(nullptr, MSG_ICONIFY);

                            } else {

                                if (!maximized && !_iconified) {
                                    // Title bar (Dragging)
                                    mXOrg = mX;
                                    mYOrg = mY;
                                    draggMode = DRAGG_POS;
                                }

                            }
                            lastClick = t;
                        }

                    }
                }

                // Window size
                if (isResizable && !_iconified && !maximized) {
                    if (mX >= posX + width - 10 && mX <= posX + width + 2 && mY >= posY + _height - 10 &&
                        mY <= posY + _height + 2) {

                        evtProcessed = true;
                        if (evt->type == SDL_MOUSEBUTTONDOWN) {
                            mXOrg = mX;
                            mYOrg = mY;
                            orgWidth = width;
                            orgHeight = _height;
                            draggMode = DRAGG_SIZE;
                            GLToolkit::SetCursor(CURSOR_SIZE);
                        }

                    } else if (mX >= posX + width - 2 && mX <= posX + width + 2 && mY >= posY && mY <= posY + _height) {

                        evtProcessed = true;
                        if (evt->type == SDL_MOUSEBUTTONDOWN) {
                            mXOrg = mX;
                            mYOrg = mY;
                            orgWidth = width;
                            orgHeight = _height;
                            draggMode = DRAGG_SIZEH;
                            GLToolkit::SetCursor(CURSOR_SIZEH);
                        }

                    } else if (mX >= posX && mX <= posX + width && mY >= posY + _height - 2 &&
                               mY <= posY + _height + 2) {

                        evtProcessed = true;
                        if (evt->type == SDL_MOUSEBUTTONDOWN) {
                            mXOrg = mX;
                            mYOrg = mY;
                            orgWidth = width;
                            orgHeight = _height;
                            draggMode = DRAGG_SIZEV;
                            GLToolkit::SetCursor(CURSOR_SIZEV);
                        }

                    }
                }

            }
        }

    if (_iconified)
        return;

    // Menubar
    if (!evtProcessed && menuBar)
        RelayEvent(menuBar, evt, 0, (isMaster ? 22 : 0));

    // Relay event to container
    RelayEvent(evt);

    // Pump non processed mouse event
    if (evt->type == SDL_MOUSEBUTTONUP || evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEMOTION) {
        if (!evtProcessed && IsInWindow(evt->button.x, evt->button.y)) {
            evtProcessed = true;
            // Cancel event (for menu closing)
            if (evt->type == SDL_MOUSEBUTTONUP || evt->type == SDL_MOUSEBUTTONDOWN) evtCanceled = true;
            GLToolkit::SetCursor(CURSOR_DEFAULT);
        }
    }

}

void GLWindow::Clip(GLComponent *src, int lMargin, int uMargin, int rMargin, int bMargin) {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int x, y, w, h;
    src->GetBounds(&x, &y, &w, &h);
    /*if (h<=(uMargin+bMargin)){
        int breaker=0;
    }*/
    glOrtho(0, w - (rMargin + lMargin), h - (uMargin + bMargin), 0, -1.0, 1.0);
    GLToolkit::SetViewport(x + posX + lMargin, y + posY + uMargin + GetUpMargin(), w - (rMargin + lMargin),
                           h - (uMargin + bMargin));

}

void GLWindow::ClipRect(GLComponent *src, int x, int y, int width, int height) {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int xc, yc, w, h;
    src->GetBounds(&xc, &yc, &w, &h);
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;
    glOrtho(0, width, height, 0, -1.0, 1.0);
    GLToolkit::SetViewport(xc + posX + x, yc + posY + y + GetUpMargin(), width, height);

}

void GLWindow::ClipToWindow() {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int mUp = GetUpMargin();
    glOrtho(0, _width, _height - mUp, 0, -1.0, 1.0);
    GLToolkit::SetViewport(posX, posY + mUp, _width, _height - mUp);

}

void GLWindow::ClipWindowExtent() {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    int wS, hS;
    GLToolkit::GetScreenSize(&wS, &hS);
    int nWidth = wS - posX + 1;
    int nHeight = hS - posY + 1;
    int mUp = GetUpMargin();
    glOrtho(0, nWidth, nHeight - mUp, 0, -1.0, 1.0);
    GLToolkit::SetViewport(posX, posY + mUp, nWidth, nHeight - mUp);

}

void GLWindow::PaintMenuBar() {
    if (menuBar && !_iconified) {
        GLWindowManager::NoClip();
        GLToolkit::DrawBox(posX, posY, _width, _height, rBack, gBack, bBack, border);
        menuBar->Paint();
    }
}

void GLWindow::PaintMenu() {

    // Components
    if (menus) {
        ClipWindowExtent();
        menus->PaintComponents();
    }

}

void GLWindow::PaintTitle(int width, int height) {

    // Background
    GLWindowManager::NoClip();
    GLToolkit::DrawBox(posX, posY, width, height, rBack, gBack, bBack, border);

    // Title bar
    bool hasTitle = strlen(_title) != 0;
    if (hasTitle) {
        glColor3f(0.25f, 0.35f, 0.75f);
        GLToolkit::DrawBar(posX + 2, posY + 1, width - 5, 18);
        //GLToolkit::DrawBox(posX+2,posY+1,width-5,18,50,80,140);
        GLToolkit::GetDialogFontBold()->SetTextColor(1.0f, 1.0f, 1.0f);
        if (_iconified)
            GLToolkit::GetDialogFontBold()->DrawText(posX + 5, posY + 3, iconTitle, false);
        else
            GLToolkit::GetDialogFontBold()->DrawText(posX + 5, posY + 3, _title, false);
    }

    // System menu
    if (hasTitle) {
        if (closable) GLToolkit::GetDialogFont()->SetTextColor(0.0f, 0.0f, 0.0f);
        else GLToolkit::GetDialogFont()->SetTextColor(0.7f, 0.7f, 0.7f);
        if (!closeState) {
            GLToolkit::DrawBox(posX + width - 18, posY + 4, 13, 12, 212, 208, 200, true, false);
            GLToolkit::GetDialogFont()->DrawText(posX + width - 15, posY + 2, "x", false);
        } else {
            GLToolkit::DrawBox(posX + width - 18, posY + 4, 13, 12, 212, 208, 200, true, true);
            GLToolkit::GetDialogFont()->DrawText(posX + width - 14, posY + 3, "x", false);
        }

        if (!isModal) {
            if (!maxState) {
                if (isResizable && !_iconified) GLToolkit::GetDialogFont()->SetTextColor(0.0f, 0.0f, 0.0f);
                else GLToolkit::GetDialogFont()->SetTextColor(0.7f, 0.7f, 0.7f);
                GLToolkit::DrawBox(posX + width - 35, posY + 4, 13, 12, 212, 208, 200, true, false);
                if (maximized && !_iconified)
                    GLToolkit::GetDialogFont()->DrawText(posX + width - 34, posY + 2, "\210", false);
                else
                    GLToolkit::GetDialogFont()->DrawText(posX + width - 34, posY + 2, "\207", false);
            } else {
                GLToolkit::DrawBox(posX + width - 34, posY + 4, 13, 12, 212, 208, 200, true, true);
                if (maximized)
                    GLToolkit::GetDialogFont()->DrawText(posX + width - 33, posY + 3, "\210", false);
                else
                    GLToolkit::GetDialogFont()->DrawText(posX + width - 33, posY + 3, "\207", false);
            }
            if (!iconState) {
                if (_iconifiable) GLToolkit::GetDialogFont()->SetTextColor(0.0f, 0.0f, 0.0f);
                else GLToolkit::GetDialogFont()->SetTextColor(0.7f, 0.7f, 0.7f);
                GLToolkit::DrawBox(posX + width - 52, posY + 4, 13, 12, 212, 208, 200, true, false);
                GLToolkit::GetDialogFont()->DrawText(posX + width - 49, posY + 2, "_", false);
            } else {
                GLToolkit::DrawBox(posX + width - 52, posY + 4, 13, 12, 212, 208, 200, true, true);
                GLToolkit::GetDialogFont()->DrawText(posX + width - 48, posY + 3, "_", false);
            }
        }
    }

}

void GLWindow::Paint() {

    int w = (_iconified ? iconWidth : _width);
    int h = (_iconified ? 20 : _height);
    PaintTitle(w, h);

    if (_iconified) return;

    if (menuBar) menuBar->Paint();

    // Sizeable sign
    if (isResizable && !maximized) {

        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        _glVertex2i(posX + _width - 5, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 5);
        _glVertex2i(posX + _width - 7, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 7);
        _glVertex2i(posX + _width - 9, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 9);
        _glVertex2i(posX + _width - 11, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 11);
        glEnd();
        glColor3f(0.4f, 0.4f, 0.4f);
        glBegin(GL_LINES);
        _glVertex2i(posX + _width - 4, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 4);
        _glVertex2i(posX + _width - 6, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 6);
        _glVertex2i(posX + _width - 8, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 8);
        _glVertex2i(posX + _width - 10, posY + _height - 2);
        _glVertex2i(posX + _width - 2, posY + _height - 10);
        glEnd();
    }

    GLToolkit::CheckGLErrors("GLWindow::Paint()");
    // Components
    ClipToWindow();
    PaintComponents();

}
