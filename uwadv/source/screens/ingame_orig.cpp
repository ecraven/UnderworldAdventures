/*
   Underworld Adventures - an Ultima Underworld hacking project
   Copyright (c) 2002 Michael Fink

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
/*! \file ingame_orig.cpp

   ingame user interface, from the original uw1

*/

// needed includes
#include "common.hpp"
#include "ingame_orig.hpp"
#include <cmath>


// ua_ingame_orig_screen methods

void ua_ingame_orig_screen::init()
{
   walk = false;

   fov = 90.0;
   playerxangle = 90.f;
   playeryangle = 0.f;

   leftbuttondown = rightbuttondown = false;

   setup_opengl();
}

void ua_ingame_orig_screen::done()
{
}

void ua_ingame_orig_screen::handle_event(SDL_Event &event)
{
   switch(event.type)
   {
   case SDL_KEYDOWN:
   case SDL_KEYUP:
      // handle key presses
      handle_key_action(event.type,event.key.keysym);
      break;

   case SDL_MOUSEMOTION:
   case SDL_MOUSEBUTTONDOWN:
   case SDL_MOUSEBUTTONUP:
      handle_mouse_action(event);
      break;
   }
}

void ua_ingame_orig_screen::handle_key_action(Uint8 type, SDL_keysym &keysym)
{
   switch(keysym.sym)
   {
   case SDLK_UP:
      walk = (type==SDL_KEYDOWN);
      walk_dir = 0.f;
      break;
   case SDLK_DOWN:
      walk = (type==SDL_KEYDOWN);
      walk_dir = 180.f;
      break;
   case SDLK_RIGHT:
      walk = (type==SDL_KEYDOWN);
      walk_dir = 90.f;
      break;
   case SDLK_LEFT:
      walk = (type==SDL_KEYDOWN);
      walk_dir = 270.f;
      break;
   }
}

void ua_ingame_orig_screen::handle_mouse_action(SDL_Event &event)
{
   switch(event.type)
   {
   case SDL_MOUSEMOTION:
      // mouse has moved
      if (leftbuttondown)
      {
         int x,y;
         SDL_GetRelativeMouseState(&x,&y);
         {
            playerxangle -= x*0.2;
            playeryangle += y*0.2;
         }

         // view rotate angle has to stay between 0 and 360 degree
         while (playerxangle > 360.f || playerxangle < 0.f )
            playerxangle = fmod(playerxangle+360.0,360.0);

         while (playeryangle > 180.f || playeryangle < -180.f )
            playeryangle = fmod(playeryangle-360.0,360.0);

         // restrict up-down view angle
         if (playeryangle < -40.f)
            playeryangle = -40.f;

         if (playeryangle > 40.f)
            playeryangle = 40.f;
      }
      break;

   case SDL_MOUSEBUTTONDOWN:
      {
         Uint8 state = SDL_GetRelativeMouseState(NULL,NULL);
         if (SDL_BUTTON(state)==SDL_BUTTON_LEFT)
            leftbuttondown = true;
         else
            leftbuttondown = false;
      }
      break;

   case SDL_MOUSEBUTTONUP:
      leftbuttondown = false;
      break;
   }
}

void ua_ingame_orig_screen::render()
{
   // clear color and depth buffers
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glLoadIdentity();

   ua_player &pl = core->get_underworld().get_player();
   double plheight = 0.6+core->get_underworld().get_player_height();

   {
      // rotation
      glRotatef( playeryangle+270.0, 1.0, 0.0, 0.0 );
      glRotatef( playerxangle+90.0, 0.0, 0.0, 1.0 );

      ua_player &pl = core->get_underworld().get_player();

      // move to position on map
      glTranslatef(-pl.get_xpos(),-pl.get_ypos(),-plheight );
   }

   ua_frustum fr(pl.get_xpos(),pl.get_ypos(),plheight,-playerxangle,-playeryangle,fov,16.0);

   // render underworld
   core->get_underworld().render(fr);
}

void ua_ingame_orig_screen::tick()
{
   if (walk)
      core->get_underworld().walk_player(-(playerxangle+walk_dir));
}

void ua_ingame_orig_screen::setup_opengl()
{
   // smooth shading
   glShadeModel(GL_SMOOTH);

   // culling
   glCullFace(GL_BACK);
   glFrontFace(GL_CCW);
   glEnable(GL_CULL_FACE);

   // z-buffer
   glEnable(GL_DEPTH_TEST);

   // enable texturing
   glEnable(GL_TEXTURE_2D);

   // alpha blending
   glDisable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

   // clear color
   glClearColor(0,0,0,0);

   // camera setup

   // set projection matrix
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   gluPerspective(fov, core->get_aspect_ratio(), 0.25, 16.0);

   // switch back to modelview matrix
   glMatrixMode(GL_MODELVIEW);
}
