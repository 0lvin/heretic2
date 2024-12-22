/*  SinView - Sin Pak File Viewer
    Copyright (C) 1998

    Trey Harrison trey@u.washington.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <math.h>
#include "resource.h"
#include "arch.h"
#include "file.h"
#include "text.h"
#include "config.h"
#include "tree_view.h"
#include "texture_view.h"
#include "model_view.h"
#include "def_read.h"
#include "dialogs.h"

HWND main_hwnd = 0;
HINSTANCE main_instance = 0;

//either one or the other of these can be shown
texture_view_class *texture_view = 0;
model_view_class   *model_view = 0;

//this is always shown
tree_view_class tree_view;

//model currently being shown
def_info_class *cur_view_model = 0;

//exit when this is true
bool exit_flag = FALSE;

static char szAppName[] = "SinView";

void ProcessWindowsMessages()
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    if (msg.message==WM_QUIT)
      exit_flag = TRUE;

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void free_texture_view()
{
  if (texture_view)
  {
    delete texture_view;
    texture_view=0;
  }
}

void free_model_view()
{
  if (model_view)
  {
    delete model_view;
    model_view = 0;
  }
  cur_view_model = 0;
}

void clear_draw_area()
{
  RECT blank_rect;
  GetClientRect(main_hwnd, &blank_rect);
  blank_rect.left = 200;

  HDC hdc_screen = GetDC(main_hwnd);
  FillRect(hdc_screen,&blank_rect,GetStockObject(GRAY_BRUSH));
  ReleaseDC(main_hwnd, hdc_screen);
}

void setup_bitmap(char *full_texture_name)
{
  //free any texture view thats already there
  free_texture_view();

  //free any model view thats already there
  free_model_view();

  //clear the client area
  clear_draw_area();

  //create and draw the texture
  texture_view = new texture_view_class(full_texture_name);
  texture_view->draw();
}

void setup_model(def_info_class *def, bool full_screen)
{
  //free any texture view thats already there
  free_texture_view();

  //clear the client area
  clear_draw_area();

  if (!model_view)
    model_view = new model_view_class();

  if (full_screen)
  {
    model_view->view_fullscreen(def);
    cur_view_model = 0;
  }
  else
  {
    model_view->view_windowed(def);
    cur_view_model = def;
  }
}

LRESULT CALLBACK SinViewWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  NMHDR *n;
  NM_TREEVIEW *t;
  PAINTSTRUCT ps;

  char window_title[256];

  switch (iMsg)
  {
  case WM_NOTIFY:
    n = (NMHDR *)lParam;
    //is it a message from the treeview control?
    if (n->hwndFrom==tree_view.hwnd)
    {
      //yes. figure out what happened
      t = (NM_TREEVIEW *)lParam;
      switch (n->code)
      {
      case TVN_SELCHANGED:
        //he clicked something new. do something about it
        tree_view.update_view(t->itemNew.hItem);
        break;
      }
    }
    break;

  case WM_LBUTTONDOWN:
    //he clicked over in the draw area.. do something about it
    if (model_view && cur_view_model)
    {
      model_view->view_fullscreen(cur_view_model);
    }
    break;

  case WM_SIZE:
    //make sure the treeview control occupes the entire
    //left column of the main window
    tree_view.resize();
    break;

  case WM_PAINT:
    BeginPaint(hwnd,&ps);
    EndPaint(hwnd,&ps);

    if (texture_view) //redraw the texture
      texture_view->draw();
    else
    if (model_view && cur_view_model) //redraw the model
      model_view->view_windowed(cur_view_model);

    break;

  case WM_COMMAND:
    switch (wParam)
    {
		case ST_HELP_ABOUT:
      DialogBox(main_instance,MAKEINTRESOURCE(ST_DIALOG_ABOUT),hwnd,(DLGPROC)AboutDlgProc);
      break;

    case ST_FILE_EXIT:
      DestroyWindow(main_hwnd);
      break;

    case ST_FILE_CONFIG:
      read_config("sinview.cfg",TRUE);

      pak_shutdown(); //close all .pak files
      tree_view.shutdown(); //close it
      free_texture_view(); //get rid of this
      free_model_view(); //this too
      clear_draw_area(); //clear this

      register_pak_file("pak0.pak"); //reregister
      tree_view.init();     //reinitialize

      //re-title the window
      sprintf(window_title,"%s - %s",szAppName,SIN_BASE_PATH);
      SetWindowText(main_hwnd, window_title);
      break;
    }
    break;

  case WM_DESTROY:
		free_texture_view();
    free_model_view();
    tree_view.shutdown(); //close it
    exit_flag = TRUE;
		return 0;
	}

  return(DefWindowProc(hwnd,iMsg,wParam,lParam));
}


HWND StartWindow(HINSTANCE hInstance, int iCmdShow)
{
  HWND                hwnd;
  WNDCLASSEX            wc;

  wc.cbSize        = sizeof(wc);
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = SinViewWindowProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(ST_ICON));//IDI_APPLICATION);
  wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground = GetStockObject(GRAY_BRUSH);
  wc.lpszMenuName  = MAKEINTRESOURCE(ST_MAIN_MENU);
  wc.lpszClassName = szAppName;
  wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(ST_ICON));//IDI_APPLICATION);
  RegisterClassEx(&wc);

  char window_title[256];
  sprintf(window_title,"%s - %s",szAppName,SIN_BASE_PATH);

  hwnd = CreateWindowEx(
		0,
		szAppName,
		window_title,
		WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME | WS_CLIPCHILDREN,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		GetDesktopWindow(),
		NULL,
		hInstance,
		NULL);

  ShowWindow(hwnd, SW_SHOWNORMAL);
  UpdateWindow(hwnd);

  main_hwnd = hwnd;
  main_instance = hInstance;

  return(hwnd);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
  read_config("sinview.cfg",FALSE);

  register_pak_file("pak0.pak");
  InitCommonControls();

  StartWindow(hInstance,iCmdShow);
  tree_view.init();

  while (!exit_flag)
  {
    ProcessWindowsMessages();

    if (model_view && cur_view_model)
      model_view->view_windowed(cur_view_model);
	}

  pak_shutdown(); //close all .pak files
  tree_view.shutdown(); //close it
  free_texture_view(); //get rid of this
  free_model_view(); //this too

  //any more messages?
  ProcessWindowsMessages();

  return 0;
}
