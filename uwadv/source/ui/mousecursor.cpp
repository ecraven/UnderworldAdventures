/*
   Underworld Adventures - an Ultima Underworld hacking project
   Copyright (c) 2002,2003 Underworld Adventures Team

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

   $Id$

*/
/*! \file mousecursor.cpp

   \brief mouse cursor

*/

// needed includes
#include "common.hpp"
#include "mousecursor.hpp"
//#include "core.hpp"


// ua_mousecursor methods

void ua_mousecursor::init(ua_game_core_interface* core, int initialtype)
{
/*TODO
   screen_width = core->get_screen_width();
   screen_height = core->get_screen_height();

   ua_settings &settings = core->get_settings();
   img_cursors.load(settings,"cursors");
   mousetex.init(&core->get_texmgr(),1,GL_LINEAR,GL_LINEAR,GL_CLAMP,GL_CLAMP);
   isvisible = false;
   settype(initialtype);
   updatepos();
*/
}

void ua_mousecursor::settype(int type)
{
   set_custom(img_cursors.get_image(type));
}

void ua_mousecursor::set_custom(ua_image& cursorimg)
{
   cursorw = cursorimg.get_xres();
   cursorh = cursorimg.get_yres();

   ua_image cursorimg2;
   cursorimg2.create(cursorw+1,cursorh+1);
   cursorimg2.paste_image(cursorimg,0,0);

   mousetex.convert(cursorimg2);
   mousetex.upload();
}

void ua_mousecursor::updatepos()
{
   int x,y;
   SDL_GetMouseState(&x,&y);
   cursorx = unsigned(double(x)/screen_width*320.0-cursorw/2.0);
   cursory = unsigned(double(y)/screen_height*200.0+cursorh/2.0);
}

void ua_mousecursor::updatepos(unsigned int relx, unsigned int rely)
{
   cursorx = relx - cursorw/2;
   cursory = rely + cursorh/2;
}

void ua_mousecursor::show(bool show)
{
   isvisible = show;
}

void ua_mousecursor::draw()
{
   if (!isvisible)
      return;

   mousetex.use();
   double u = mousetex.get_tex_u(), v = mousetex.get_tex_v();
   u -= mousetex.get_tex_u()/mousetex.get_xres();
   v -= mousetex.get_tex_v()/mousetex.get_yres();

   glBegin(GL_QUADS);
   glTexCoord2d(0.0, v  ); glVertex2i(cursorx, 200-cursory);
   glTexCoord2d(u  , v  ); glVertex2i(cursorx+cursorw, 200-cursory);
   glTexCoord2d(u  , 0.0); glVertex2i(cursorx+cursorw, 200-cursory+cursorh);
   glTexCoord2d(0.0, 0.0); glVertex2i(cursorx, 200-cursory+cursorh);
   glEnd();
}

void ua_mousecursor::done()
{
   mousetex.done();
}