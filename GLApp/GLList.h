/*
  File:        GLList.h
  Description: Scrolled table class (SDL/OpenGL OpenGL application framework)
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
#include "GLScrollbar.h"
#include "GLTextField.h"
class Worker;

#ifndef _GLLISTH_
#define _GLLISTH_

// Cell alignment
#define ALIGN_LEFT   0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT  2

// Column colors
#define COLOR_BLACK   0
#define COLOR_RED 1
#define COLOR_GREEN  2
#define COLOR_BLUE 3

// Editing cell format
#define EDIT_STRING  1
#define EDIT_NUMBER  2

// Selection mode
#define SINGLE_ROW    0
#define MULTIPLE_ROW  1
#define SINGLE_CELL   2
#define BOX_CELL      3

class GLList : public GLComponent {

public:

  // Construction
  GLList(int compId);
  ~GLList();

  // Component methods
  void SetWorker(Worker *w);
  void SetSize(int nbColumn,int nbRow,BOOL keepData=FALSE,BOOL showProgress=FALSE);
  void SetColumnLabels(char **names);
  void SetColumnLabel(int colId,char *name);
  void SetAutoColumnLabel(BOOL enable);
  void SetColumnLabelVisible(BOOL visible);
  void SetRowLabels(char **names);
  void SetRowLabel(int rowId,char *name);
  void SetRowLabelAlign(int align);
  void SetAutoRowLabel(BOOL enable);
  void SetRowLabelMargin(int margin);
  void SetRowLabelVisible(BOOL visible);
  void SetColumnWidths(int *widths);
  void SetColumnWidth(int colId,int width);
  void SetColumnWidthForAll(int width);
  void AutoSizeColumn();
  void SetColumnAligns(int *aligns);
  void SetColumnAlign(int colId,int align);
  void SetAllColumnAlign(int align);
  void SetColumnColors(int *aligns);
  void SetColumnColor(int colId, int align);
  void SetAllColumnColors(int align);
  void SetColumnEditable(int *editables);
  BOOL GetSelectionBox(int *row,int *col,int *rowLength,int *colLength);
  void SetVScrollVisible(BOOL visible);
  void SetHScrollVisible(BOOL visible);
  void SetSelectionMode(int mode);
  void GLList::SetSelectedCell(int column,int row);
  void SetCornerLabel(char *text);
  void Clear(BOOL keepColumns=FALSE,BOOL showProgress=FALSE);
  void ResetValues();
  int  GetNbRow();
  int  GetNbColumn();
  void SetRow(int row,char **values);
  void ScrollToVisible();
  void ScrollToVisible(int row,int col,BOOL searchIndex=FALSE);
  void ScrollUp();
  void ScrollDown();
  char *GetValueAt(int col,int row);
  int  GetUserValueAt(int col,int row);
  void SetValueAt(int col,int row,const char *value,int userData=0,BOOL searchIndex=FALSE);
  int  GetRowForLocation(int x,int y);
  int  GetColForLocation(int x,int y);
  void SetMotionSelection(BOOL enable);
  int  GetSelectedRow(BOOL searchIndex=FALSE);
  int  GetSelectedColumn();
  void SetSelectedRow(int row,BOOL searchIndex=FALSE);
  void AddSelectedRow(int row,BOOL searchIndex=FALSE);
  void SetSelectedRows(int *rows,int nbRow,BOOL searchIndex=FALSE);
  void SelectAllRows();
  int  GetNbSelectedRow();
  void GetSelectedRows(int **rows,int *nbRow,BOOL searchIndex=FALSE);
  void ClearSelection();
  int  GetDraggedCol();
  int  GetColWidth(int col);
  void GetVisibleRows(int *start,int *end);
  void SetGrid(BOOL visible);
  void CopyToClipboard(int row,int col,int rowLght,int colLgth);
  void CopyAllToClipboard();
  void CopySelectionToClipboard();
  int  FindIndex(int index,int inColumn);
  int GetValueInt(int row, int column);
  double GetValueDouble(int row, int column);
  //void UpdateAllRows();
  void ReOrder();
  void PasteClipboardText(BOOL allowExpandRows, BOOL allowExpandColumns, int extraRowsAtEnd=0);
  void SetFontColor(int r, int g, int b);

  
  int   lastRowSel;
  BOOL Sortable;
  int  *cEdits;

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y,int width,int height);
  void SetParent(GLContainer *parent);
  void SetFocus(BOOL focus);

private:

  Worker       *worker;
  void UpdateSBRange();
  void CreateAutoLabel();
  int  GetRowForLocationSat(int x,int y);
  int  GetColForLocationSat(int x,int y);
  int  GetColsWidth(int c,int lgth);
  void HandleWheel(SDL_Event *evt);

  GLScrollBar *sbH;
  GLScrollBar *sbV;
  GLTextField *edit;
  GLMenu      *menu;
  int   nbCol;
  int   nbRow;
  int  *cWidths;
  int   cHeight;
  int  *cAligns;
  int  *cColors;
  char **cNames;
  char **rNames;
  char **values;
  int  *uValues;
  
  int   sbDragged;
  BOOL  colDragged;
  BOOL  motionSelection;
  BOOL  showCLabel;
  BOOL  showRLabel;
  int   rowLabelAlign;
  BOOL  autoColumnName;
  BOOL  autoRowName;
  BOOL  vScrollVisible;
  BOOL  hScrollVisible;
  BOOL  isEditable;
  BOOL  isEditing;
  int   sbWidth;
  int   sbHeight; //selection band height?
  int   labHeight; //label spacrholder height
  int   labWidth;
  BOOL  gridVisible;

  int   selectionMode;
  int   nbSelectedRow;
  int   *selectedRows;
  int   selectedCol;
  int   lastColSel;
  BOOL  selDragged;
  int   labelRowMargin;
  char  *cornerLabel;

  int FontColorR;
  int FontColorG;
  int FontColorB;

  int   lastColX;
  int   lastColY;
  int   draggedColIdx;
  void  MoveColumn(int x,int y);
  int   GetColumnEdge(int x,int y);
  void  MapEditText();
  int   GetColumnStart(int colIdx);
  int   RelayToEditText(SDL_Event *evt);
  void  UpdateCell();
  void  CancelEdit();

};

#endif /* _GLLISTH_ */