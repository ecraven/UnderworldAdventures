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
/*! \file ingame_orig.hpp

   \brief original, old style game screen user interface

*/

// include guard
#ifndef uwadv_ingame_orig_hpp_
#define uwadv_ingame_orig_hpp_

// needed includes
#include "screen.hpp"
#include "image.hpp"
#include "font.hpp"
#include "keymap.hpp"
#include "renderer.hpp"
#include "textscroll.hpp"
#include "uwscript.hpp"
#include "debug.hpp"
#include "panel.hpp"


// enums

//! game modes
typedef enum
{
   ua_mode_default=0, // nothing selected
   ua_mode_options,
   ua_mode_talk,
   ua_mode_get,
   ua_mode_look,
   ua_mode_fight,
   ua_mode_use,

} ua_ingame_orig_game_mode;


//! screen area values
enum ua_screen_area_id
{
   ua_area_screen3d=1,
   ua_area_textscroll,
   ua_area_gargoyle,
   ua_area_powergem,

   ua_area_menu_button0,
   ua_area_menu_button1,
   ua_area_menu_button2,
   ua_area_menu_button3,
   ua_area_menu_button4,
   ua_area_menu_button5,

   ua_area_compass,
   ua_area_move_left,
   ua_area_move_foreward,
   ua_area_move_right,

   ua_area_spell0,
   ua_area_spell1,
   ua_area_spell2,
   ua_area_bookshelf,
   ua_area_flask_vitality,
   ua_area_flask_mana,
   ua_area_stats_chain,

   // inventory stuff
   ua_area_inv_slot0,
   ua_area_inv_slot1,
   ua_area_inv_slot2,
   ua_area_inv_slot3,
   ua_area_inv_slot4,
   ua_area_inv_slot5,
   ua_area_inv_slot6,
   ua_area_inv_slot7,

   ua_area_inv_container,
   ua_area_inv_scroll_up,
   ua_area_inv_scroll_down,

   ua_area_equip_left_hand,
   ua_area_equip_left_shoulder,
   ua_area_equip_left_ring,

   ua_area_equip_right_hand,
   ua_area_equip_right_shoulder,
   ua_area_equip_right_ring,

   ua_area_paperdoll_head,
   ua_area_paperdoll_chest,
   ua_area_paperdoll_hand,
   ua_area_paperdoll_legs,
   ua_area_paperdoll_feet,
};


// classes

//! user interface abstract base class
class ua_ingame_orig_screen:
   public ua_ui_screen_base,
   public ua_underworld_script_callback
{
public:
   //! ctor
   ua_ingame_orig_screen(){}
   virtual ~ua_ingame_orig_screen(){}

   // virtual functions from ua_screen_ctrl_base
   virtual void init(ua_game_core_interface* core);
   virtual void suspend();
   virtual void resume();
   virtual void done();
   virtual void handle_event(SDL_Event& event);
   virtual void render();
   virtual void tick();

   // virtual functions from ua_underworld_script_callback
   virtual void ui_changed_level(unsigned int level);
   virtual void ui_start_conv(unsigned int level, unsigned int objpos);
   virtual void ui_show_cutscene(unsigned int cutscene);
   virtual void ui_print_string(const char* str);
   virtual void ui_show_ingame_anim(unsigned int anim);
   virtual void ui_cursor_use_item(Uint16 item_id);
   virtual void ui_cursor_target();

protected:
   //! sets up OpenGL stuff, flags, etc.
   void setup_opengl();

   //! renders 2d user interface
   void render_ui();

   //! handles keyboard action
   void handle_key_action(Uint8 type, SDL_keysym &keysym);

   //! called for a given mouse action; click is false for mouse moves
   virtual void mouse_action(bool click, bool left_button, bool pressed);

   //! finds out selection on specific mouse cursor position
   GLuint get_selection(unsigned int xpos, unsigned int ypos);

   //! takes screenshot from current image
   void do_screenshot(bool with_menu, unsigned int xres=0, unsigned int yres=0);

protected:
   // constants

   //! time to fade in/out
   static const double fade_time;


   // mouse cursor

   //! mouse cursor image
   unsigned int cursor_image;

   //! current cursor image
   unsigned int cursor_image_current;

   //! indicates if cursor is an object icon
   bool cursor_is_object;

   //! mouse cursor
   ua_mousecursor mousecursor;

   //! indicates that mouse movement is currently on
   bool mouse_move;

   //! indicates if there is a movement key pressed
   bool move_turn_left, move_turn_right,
      move_walk_forward, move_run_forward, move_walk_backwards;


   //! is true when the mouse cursor is in 3d screen
   bool in_screen3d;


   // misc. stuff

   //! ticks since start of screen
   unsigned long tickcount;

   //! key mappings
   ua_keymap keymap;

   //! selected game mode
   ua_ingame_orig_game_mode gamemode;

   //! underworld renderer
   ua_renderer renderer;


   // fading in/out

   //! current fade state
   unsigned int fade_state;

   //! current fade tickcount
   unsigned int fade_ticks;

   //! fadeout action to carry out
   unsigned int fadeout_action;

   //! fadeout parameter for specific actions
   unsigned int fadeout_param;

   //! level of npc object to converse with
   unsigned int conv_level;

   //! objpos of npc object to converse with
   unsigned int conv_objpos;


   // images, textures and fonts

   //! background image quad
   ua_image_quad img_back;

   //! compass images
   ua_image_list img_compass;

   //! compass texture
   ua_texture tex_compass;

   //! current compass image
   unsigned int compass_curimg;

   //! vitality/mana/poisoned flasks
   ua_image_list img_flasks[3];

   //! flasks textures
   ua_texture tex_flasks;

   //! current flask image numbers
   unsigned int flasks_curimg[2];


   // command buttons

   //! command buttons image
   ua_image img_cmd_buttons;

   //! command buttons texture
   ua_texture tex_cmd_buttons;


   //! inventory/stats/runebag panel
   ua_panel panel;


   //! text scroll
   ua_textscroll textscroll;

   //! text scroll width
   unsigned int scrollwidth;


   //! screenshot image in rgba format
   std::vector<Uint32> screenshot_rgba;

   //! screenshot image resolution
   unsigned int screenshot_xres,screenshot_yres;
};

#endif
