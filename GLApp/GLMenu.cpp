/*
  File:        GLMenu.cpp
  Description: Menu class (SDL/OpenGL OpenGL application framework)
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

#include "GLWindow.h"
#include "GLMenuBar.h"
#include "GLToolkit.h"
#include "GLWindowManager.h"

#include <malloc.h>

#define SIDE_MARGIN 24

// -----------------------------------------------------------

GLMenu::GLMenu():GLComponent(0) {
  selMenu = -1;
  nbItem = 0;
  items = NULL;
  pBar = NULL;
  pMenu = NULL;
  rCode = -1;
  hasAcc = FALSE;

  SetBorder(BORDER_BEVEL_OUT);
  SetVisible(FALSE);
  SetFocusable(FALSE);
  SetBackgroundColor(225,220,220);
}

// -----------------------------------------------------------

GLMenu::~GLMenu() {
  for(int i=0;i<nbItem;i++) {
    if(!items[i].subMenu->GetParent())
      SAFE_DELETE(items[i].subMenu);
    SAFE_FREE(items[i].accName);
  }
  SAFE_FREE(items);
}

// -----------------------------------------------------------

int GLMenu::GetMenu(int mx,int my) {

  BOOL found = FALSE;
  int i = 0;
  while(!found && i<nbItem) {
    if(!items[i].isSeparator) found = IsInItem(items+i,mx,my);
    if(!found) i++;
  }
  if(found) return i;
  else      return -1;

}

// -----------------------------------------------------------

void GLMenu::SetParentMenuBar(GLMenuBar *p) {
  pBar = p;
}

// -----------------------------------------------------------

void GLMenu::SetParentMenu(GLMenu *p) {
  pMenu = p;
}

// -----------------------------------------------------------

int GLMenu::GetNbItem() {
  return nbItem;
}

// -----------------------------------------------------------

void GLMenu::SetState(int itemId,BOOL checked) {

  int i = GetMenu(itemId);
  if( i>=0 ) items[i].checked = checked;

}

void GLMenu::SetIcon(int itemId,int x,int y) {

  int i = GetMenu(itemId);
  if( i>=0 ) {
    items[i].iconX = x;
    items[i].iconY = y;
  }

}

BOOL GLMenu::GetCheck(int itemId) {

  int i = GetMenu(itemId);
  if( i>=0 )  return items[i].checked;
  else        return FALSE;

}

void  GLMenu::SetEnabled(int itemId,BOOL enabled) {

  int i = GetMenu(itemId);
  if( i>=0 ) items[i].enabled = enabled;

}

// -----------------------------------------------------------

int GLMenu::GetMenu(int id) {

  BOOL found = FALSE;
  int i=0;
  while(!found && i<nbItem) {
    found = (items[i].itemId == id);
    if(!found) i++;
  }
  if( found ) return i;
  else        return -1;

}

// -----------------------------------------------------------

BOOL GLMenu::IsInItem(MENUITEM *p,int mx,int my) {
    return mx>(p->x) && mx<=(p->x)+width &&
           my>(p->y) && my<=(p->y)+(p->height);
}


// -----------------------------------------------------------

void GLMenu::Clear() {

  for(int i=0;i<nbItem;i++) {
    if(!parent) {
      SAFE_DELETE(items[i].subMenu);
    } else {
      parent->PostDelete(items[i].subMenu);
    }
  }
  int size = sizeof(MENUITEM)*MAX_MENU_ITEM;
  if(items) memset(items,0,size);
  nbItem = 0;
  hasAcc = FALSE;

}

// -----------------------------------------------------------

void GLMenu::Add(char *itemName,int itemId,int accKeyCode,int accKeyModifier) {

  if(nbItem>=MAX_MENU_ITEM) return;

  if(!items) {
    // Allocate
    int size = sizeof(MENUITEM)*MAX_MENU_ITEM;
    items = (MENUITEM *)malloc(size);
    memset(items,0,size);
  }

  int sx = 3;
  int sy = 3;
  int i;
  for(i=0;i<nbItem;i++) sy+=items[i].height;

  // Accelerators
  if( accKeyCode ) {

    GLWindowManager::RegisterAcc(this,accKeyCode,accKeyModifier,nbItem);
    hasAcc = TRUE;
    items[i].accName = _strdup( GLWindowManager::GetAccStr(accKeyCode,accKeyModifier) );
    items[i].accWidth = GLToolkit::GetDialogFont()->GetTextWidth(items[i].accName);

  }

  items[i].itemId = itemId;
  if(itemName) {
    strncpy(items[i].itemName,itemName,MAX_ITEM_LGTH);
    items[i].itemName[MAX_ITEM_LGTH-1]=0;
    GLWindowManager::RemoveAccFromStr(items[i].itemName,&(items[i].shortcut),&(items[i].sctPos),&(items[i].sctWidth));
    items[i].width  = 2*SIDE_MARGIN + GLToolkit::GetDialogFont()->GetTextWidth(items[i].itemName) + items[i].accWidth;
    items[i].height = 17;
    items[i].subMenu = new GLMenu();
  } else {
    items[i].height = 5;
    items[i].isSeparator = TRUE;
  }
  items[i].x = sx;
  items[i].y = sy;
  items[i].enabled = TRUE;

  nbItem++;

}

// -----------------------------------------------------------

GLMenu *GLMenu::GetSubMenu(char *itemName) {

  char tmpName[256];
  strcpy(tmpName,itemName);
  GLWindowManager::RemoveAccFromStr(tmpName);

  BOOL found = FALSE;
  int i = 0;
  while(!found && i<nbItem) {
    if(!items[i].isSeparator) found = (strcmp(tmpName,items[i].itemName)==0);
    if(!found) i++;
  }
  if(found) {
    items[i].subMenu->SetParentMenuBar(pBar);
    items[i].subMenu->SetParentMenu(pMenu);
    return items[i].subMenu;
  } else
    return NULL;

}

// -----------------------------------------------------------

BOOL GLMenu::HasSub(int s) {
  if(s>=0 && s<nbItem) {
    GLMenu *sub = items[s].subMenu;
    if(sub) return (sub->GetNbItem()>0);
  }
  return FALSE;
}

// -----------------------------------------------------------

void GLMenu::ProcessMenuItem(int m) {

  // Menu item
  if( items[m].enabled ) {
    id = items[m].itemId;
    if( pBar )        pBar->Close();
    else if ( pMenu ) pMenu->Close();
    else              Close();
    if( !pMenu ) {
      if( !parent ) {
        GLWindowManager::GetTopLevelWindow()->ProcessMessage(this,MSG_MENU);
      } else {
        parent->ProcessMessage(this,MSG_MENU);
      }
    } else
      pMenu->rCode = id;
  }

}

// -----------------------------------------------------------

void GLMenu::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int mx = GetWindow()->GetX(this,evt);
  int my = GetWindow()->GetY(this,evt);

  if(evt->type == SDL_MOUSEMOTION) {
    int newMenu = GetMenu(mx,my);
    if( newMenu>=0 && newMenu!=selMenu ) {
      CloseSub();
      selMenu = newMenu;
      if( HasSub(selMenu) ) DropSub(selMenu);
    }
  }

  if(evt->type == SDL_MOUSEBUTTONUP)
  if(evt->button.button == SDL_BUTTON_LEFT) {
    int newMenu = GetMenu(mx,my);
    if( newMenu>=0 ) {
      selMenu = newMenu;
      if( HasSub(selMenu) ) {
        // Drop sub menu
        DropSub(selMenu);
      } else {
        ProcessMenuItem(selMenu);
      }
    }
  }

  // Active event (Mouse entering-leaving event)
  if( evt->type == SDL_ACTIVEEVENT ) {
    if( evt->active.state == SDL_APPMOUSEFOCUS ) {
      if( selMenu>=0 ) {
        if( !items[selMenu].subMenu )
          selMenu = -1;
        else
          if( !items[selMenu].subMenu->IsVisible() )
            selMenu = -1;
      }
    }
  }

  if( nbItem==0 ) return;

  GLMenu *sub = NULL;
  if( selMenu>=0 ) sub = items[selMenu].subMenu;

  if( evt->type == SDL_KEYDOWN ) {
    int unicode = (evt->key.keysym.unicode & 0x7F);
    if( !unicode ) unicode = evt->key.keysym.sym;
    switch(unicode) {
      case SDLK_DOWN:
        if( !(sub && sub->IsVisible()) ) {
          selMenu++;
          if(selMenu>=nbItem) selMenu=0;
          CloseSub(FALSE);
          return;
        }
        break;
      case SDLK_UP:
        if( !(sub && sub->IsVisible()) ) {
          selMenu--;
          if(selMenu<0) selMenu=nbItem-1;
          CloseSub(FALSE);
          return;
        }
        break;
      case SDLK_RETURN:
        if( selMenu>=0 && !HasSub(selMenu) ) {
          ProcessMenuItem(selMenu);
          return;
        } else {
          if(HasSub(selMenu) && !sub->IsVisible()) {
            DropSub(selMenu);
            return;
          }
        }
        break;
      case SDLK_RIGHT:
        if(HasSub(selMenu) && !sub->IsVisible()) {
          DropSub(selMenu);
          sub->selMenu=0;
          return;
        }
        if(!HasSub(selMenu)) {
          if(pBar) pBar->GoRight();
          return;
        }
        break;
      case SDLK_LEFT:
        if(HasSub(selMenu) && sub->IsVisible() ) {
          CloseSub(FALSE);
        } else {
          if(pBar) pBar->GoLeft();
        }
        return;
      default:
        if( ProcessShortcut(evt) ) return;
    }
  }

  // Replay event to sub
  if(sub) sub->ManageEvent(evt);

}

// -----------------------------------------------------------

void GLMenu::ProcessAcc(int accId) {

  if(accId>=nbItem) return;

  if( !HasSub(accId) ) {
    ProcessMenuItem(accId);
  } else {
    if(HasSub(accId) && !items[accId].subMenu->IsVisible()) {
      DropSub(accId);
    }
  }

}

// -----------------------------------------------------------

BOOL GLMenu::ProcessShortcut(SDL_Event *evt) {

  int unicode = (evt->key.keysym.unicode & 0x7F);
  if( !unicode ) unicode = evt->key.keysym.sym;
  if( unicode==0 ) return FALSE;

  BOOL found = FALSE;
  int i = 0;
  while(!found && i<nbItem) {
    found = (unicode==items[i].shortcut);
    if(!found) i++;
  }

  if(found) ProcessAcc(i);

  return found;

}

// -----------------------------------------------------------

void GLMenu::Close() {

  CloseSub();
  SetVisible(FALSE);

}

// -----------------------------------------------------------

void GLMenu::Paint() {

  if(!parent) return;
  GLComponent::Paint();
  GLToolkit::DrawHGradientBox(posX+1,posY+2,20,height-3);

  GLFont2D *font = GLToolkit::GetDialogFont();

  for(int i=0;i<nbItem;i++) {

    MENUITEM *p = items + i;

    if( p->isSeparator ) {

      // Etched colors
      float rL = (float)rBack / 100.0f;
      float gL = (float)gBack / 100.0f;
      float bL = (float)bBack / 100.0f;
      SATURATE(rL,0.0f,1.0f);
      SATURATE(gL,0.0f,1.0f);
      SATURATE(bL,0.0f,1.0f);
      float rD = (float)rBack / 500.0f;
      float gD = (float)gBack / 500.0f;
      float bD = (float)bBack / 500.0f;
      SATURATE(rD,0.0f,1.0f);
      SATURATE(gD,0.0f,1.0f);
      SATURATE(bD,0.0f,1.0f);

      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      glColor3f(rD,gD,bD);
      glBegin(GL_LINES);
      _glVertex2i(posX+p->x+SIDE_MARGIN-4,posY+p->y+3);
      _glVertex2i(posX+p->x+width-4,posY+p->y+3);
      glEnd();
      glColor3f(rL,gL,bL);
      glBegin(GL_LINES);
      _glVertex2i(posX+p->x+SIDE_MARGIN-4,posY+p->y+4);
      _glVertex2i(posX+p->x+width-4,posY+p->y+4);
      glEnd();

    } else {

      if( p->enabled ) {

        if( selMenu==i ) {
          GLToolkit::DrawBox(posX+p->x+SIDE_MARGIN-4,posY+p->y,width-2-SIDE_MARGIN,p->height+1,60,60,200);
          font->SetTextColor(1.0f,1.0f,1.0f);
        } else {
          font->SetTextColor(0.0f,0.0f,0.0f);
        }
        font->DrawText(posX+4+p->x+SIDE_MARGIN,posY+p->y+3,p->itemName,FALSE);

        // Underline shortcut char
        if( p->shortcut ) {
          glDisable(GL_BLEND);
          glDisable(GL_TEXTURE_2D);
          glBegin(GL_LINES);
          _glVertex2i(SIDE_MARGIN+5+posX+p->x+p->sctPos,posY+p->y+15);
          _glVertex2i(SIDE_MARGIN+4+posX+p->x+p->sctPos+p->sctWidth,posY+p->y+15);
          glEnd();
        }

      } else {

        font->SetTextColor(1.0f,1.0f,1.0f);
        font->DrawText(posX+p->x+SIDE_MARGIN+5,posY+p->y+4,p->itemName,FALSE);
        font->SetTextColor(0.4f,0.4f,0.4f);
        font->DrawText(posX+p->x+SIDE_MARGIN+4,posY+p->y+3,p->itemName,FALSE);

      }

      GLMenu *sub = p->subMenu;
      if( sub && sub->GetNbItem() ) {
        font->SetTextColor(0.0f,0.0f,0.0f);
        font->DrawText(posX+p->x+width-15,posY+p->y+2,"\213",FALSE);
      }

      if( !HasSub(i) && p->accName ) {
        font->SetTextColor(0.0f,0.0f,0.0f);
        font->DrawText(posX+p->x+width-items[i].accWidth-10,posY+p->y+3,p->accName,FALSE);
      }

      if( p->checked ) {
        font->SetTextColor(0.0f,0.0f,0.0f);
        font->DrawText(posX+SIDE_MARGIN-1,posY+p->y+3,"\215",FALSE);
      }

      if( p->iconX || p->iconY ) {
        GLToolkit::Draw16x16(posX+2,posY+p->y+1,p->iconX,p->iconY);
      }
    
    }

  }

}

// -----------------------------------------------------------

void GLMenu::DropSub(int s) {

  CloseSub();
  if( s>=0 ) {
    MENUITEM *p = items + s;
    p->subMenu->SetParentMenu(pMenu);
    p->subMenu->Drop(parent,posX+width+1,posY+p->y);
  }
  selMenu = s;

}

// -----------------------------------------------------------

void GLMenu::CloseSub(BOOL resetSel) {

  for(int i=0;i<nbItem;i++) {
    GLMenu *sub = items[i].subMenu;
    if(sub) sub->Close();
  }
  if(resetSel) selMenu = -1;

}
// -----------------------------------------------------------

int GLMenu::Track(GLWindow *parent,int x,int y) {

  if(!parent) parent = GLWindowManager::GetTopLevelWindow();

  // Measure menu
  int menuWidth  = 0;
  int menuHeight = 0;
  for(int i=0;i<nbItem;i++) { 
    MENUITEM *p = items + i;
    menuWidth=MAX(menuWidth,p->width);
    menuHeight += p->height;
  }
  // margin
  menuHeight += 6;
  menuWidth  += 0;

  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( x+menuWidth  > wS    ) x -= menuWidth;
  if( y+menuHeight > hS-30 ) y -= menuHeight;
  SetBounds(x,y,menuWidth,menuHeight);
  parent->FreezeComp();
  parent->AddMenu(this);
  SetParentMenu(this);
  SetVisible(TRUE);
  rCode = -1;

  // Modal Loop
  SDL_Event evt;
  while( IsVisible() )
  {

    //While there are events to handle
    while( SDL_PollEvent( &evt ) ) {
      parent->ManageMenu(&evt);
    }

    if(!parent->IsEventProcessed()) {
      if( evt.type==SDL_MOUSEBUTTONDOWN )
        // Click outside
        Close();
    }

    if(evt.type == SDL_VIDEOEXPOSE)
      GLWindowManager::FullRepaint();

    // Close modal window window on [ESC]
    if( evt.type == SDL_KEYDOWN ) {
      int unicode = (evt.key.keysym.unicode & 0x7F);
      if( !unicode ) unicode = evt.key.keysym.sym;
      if( unicode == SDLK_ESCAPE ) SetVisible(FALSE);
    }

    if( IsVisible() ) {
      GLWindowManager::Repaint();
      Sleep(30);
    }

  }

  parent->UnfreezeComp();
  GLWindowManager::FullRepaint();
  return rCode;

}

// -----------------------------------------------------------

void GLMenu::Drop(GLContainer *parent,int x,int y) {
  
  if(!parent) parent = GLWindowManager::GetTopLevelWindow();

  // Measure menu
  int menuWidth  = 0;
  int menuHeight = 0;
  for(int i=0;i<nbItem;i++) { 
    MENUITEM *p = items + i;
    menuWidth=MAX(menuWidth,p->width);
    menuHeight += p->height;
  }
  // margin
  menuHeight += 6;
  menuWidth  += 40;

  SetBounds(x,y,menuWidth,menuHeight);
  parent->GetWindow()->AddMenu(this);
  SetVisible(TRUE);

}

