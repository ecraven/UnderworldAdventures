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
/*! \file uwadv.cpp

   \brief main game loop code

*/

// needed includes
#include "common.hpp"
#include "uwadv.hpp"
#include "screens/start_splash.hpp"


// ua_game methods

ua_game::ua_game():tickrate(20),exit_game(false),audio(NULL),screen(NULL),
   screen_to_destroy(NULL)
{
   printf("Underworld Adventures\n");
   printf("http://uwadv.sourceforge.net/\n\n");
}

void ua_game::init()
{
   // load settings
   settings.load();

   if (settings.gtype == ua_game_none)
      throw ua_exception("could not find relevant game files");

   if (settings.gtype == ua_game_uw2)
      throw ua_exception("you can't play with Ultima Underworld 2");

   // First, initialize SDL's video subsystem
   if( SDL_Init(SDL_INIT_VIDEO) < 0 )
   {
      std::string text("video initialization failed: ");
      text.append(SDL_GetError());
      throw ua_exception(text.c_str());
   }

   // Information about the current video settings
   const SDL_VideoInfo* info = NULL;
   info = SDL_GetVideoInfo();

   if(!info)
   {
      // this should probably never happen
      std::string text("video query failed: ");
      text.append(SDL_GetError());
      throw ua_exception(text.c_str());
   }

   // enable system event messages
   SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);

   // set window caption
   SDL_WM_SetCaption("Underworld Adventures",NULL);

   // set up GL attributes
   SDL_GL_SetAttribute(SDL_GL_RED_SIZE,5);
   SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,5);
   SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,5);
   SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

   // setup video mode
   width = 640;
   height = 480;
   int bpp = info->vfmt->BitsPerPixel;
   int flags = SDL_OPENGL | (settings.fullscreen ? SDL_FULLSCREEN : 0);

   if(SDL_SetVideoMode(width, height, bpp, flags)==0)
   {
      std::string text("video mode set failed: ");
      text.append(SDL_GetError());
      throw ua_exception(text.c_str());
   }

   // setup OpenGL viewport
   glViewport(0, 0, width, height);

   // init textures
   texmgr.init(settings);

   // init files mgr
   filesmgr.init(settings);

   // load game strings
   gstr.load(settings);

   // initializes underworld
   underworld.init(this);

   // init audio
   audio = ua_audio_interface::get_audio_interface();
   audio->init(settings);

   screenstack.clear();

   // create new user interface screen
   push_screen(new ua_start_splash_screen);
}

#define HAVE_FRAMECOUNT

void ua_game::run()
{
   Uint32 now, then;
   Uint32 fcstart;
   fcstart = then = SDL_GetTicks();

   unsigned int ticks=0,renders=0;

   // main game loop
   while(!exit_game)
   {
      now = SDL_GetTicks();

      Uint32 elapsed = (now - then);

      while ((now - then) > (1000.f/tickrate))
      {
         then += Uint32(1000.f/tickrate);

         // do game logic
         screen->tick();
         ticks++;

         // check if there is a screen to destroy
         if (screen_to_destroy!=NULL)
         {
            screen_to_destroy->done();
            delete screen_to_destroy;
            screen_to_destroy = NULL;
            break;
         }
      }

      if (exit_game) break;

      // process incoming events
      process_events();

      // draw the screen
      draw_screen();
      renders++;

      if ((now - then) > (1000.f/tickrate))
         then = now - Uint32(1000.f/tickrate);

#ifdef HAVE_FRAMECOUNT
      now = SDL_GetTicks();

      if (now-fcstart > 2000)
      {
         // set new caption
         char buffer[256];
         sprintf(buffer,"Underworld Adventures: %3.1f ticks/s, %3.1f frames/s",
            ticks*1000.f/(now-fcstart),renders*1000.f/(now-fcstart));

         SDL_WM_SetCaption(buffer,NULL);

         // restart counting
         ticks = renders = 0;
         fcstart = now;
      }
#endif
   }
}

void ua_game::done()
{
   // free current screen
   if (screen!=NULL)
   {
      screen->done();
      delete screen;
   }

   // free all screens on screen stack
   int max = screenstack.size();
   for(int i=0; i<max; i++)
   {
      screenstack[i]->done();
      delete screenstack[i];
   }

   delete audio;
   SDL_Quit();
}

void ua_game::error_msg(const char *msg)
{
   fprintf(stderr,msg);
}


// private ua_game methods

void ua_game::process_events()
{
   SDL_Event event;

   // get another event
   while(SDL_PollEvent(&event))
   {
      screen->handle_event(event);

      switch(event.type)
      {
      case SDL_KEYDOWN:
         // handle key presses
         handle_key_down(&event.key.keysym);
         break;

      case SDL_SYSWMEVENT:
         // system message
         system_message(event.syswm);
         break;

      case SDL_QUIT:
         // handle quit requests
         exit_game=true;
         break;
      }
   }
}

void ua_game::handle_key_down(SDL_keysym *keysym)
{
   switch(keysym->sym)
   {
   case SDLK_x:
      // exit when pressing Alt + x
      if ((keysym->mod & KMOD_ALT)!=0)
         exit_game = true;
      break;
   }
}

void ua_game::draw_screen()
{
   screen->render();

   // finished
   SDL_GL_SwapBuffers();
}

void ua_game::push_screen(ua_ui_screen_base *newscreen)
{
   // save old screen on stack
   if (screen!=NULL)
      screenstack.push_back(screen);

   // we have a new screen
   screen = newscreen;

   screen->set_core(this);
   screen->init();
}

void ua_game::replace_screen(ua_ui_screen_base *newscreen)
{
   pop_screen();
   push_screen(newscreen);
   exit_game = false;
}

void ua_game::pop_screen()
{
   screen_to_destroy = screen;

   // use 
   if (screenstack.size()==0)
   {
      // no more screens available
      screen=NULL;
      exit_game = true;
   }
   else
   {
      // get last pushed screen
      screen = screenstack.back();
      screenstack.pop_back();
   }
}
