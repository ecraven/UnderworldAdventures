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
/*! \file cutscene_view.cpp

   \brief cutscene view implementation

*/

// needed includes
#include "common.hpp"
#include "cutscene_view.hpp"


// constants

//! frames per second for cutscene animation
const double ua_cutscene_view_anim_fps = 5.0;

//! time needed to fade in/out text
const double ua_cutscene_view_fade_time = 0.5;

//! name of lua userdata variable containing the "this" pointer
const char* ua_cutscene_view_lua_thisptr_name = "self";


// ua_cutscene_view_screen methods

void ua_cutscene_view_screen::init()
{
   ua_trace("cutscene animation %u started\n",cutscene);

   // determine cutscene type
   ua_settings &settings = core->get_settings();
   {
      std::string cutsntype(settings.get_string(ua_setting_cuts_narration));

      canplaysound = canshowtext = false;

      if (cutsntype.compare("both")==0)
         canplaysound = canshowtext = true;

      if (cutsntype.compare("sound")==0)
         canplaysound = true;

      if (cutsntype.compare("subtitles")==0)
         canshowtext = true;

      // enable showing text when audio wasn't enabled
      if (!settings.get_bool(ua_setting_audio_enabled))
         canshowtext = true;
   }

   ended = false;
   tickcount = 0;

   showanim = false;
   loopanim = false;
   curframe = 0;
   animcount = 0.0;
   anim_fade_state = 0;
   anim_fadecount = 0;

   showtext = false;
   strblock = 0x0c00;
   textcolor = 11;
   text_fade_state = 0;
   text_fadecount = 0;

   // stop audio track
   core->get_audio().stop_music();

   // init lua scripting

   // get a new lua state
   L = lua_open(128);

   // register C functions
   lua_register(L,"cuts_do_action",cuts_do_action);

   // load lua cutscene script
   if (0!=core->get_filesmgr().load_lua_script(L,"uw1/scripts/cutscene"))
      ended = true;

   // set "this" pointer
   lua_pushuserdata(L,this);
   lua_setglobal(L,ua_cutscene_view_lua_thisptr_name);

   // call "cuts_init(cutscene)"
   lua_getglobal(L,"cuts_init");
   lua_pushnumber(L,static_cast<double>(cutscene));
   int ret = lua_call(L,1,0);
   if (ret!=0)
   {
      ua_trace("Lua function call cuts_init(%u) ended with error code %u\n",
         cutscene,ret);
      ended = true;
   }

   // init OpenGL

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

   // init subtitle text
   font_big.init(settings,ua_font_big);
}

void ua_cutscene_view_screen::done()
{
   // stop audio track
   core->get_audio().stop_sound();

   img_text.done();
   cuts_anim.done();
   core->get_texmgr().using_new_texname(0);

   lua_close(L);
}

void ua_cutscene_view_screen::handle_event(SDL_Event &event)
{
   switch(event.type)
   {
   case SDL_KEYDOWN:
      // handle key presses
      switch(event.key.keysym.sym)
      {
      case SDLK_RETURN:
      case SDLK_ESCAPE:
         ua_trace("cutscene ended by return/escape\n");
         ended = true;

         // initiate text and anim fadeout
         text_fade_state = 2;
         text_fadecount = 0;
         anim_fade_state = 2;
         anim_fadecount = 0;
         break;
      default: break;
      }
   default: break;
   }
}

void ua_cutscene_view_screen::render()
{
   glClear(GL_COLOR_BUFFER_BIT);

   if (showanim)
   {
      // render animation frame

      // prepare image texture
      cuts_anim.get_frame(curframe);
      cuts_anim.convert_upload();

      // set text color
      Uint8 light = 255;
      switch(anim_fade_state)
      {
      case 0: // show
         light = 255;
         break;

      case 1: // fade in
         light = static_cast<Uint8>( 255*double(anim_fadecount) /
            (ua_cutscene_view_fade_time*core->get_tickrate()) );
         break;

      case 2: // fade out
         light = 255-static_cast<Uint8>( 255*double(anim_fadecount) /
            (ua_cutscene_view_fade_time*core->get_tickrate()) );
         break;
      }
      glColor3ub(light,light,light);

      cuts_anim.render();
   }

   if (showtext)
   {
      // render subtitle text

      // enable blending, in case text overlaps animation
      glEnable(GL_BLEND);

      // set text color
      Uint8 light = 255;
      switch(text_fade_state)
      {
      case 0: // show
         light = 255;
         break;

      case 1: // fade in
         light = static_cast<Uint8>( 255*double(text_fadecount) /
            (ua_cutscene_view_fade_time*core->get_tickrate()) );
         break;

      case 2: // fade out
         light = 255-static_cast<Uint8>( 255*double(text_fadecount) /
            (ua_cutscene_view_fade_time*core->get_tickrate()) );
         break;
      }
      glColor3ub(light,light,light);

      img_text.render();

      glDisable(GL_BLEND);
   }
}

void ua_cutscene_view_screen::tick()
{
   if (ended && anim_fadecount >= ua_cutscene_view_fade_time*core->get_tickrate())
   {
      // we're finished
      core->pop_screen();
      return;
   }

   // check anim looping
   if (showanim && loopanim && !(anim_fade_state == 1 || anim_fade_state == 2))
   {
      // count up animcount
      animcount += 1.0/core->get_tickrate();

      if (animcount>=1.0/ua_cutscene_view_anim_fps)
      {
         // calculate remaining time
         animcount -= 1.0/ua_cutscene_view_anim_fps;

         // count up frames
         curframe++;
         if (curframe>=cuts_anim.get_maxframes())
            curframe = 0;

         // check if we should stop at that frame
         if (stopframe >= 0 && curframe == stopframe)
         {
            // disable animation; works like "cuts_anim_stop"
            loopanim = false;
         }
      }
   }

   // check anim fading
   if ((showanim || ended) && (anim_fade_state == 1 || anim_fade_state == 2))
   {
      ++anim_fadecount;

      // end of fade reached?
      if (anim_fadecount >= ua_cutscene_view_fade_time*core->get_tickrate())
      {
         if (anim_fade_state == 1)
            anim_fade_state = 0;
         else
            showanim = false;
      }
   }

   // check text fading
   if (showtext && (text_fade_state == 1 || text_fade_state == 2))
   {
      ++text_fadecount;

      // end of fade reached?
      if (text_fadecount >= ua_cutscene_view_fade_time*core->get_tickrate())
      {
         if (text_fade_state == 1)
            text_fade_state = 0;
         else
            showtext = false;
      }
   }


   // calculate current ticktime
   double ticktime = double(tickcount) / core->get_tickrate();

   // call lua "cuts_tick(ticktime)" function
   if (!ended)
   {
      lua_getglobal(L,"cuts_tick");
      lua_pushnumber(L,ticktime);
      int ret = lua_call(L,1,0);
      if (ret!=0)
      {
         ua_trace("Lua function call cuts_tick(%3.2f) ended with error code %u\n",
            ticktime,ret);
         ended = true;
         return;
      }
   }

   ++tickcount;
}

void ua_cutscene_view_screen::create_text_image(const char *str)
{
   unsigned int maxlen = 310;

   img_text.clear(0);

   if (font_big.calc_length(str)<maxlen)
   {
      // easy task: all text fits into one line
      font_big.create_string(img_text,str,textcolor);
   }
   else
   {
      std::vector<ua_image> lines;
      std::string text(str),part;

      // collect all text string lines
      do
      {
         part.assign(text);

         std::string::size_type pos;

         // cut down string on ' ' boundaries, until it fits into an image
         while(font_big.calc_length(part.c_str()) > maxlen)
         {
            pos = part.find_last_of(' ');
            if (pos != std::string::npos)
            {
               part.erase(pos);
            }
            else
               break; // string too long, without a space in between
         }

         // create line image
         {
            ua_image img_temp;
            font_big.create_string(img_temp,part.c_str(),textcolor);
            lines.push_back(img_temp);
         }

         // cut down string
         text.erase(0,part.size()+1);

      } while(!text.empty());

      // re-assemble the text strings in a new image
      if (lines.size()==0)
         return; // there definitely went something wrong

      // create new image
      {
         unsigned int lineheight = lines[0].get_yres();
         unsigned int max = lines.size();

         img_text.create(320, lineheight*max);

         // paste all lines into the new image
         for(unsigned int i=0; i<max; i++)
         {
            const ua_image &img_temp = lines[i];

            unsigned int destx = (320-img_temp.get_xres())/2;
            img_text.paste_image(img_temp,destx,lineheight*i);
         }
      }
   }

   // calculate screen position
   unsigned int startx = (320-img_text.get_xres())/2;
   unsigned int starty = 200-5-img_text.get_yres();

   // init after new creation
   img_text.init(&core->get_texmgr(),startx,starty,img_text.get_xres(),
      img_text.get_yres());

   // upload texture
   img_text.convert_upload();
}

/*! stack indices/values:
    -2: actioncode
    -1: actionvalue
*/
void ua_cutscene_view_screen::do_action()
{
   unsigned int action = static_cast<unsigned int>(lua_tonumber(L,-2));

   // note: the case values must be in sync with the ones in the script
   switch(action)
   {
   case 0: // cuts_finished
      ended = true;
      anim_fadecount = static_cast<unsigned int>(
         ua_cutscene_view_fade_time*core->get_tickrate() ) + 1;
      break;

   case 1: // cuts_set_string_block
      strblock = static_cast<unsigned int>(lua_tonumber(L,-1));
      break;

   case 2: // cuts_sound_play
      if (canplaysound)
      {
         const char *str = lua_tostring(L,-1);
         core->get_audio().play_sound(str);
      }
      break;

   case 3: // cuts_text_set_color
      textcolor = static_cast<Uint8>(lua_tonumber(L,-1));
      break;

   case 4: // cuts_text_fadein
      if (canshowtext)
      {
         unsigned int strnum = static_cast<unsigned int>(lua_tonumber(L,-1));
         create_text_image( core->get_strings().get_string(strblock,strnum).c_str() );
         text_fade_state = 1; // fade in
         text_fadecount = 0;
         showtext = true;
      }
      break;

   case 5: // cuts_text_fadeout
      if (canshowtext)
      {
         text_fade_state = 2; // fade out
         text_fadecount = 0;
         showtext = true;
      }
      break;

   case 6: // cuts_text_show
      if (canshowtext)
      {
         unsigned int strnum = static_cast<unsigned int>(lua_tonumber(L,-1));
         create_text_image( core->get_strings().get_string(strblock,strnum).c_str() );
         showtext = true;
         text_fade_state = 0;
      }
      break;

   case 7: // cuts_text_hide
      showtext = false;
      break;

   case 8: // cuts_anim_fadein
      anim_fade_state = 1;
      anim_fadecount = 0;
      // falls through to "cuts_anim_show"

   case 9: // cuts_anim_show
      {
         std::string animname("cuts/");
         animname.append(lua_tostring(L,-1));

         // load animation
         cuts_anim.load(core->get_settings(),animname.c_str());
         cuts_anim.init(&core->get_texmgr());
         showanim = true;
         loopanim = true;
         curframe = 0;
         animcount = 0.0;
         stopframe = -1;
      }
      break;

   case 10: // cuts_anim_set_stopframe
      // get "stop frame" parameter
      stopframe = static_cast<int>(lua_tonumber(L,-1));
      break;

   case 11: // cuts_anim_fadeout
      anim_fade_state = 2;
      anim_fadecount = 0;
      break;

   case 12: // cuts_anim_stop
      loopanim = false;
      curframe = static_cast<unsigned int>(lua_tonumber(L,-1));
      break;

   case 13: // cuts_anim_hide
      showanim = false;
      break;

   case 14: // cuts_anim_continue
      loopanim = true;
      break;
   }
}


// static ua_cutscene_view_screen methods

int ua_cutscene_view_screen::cuts_do_action(lua_State* L)
{
   // check for "self" parameter being userdata

   lua_getglobal(L,ua_cutscene_view_lua_thisptr_name);
   if (lua_isuserdata(L,-1))
   {
      // get pointer to screen
      ua_cutscene_view_screen *self =
         reinterpret_cast<ua_cutscene_view_screen*>(lua_touserdata(L,-1));

      if (self->L != L)
         throw ua_exception("wrong 'self' parameter in Lua script");

      lua_pop(L,1);

      // perform action
      self->do_action();
   }
   else
      throw ua_exception("'self' parameter wasn't set by Lua script");

   return 0;
}
