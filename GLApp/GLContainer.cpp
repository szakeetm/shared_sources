/*
  File:        GLContainer.h
  Description: Base component container class (SDL/OpenGL OpenGL application framework)
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

#include "GLContainer.h"
#include "GLWindow.h"
#include "GLComponent.h"
#include "GLToolkit.h"
#include "GLWindowManager.h"
#include "GLApp.h"
#include <malloc.h>

// --------------------------------------------------------

GLContainer::GLContainer() {

  list = NULL;
  lastFocus = NULL;
  draggedComp = NULL;
  lastClick = 0;
  evtProcessed = FALSE;
  evtCanceled = FALSE;
  parentWin = NULL;
  redirect = NULL;

}

// --------------------------------------------------------

GLContainer::~GLContainer() {
  Clear();
}

// ---------------------------------------------------------------

void GLContainer::Clear() {

  COMPLINK *node = list;
  COMPLINK *lnode;

  while(node!=NULL) {
    node->comp->InvalidateDeviceObjects();
    SAFE_DELETE(node->comp);
    lnode = node;
    node = node->next;
    SAFE_FREE(lnode);
  }
  list = NULL;

}

// ---------------------------------------------------------------

void GLContainer::SetWindow(GLWindow *parent) {

  parentWin = parent;
  // Relay to sub
  COMPLINK *node = list;
  while(node!=NULL) {
    node->comp->SetWindow(parent);
    node = node->next;
  }

}

GLWindow *GLContainer::GetWindow() {
  return parentWin;
}

// ---------------------------------------------------------------

BOOL GLContainer::IsEventProcessed() {
  return evtProcessed;
}

// ---------------------------------------------------------------

BOOL GLContainer::IsEventCanceled() {
  return evtCanceled;
}

// ---------------------------------------------------------------

BOOL GLContainer::IsDragging() {
  return (draggedComp!=NULL);
}

// ---------------------------------------------------------------

void GLContainer::RedirectMessage(GLContainer *cont) {
  redirect = cont;
}

GLContainer *GLContainer::GetRedirect() {
  return redirect;
}

// ---------------------------------------------------------------

void GLContainer::CancelDrag(SDL_Event *evt) {

  draggedComp=NULL;

}

// ---------------------------------------------------------------

void GLContainer::RestoreDeviceObjects() {

  COMPLINK *node = list;
  while(node!=NULL) {
    node->comp->RestoreDeviceObjects();
    node = node->next;
  }

}

// ---------------------------------------------------------------

void GLContainer::InvalidateDeviceObjects() {

  COMPLINK *node = list;
  while(node!=NULL) {
    node->comp->InvalidateDeviceObjects();
    node = node->next;
  }

}

// ---------------------------------------------------------------

void GLContainer::SetFocus(GLComponent *src) {
  if( lastFocus ) lastFocus->SetFocus(FALSE);
  src->SetFocus(TRUE);
  lastFocus = src;
}

// ---------------------------------------------------------------

void GLContainer::PostDelete(GLComponent *comp) {

  // Mark for post deletion
  COMPLINK *node = list;
  BOOL found = FALSE;
  while(!found && node) {
    found = (node->comp == comp);
    if( !found ) node=node->next;
  }

  if( found ) node->postDelete = TRUE;

}

// ---------------------------------------------------------------

void GLContainer::Remove(GLComponent *comp) {

  COMPLINK *node = list;
  COMPLINK *prevNode = NULL;

  BOOL found = FALSE;
  while(!found && node) {
    found = (node->comp == comp);
    if( !found ) {
      prevNode = node;
      node=node->next;
    }
  }

  if( found ) {
    if( prevNode ) {
      prevNode->next = node->next;
    } else {
      list = list->next;
    }
    node->comp->SetParent(NULL);
    SAFE_FREE(node);
  }

}

// ---------------------------------------------------------------

void GLContainer::Add(GLComponent *comp) {

  COMPLINK *node = list;
  GLContainer *cParent = comp->GetParent();

  // Already added
  BOOL found = FALSE;
  while(!found && node) {
    found = (node->comp == comp);
    if( !found ) node=node->next;
  }
  if( found ) {
    node->canProcess = TRUE;
    return;
  }

  // Remove from other window
  if ( cParent ) cParent->Remove(comp);

  node = list;
  if( !node ) {
    // Create head
    node = (COMPLINK *)malloc(sizeof(COMPLINK));
    node->next = list;
    node->comp = comp;
    node->canProcess = TRUE;
    node->postDelete = FALSE;
    node->comp->SetParent(this);
    list = node;
  } else {
    // Add at the end
    while(node->next) node=node->next; 
    node->next = (COMPLINK *)malloc(sizeof(COMPLINK));
    node->next->next = NULL;
    node->next->comp = comp;
    node->next->postDelete = FALSE;
    node->next->canProcess = TRUE;
    node->next->comp->SetParent(this);
  }

}

// ---------------------------------------------------------------

void GLContainer::FreezeComp() {

  COMPLINK *node = list;
  while(node) {
    node->canProcess = FALSE;
    node=node->next;
  }

}

// ---------------------------------------------------------------

void GLContainer::UnfreezeComp() {

  COMPLINK *node = list;
  while(node) {
    node->canProcess = TRUE;
    node=node->next;
  }

}

// ----------------------------------------------------------

void GLContainer::ManageEvent(SDL_Event *evt) {

  evtProcessed = FALSE;
  evtCanceled = FALSE;

  // Cancel dragging
  if( evt->type == SDL_MOUSEBUTTONUP ) {
    if(draggedComp) {
      ManageComp(draggedComp,evt);
      DoPostDelete();
    }
    CancelDrag(evt);
  }

}

// ----------------------------------------------------------
GLComponent* GLContainer::GetFirstChildComp() {
	return (list->next->comp);
}

// ----------------------------------------------------------

void GLContainer::RelayEvent(SDL_Event *evt) {

  if( parentWin ) {
    RelayEventReverse(list,evt);
    DoPostDelete();
  }

}

// ---------------------------------------------------------------

void GLContainer::RelayEventReverse(COMPLINK *lst,SDL_Event *evt) {
  if( lst ) {
    RelayEventReverse(lst->next,evt);
    if(!lst->postDelete && lst->canProcess) RelayEvent(lst->comp,evt);
  }
}

// ---------------------------------------------------------------

void GLContainer::DoPostDelete() {

  COMPLINK *node = list;
  COMPLINK *prevNode = NULL;
  COMPLINK *toFree = NULL;

  while(node) {
    if (node->postDelete) {
      if( prevNode==NULL )
        // Remove head
        list=list->next;
      else
        prevNode->next = node->next;
      node->comp->InvalidateDeviceObjects();
      if(node->comp==lastFocus) lastFocus = NULL;
      SAFE_DELETE(node->comp);
      toFree=node;
    } else {
      prevNode = node;
    }
    node=node->next;
    SAFE_FREE(toFree);
  }

}
// ---------------------------------------------------------------

void GLContainer::ManageComp(GLComponent *comp,SDL_Event *evt) {

  comp->ManageEvent(evt);
  if(!comp->IsEventProcessed()) GLToolkit::SetCursor(comp->GetCursor());
  evtProcessed = TRUE;

}

// ---------------------------------------------------------------

void GLContainer::RelayEvent(GLComponent *comp,SDL_Event *evt,int ox,int oy) {

  if(comp->IsEnabled() && comp->IsVisible() && !evtProcessed) {

    // Focus
    if( evt->type == SDL_MOUSEBUTTONDOWN ) {
      if(parentWin->IsInComp(comp,evt->button.x+ox,evt->button.y+oy) && comp->IsFocusable()) {
        if( comp != lastFocus ) {
          comp->SetFocus(TRUE);
          if( lastFocus ) lastFocus->SetFocus(FALSE);
          lastFocus = comp;
          lastClick = SDL_GetTicks();
        } else {
          // Emulate double click
          int t = SDL_GetTicks();
          if( lastClick && evt->button.button==SDL_BUTTON_LEFT ) {
            if( t-lastClick<250 ) {
              t = 0;
              evt->type = SDL_MOUSEBUTTONDBLCLICK;
            }
          }
          lastClick = t;
        }
        draggedComp = comp;
      }
    }

    // Relay events
    if( evt->type == SDL_MOUSEBUTTONUP || evt->type == SDL_MOUSEBUTTONDOWN ) {
      if(parentWin->IsInComp(comp,evt->button.x+ox,evt->button.y+oy)) {
        ManageComp(comp,evt);
      }
    } else if( evt->type == SDL_MOUSEMOTION ) {
      if( draggedComp ) {
        if(draggedComp==comp) ManageComp(comp,evt);
      } else {
        if(parentWin->IsInComp(comp,evt->motion.x+ox,evt->motion.y+oy)) {
          ManageComp(comp,evt);
        }
      }
    } else if( evt->type == SDL_ACTIVEEVENT ) {
      ManageComp(comp,evt);
    } else {
      if( comp->HasFocus() ) {
        ManageComp(comp,evt);
      }
    }

  }

}

// ---------------------------------------------------------------

void GLContainer::ProcessAcc(int accId) {
}

// ---------------------------------------------------------------

void GLContainer::ProcessMessage(GLComponent *src,int message) {
  if(redirect) redirect->ProcessMessage(src,message);
}

// ---------------------------------------------------------------

void GLContainer::PaintComponents() {
  
  COMPLINK *node = list;
  while(node!=NULL) {
    if(node->comp->IsVisible()) node->comp->Paint();
	GLToolkit::CheckGLErrors("GLContainer::PaintComponents()");
    node = node->next;
  }

}
