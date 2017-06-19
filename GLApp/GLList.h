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
#ifndef _GLLISTH_
#define _GLLISTH_

#include <vector>
#include "GLComponent.h"
class Worker;
class GLMenu;
class GLTextField;
class GLScrollBar;



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
  void SetSize(size_t nbColumn,size_t nbRow,bool keepData=false,bool showProgress=false);
  void SetColumnLabels(char **names);
  void SetColumnLabel(size_t colId,char *name);
  void SetAutoColumnLabel(bool enable);
  void SetColumnLabelVisible(bool visible);
  void SetRowLabels(char **names);
  void SetRowLabel(size_t rowId,char *name);
  void SetRowLabelAlign(int align);
  void SetAutoRowLabel(bool enable);
  void SetRowLabelMargin(int margin);
  void SetRowLabelVisible(bool visible);
  void SetColumnWidths(int *widths);
  void SetColumnWidth(size_t colId, int width);
  void SetColumnWidthForAll(int width);
  void AutoSizeColumn();
  void SetColumnAligns(int *aligns);
  void SetColumnAlign(size_t colId,int align);
  void SetAllColumnAlign(int align);
  void SetColumnColors(int *aligns);
  void SetColumnColor(size_t colId, int color);
  void SetAllColumnColors(int color);
  void SetColumnEditable(int *editables);
  bool GetSelectionBox(size_t *row, size_t *col, size_t *rowLength, size_t *colLength);
  void SetVScrollVisible(bool visible);
  void SetHScrollVisible(bool visible);
  void SetSelectionMode(int mode);
  void SetSelectedCell(size_t column,size_t row);
  void SetCornerLabel(char *text);
  void Clear(bool keepColumns=false,bool showProgress=false);
  void ResetValues();
  size_t  GetNbRow();
  size_t  GetNbColumn();
  void SetRow(size_t row,char **values);
  void ScrollToVisible();
  void ScrollToVisible(size_t row, size_t col,bool searchIndex=false);
  void ScrollUp();
  void ScrollDown();
  char *GetValueAt(size_t col, size_t row);
  int  GetUserValueAt(size_t col, size_t row);
  void SetValueAt(size_t col,size_t row,const char *value,int userData=0,bool searchIndex=false);
  int  GetRowForLocation(int x,int y);
  int  GetColForLocation(int x,int y);
  void SetMotionSelection(bool enable);
  int  GetSelectedRow(bool searchIndex=false);
  int  GetSelectedColumn();
  void SetSelectedRow(int row,bool searchIndex=false);
  void AddSelectedRow(int row,bool searchIndex=false);
  void SetSelectedRows(std::vector<size_t> selection,bool searchIndex=false);
  void SelectAllRows();
  std::vector<size_t> GetSelectedRows(bool searchIndex=false);
  void ClearSelection();
  int  GetDraggedCol();
  int  GetColWidth(size_t col);
  void GetVisibleRows(int *start, int *end);
  void SetGrid(bool visible);
  void CopyToClipboard(size_t row, size_t col, size_t rowLght, size_t colLgth);
  void CopyAllToClipboard();
  void CopySelectionToClipboard();
  int  FindIndex(size_t index,size_t inColumn);
  int GetValueInt(size_t row, size_t column);
  double GetValueDouble(size_t row, size_t column);
  //void UpdateAllRows();
  void ReOrder();
  void PasteClipboardText(bool allowExpandRows, bool allowExpandColumns, int extraRowsAtEnd=0);
  void SetFontColor(int r, int g, int b);

  int  *cEdits;
  int   lastRowSel;
  bool Sortable;

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y, int width, int height);
  void SetParent(GLContainer *parent);
  void SetFocus(bool focus);

private:

  Worker       *worker;
  void UpdateSBRange();
  void CreateAutoLabel();
  size_t  GetRowForLocationSat(int x,int y);
  size_t  GetColForLocationSat(int x,int y);
  int  GetColsWidth(size_t c, size_t lgth);
  void HandleWheel(SDL_Event *evt);

  GLScrollBar *sbH;
  GLScrollBar *sbV;
  GLTextField *edit;
  GLMenu      *menu;
  size_t   nbCol;
  size_t   nbRow;
  int  *cWidths;
  int   cHeight;
  int  *cAligns;
  int  *cColors;
  char **cNames;
  char **rNames;
  char **values;
  int  *uValues;
  
  int   sbDragged;
  bool  colDragged;
  bool  motionSelection;
  bool  showCLabel;
  bool  showRLabel;
  int   rowLabelAlign;
  bool  autoColumnName;
  bool  autoRowName;
  bool  vScrollVisible;
  bool  hScrollVisible;
  bool  isEditable;
  bool  isEditing;
  int   sbWidth;
  int   sbHeight; //selection band height?
  int   labHeight; //label spacrholder height
  int   labWidth;
  bool  gridVisible;

  int   selectionMode;
  std::vector<size_t> selectedRows;
  int   selectedCol;
  int   lastColSel;
  bool  selDragged;
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
  int   GetColumnStart(size_t colIdx);
  int   RelayToEditText(SDL_Event *evt);
  void  UpdateCell();
  void  CancelEdit();

};

#endif /* _GLLISTH_ */