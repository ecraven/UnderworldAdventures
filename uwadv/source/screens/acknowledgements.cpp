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
/*! \file acknowledgements.cpp

   \brief acknowledgements implementation

   stage 0: show animation frame
   stage 1: crossfade to next frame

*/

// needed includes
#include "common.hpp"
#include "acknowledgements.hpp"
#include "image.hpp"


// constants

//! time to show one credits image
const double ua_acknowledgements_screen::show_time = 3.0;

//! time to crossfade between two screens
const double ua_acknowledgements_screen::xfade_time = 0.5;


// ua_acknowledgements_screen methods

void ua_acknowledgements_screen::init(ua_game_core_interface* thecore)
{
   ua_ui_screen_base::init(thecore);

   ua_trace("acknowledgements screen started\n");

   // init OpenGL stuff

   // setup orthogonal projection
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0,320,0,200);
   glMatrixMode(GL_MODELVIEW);

   // set OpenGL flags
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,0);

   glDisable(GL_DEPTH_TEST);
   glDisable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

   // setup misc. variables
   ended = false;
   stage = 0; // crossfade
   tickcount = unsigned(show_time * core->get_tickrate()) - 3;
   curframe = unsigned(-1);

   cuts_ack.load(core->get_settings(),"cuts/cs012.n01");
   cuts_ack.init(&core->get_texmgr());

   // trick to copy image properties and palette
   img[0].init(&core->get_texmgr(),0,0,320,200,false);
   memcpy(img[0].get_quadpalette(),cuts_ack.get_quadpalette(),sizeof(ua_onepalette));

   img[1].init(&core->get_texmgr(),0,0,320,200,false);
   memcpy(img[1].get_quadpalette(),cuts_ack.get_quadpalette(),sizeof(ua_onepalette));

   // tex 0 is a blank frame / fading-out frame
   img[0].clear(1);
   img[0].convert_upload();

   // tex 1 is next fading-in frame
   img[1].clear(1);
   img[1].convert_upload();
}

void ua_acknowledgements_screen::done()
{
   img[0].done();
   img[1].done();
}

void ua_acknowledgements_screen::handle_event(SDL_Event& event)
{
   switch(event.type)
   {
   case SDL_MOUSEBUTTONDOWN:
      // start crossfade immediately
      if (stage==0)
         tickcount = unsigned(core->get_tickrate()*show_time) + 1;
      break;

   case SDL_KEYDOWN:
      // handle key presses
      switch(event.key.keysym.sym)
      {
      case SDLK_RETURN:
      case SDLK_ESCAPE:
         ua_trace("acknowledgements ended by return/escape\n");
         ended = true;

         fadeout_end();
         break;
      default: break;
      }
   default: break;
   }
}

void ua_acknowledgements_screen::render()
{
   glClear(GL_COLOR_BUFFER_BIT);
   glDisable(GL_BLEND);

   // render first quad; image to show / to fade in
   glColor3ub(255,255,255);

   // draw first quad
   unsigned int imgnum = 1-(curframe&1);
   img[imgnum].render();

   if (stage==1)
   {
      // render second quad using alpha blending
      glEnable(GL_BLEND);

      // calculate alpha
      Uint8 alpha = 255-Uint8(255*(double(tickcount)/(core->get_tickrate()*xfade_time)));
      glColor4ub(255,255,255,alpha);

      // draw second quad
      imgnum = curframe&1;
      img[imgnum].render();
   }
}

void ua_acknowledgements_screen::tick()
{
   if (stage==0 && double(tickcount)/core->get_tickrate() >= show_time)
   {
      // switch to crossfade
      stage = 1;
      tickcount = 0;
      if (curframe==13)
      {
         ended=true;
         fadeout_end();
         return;
      }

      ++curframe;

      // load new animation frame
      cuts_ack.get_frame(curframe);

      // upload textures
      unsigned int imgnum = 1-(curframe&1);
      img[imgnum].paste_image(cuts_ack,0,0);
      img[imgnum].convert_upload();

      //ua_trace("uploading frame %u to image %u\n",curframe,imgnum);
      //ua_trace("showing image %u, blending image %u\n",1-(curframe&1),curframe&1);

      return;
   }

   if (stage==1 && double(tickcount)/core->get_tickrate() >= xfade_time)
   {
      //ua_trace("showing image %u, current frame %u\n",1-(curframe&1),curframe);

      if (ended)
         core->pop_screen();

      // switch to show mode
      stage = 0;
      tickcount = 0;

      return;
   }

   ++tickcount;
}

void ua_acknowledgements_screen::fadeout_end()
{
   // initiate fadeout
   stage = 1;
   tickcount=0;

   // insert black frame as fadeout frame
   {
      ua_image img_clear1,img_clear2;

      unsigned int imgnum = curframe&1;
      img[imgnum].clear(1);
      img[imgnum].convert_upload();
   }
   ++curframe;
}
