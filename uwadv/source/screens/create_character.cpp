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

*/
/*! \file create_character.cpp

   \brief character creation screen

*/

// needed includes
#include "common.hpp"
#include "create_character.hpp"

// constants

//! time needed to fade in/out text
const double ua_fade_time = 0.5;

//! maximum number of buttons in a group (shown on-screen at once)
const unsigned char ua_maxbuttons = 10;

//! script actions 
enum ua_elua_action
{
   actEnd = 0,            // ends the character creation screen (no params)
   actSetStringBlock = 1, // sets the stringblock (param1=stringblock)
   actSetBtnGroup = 2,    // sets the specified button group (param1=heading, param2=buttontype, param3=buttontable)
   actSetText = 3,        // sets the specified text at the specified location (param1=stringno, param2=x-coord, param3=y-coord)
   actSetName = 4,        // sets the specified name at the specified location (param1=name, param2=x-coord, param3=y-coord, param4=alignment)
   actSetNumber = 5,      // sets the specifed number at the specified location
   actSetImg = 6,         // sets the specified image at the specified location
   actUpdate = 7,         // updates the screen after a change
   actClear = 8           // clears all screen elements (not the background)
};

//! custom type for identifying buttons/text 
enum ua_echarcreationvalue
{
   ccvNone = 0
};

//! custom type for identifying buttons/text 
enum ua_ebuttontype
{
   btText = 0,      // standard button with text from string table, btns contains stringtable index
   btImage = 3,     // square button with image, btns contains index of image
   btInput = 6      // input area
};



// ua_create_character_screen methods

void ua_create_character_screen::init()
{
   ua_trace("character creation screen started\n");

   // get a pointer to to current player
   pplayer = &(core->get_underworld().get_player());

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

   // load background image
   bgimg.load_raw(core->get_settings(),"data/chargen.byt",3);

   // init screen image
   img.create(320, 200, 0, 3);
   tex.init(&core->get_texmgr());
   tex.convert(img);
   tex.use();
   tex.upload();

   // get palette #3
   memcpy(palette,core->get_texmgr().get_palette(3),sizeof(ua_onepalette));

   // load button graphics
   img_buttons.load(core->get_settings(),"chrbtns",0,0,3);

   // init text
   font.init(core->get_settings(), ua_font_chargen);

   // intial variable values
   ended=false;
   selected_button=0;
   prev_button=0;
   changed=false;
   stage=0;
   tickcount=0;
   buttondown=false;
   strblock=0x0002;
   btng_caption=0;
   btng_buttontype=0;
   btng_buttoncount = 0;
   btng_buttons = new unsigned int[ua_maxbuttons];

   // init lua scripting
   initluascript();
}

void ua_create_character_screen::initluascript()
{
   // get a new lua state
   L = lua_open(128);

   // register C functions
   lua_register(L,"cchar_do_action", cchar_do_action);

   // load lua cutscene script
   if (0!=core->get_filesmgr().load_lua_script(L,"uw1/scripts/createchar"))
      ended = true;

   // call "cchar_init(this)"
   lua_getglobal(L,"cchar_init");
   lua_pushuserdata(L,this);
   int ret = lua_call(L,1,0);
   if (ret!=0)
   {
      ua_trace("Lua function call cchar_init(0x%08x) ended with error code %u\n",
         this,ret);
      ended = true;
   }
}

void ua_create_character_screen::do_action()
{
   int n=lua_gettop(L);
   unsigned int action = (n<2) ? 0 : static_cast<unsigned int>(lua_tonumber(L,2));

   ua_trace("character creation script action %d received\n", action);

   switch(static_cast<ua_elua_action>(action))
   {
   case actEnd:
      ended = true;
      break;

   case actSetStringBlock:
      if (n<3) break;
      strblock = static_cast<unsigned int>(lua_tonumber(L,3));
      break;

   case actSetBtnGroup:
   {
      if (n<4) break;
      btng_caption = static_cast<unsigned int>(lua_tonumber(L,3));
      btng_buttontype = static_cast<unsigned int>(lua_tonumber(L,4));
      btng_buttoncount = lua_getn(L, 5);
      for (int i=0; i<btng_buttoncount; i++)
      {
         lua_rawgeti(L, 5, i+1);
         btng_buttons[i] = static_cast<unsigned int>(lua_tonumber(L,6));
         lua_pop(L, 1);
      }
      selected_button=prev_button=0;
      drawbuttongroup();
      break;
   }

   case actSetText:
   {
      if (n<6) break;
      drawtext(static_cast<unsigned int>(lua_tonumber(L,3)),
               static_cast<unsigned int>(lua_tonumber(L,4)),
               static_cast<unsigned int>(lua_tonumber(L,5)),
               static_cast<unsigned int>(lua_tonumber(L,6)));
      break;
   }

   case actSetName:
   {
      if (n<6) break;
      drawtext(lua_tostring(L,3),
               static_cast<unsigned int>(lua_tonumber(L,4)),
               static_cast<unsigned int>(lua_tonumber(L,5)),
               static_cast<unsigned int>(lua_tonumber(L,6)));
      break;
   }

   case actSetNumber:
   {
      if (n<5) break;
      drawnumber(static_cast<unsigned int>(lua_tonumber(L,3)),
               static_cast<unsigned int>(lua_tonumber(L,4)),
               static_cast<unsigned int>(lua_tonumber(L,5)));
      break;
   }

   case actSetImg:
   {
      if (n<5) break;
      ua_image cimg = img_buttons.get_image(static_cast<unsigned int>(lua_tonumber(L,3)));
      img.paste_image(cimg, static_cast<unsigned int>(lua_tonumber(L,4)) - cimg.get_xres()/2, 
                            static_cast<unsigned int>(lua_tonumber(L,5)) - cimg.get_yres()/2, true);
      break;
   }

   case actClear:
      img.paste_image(bgimg, 0, 0, false);
      break;

   case actUpdate:
      changed = true;
      break;
   }
}

void ua_create_character_screen::done()
{
   delete[] btng_buttons;

   tex.done();

   // clear screen
   glClearColor(0,0,0,0);
   glClear(GL_COLOR_BUFFER_BIT);
   SDL_GL_SwapBuffers();

   // close Lua
   lua_close(L);
   ua_trace("character creation screen ended\n");
}

void ua_create_character_screen::handle_event(SDL_Event &event)
{
   switch(event.type)
   {
   case SDL_KEYDOWN:
      // handle key presses
      switch(event.key.keysym.sym)
      {
      case SDLK_ESCAPE:
         ended = true;
         break;

      case SDLK_PAGEUP:
      case SDLK_HOME:
         selected_button=0;
         break;

      case SDLK_PAGEDOWN:
      case SDLK_END:
         selected_button=btng_buttoncount-1;
         break;

      case SDLK_UP:
         // select the area above, if possible
         if (selected_button==-1) selected_button=0;
         if (selected_button>0) selected_button--;
         break;

      case SDLK_DOWN:
         // select the area below, if possible
         if (selected_button+1<btng_buttoncount)
            selected_button++;
         break;

      case SDLK_RETURN:
         // simulate clicking on that area
         if (stage==1)
         {
            stage++;
            tickcount=0;
         }
         break;
      }
      break;

   case SDL_MOUSEBUTTONDOWN:
      // select the area where the mouse button is pressed
      buttondown=true;
      if (stage==1)
      {
         int ret = getbuttonover();
         if (ret>=0)
            selected_button = ret;
      }
      break;

   case SDL_MOUSEMOTION:
      if (stage==1 && buttondown)
      {
         int ret = getbuttonover();
         if (ret>=0)
            selected_button = ret;
      }
      break;

   case SDL_MOUSEBUTTONUP:
      buttondown=false;
      if (stage==1)
      {
         // determine if user released the mouse button over the same area
         int ret = getbuttonover();
         if ((ret>=0) && (ret==selected_button))
         {
            stage++;
            tickcount=0;
         }
      }
      break;
   }
}

void ua_create_character_screen::drawtext(const char* str, int x, int y, int xalign, unsigned char color)
{
   // set default text color
   if (color==0)
      color = 73;
   ua_image img_text;
   img_text.clear(0);
   font.create_string(img_text, str, color);
   if (xalign!=0)
   {
       // adjust horizontal alignment
       unsigned int textlength = font.calc_length(str);
       if (xalign==1)
          x -= textlength/2;
       else
          if (xalign==2)
             x -= textlength;
   }
   img.paste_image(img_text, x, y, true);
}

void ua_create_character_screen::drawnumber(unsigned int num, int x, int y, unsigned char color)
{
   char ca[12];
   sprintf(ca, "%d", num);
   drawtext(ca, x, y, 2, color);
}

void ua_create_character_screen::drawtext(int strnum, int x, int y, int xalign, unsigned char color)
{
   const char* ptext = core->get_strings().get_string(2, strnum).c_str();
   if (ptext)
      drawtext(ptext, x, y, xalign, color);
}

void ua_create_character_screen::drawbutton(int buttontype, bool highlight, int strnum, int xc, int y)
{
   ua_image button = img_buttons.get_image(buttontype);
   int x = xc - button.get_xres()/2;
   img.paste_image(button, x, y, false);
   if (buttontype==btText)
      drawtext(strnum, xc, y+3, 1, (highlight ? 68 : 73));
   else if (buttontype==btInput)
      drawtext(strnum, x+5, y+3, 0);
   else if (buttontype==btImage)
   {
      button = img_buttons.get_image(strnum);
      img.paste_image(button, x, y, true);
   }
   if (highlight && (buttontype!=btInput))
   {
      button = img_buttons.get_image(buttontype+2);
      img.paste_image(button, x, y, true);
   }
}

void ua_create_character_screen::drawbuttongroup()
{
   ua_image button = img_buttons.get_image(btng_buttontype);
   int columnsplit = (btng_buttoncount>8) ? btng_buttoncount/2 : btng_buttoncount;
   int inity = columnsplit*button.get_yres() + ((columnsplit-1)*5);
   if (btng_caption!=ccvNone)
       inity += 15;
   inity = (200-inity)/2;
   if (btng_caption!=ccvNone)
   {
      drawtext(btng_caption, 240, inity, 1);
      inity += 15;
   }

   int y = inity;
   int x = 240;
   if (columnsplit!=btng_buttoncount)
      x -= button.get_xres()/2+3;
   else
      columnsplit = -1;
   for(int i=0; i<btng_buttoncount; i++)
   {
      drawbutton(btng_buttontype, (selected_button==i), btng_buttons[i], x, y);
      if (i==columnsplit-1)
      {
         y = inity;
         x += button.get_xres()+6;
      } else
         y += button.get_yres()+5;
   }
}

int ua_create_character_screen::getbuttonover()
{
   // get mouse coordinates
   int x, y;
   SDL_GetMouseState(&x,&y);
   unsigned int xpos,ypos;
   xpos = unsigned(double(x)/core->get_screen_width()*320);
   ypos = unsigned(double(y)/core->get_screen_height()*200);
   int btnhwidth = img_buttons.get_image(btng_buttontype).get_xres()/2;
   if ((xpos<240-btnhwidth) || (xpos>240+btnhwidth))
      return -1;

   int btnheight = img_buttons.get_image(btng_buttontype).get_yres();
   y = btng_buttoncount*btnheight + ((btng_buttoncount-1)*5);
   if (btng_caption!=ccvNone)
       y += 15;
   y = (200-y)/2;
   if (btng_caption!=ccvNone)
      y += 15;
   for(int i=0; i<btng_buttoncount; i++)
   {
        if ((ypos>y) && (ypos<y+btnheight))
            return i;
        y += btnheight + 5;
    }
   return -1;
}

void ua_create_character_screen::render()
{
   glClear(GL_COLOR_BUFFER_BIT);
   glLoadIdentity();

   if (selected_button!=prev_button)
   {
      drawbuttongroup();
      prev_button=selected_button;
      changed = true;
   }

   if (changed)
   {
      // set brightness of texture quad
      glColor3ub(255,255,255);

      // prepare image texture
      tex.convert(img);
      tex.use();
      tex.upload();

      changed = false;
   }

   double u = tex.get_tex_u(), v = tex.get_tex_v();

   // draw quad
   glBegin(GL_QUADS);
   glTexCoord2d(0.0, v  ); glVertex2i(  0,  0);
   glTexCoord2d(u  , v  ); glVertex2i(320,  0);
   glTexCoord2d(u  , 0.0); glVertex2i(320,200);
   glTexCoord2d(0.0, 0.0); glVertex2i(  0,200);
   glEnd();

}

void ua_create_character_screen::tick()
{
   // when fading in or out, check if blend time is over
   if ((stage==0 || stage==2) &&
      ++tickcount >= (core->get_tickrate()*ua_fade_time))
   {
      // do next stage
      stage++;
      tickcount=0;
   }

   if (ended)
   {
      // finished
      core->pop_screen();
      return;
   }

   // in stage 3 the selected button is pressed
   if (stage==3)
   {
      press_button(selected_button);
      stage=0;
      tickcount=0;
   }
}

void ua_create_character_screen::press_button(int button)
{
   ua_trace("character creation button %d pressed\n", button);

   // call "cchar_buttonclick(button)"
   lua_getglobal(L,"cchar_buttonclick");
   lua_pushnumber(L,static_cast<int>(button));
   int ret = lua_call(L,1,0);
   if (ret!=0)
   {
      ua_trace("Lua function call cchar_buttonclick(0x%08x,%u) ended with error code %u\n",
         this, button, ret);
      ended = true;
   }
}


// static ua_create_character_view_screen methods

int ua_create_character_screen::cchar_do_action(lua_State *L)
{
   // check for "self" parameter being userdata
   int n=lua_gettop(L);
   if ((n>0) && lua_isuserdata(L,1))
   {
      // get pointer to object
      ua_create_character_screen *self =
         reinterpret_cast<ua_create_character_screen*>(lua_touserdata(L,1));

      if (self->L != L)
         throw ua_exception("wrong 'self' parameter in Lua script");

      // perform action
      self->do_action();

      return 0;
   }
   return 1;
}
