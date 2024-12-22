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
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "arch.h"
#include "file.h"
#include "config.h"
#include "engine/engine_input.h"
#include "engine/render_window.h"
#include "sin_model.h"
#include "def_read.h"
#include "model_view.h"

extern HWND main_hwnd;
extern HINSTANCE main_instance;
void ProcessWindowsMessages();

static char szModelName[] = "SinModelView";

LRESULT CALLBACK ModelWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC         hdc;

  //grab all these messages and just ignore 'em
  if (iMsg>=WM_KEYFIRST && iMsg<=WM_KEYLAST)
    return 0;

  if (iMsg>=WM_MOUSEFIRST && iMsg<=WM_MOUSELAST)
    return 0;

  switch (iMsg)
  {
  case WM_COMMAND:
  case WM_SYSCOMMAND:
  case WM_NOTIFY:
  case WM_MOUSEMOVE:
    return 0;

 	case WM_PAINT:
		BeginPaint(hwnd,&ps);
		EndPaint(hwnd,&ps);
    hdc = GetDC(hwnd);
    //i dunno why the 3dfx sometimes switches back to VGA display.
    //this will tell them how to get it back, if it happens
    TextOut(hdc,
            0,
            0,
            "READ README.TXT FOR INSTRUCTIONS",
            32);

    TextOut(hdc,
            0,
            GetSystemMetrics(SM_CYSCREEN)/2,
            "Hit Left Control + F9 to view the model",
            39);

    ReleaseDC(hwnd,hdc);
    break;
  }

  return(DefWindowProc(hwnd,iMsg,wParam,lParam));
}

HWND StartModelWindow()
{
  HWND                hwnd;
  WNDCLASSEX            wc;

  wc.cbSize        = sizeof(wc);
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = ModelWindowProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = main_instance;
  wc.hIcon         = 0;//LoadIcon(main_instance, IDI_APPLICATION);
  wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground = GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = szModelName;
  wc.hIconSm       = LoadIcon(main_instance, IDI_APPLICATION);
  RegisterClassEx(&wc);

  hwnd = CreateWindowEx(
         0,
         szModelName,
         szModelName,
         WS_POPUP | WS_EX_TOPMOST | WS_VISIBLE,
		     0,
         0,
         GetSystemMetrics(SM_CXSCREEN),
		     GetSystemMetrics(SM_CYSCREEN),
		     GetDesktopWindow(),
		     NULL,
         main_instance,
         NULL);

  ShowWindow(hwnd, SW_SHOWNORMAL);
  UpdateWindow(hwnd);

	return(hwnd);
}

model_view_class::model_view_class()
{
  model_hwnd = 0;
  cur_model  = 0;
  cur_render = 0;
  cur_texture = 0;
  cur_skin_name[0]=0;
  frame = 0.f;
  speed = 0.125f;

  //setup the device independent bitmap
	//(windows bullshit, basically)
  BITMAPINFOHEADER *bi=0;

  bitmap_info = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
  memset(bitmap_info,0,(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)));
  bi = (BITMAPINFOHEADER *)&bitmap_info->bmiHeader;
  bi->biSize = sizeof(BITMAPINFOHEADER);

  //negative height in a windows bitmap makes it display
  //like normal vga ((0,0) in the upper left corner, (width,height) in
  //the lower right
  bi->biPlanes = 1;
  bi->biCompression = BI_BITFIELDS;
  bi->biClrUsed=0;
  //set binfo information to 16bit rendering
  bi->biBitCount = 16;
  bi->biHeight = -256;
  bi->biWidth  = 256;

  //super secret way to make Win32 565 16bit bitmaps
  w32 *d = (w32 *)&bitmap_info->bmiColors[0];
  d[0] = 0xF800;
  d[1] = 0x07E0;
  d[2] = 0x001F;
}

model_view_class::~model_view_class()
{
  if (cur_model)
  {
    delete cur_model;
    cur_model = 0;
  }

  if (cur_render)
  {
    if (cur_texture)
    {
      cur_render->free_texture(cur_texture);
      cur_texture=0;
    }

    delete cur_render;
    cur_render = 0;
  }

  cur_skin_name[0]=0;

  if (bitmap_info)
  {
    free(bitmap_info);
    bitmap_info = 0;
  }
}

void model_view_class::view_windowed(def_info_class *def)
{
  //not accepting input in windowed mode
  main_input = 0;

  //make sure the renderer is in the right resolution
  if (!cur_render || cur_render->width != 256 || cur_render->height != 256)
  {
    if (cur_render)
    {
      if (cur_texture)
      {
        cur_render->free_texture(cur_texture);
        cur_texture=0;
      }
      delete cur_render;
    }

    cur_render = new engine_render_window_class(256,256,(w32)main_hwnd);

    //dont want it to be "visible" (this makes the 3dfx show the vga signal)
    cur_render->deactivate();

    //camera setup
    cur_render->camera.x = -7.1f;
    cur_render->camera.y = 48.4f;
    cur_render->camera.z = 55.3f;
    cur_render->camera_orientation.x = 12.88f;
    cur_render->camera_orientation.y = 0.f;
    cur_render->camera_orientation.z = 3.02f;

    cur_skin_name[0] = 0;
  }

  //need to load the shit up
  char base_name[256];
  char anim_name[256];
  char skin_name[256];

  sprintf(base_name,"%s/%s",def->path,def->base_name);
  sprintf(skin_name,"%s/%s",def->path,def->skin_names[def->cur_skin]);
  sprintf(anim_name,"%s/%s",def->path,def->animation_names[def->cur_animation*2+1]);

  //are we still viewing the same model?
  if (cur_model)
  {
    if (strcmp(cur_model->animation_name,anim_name)!=0 ||
        strcmp(cur_model->base_name,base_name)!=0)
    {
      delete cur_model;
      cur_model = 0;
    }
  }

  //so maybe the model hasnt been loaded.. load it here
  if (!cur_model)
  {
    cur_model = new sin_model_class(base_name);
    cur_model->set_animation(anim_name);

    strcpy(cur_model->base_name, base_name);
    strcpy(cur_model->animation_name, anim_name);

    //object setup
    cur_model->origin.x = 0.f;
    cur_model->origin.y = 0.f;
    cur_model->origin.z = 0.f;
    cur_model->orientation.x = 0.f;
    cur_model->orientation.y = 0.f;
    cur_model->orientation.z = -278.f;
  }

  //wrong texture?
  if (cur_skin_name[0]==0 || strcmp(cur_skin_name,skin_name)!=0)
  {
    if (cur_texture)
      cur_render->free_texture(cur_texture);

    //texture setup
    cur_texture = cur_render->register_texture(skin_name);
    cur_render->set_texture(cur_texture);
    strcpy(cur_skin_name, skin_name);
  }

  //rotate the object a bit
  cur_model->orientation.z++;

  //update the cam transform
  cur_render->update_camera_transform();

  //advance to the next frame
  frame += speed;

  //set up interpolation info for model
  cur_model->frame = (sw32)frame;
  cur_model->ratio = frame - (float)cur_model->frame;

  //draw it
  cur_render->draw_object(cur_model);

  //grab the frame buffer
  cur_render->grab_framebuffer(bitmap);

  //flush / flip / clear the buffer.. ya know what i mean..
  cur_render->flush(1);

  //now copy it to the win32 bitmap and show it in a window
  RECT rect;
  GetClientRect(main_hwnd, &rect);
  rect.left = 200;

  sw32 mid_x = (rect.left + rect.right)/2;
  sw32 mid_y = (rect.top  + rect.bottom)/2;

  HDC hdc = GetDC(main_hwnd);
  //my favorite windows blit
  SetDIBitsToDevice(hdc,
                    mid_x - 128, mid_y - 128,
                    mid_x + 128, mid_y + 128,
                    0,0,0,256,
                    bitmap,
                    bitmap_info,
                    DIB_RGB_COLORS);
	ReleaseDC(main_hwnd,hdc);
}

void model_view_class::view_fullscreen(def_info_class *def)
{
  //start the fullscreen window (just covers up the screen and accepts input)
  model_hwnd = StartModelWindow();

  //the input class
  engine_input_class input;
  main_input = &input;
  input.init(model_hwnd);

  //make sure the renderer is in the right resolution
  if (!cur_render || cur_render->width != 640 || cur_render->height != 480)
  {
    if (cur_render)
    {
      if (cur_texture)
      {
        cur_render->free_texture(cur_texture);
        cur_texture=0;
      }
      delete cur_render;
    }

    cur_render = new engine_render_window_class(640,480,(w32)model_hwnd);
    cur_skin_name[0] = 0;
  }

  //make sure its "activated"
  cur_render->activate();

  //need to load the shit up
  char base_name[256];
  char anim_name[256];
  char skin_name[256];

  sprintf(base_name,"%s/%s",def->path,def->base_name);
  sprintf(skin_name,"%s/%s",def->path,def->skin_names[def->cur_skin]);
  sprintf(anim_name,"%s/%s",def->path,def->animation_names[def->cur_animation*2+1]);

  //are we still viewing the same model?
  if (cur_model)
  {
    if (strcmp(cur_model->animation_name,anim_name)!=0 ||
        strcmp(cur_model->base_name,base_name)!=0)
    {
      delete cur_model;
      cur_model = 0;
    }
  }

  //so maybe the model hasnt been loaded.. load it here
  if (!cur_model)
  {
    cur_model = new sin_model_class(base_name);
    cur_model->set_animation(anim_name);

    strcpy(cur_model->base_name, base_name);
    strcpy(cur_model->animation_name, anim_name);

    //object setup
    cur_model->origin.x = 0.f;
    cur_model->origin.y = 0.f;
    cur_model->origin.z = 0.f;
    cur_model->orientation.x = 0.f;
    cur_model->orientation.y = 0.f;
    cur_model->orientation.z = -278.f;
  }

  //wrong texture?
  if (strcmp(cur_skin_name,skin_name)!=0)
  {
    if (cur_texture)
      cur_render->free_texture(cur_texture);

    //texture setup
    cur_texture = cur_render->register_texture(skin_name);
    cur_render->set_texture(cur_texture);
    strcpy(cur_skin_name, skin_name);
  }

  //reinit these vars
  frame = 0.f;
  speed = 0.125f;

  //camera setup
  cur_render->camera.x = -7.1f;
  cur_render->camera.y = 48.4f;
  cur_render->camera.z = 55.3f;
  cur_render->camera_orientation.x = 12.88f;
  cur_render->camera_orientation.y = 0.f;
  cur_render->camera_orientation.z = 3.02f;

  //key setup
  while (input.key_down('S'));
  while (input.key_down('L'));
  while (input.key_down('N'));
  while (input.key_down('M'));
  while (input.key_down(VK_ESCAPE));

  //lock mouse in center of screen
  input.lock_mouse();

  while (1)
  {
    //keep the animation speed within a decent range
    if (input.key_down(VK_ADD))
    {
      speed += 0.01f;
      if (speed>10.f)
        speed=10.f;
    }

    if (input.key_down(VK_SUBTRACT))
    {
      speed -= 0.01f;
      if (speed<0.f)
        speed=0.f;
    }

    //rotate the model?
    if (input.key_down('J')) cur_model->orientation.z--;
    if (input.key_down('K')) cur_model->orientation.z++;

    //do we wanna change skins?
    bool change_skin=FALSE;
    if (input.key_hit('N'))
    {
      def->cur_skin--;
      if (def->cur_skin<0)
        def->cur_skin=def->num_skins-1;

      change_skin=TRUE;
    }

    if (input.key_hit('M'))
    {
      def->cur_skin++;
      if (def->cur_skin>=def->num_skins)
        def->cur_skin=0;
      change_skin=TRUE;
    }

    if (change_skin)
    {
      //get the name
      sprintf(cur_skin_name,"%s/%s",def->path,def->skin_names[def->cur_skin]);

      //texture setup
      if (cur_texture)
        cur_render->free_texture(cur_texture);

      cur_texture = cur_render->register_texture(cur_skin_name);
      cur_render->set_texture(cur_texture);
    }

    //redirect the light
    if (input.key_hit('L'))
    {
      //make light point from the direction the camera is
      //currently facing
      cur_render->light_dir.x = -cur_render->camera_transform.z.x;
      cur_render->light_dir.y = -cur_render->camera_transform.z.y;
      cur_render->light_dir.z = -cur_render->camera_transform.z.z;
      vec3_normalize(&cur_render->light_dir, &cur_render->light_dir);
    }

    //handle lighting intensity changes
    float shift = input.key_down(VK_SHIFT)?(-2.f):(2.f);

    //adjust the light intensity
    char rgb[] = "RGB";
    sw32 i;
    for (i=0; i<3; i++)
    {
      if (input.key_down(rgb[i]))
      {
        cur_render->l_intensity[i] += shift;

        //keep them between 0 and 255
        if (cur_render->l_intensity[i] > 255.f)
          cur_render->l_intensity[i] = 255.f;
        else
        if (cur_render->l_intensity[i] < 0.f)
          cur_render->l_intensity[i] = 0.f;
      }
    }

    sw32 mx, my; //how much has the mouse moved?
    input.get_mouse_move(&mx,&my);

    //rotate the camera
    cur_render->camera_orientation.x += (scalar)my * 0.01f;
    cur_render->camera_orientation.z += (scalar)mx * 0.01f;

    //is the camera going to move?
    vec3 move;
    move.x = move.y = move.z = 0.f;

    scalar move_amount = 2.f;
    if (input.key_down(VK_RBUTTON))
      move.z += move_amount;

    if (input.key_down(VK_LBUTTON))
      move.z -= move_amount;

    if (input.key_down(VK_UP))
      move.y -= move_amount;

    if (input.key_down(VK_DOWN))
      move.y += move_amount;

    if (input.key_down(VK_LEFT))
      move.x -= move_amount;

    if (input.key_down(VK_RIGHT))
      move.x += move_amount;

    vec3 res;
    cur_render->camera_transform.inverse_transform_3x3(move, res);

    cur_render->camera.x += res.x;
    cur_render->camera.y += res.y;
    cur_render->camera.z += res.z;

    if (input.key_down('A')) cur_render->camera.z += move_amount;
    if (input.key_down('Z')) cur_render->camera.z -= move_amount;

    //update the transform
    cur_render->update_camera_transform();

    //advance to the next frame
    frame += speed;

    //set up interpolation info for model
    cur_model->frame = (sw32)frame;
    cur_model->ratio = frame - (float)cur_model->frame;

    //draw it
    cur_render->draw_object(cur_model);

    //take a screenshot?
    if (input.key_hit('S'))
      cur_render->take_screen_shot();

    //flush / flip / etc
    cur_render->flush(1);

    if (input.key_hit(VK_ESCAPE))
    {
      DestroyWindow(model_hwnd);
      model_hwnd = 0;
      break;
    }

	  //process windows messages
    ProcessWindowsMessages();
  }

  //show the VGA signal
  cur_render->deactivate();
}
